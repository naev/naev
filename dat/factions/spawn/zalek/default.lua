local scom = require "factions.spawn.lib.common"
local var = require "shipvariants"

--local sdronescout = ship.get("Za'lek Scout Drone")
local sdronelight = ship.get("Za'lek Light Drone")
local sdronebomber= ship.get("Za'lek Bomber Drone")
local sdroneheavy = ship.get("Za'lek Heavy Drone")
local sdiablo     = ship.get("Za'lek Diablo")
local smammon     = ship.get("Za'lek Mammon")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol( pilots )
   pilots = pilots or { __doscans=true }
   return scom.doTable( pilots, {
      { w=0.2, sdronelight, sdronelight },
      { w=0.3, sdronebomber, sdronebomber },
      { w=0.5, sdroneheavy, sdronelight },
      { w=0.7, sdronebomber, sdronelight, sdronelight },
      { w=0.8, sdroneheavy, sdronelight, sdronelight },
      { var.zalek_sting },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   local pilots = scom.doTable( { __doscans=(rnd.rnd() < 0.5) }, {
      { w=0.5, var.zalek_sting },
      { w=0.8, var.zalek_sting, sdroneheavy, sdroneheavy },
      { var.zalek_demon },
   } )
   return spawn_patrol( pilots ) -- MOAR DRONES
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   -- Generate the capship
   local pilots = scom.doTable( {}, {
      { w=0.1, smammon },
      { w=0.55, var.zalek_mephisto },
      { sdiablo },
   } )

   -- Generate the escorts
   return scom.doTable( pilots, {
      { w=0.5, sdroneheavy, sdroneheavy, sdronebomber, sdronelight, sdronelight, sdronelight },
      { w=0.8, var.zalek_sting, sdronebomber, sdronebomber },
      { var.zalek_demon, sdroneheavy, sdronebomber, sdronelight, sdronelight },
   } )
end

return function ( t, max )
   t.patrol  = { f = spawn_patrol,  w = 300 }
   t.squad   = { f = spawn_squad,   w = math.max(1, -80 + 0.80 * max) }
   t.capship = { f = spawn_capship, w = math.max(1, -500 + 1.70 * max) }
end, 10
