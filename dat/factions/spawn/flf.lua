local scom = require "factions.spawn.lib.common"

local slancelot   = ship.get("Lancelot")
local svendetta   = ship.get("Vendetta")
local spacifier   = ship.get("Pacifier")

-- @brief Spawns a small fleet.
local function spawn_patrol ()
   return scom.doTable( {}, {
      { w=0.5, slancelot, slancelot },
      { w=0.8, slancelot, svendetta },
      { slancelot, slancelot, svendetta },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( {}, {
      { w=0.5, slancelot, slancelot, svendetta },
      { w=0.8, slancelot, svendetta, svendetta },
      { spacifier, slancelot },
   } )
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
