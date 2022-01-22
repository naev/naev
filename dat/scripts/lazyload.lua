--[[--
   Laziest lib there is.
   @module lazyload
--]]
local lazyload -- If you write "local function lazyload", LDoc is too lazy to document it.

local lib = {} -- Has to be here so subsequent calls to lazyload work happy

--[[--
   Main function that wraps a library to lazy load it.

   @usage return require("lazyload")( "mylib", false )

      @tparam string libname Name of library to lazy load (gets passed to require)
      @tparam[opt=false] boolean rw Whether or not the library should be writable.
      @treturn table A library that can be used in place of the real library with lazy loading.
--]]
function lazyload( libname, rw )
   local locallib
   local mt = {
      __index = function( self, key )
         if not locallib then
            if not lib[libname] then
               lib[libname] = require(libname)
            end
            locallib = lib[libname]
            self.__index = locallib
         end
         return locallib[key]
      end
   }
   if rw then
      mt.__newindex = function( self, key, value )
         if not locallib then
            if not lib[libname] then
               lib[libname] = require(libname)
            end
            locallib = lib[libname]
            self.__index = locallib
         end
         locallib[key] = value
      end
   end
   return setmetatable( {}, mt )
end

return lazyload
