local config = require "core.config"
local core = {}

function core.redraw()
end

function core.loop()
	local config_dir = os.getenv("XDG_CONFIG_HOME") or os.getenv("HOME") .. "/.config"
	local loaded = false
	for _, path in ipairs({ config_dir .. "/unibar/init.lua", "/etc/unibar/init.lua", "./default" }) do
		loaded = loaded or pcall(require, path)
	end
	if not loaded then
		error("No config found!")
	end

  for _, cfg in ipairs(config.bars) do
    local bar = system.create_unibar()
    system.show_window(bar.window)
  end

  for event in system.wait_event do
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
