---@alias module
---| string # module name
---| any[] # module options

---@class bar_config
---@field screen integer?
---@field side "top" | "bottom"
---@field height integer?
---@field border integer?
---@field background string?
---@field modules module[]

---@class config
local config = {
	---@type string[]
	fonts = {},
  ---@type bar_config[]
  bars = {}
}

---@param cfg bar_config
function config.create_bar(cfg)
  table.insert(config.bars, cfg)
end

return config
