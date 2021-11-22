-- Lazy loader for pilotname
local pilotname
local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not pilotname then
         pilotname = require "pilotname.core"
         self.__index = pilotname
      end
      return self.__index[key]
   end,
   __newindex = function( _self, _key, _value )
      warn("setting value is unsupported")
   end
} )
return lazy
