-- Lazy loader for luatk
local luatk
local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not luatk then
         luatk = require "luatk.core"
         self.__index = luatk
      end
      return luatk[key]
   end,
   __newindex = function( _self, _key, _value )
      warn("setting value is unsupported")
   end
} )
return lazy
