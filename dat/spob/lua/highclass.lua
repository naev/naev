local luaspob = require "spob.lua.lib.spob"

function init( spb )
   -- Can't bribe
   return luaspob.init( spb, {
      std_bribe = 0,
   } )
end

load = luaspob.load
unload = luaspob.unload
can_land = luaspob.can_land
comm = luaspob.comm
