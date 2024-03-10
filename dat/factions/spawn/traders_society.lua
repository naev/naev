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

local ftraderssociety = faction.get("Traders Society")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)

   return scom.init( ftraderssociety, weights, max )
end
