---@meta

---@class unibar
---@field window integer
local unibar = {}

---@param cmd string
function unibar:draw(cmd, ...) end
function unibar:draw() end

function unibar:show() end

---@type string[]
ARGV = {}

---@class systemlib
system = {}

---@return unibar
function system.create_unibar() end

---@return string
---@return any ...
function system.wait_event() end
