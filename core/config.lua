---@alias module
---| string # module name
---| any[] # module options

---@class bar
---@field modules module[]

---@class config
local config = {
	---@type string[]
	fonts = {},
  ---@type bar[]
  bars = {}
}

---@param cfg bar
function config.create_bar(cfg)
  table.insert(config.bars, cfg)
end

return config
