--[[
   Laziest lib there is.
--]]
local lib

local function lazyload( libname, rw )
   local mt = {
      __index = function( self, key )
         if not lib then
            lib = require(libname)
            self.__index = lib
         end
         return lib[key]
      end
   }
   if rw then
      mt.__newindex = function( self, key, value )
         if not lib then
            lib = require(libname)
            self.__index = lib
         end
         lib[key] = value
      end
   end
   return setmetatable( {}, mt )
end

return lazyload
