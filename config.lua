local config = require "core.config"

config.fonts = {"monospace"}

config.create_bar {
  modules = {"workspaces", "spacer", "date"},
}
