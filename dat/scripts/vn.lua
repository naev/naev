-- Lazy loader for vn
local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not vn then
         self.__index = require "vn.core"
      end
      return self.__index[key]
   end
} )
return lazy
