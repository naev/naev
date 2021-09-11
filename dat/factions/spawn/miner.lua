local scom = require "factions.spawn.lib.common"

local sllama   = ship.get("Llama")
local skoala   = ship.get("Koala")
local smule    = ship.get("Mule")

-- @brief Spawns a small group of miners
function spawn_patrol ()
   local pilots = {}
   local r = rnd.rnd()
   if r < 0.5 then
      scom.addPilot( pilots, sllama, {name=_("Miner Llama"), ai="miner"}  )
   elseif r < 0.8 then
      scom.addPilot( pilots, skoala, {name=_("Miner Koala"), ai="miner"} )
   else
      scom.addPilot( pilots, smule,  {name=_("Miner Mule"), ai="miner"} )
   end

   return pilots
end

local fminer = faction.get("Miner")
-- @brief Creation hook.
function create ( max )
   local weights = {}

    -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100

   return scom.init( fminer, weights, max )
end
