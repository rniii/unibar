local core = {}

function core.main()
	local config_dir = os.getenv("XDG_CONFIG_HOME") or os.getenv("HOME") .. "/.config"
	package.path = config_dir .. "/unibar/?.lua;" .. package.path
	package.path = "/etc/unibar/?.lua" .. package.path

	assert(pcall(require, "config"), "No config found!")
end

function core.run()
	local ok = xpcall(core.main, function(err)
		print("Error: " .. tostring(err))
		print(debug.traceback())
	end)
	return ok and 0 or 1
end

return core
