local luaspob = require "spob.lua.lib.spob"

function init( spb )
   -- Easier to bribe
   return luaspob.init( spb, {
      std_land = -20,
      std_bribe = -80,
   } )
end

load = luaspob.load
unload = luaspob.unload
can_land = luaspob.can_land
comm = luaspob.comm
