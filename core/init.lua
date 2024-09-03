local config = require "core.config"
local core = {}

---@type table<integer, unibar>
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
	for _, bar in pairs(core.bars) do
		bar:draw("paint", color("#151515"))
		bar:draw("text", "meow", color("#d8d0d5"))
		bar:draw()
	end
end

function core.main()
  warn("@on")

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
	local ok = xpcall(core.main, function(err)
		print("Error: " .. tostring(err))
		print(debug.traceback())
	end)
	return ok and 0 or 1
end

return core
