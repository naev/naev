-- Lazy loader for luaspfx
local luaspfx
local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not luaspfx then
         luaspfx = require "luaspfx.core"
         self.__index = luaspfx
      end
      return luaspfx[key]
   end,
   __newindex = function( _self, _key, _value )
      warn("setting value is unsupported")
   end
} )
return lazy
