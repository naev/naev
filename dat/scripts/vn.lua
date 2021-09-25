-- Lazy loader for vn
local _vn
local lazy = setmetatable( {}, {
   __index = function( tbl, key )
      if not _vn then
         _vn = require "vn.core"
      end
      return _vn[key]
   end
} )
return lazy
