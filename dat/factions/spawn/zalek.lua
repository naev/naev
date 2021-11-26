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
   pilots = pilots or { __doscans = true }
   local r = rnd.rnd()

   if r < 0.2 then
      scom.addPilot( pilots, sdronelight )
      scom.addPilot( pilots, sdronelight )
   elseif r < 0.3 then
      scom.addPilot( pilots, sdronebomber )
      scom.addPilot( pilots, sdronebomber )
   elseif r < 0.5 then
      scom.addPilot( pilots, sdroneheavy )
      scom.addPilot( pilots, sdronelight )
   elseif r < 0.7 then
      scom.addPilot( pilots, sdronebomber )
      scom.addPilot( pilots, sdronelight )
      scom.addPilot( pilots, sdronelight )
   elseif r < 0.8 then
      scom.addPilot( pilots, sdroneheavy )
      scom.addPilot( pilots, sdronelight )
      scom.addPilot( pilots, sdronelight )
   else
      scom.addPilot( pilots, ssting )
   end

   return pilots
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   local pilots = {}
   if rnd.rnd() < 0.5 then
      pilots.__doscans = true
   end
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, ssting )
      spawn_patrol( pilots )
   elseif r < 0.8 then
      scom.addPilot( pilots, ssting )
      scom.addPilot( pilots, sdroneheavy )
      scom.addPilot( pilots, sdroneheavy )
      spawn_patrol( pilots )
   else
      scom.addPilot( pilots, sdemon )
      spawn_patrol( pilots )
   end

   return pilots
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   local pilots = {}
   local r = rnd.rnd()

   -- Generate the capship
   if r < 0.1 then
      scom.addPilot( pilots, smammon )
   elseif r < 0.55 then
      scom.addPilot( pilots, smephisto )
   else
      scom.addPilot( pilots, sdiablo )
   end

   -- Generate the escorts
   r = rnd.rnd()
   if r < 0.5 then
      scom.addPilot( pilots, sdroneheavy )
      scom.addPilot( pilots, sdroneheavy )
      scom.addPilot( pilots, sdronebomber )
      scom.addPilot( pilots, sdronelight )
      scom.addPilot( pilots, sdronelight )
      scom.addPilot( pilots, sdronelight )
   elseif r < 0.8 then
      scom.addPilot( pilots, ssting )
      scom.addPilot( pilots, sdronebomber )
      scom.addPilot( pilots, sdronebomber )
   else
      scom.addPilot( pilots, sdemon )
      scom.addPilot( pilots, sdroneheavy )
      scom.addPilot( pilots, sdronebomber )
      scom.addPilot( pilots, sdronelight )
      scom.addPilot( pilots, sdronelight )
   end

   return pilots
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
