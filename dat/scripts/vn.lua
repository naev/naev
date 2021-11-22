-- Lazy loader for vn
local vn
local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not vn then
         vn = require "vn.core"
         self.__index = vn
      end
      return vn[key]
   end,
   __newindex = function( self, key, value )
      if not vn then
         vn = require "vn.core"
         self.__index = vn
      end
      vn[key] = value
   end
} )
return lazy
