local scom = require "factions.spawn.lib.common"

local slancelot   = ship.get("Lancelot")
local sgoddard    = ship.get("Goddard")

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   local pilots = {}

   -- Generate the capship
   scom.addPilot( pilots, sgoddard )

   -- Generate the escorts
   local r = rnd.rnd()
   if r < 0.5 then
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
   elseif r < 0.8 then
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
   else
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
   end

   return pilots
end

local fgoddard = faction.get("Goddard")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_capship ] = 100

   return scom.init( fgoddard, weights, max )
end
