local scom = require "factions.spawn.lib.common"

local slancelot   = ship.get("Lancelot")
local svendetta   = ship.get("Vendetta")
local spacifier   = ship.get("Pacifier")

-- @brief Spawns a small fleet.
local function spawn_patrol ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
   elseif r < 0.8 then
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, svendetta )
   else
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, svendetta )
   end

   return pilots
end


-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, svendetta )
   elseif r < 0.8 then
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, svendetta )
      scom.addPilot( pilots, svendetta )
   else
      scom.addPilot( pilots, spacifier )
      scom.addPilot( pilots, slancelot )
   end

   return pilots
end


local fflf = faction.get("FLF")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)

   return scom.init( fflf, weights, max )
end
