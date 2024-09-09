local config = require "core.config"
local core = {}

---@class corebar
---@field bg integer[]
---@field inner unibar

---@type table<integer, corebar>
core.bars = {}

---@param hex string
local function color(hex)
	local function num(h)
		return tonumber(h, 16)
	end

	local pat
	if hex:len() == 4 then
		pat = "^#(%x)(%x)(%x)$"
	elseif hex:len() == 5 then
		pat = "^#(%x)(%x)(%x)(%x)$"
	elseif hex:len() == 7 then
		pat = "^#(%x%x)(%x%x)(%x%x)$"
	else
		pat = "^#(%x%x)(%x%x)(%x%x)(%x%x)$"
	end

	local i, _, r, g, b, a = hex:find(pat)
	if i == nil then
		warn("invalid color: " .. hex)
		return 0, 0, 0
	end

	return num(r), num(g), num(b), a and num(a)
end

function core.redraw()
	for _, bar in ipairs(core.bars) do
		bar.inner:draw("paint", table.unpack(bar.bg))
		bar.inner:draw()
	end
end

function core.main()
	warn("@on")

	local config_dir = os.getenv("XDG_CONFIG_HOME") or os.getenv("HOME") .. "/.config"
	package.path = config_dir .. "/unibar/?.lua;" .. package.path
	package.path = "/etc/unibar/?.lua" .. package.path

	assert(pcall(require, "config"), "No config found!")

	for _, cfg in ipairs(config.bars) do
		local screen_num = cfg.screen or 1
		local screen = system.screens[screen_num]

		local rect
		local border = cfg.border or 0
		local height = cfg.height or 34
		if cfg.side == "bottom" then
			rect = { border, screen.height - height - border, screen.width - border * 2, height }
		else
			rect = { border, border, screen.width - border * 2, height }
		end

		local bar = system.create_unibar { screen = screen_num, rect = rect }

		core.bars[bar.window] = {
			bg = table.pack(color(cfg.background or "#181818fa")),
			inner = bar,
		}

		bar:show()
	end

	while true do
		core.redraw()

		local event = system.wait_event()
	end
end

function core.run()
	local ok = xpcall(core.main, function(err)
		print("Error: " .. tostring(err))
		print(debug.traceback())
	end)
	return ok and 0 or 1
end

return core
