-- Lazy loader for pilotname
local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not vn then
         self.__index = require "pilotname.core"
      end
      return self.__index[key]
   end
} )
return lazy
