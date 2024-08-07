local scom = require "factions.spawn.lib.common"

local sllama      = ship.get("Llama")
local sgawain     = ship.get("Gawain")
local skoala      = ship.get("Koala")
local squicksilver= ship.get("Quicksilver")
local smule       = ship.get("Mule")
local srhino      = ship.get("Rhino")
--local sshark      = ship.get("Shark")

-- @brief Spawns a small trade fleet.
local function spawn_patrol ()
   return scom.doTable( {}, {
      { w=0.5, sllama },
      { w=0.8, sllama, sllama },
      { skoala, sllama },
   } )
end

-- @brief Spawns a larger trade fleet.
local function spawn_squad ()
   if rnd.rnd() < 0.5 then
      return scom.doTable( {}, {
         { w=0.5, skoala, sllama, sllama },
         { skoala, sgawain, sgawain },
      } )
   else
      local pilots = {}
      if rnd.rnd() < 0.7 then
         scom.addPilot( pilots, smule )
      else
         scom.addPilot( pilots, srhino )
      end
      return scom.doTable( {}, {
         { w=0.4, sllama, sllama },
         { w=0.7, skoala, skoala },
         { squicksilver, squicksilver },
      } )
   end
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 100 }
   t.squad   = { f = spawn_squad,   w = math.max(1, -80 + 0.80 * max) }
end, 10
