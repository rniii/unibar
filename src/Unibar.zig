const Unibar = @This();

screen: *c.xcb_screen_t,
visual: *c.xcb_visualtype_t,
window: c.xcb_window_t,
sf: *c.cairo_surface_t,
cr: *c.cairo_t,

pub const Type = "unibar";

pub fn open(lua: ?*c.lua_State) void {
    const decls = &[_]c.luaL_Reg{
        .{ .name = "create", .func = lua_create },
        .{ .name = null, .func = null },
    };
    c.lua_createtable(lua, 0, decls.len);
    c.luaL_setfuncs(lua, decls, 0);
    c.lua_setglobal(lua, "Unibar");
}

pub fn lua_create(lua: ?*c.lua_State) callconv(.C) c_int {
    _ = c.lua_getfield(lua, 1, "screen");
    const screen_num: c_int = @intCast(c.lua_tointegerx(lua, 2, null));

    _ = c.lua_getfield(lua, 1, "rect");
    var rect: @Vector(4, u15) = @splat(0);
    inline for (0..4) |i| {
        _ = c.lua_geti(lua, 3, i + 1);
        rect[i] = @intCast(c.lua_tointegerx(lua, 4 + i, null));
    }

    const screen = c.xcb_aux_get_screen(root.conn, screen_num - 1);
    if (screen == null) {
        _ = c.luaL_error(lua, "No such screen: %i", screen_num);
        return 0;
    }

    const colormap = c.xcb_generate_id(root.conn);
    const visual =
        c.xcb_aux_find_visual_by_attrs(screen_num, c.XCB_VISUAL_CLASS_TRUE_COLOR, 32);
    assert(visual != null);

    _ = c.xcb_create_colormap(
        root.conn,
        c.XCB_COLORMAP_ALLOC_NONE,
        colormap,
        screen.*.root,
        visual.*.visual_id,
    );

    const window = c.xcb_generate_id(root.conn);
    _ = c.xcb_create_window(
        root.conn,
        32,
        window,
        screen.*.root,
        rect[0],
        rect[1],
        rect[2],
        rect[3],
        0,
        c.XCB_WINDOW_CLASS_INPUT_OUTPUT,
        visual.*.visual_id,
        c.XCB_CW_EVENT_MASK | c.XCB_CW_COLORMAP,
        &[_]u32{ c.XCB_EVENT_MASK_BUTTON_PRESS | c.XCB_EVENT_MASK_EXPOSURE, colormap },
    );

    const sf = c.cairo_xcb_surface_create(root.conn, window, visual, rect[1], rect[2]);
    const cr = c.cairo_create(sf);

    const unibar: *Unibar = @ptrCast(@alignCast(c.lua_newuserdata(lua, @sizeOf(Unibar))));
    unibar.screen = screen;
    unibar.window = window;
    unibar.visual = visual;
    unibar.sf = sf.?;
    unibar.cr = cr.?;

    return 0;
}

pub fn lua_index(lua: ?*c.lua_State) callconv(.C) c_int {
    const unibar: *Unibar = lua_checkunibar(lua);
    const name = std.mem.span(c.luaL_checklstring(lua, 1, null));

    if (std.mem.eql(u8, name, "window")) {
        c.lua_pushinteger(lua, unibar.window);
        return 1;
    } else if (std.mem.eql(u8, name, "draw")) {
        c.lua_pushcfunction(lua, lua_draw);
        return 1;
    } else if (std.mem.eql(u8, name, "show")) {
        c.lua_pushcfunction(lua, lua_show);
        return 1;
    } else {
        return 0;
    }
}

pub fn lua_draw(lua: ?*c.lua_State) callconv(.C) c_int {
    _ = lua;

    return 0;
}

pub fn lua_show(lua: ?*c.lua_State) callconv(.C) c_int {
    _ = lua;

    return 0;
}

pub fn lua_gc(lua: ?*c.lua_State) callconv(.C) c_int {
    const unibar: *Unibar = lua_checkunibar(lua);

    _ = c.xcb_destroy_window(root.conn, unibar.window);
    c.cairo_surface_destroy(unibar.sf);
    c.cairo_destroy(unibar.cr);

    return 0;
}

fn lua_checkunibar(lua: ?*c.lua_State) *Unibar {
    return @ptrCast(@alignCast(c.luaL_checkudata(lua, 1, Unibar.Type)));
}

const std = @import("std");
const root = @import("main.zig");
const c = root.c;
const assert = std.debug.assert;
