local hypergate = require "spob.lua.lib.hypergate"

init  = hypergate.init

function load ()
   return hypergate.load{
         basecol = { 0.8, 0.2, 0.2 }, -- Za'lek
         cost_mod = {
            [100] = 0,
            [90]  = 0.1,
            [80]  = 0.2,
            [70]  = 0.3,
            [50]  = 0.5,
            [30]  = 0.8,
            [10]  = 0.9,
         },
         tex = "hypergate_zalek_activated.webp",
      }
end

unload   = hypergate.unload
update   = hypergate.update
render   = hypergate.render
can_land = hypergate.can_land
land     = hypergate.land
