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
system = {
  ---@type { width: integer, height: integer }[]
  screens = {}
}

---@param bar any
---@return unibar
function system.create_unibar(bar) end

---@return string
---@return any ...
function system.wait_event() end
