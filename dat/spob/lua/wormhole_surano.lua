local wormhole = require "spob.lua.lib.wormhole"

function init( p )
   return wormhole.init( p, "Wormhole Carrza", {
      col_inner   ={ 1.0, 0.8, 0.2},
      col_outter  = {1.0, 0.2, 0.0},
   } )
end

load     = wormhole.load
unload   = wormhole.unload
update   = wormhole.update
render   = wormhole.render
can_land = wormhole.can_land
land     = wormhole.land
