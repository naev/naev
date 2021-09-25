-- Lazy loader for vn
local _vn
local lazy = setmetatable( {}, {
   __index = function( self, key )
      if not _vn then
         _vn = require "vn.core"
         self.__index = _vn
      end
      return _vn[key]
   end
} )
return lazy
