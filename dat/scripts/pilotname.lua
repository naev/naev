-- Lazy loader for pilotname
local pilotname
local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not pilotname then
         pilotname = require "pilotname.core"
         self.__index = pilotname
      end
      return self.__index[key]
   end
} )
return lazy
