---@meta

---@type string[]
ARGV = {}

---@class systemlib
system = {}

---@class unibar
---@field window integer
local unibar = {}

---@param cmd string
function unibar:draw(cmd, ...) end

function unibar:draw() end

function unibar:show() end

---@return unibar
function system.create_unibar() end

---@return string
---@return any ...
function system.wait_event() end
