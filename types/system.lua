---@meta

---@class systemlib
system = {}

---@type {width: integer, height: integer}
system.screen = {}

function system.show_window() end
function system.end_frame() end

---@return string # event type
---@return any ... # parameters
function system.wait_event() end

---@param ms integer
function system.sleep(ms) end
