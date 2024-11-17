local scom = require "factions.spawn.lib.common"
local var = require "shipvariants"

-- @brief Spawns a small trade fleet.
local function spawn_patrol ()
   return scom.doTable( {}, {
      { w=0.5, var.llama },
      { w=0.8, var.llama, var.llama },
      { var.koala, var.llama },
   } )
end

-- @brief Spawns a larger trade fleet.
local function spawn_squad ()
   if rnd.rnd() < 0.5 then
      return scom.doTable( {}, {
         { w=0.5, var.koala, var.llama, var.llama },
         { var.koala, var.gawain, var.gawain },
      } )
   else
      local pilots = {}
      if rnd.rnd() < 0.7 then
         scom.addPilot( pilots, var.mule )
      else
         scom.addPilot( pilots, var.rhino )
      end
      return scom.doTable( {}, {
         { w=0.4, var.llama, var.llama },
         { w=0.7, var.koala, var.koala },
         { var.quicksilver, var.quicksilver },
      } )
   end
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 100 }
   t.squad   = { f = spawn_squad,   w = math.max(1, -80 + 0.80 * max) }
end, 10
