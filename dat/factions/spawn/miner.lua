local scom = require "factions.spawn.lib.common"
local var = require "shipvariants"

-- Higher chance of being a hardhadt
local smule = scom.variants{
   { w=1,   s=ship.get("Mule") },
   { w=0.2, s=ship.get("Mule Hardhat") },
}
local sclydesdale = ship.get("Clydesdale")

-- @brief Spawns a small group of miners
local function spawn_lone_miner ()
   local pilots = {}
   local r = rnd.rnd()
   if r < 0.35 then
      scom.addPilot( pilots, var.llama, {name=_("Miner Llama"), ai="miner"} )
   elseif r < 0.65 then
      scom.addPilot( pilots, var.koala, {name=_("Miner Koala"), ai="miner"} )
   elseif r < 0.85 then
      scom.addPilot( pilots, smule,  {name=_("Miner Mule"), ai="miner"} )
   elseif r < 0.95 then
      scom.addPilot( pilots, var.rhino, {name=_("Miner Rhino"), ai="miner"} )
   else
      scom.addPilot( pilots, sclydesdale, {name=_("Miner Clydesdale"), ai="miner"} )
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
