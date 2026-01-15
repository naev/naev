local scom = require "factions.spawn.lib.common"
local var = require "shipvariants"

local svendetta   = ship.get("Dvaered Vendetta")
local sphalanx    = ship.get("Dvaered Phalanx")
local svigilance  = ship.get("Dvaered Vigilance")
local sretribution= ship.get("Dvaered Retribution")
local sgoddard    = ship.get("Dvaered Goddard")
local sarsenal    = ship.get("Dvaered Arsenal")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( { __doscans = true }, {
      { w=0.5, svendetta, var.dvaered_ancestor },
      { w=0.8, svendetta, svendetta, var.dvaered_ancestor },
      { sphalanx, svendetta, var.dvaered_ancestor },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( { __doscans = (rnd.rnd() < 0.5) }, {
      { w=0.5, svigilance, svendetta, var.dvaered_ancestor },
      { w=0.8, svigilance, svendetta, svendetta, var.dvaered_ancestor },
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
      { w=0.5, svendetta, svendetta, var.dvaered_ancestor },
      { w=0.8, sphalanx, svendetta, var.dvaered_ancestor },
      { svigilance, svendetta, svendetta },
   } )
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 300 }
   t.squad   = { f = spawn_squad,   w = math.max(1, -80 + 0.80 * max) }
   t.capship = { f = spawn_capship, w = math.max(1, -500 + 1.70 * max) }
end, 10
