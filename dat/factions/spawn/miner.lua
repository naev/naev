local scom = require "factions.spawn.lib.common"
local var = require "shipvariants"

local smule = scom.variants{
   { w=1,   s=ship.get("Mule") },
   { w=0.2, s=ship.get("Mule Hardhat") },
}

-- @brief Spawns a small group of miners
local function spawn_lone_miner ()
   local pilots = {}
   local r = rnd.rnd()
   if r < 0.4 then
      scom.addPilot( pilots, var.llama, {name=_("Miner Llama"), ai="miner"} )
   elseif r < 0.7 then
      scom.addPilot( pilots, var.koala, {name=_("Miner Koala"), ai="miner"} )
   elseif r < 0.9 then
      scom.addPilot( pilots, smule,  {name=_("Miner Mule"), ai="miner"} )
   else
      scom.addPilot( pilots, var.rhino, {name=_("Miner Rhino"), ai="miner"} )
   end

   return pilots
end

local fminer = faction.get("Miner")
-- @brief Creation hook.
function create ( max )
   local weights = {}

    -- Create weights for spawn table
   weights[ spawn_lone_miner  ] = 100

   return scom.init( fminer, weights, max )
end
