local wormhole = require "spob/lua/lib/wormhole"

function load( p )
   return wormhole.load( p, "Wormhole NGC-1931" )
end

unload   = wormhole.unload
update   = wormhole.update
render   = wormhole.render
can_land = wormhole.can_land
land     = wormhole.land
