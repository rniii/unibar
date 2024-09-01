---@meta

---@type string[]
ARGV = {}

---@class systemlib
system = {}

---@class unibar
---@field window integer

---@return unibar
function system.create_unibar() end

---@param window integer
function system.show_window(window) end

---@return string
---@return any ...
function system.wait_event() end
