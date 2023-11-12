local scom = require "factions.spawn.lib.common"

local slancelot   = ship.get("Lancelot")
local sgoddard    = ship.get("Goddard")

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   local pilots = {}
   -- Generate the capship
   scom.addPilot( pilots, sgoddard )

   -- Generate the escorts
   return scom.doTable( pilots, {
      { w=0.5, slancelot, slancelot },
      { w=0.8, slancelot, slancelot, slancelot },
      { slancelot, slancelot, slancelot, slancelot },
   } )
end

local fgoddard = faction.get("Goddard")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_capship ] = 100

   return scom.init( fgoddard, weights, max )
end
