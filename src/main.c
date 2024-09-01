#include <cairo/cairo-xcb.h>
#include <cairo/cairo.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_util.h>

#include <stdlib.h>
#include <unistd.h>

xcb_connection_t *connection;
xcb_ewmh_connection_t ewmh;

typedef struct {
  xcb_screen_t *screen;
  xcb_window_t window;

  cairo_surface_t *surface;
  cairo_t *cr;
} unibar_t;

static int f_create_unibar(lua_State *L) {
  // TODO: allow selecting screen number
  xcb_screen_t *screen = xcb_aux_get_screen(connection, 0);

  xcb_window_t window = xcb_generate_id(connection);

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = {
    screen->white_pixel, XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_EXPOSURE};
  xcb_create_window(connection, screen->root_depth, window, screen->root, 8, 8,
    screen->width_in_pixels - 16, 32, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
    screen->root_visual, mask, values);

  char wm_class[] = "unibar";
  xcb_icccm_set_wm_class(connection, window, sizeof(wm_class), wm_class);
  xcb_atom_t wm_state[] = {ewmh._NET_WM_STATE_ABOVE};
  xcb_ewmh_set_wm_state(&ewmh, window, 1, wm_state);
  xcb_atom_t wm_window_type[] = {ewmh._NET_WM_WINDOW_TYPE_DOCK};
  xcb_ewmh_set_wm_window_type(&ewmh, window, 1, wm_window_type);

  xcb_visualtype_t *visual =
    xcb_aux_get_visualtype(connection, 0, screen->root_visual);
  cairo_surface_t *surface =
    cairo_xcb_surface_create(connection, window, visual, 150, 150);
  cairo_t *cr = cairo_create(surface);

  unibar_t *unibar = lua_newuserdata(L, sizeof(unibar_t));
  unibar->screen = screen;
  unibar->window = window;
  unibar->surface = surface;
  unibar->cr = cr;

  luaL_setmetatable(L, "unibar");

  return 1;
}

static int f_get_unibar_prop(lua_State *L) {
  unibar_t *unibar = lua_touserdata(L, 1);
  lua_pushinteger(L, unibar->window);

  const char *props[] = {"window"};
  int prop = luaL_checkoption(L, 2, NULL, props);

  lua_settop(L, 1);

  switch (prop) {
  case 0:
    lua_pushinteger(L, unibar->window);
    return 1;
  default:
    return 0;
  }
}

static int f_destroy_unibar(lua_State *L) {
  unibar_t *unibar = lua_touserdata(L, 1);

  free(unibar->screen);
  xcb_destroy_window(connection, unibar->window);
  cairo_surface_destroy(unibar->surface);
  cairo_destroy(unibar->cr);

  return 0;
}

static int f_show_window(lua_State *L) {
  xcb_window_t window = luaL_checkinteger(L, 1);
  xcb_map_window(connection, window);
  xcb_flush(connection);

  return 0;
}

static int f_wait_event(lua_State *L) {
  xcb_generic_event_t *event;

  while ((event = xcb_wait_for_event(connection))) {
    switch (event->response_type & ~0x80) {
    case 0: {
      fprintf(stderr, "X11 Error: %s\n",
        xcb_event_get_error_label(((xcb_generic_error_t *)event)->error_code));
      free(event);
      continue;
    }
    case XCB_BUTTON_PRESS: {
      xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
      lua_pushstring(L, "mousepress");
      lua_pushinteger(L, press->detail);
      lua_pushinteger(L, press->event_x);
      lua_pushinteger(L, press->event_y);
      free(event);
      return 4;
    }
    default:
      free(event);
      continue;
    }
  }

  return 0;
}

static const luaL_Reg sys_lib[] = {{"create_unibar", f_create_unibar},
  {"show_window", f_show_window}, {"wait_event", f_wait_event}, {NULL, NULL}};

static int open_sys(lua_State *L) {
  luaL_newlib(L, sys_lib);
  return 1;
}

int main(int argc, char **argv) {
  int screen_num = 0;

  connection = xcb_connect(NULL, &screen_num);
  if (xcb_connection_has_error(connection)) {
    fprintf(stderr, "Failed to connect to X display.");
    xcb_disconnect(connection);
    return 1;
  }

  xcb_intern_atom_cookie_t *atom_cookies =
    xcb_ewmh_init_atoms(connection, &ewmh);
  if (!xcb_ewmh_init_atoms_replies(&ewmh, atom_cookies, NULL)) {
    fprintf(stderr, "Failed initialise EWMH data.");
    xcb_disconnect(connection);
    return 1;
  }

  lua_State *L = luaL_newstate();

  luaL_newmetatable(L, "unibar");
  lua_pushcfunction(L, f_get_unibar_prop);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, f_destroy_unibar);
  lua_setfield(L, -2, "__gc");

  luaL_openlibs(L);
  luaL_requiref(L, "system", open_sys, 1);

  lua_createtable(L, argc, 0);
  for (int i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_seti(L, -2, i + 1);
  }
  lua_setglobal(L, "ARGV");

  (void)luaL_dostring(L, "require('core').run()");

  lua_close(L);

  xcb_disconnect(connection);

  return 0;
}
