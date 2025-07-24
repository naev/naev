local equipopt = {}
equipopt.optimize = require 'equipopt.optimize'
equipopt.params   = require 'equipopt.params'
equipopt.cores    = require 'equipopt.cores'

-- Main equipment functions
local lf = require "love.filesystem"

-- So here we look up all the templates and try to load them to be a tad faster
local templates_lib = {}
for k,v in ipairs(lf.getDirectoryItems("scripts/equipopt/templates")) do
   local name = string.gsub(v,".lua","")
   templates_lib[name] = "equipopt.templates."..name
end
local templates = {}
local mt = {
   __index = function( _self, key )
      -- If found, just return
      local val = templates[key]
      if val then
         return val
      end
      -- Have to try to load the library
      local lib = templates_lib[key]
      if not lib then
         return lib
      end
      val = require(lib)
      templates[key] = val
      return val
   end
}
setmetatable( equipopt, mt )

return equipopt
