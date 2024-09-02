local config = require "core.config"
local core = {}

---@type table<integer, unibar>
core.bars = {}

---@param hex string
local function color(hex)
	local i, _, r, g, b = hex:find("#(%x%x)(%x%x)(%x%x)")
	if i == nil then
		return 0, 0, 0
	else
		return tonumber(r, 16) / 255, tonumber(g, 16) / 255, tonumber(b, 16) / 255
	end
end

function core.redraw()
	for _, bar in pairs(core.bars) do
		bar:draw("paint", color("#151515"))
		bar:draw("text", "meow", color("#d8d0d5"))
		bar:draw()
	end
end

function core.loop()
	local config_dir = os.getenv("XDG_CONFIG_HOME") or os.getenv("HOME") .. "/.config"
  package.path = config_dir .. "/unibar/?.lua;" .. package.path
  package.path = "/etc/unibar/?.lua" .. package.path

  assert(pcall(require, "config"), "No config found!")

	for _, cfg in ipairs(config.bars) do
		local bar = system.create_unibar()
		core.bars[bar.window] = bar

		bar:show()
	end

	while true do
		core.redraw()

		local event = system.wait_event()
	end
end

function core.run()
	xpcall(core.loop, function(err)
		print("Error: " .. tostring(err))
		print(debug.traceback())
		os.exit(1)
	end)
end

return core
