---@alias module
---| string # module name
---| [string, any ...] # module options

---@class config
return {
	---@type string[]
	fonts = {},
	---@type module[]
	modules = {},
}
