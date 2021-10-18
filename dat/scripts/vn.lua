-- Lazy loader for vn
local vn
local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not vn then
         vn = require "vn.core"
         self.__index = vn
      end
      return vn[key]
   end
} )
return lazy
