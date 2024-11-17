local scom = require "factions.spawn.lib.common"

local seuler      = ship.get("Proteron Euler")
local sdalton     = ship.get("Proteron Dalton")
local shippocrates= ship.get("Proteron Hippocrates")
local sgauss      = ship.get("Proteron Gauss")
local spythagoras = ship.get("Proteron Pythagoras")
local sarchimedes = ship.get("Proteron Archimedes")
local swatson     = ship.get("Proteron Watson")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   return scom.doTable( { __doscans=true }, {
      { w=0.5, seuler },
      { w=0.8, seuler, seuler },
      { sdalton, sdalton, sdalton },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( { __doscans=(rnd.rnd() < 0.5) }, {
      { w=0.5, sgauss, shippocrates },
      { w=0.8, sgauss, sgauss, shippocrates },
      { spythagoras },
   } )
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   -- Generate the capship
   local pilots = scom.doTable( {}, {
      { w=0.5, sarchimedes },
      { swatson },
   } )

   -- Generate the escorts
   return scom.doTable( pilots, {
      { w=0.5, sgauss, shippocrates },
      { w=0.8, sgauss, sgauss },
      { spythagoras },
   } )
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 300 }
   t.squad   = { f = spawn_squad,   w = math.max(1, -80 + 0.80 * max) }
   t.capship = { f = spawn_capship, w = math.max(1, -500 + 1.70 * max) }
end, 10
