pub var default_screen: c_int = 0;
pub var conn: *c.xcb_connection_t = undefined;

pub fn main() !void {
    conn = c.xcb_connect(null, &default_screen).?;
    defer c.xcb_disconnect(conn);

    if (c.xcb_connection_has_error(conn) != 0) return error.XcbConnectionError;

    const lua = c.luaL_newstate();
    defer c.lua_close(lua);

    assert(c.luaL_newmetatable(lua, Unibar.Type) == 1);
    c.luaL_setfuncs(lua, &[_]c.luaL_Reg{
        .{ .name = "__index", .func = Unibar.lua_index },
        .{ .name = "__gc", .func = Unibar.lua_gc },
        .{ .name = null, .func = null },
    }, 0);
    c.lua_pop(lua, 1);

    c.lua_createtable(lua, @intCast(std.os.argv.len), 0);
    for (std.os.argv, 1..) |arg, i| {
        _ = c.lua_pushstring(lua, arg);
        c.lua_seti(lua, -2, @intCast(i));
    }
    c.lua_setglobal(lua, "ARGV");

    c.luaL_openlibs(lua);
    Unibar.open(lua);
}

pub const c = @cImport({
    @cInclude("cairo/cairo.h");
    @cInclude("cairo/cairo-xcb.h");

    @cInclude("xcb/xcb.h");
    @cInclude("xcb/xcb_ewmh.h");
    @cInclude("xcb/xcb_icccm.h");
    @cInclude("xcb/xcb_util.h");

    @cInclude("lua.h");
    @cInclude("lualib.h");
    @cInclude("lauxlib.h");
});

const std = @import("std");
const Unibar = @import("Unibar.zig");
const assert = std.debug.assert;
