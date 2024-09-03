local config = require "core.config"

config.fonts = {"monospace"}

config.create_bar {
  background = "#181818",
  modules = {"workspaces", "spacer", "date"},
}
