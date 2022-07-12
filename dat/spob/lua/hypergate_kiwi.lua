local hypergate = require "spob.lua.lib.hypergate"

init     = hypergate.init

function load ()
   return hypergate.load{
         basecol = { 0.8, 0.8, 0.2 }, -- Sirius
         cost_mod = {
            [100] = 0,
            [90]  = 0.1,
            [75]  = 0.2,
            [60]  = 0.3,
            [45]  = 0.5,
            [30]  = 0.75,
            [10]  = 0.9,
         },
         tex = "hypergate_sirius_activated.webp",
      }
end

unload   = hypergate.unload
update   = hypergate.update
render   = hypergate.render
can_land = hypergate.can_land
land     = hypergate.land
