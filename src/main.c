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

#define UNIBAR_TYPE "unibar"

xcb_connection_t *connection;
xcb_ewmh_connection_t ewmh;

typedef struct {
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_visualtype_t *visual;
  cairo_surface_t *sf;
  cairo_t *cr;
} unibar_t;

static int lua_create_unibar(lua_State *L) {
  lua_getfield(L, 1, "screen");
  int screen_idx = lua_tonumber(L, 2) - 1;

  lua_getfield(L, 1, "rect");
  int x, y, w, h;
  lua_geti(L, 3, 1), x = lua_tonumber(L, 4);
  lua_geti(L, 3, 2), y = lua_tonumber(L, 5);
  lua_geti(L, 3, 3), w = lua_tonumber(L, 6);
  lua_geti(L, 3, 4), h = lua_tonumber(L, 7);

  xcb_screen_t *screen = xcb_aux_get_screen(connection, screen_idx);
  if (!screen) {
    luaL_error(L, "No such screen: %i", screen_idx);
    return 0;
  }

  uint8_t depth = 32;
  xcb_visualtype_t *visual =
    xcb_aux_find_visual_by_attrs(screen, XCB_VISUAL_CLASS_TRUE_COLOR, depth);
  xcb_colormap_t colormap;
  if (visual) {
    colormap = xcb_generate_id(connection);
    xcb_create_colormap(connection, XCB_COLORMAP_ALLOC_NONE, colormap,
      screen->root, visual->visual_id);
  } else {
    colormap = XCB_COPY_FROM_PARENT;
    visual = xcb_aux_find_visual_by_id(screen, screen->root_visual);
    depth = screen->root_depth;
  }

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK |
    XCB_CW_COLORMAP;
  uint32_t values[] = {
    0, 0, XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_EXPOSURE, colormap};

  xcb_window_t window = xcb_generate_id(connection);
  xcb_create_window(connection, depth, window, screen->root, x, y, w, h, 0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT, visual->visual_id, mask, values);

  char wm_class[] = "unibar";
  xcb_atom_t wm_state[] = {ewmh._NET_WM_STATE_ABOVE};
  xcb_atom_t wm_window_type[] = {ewmh._NET_WM_WINDOW_TYPE_DOCK};
  xcb_icccm_set_wm_class(connection, window, sizeof(wm_class), wm_class);
  xcb_ewmh_set_wm_state(&ewmh, window, 1, wm_state);
  xcb_ewmh_set_wm_window_type(&ewmh, window, 1, wm_window_type);

  cairo_surface_t *sf =
    cairo_xcb_surface_create(connection, window, visual, w, h);
  cairo_t *cr = cairo_create(sf);

  cairo_select_font_face(
    cr, "@cairo:", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, 16);

  xcb_flush(connection);

  unibar_t *unibar = lua_newuserdata(L, sizeof(unibar_t));
  unibar->screen = screen;
  unibar->window = window;
  unibar->visual = visual;
  unibar->sf = sf;
  unibar->cr = cr;

  luaL_setmetatable(L, UNIBAR_TYPE);

  return 1;
}

static int lua_destroy_unibar(lua_State *L) {
  unibar_t *unibar = luaL_checkudata(L, 1, UNIBAR_TYPE);
  xcb_destroy_window(connection, unibar->window);
  cairo_surface_destroy(unibar->sf);
  cairo_destroy(unibar->cr);

  return 0;
}

static int lua_set_cairo_source(lua_State *L, int idx) {
  unibar_t *unibar = lua_touserdata(L, 1);

  switch (lua_type(L, idx)) {
  case LUA_TNUMBER:
    double r, g, b, a;
    r = lua_tonumber(L, idx + 0);
    g = lua_tonumber(L, idx + 1);
    b = lua_tonumber(L, idx + 2);
    a = lua_isnil(L, idx + 3) ? 1 : lua_tonumber(L, idx + 3);
    cairo_set_source_rgba(unibar->cr, r, g, b, a);
    return 0;
  default:
    return 0;
  }
}

static int lua_draw_unibar(lua_State *L) {
  unibar_t *unibar = luaL_checkudata(L, 1, UNIBAR_TYPE);

  const char *commands[] = {"flush", "stroke", "fill", "paint", "text", NULL};
  int opt = luaL_checkoption(L, 2, "flush", commands);

  switch (opt) {
  case 0: // flush
    cairo_surface_flush(unibar->sf);
    xcb_flush(connection);
    return 0;
  case 1: // stroke
    return 0;
  case 2: // fill
    return 0;
  case 3: // paint
    lua_set_cairo_source(L, 2);
    cairo_paint(unibar->cr);
    return 0;
  case 4: // text
    const char *text = luaL_checkstring(L, 2);
    lua_set_cairo_source(L, 3);
    cairo_move_to(unibar->cr, 0, 0);
    cairo_show_text(unibar->cr, text);
    return 0;
  default:
    return 0;
  }
}

static int lua_show_unibar(lua_State *L) {
  unibar_t *unibar = luaL_checkudata(L, 1, UNIBAR_TYPE);
  xcb_map_window(connection, unibar->window);
  xcb_flush(connection);

  return 0;
}

static int lua_index_unibar(lua_State *L) {
  unibar_t *unibar = luaL_checkudata(L, 1, UNIBAR_TYPE);

  const char *props[] = {"window", "draw", "show", NULL};
  switch (luaL_checkoption(L, 2, NULL, props)) {
  case 0:
    lua_pushinteger(L, unibar->window);
    return 1;
  case 1:
    lua_pushcfunction(L, lua_draw_unibar);
    return 1;
  case 2:
    lua_pushcfunction(L, lua_show_unibar);
    return 1;
  default:
    unreachable();
  }
}

static int lua_wait_event(lua_State *L) {
  xcb_generic_event_t *event;

  while ((event = xcb_wait_for_event(connection))) {
    switch (event->response_type & ~0x80) {
    case 0:
      fprintf(stderr, "X11 Error: %s\n",
        xcb_event_get_error_label(((xcb_generic_error_t *)event)->error_code));
      free(event);
      continue;
    case XCB_BUTTON_PRESS:
      xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
      lua_pushstring(L, "mousepress");
      lua_pushinteger(L, press->event);
      lua_pushinteger(L, press->detail);
      lua_pushinteger(L, press->event_x);
      lua_pushinteger(L, press->event_y);
      free(event);
      return 5;
    default:
      free(event);
      continue;
    }
  }

  return 0;
}

static int open_system(lua_State *L) {
  const luaL_Reg system_lib[] = {
    {"create_unibar", lua_create_unibar},
    {"wait_event", lua_wait_event},
    {NULL, NULL},
  };
  luaL_newlib(L, system_lib);

  xcb_screen_iterator_t roots =
    xcb_setup_roots_iterator(xcb_get_setup(connection));
  int i = 1;
  lua_createtable(L, roots.rem, 0);
  for (; roots.rem; xcb_screen_next(&roots)) {
    lua_newtable(L);
    lua_pushinteger(L, roots.data->width_in_pixels);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, roots.data->height_in_pixels);
    lua_setfield(L, -2, "height");
    lua_seti(L, -2, i++);
  }
  lua_setfield(L, -2, "screens");

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

  const luaL_Reg unibar_mt[] = {
    {"__index", lua_index_unibar},
    {"__gc", lua_destroy_unibar},
    {NULL, NULL},
  };
  luaL_newmetatable(L, UNIBAR_TYPE);
  luaL_setfuncs(L, unibar_mt, 0);
  lua_pop(L, 1);

  lua_createtable(L, argc, 0);
  for (int i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_seti(L, -2, i + 1);
  }
  lua_setglobal(L, "ARGV");

  luaL_openlibs(L);
  luaL_requiref(L, "system", open_system, 1);

  luaL_dostring(L, "return require('core').run()");
  int ret = lua_tonumber(L, -1);

  lua_close(L);

  xcb_disconnect(connection);

  return ret;
}
