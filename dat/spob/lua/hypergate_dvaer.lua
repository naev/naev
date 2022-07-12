local hypergate = require "spob.lua.lib.hypergate"

init     = hypergate.init

function load ()
   return hypergate.load{
         basecol = { 0.8, 0.5, 0.2 }, -- Dvaered
         cost_mod = {
            [100] = 0,
            [90]  = 0.1,
            [70]  = 0.3,
            [50]  = 0.5,
            [20]  = 0.8,
         },
         tex = "hypergate_dvaered_activated.webp",
      }
end

unload   = hypergate.unload
update   = hypergate.update
render   = hypergate.render
can_land = hypergate.can_land
land     = hypergate.land
