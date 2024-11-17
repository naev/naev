local wormhole = require "spob.lua.lib.wormhole"

function init( p )
   return wormhole.init( p, "Wormhole Ivella", {
      col_inner   = {0.2, 0.8, 1.0},
      col_outter  = {0.3, 0.2, 1.0},
   } )
end

load     = wormhole.load
unload   = wormhole.unload
update   = wormhole.update
render   = wormhole.render
can_land = wormhole.can_land
land     = wormhole.land
