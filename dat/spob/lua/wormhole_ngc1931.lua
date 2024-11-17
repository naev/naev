local wormhole = require "spob.lua.lib.wormhole"

function init( spb )
   return wormhole.init( spb, "Wormhole NGC-13674" )
end

load     = wormhole.load
unload   = wormhole.unload
update   = wormhole.update
render   = wormhole.render
can_land = wormhole.can_land
land     = wormhole.land
