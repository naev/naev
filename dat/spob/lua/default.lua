local luaspob = require "spob.lua.lib.spob"

function load( spb )
   return luaspob.setup( spb, {
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
