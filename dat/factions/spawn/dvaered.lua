local scom = require "factions.spawn.lib.common"

local svendetta   = ship.get("Dvaered Vendetta")
local sancestor   = ship.get("Dvaered Ancestor")
local sphalanx    = ship.get("Dvaered Phalanx")
local svigilance  = ship.get("Dvaered Vigilance")
local sretribution= ship.get("Dvaered Retribution")
local sgoddard    = ship.get("Dvaered Goddard")
local sarsenal    = ship.get("Dvaered Arsenal")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( { __doscans = true }, {
      { w=0.5, svendetta, sancestor },
      { w=0.8, svendetta, svendetta, sancestor },
      { sphalanx, svendetta, sancestor },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( { __doscans = (rnd.rnd() < 0.5) }, {
      { w=0.5, svigilance, svendetta, sancestor },
      { w=0.8, svigilance, svendetta, svendetta, sancestor },
      { svigilance, sphalanx, svendetta },
   } )
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   -- Generate the capship
   local pilots = scom.doTable( {}, {
      { w=0.1, sarsenal },
      { w=0.4, sretribution },
      { sgoddard },
   } )

   -- Generate the escorts
   return scom.doTable( pilots, {
      { w=0.5, svendetta, svendetta, sancestor },
      { w=0.8, sphalanx, svendetta, sancestor },
      { svigilance, svendetta, svendetta },
   } )
end

local fdvaered = faction.get("Dvaered")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 300
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)
   weights[ spawn_capship ] = math.max(1, -500 + 1.70 * max)

   return scom.init( fdvaered, weights, max, {patrol=true} )
end
