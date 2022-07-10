local luaspob = require "spob.lua.lib.spob"

function load( spb )
   -- Can't bribe
   return luaspob.setup( spb, {
      std_bribe = 0,
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
