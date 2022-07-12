local luaspob = require "spob.lua.lib.spob"

function init( spb )
   return luaspob.init( spb, {
   } )
end

load = luaspob.load
unload = luaspob.unload
can_land = luaspob.can_land
comm = luaspob.comm
