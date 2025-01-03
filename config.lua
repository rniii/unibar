local config = require "core.config"

config.fonts = {"monospace"}

config.create_bar {
  height = 34,
  border = 8,
  background = "#181818fa",
  modules = {"workspaces", "spacer", "mpris", "spacer", "time"},
}
