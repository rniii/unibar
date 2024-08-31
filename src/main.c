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
xcb_ewmh_connection_t ewmh_connection;
xcb_screen_t *screen;
xcb_window_t window;
cairo_surface_t *surface;
cairo_t *cr;

static int f_show_window(lua_State *L) {
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

static int f_sleep(lua_State *L) {
  usleep(luaL_checknumber(L, 1) * 1000);
  return 0;
}

static const luaL_Reg sys_lib[] = {{"show_window", f_show_window},
  {"wait_event", f_wait_event}, {"sleep", f_sleep}, {NULL, NULL}};

static int open_sys(lua_State *L) {
  luaL_newlib(L, sys_lib);

  lua_newtable(L);
  lua_pushinteger(L, screen->width_in_pixels);
  lua_setfield(L, -2, "width");
  lua_pushinteger(L, screen->height_in_pixels);
  lua_setfield(L, -2, "height");

  lua_setfield(L, -2, "screen");

  return 1;
}

int main(void) {
  connection = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(connection)) {
    fprintf(stderr, "Failed to connect to X display.");
    xcb_disconnect(connection);
    return 1;
  }

  if (!xcb_ewmh_init_atoms(connection, &ewmh_connection)) {
    fprintf(stderr, "Failed initialise EWMH data.");
    xcb_disconnect(connection);
    return 1;
  }

  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

  window = xcb_generate_id(connection);
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t values[2] = {
    screen->white_pixel, XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_EXPOSURE};
  xcb_create_window(connection, screen->root_depth, window, screen->root, 0, 0,
    screen->width_in_pixels, 32, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
    screen->root_visual, mask, values);
  xcb_flush(connection);

  xcb_ewmh_set_wm_state(&ewmh_connection, window, 0, NULL);

  xcb_visualtype_t *visual =
    xcb_aux_get_visualtype(connection, 0, screen->root_visual);
  surface = cairo_xcb_surface_create(connection, window, visual, 150, 150);
  cr = cairo_create(surface);

  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaL_requiref(L, "system", open_sys, 1);

  (void)luaL_dostring(L, "require('core').run()");

  lua_close(L);

  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  xcb_destroy_window(connection, window);
  xcb_disconnect(connection);

  return 0;
}
