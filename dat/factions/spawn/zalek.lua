local scom = require "factions.spawn.lib.common"

--local sdronescout = ship.get("Za'lek Scout Drone")
local sdronelight = ship.get("Za'lek Light Drone")
local sdronebomber= ship.get("Za'lek Bomber Drone")
local sdroneheavy = ship.get("Za'lek Heavy Drone")
local ssting      = ship.get("Za'lek Sting")
local sdemon      = ship.get("Za'lek Demon")
local smephisto   = ship.get("Za'lek Mephisto")
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
      { ssting },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   local pilots = scom.doTable( { __doscans=(rnd.rnd() < 0.5) }, {
      { w=0.5, ssting },
      { w=0.8, ssting, sdroneheavy, sdroneheavy },
      { sdemon },
   } )
   return spawn_patrol( pilots ) -- MOAR DRONES
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   -- Generate the capship
   local pilots = scom.doTable( {}, {
      { w=0.1, smammon },
      { w=0.55, smephisto },
      { sdiablo },
   } )

   -- Generate the escorts
   return scom.doTable( pilots, {
      { w=0.5, sdroneheavy, sdroneheavy, sdronebomber, sdronelight, sdronelight, sdronelight },
      { w=0.8, ssting, sdronebomber, sdronebomber },
      { sdemon, sdroneheavy, sdronebomber, sdronelight, sdronelight },
   } )
end

local fzalek = faction.get("Za'lek")
-- @brief Creation hook.
function create( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 300
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)
   weights[ spawn_capship ] = math.max(1, -500 + 1.70 * max)

   return scom.init( fzalek, weights, max, {patrol=true} )
end
