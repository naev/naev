local luaspob = require "spob.lua.lib.spob"

function load( spb )
   -- Easier to bribe
   return luaspob.setup( spb, {
      std_land = -20,
      std_bribe = -80,
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
