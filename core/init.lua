local core = {}

function core.redraw()
  system.end_frame()
end

function core.loop()
  local config_dir = os.getenv("XDG_CONFIG_HOME") or os.getenv("HOME") .. "/.config"
  local loaded = false
  for _, dir in ipairs({ config_dir, "/etc" }) do
    loaded = loaded or pcall(require, dir .. "/unibar/init.lua")
  end
  if not loaded then
    error("No config found!")
  end

  system.show_window()

  for event in system.wait_event do
    core.redraw()
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
