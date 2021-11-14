local scom = require "factions.spawn.lib.common"

local sfidelity   = ship.get("Sirius Fidelity")
local sshaman     = ship.get("Sirius Shaman")
local spreacher   = ship.get("Sirius Preacher")
local sdogma      = ship.get("Sirius Dogma")
local sdivinity   = ship.get("Sirius Divinity")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   local pilots = { __doscans = true }
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, sfidelity )
   elseif r < 0.8 then
      scom.addPilot( pilots, sfidelity )
      scom.addPilot( pilots, sfidelity )
   else
      scom.addPilot( pilots, sshaman )
      scom.addPilot( pilots, sfidelity )
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
      scom.addPilot( pilots, spreacher )
      scom.addPilot( pilots, sshaman )
      scom.addPilot( pilots, sfidelity )
   elseif r < 0.8 then
      scom.addPilot( pilots, spreacher )
      scom.addPilot( pilots, spreacher )
   else
      scom.addPilot( pilots, spreacher )
      scom.addPilot( pilots, sshaman )
      scom.addPilot( pilots, sfidelity )
      scom.addPilot( pilots, sfidelity )
   end

   return pilots
end

-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   local pilots = {}
   local r = rnd.rnd()
   -- Generate the capship
   if r < 0.5 then
      scom.addPilot( pilots, sdogma )
   else
      scom.addPilot( pilots, sdivinity )
   end

   -- Generate the escorts
   r = rnd.rnd()
   if r < 0.5 then
      scom.addPilot( pilots, sshaman )
      scom.addPilot( pilots, sfidelity )
      scom.addPilot( pilots, sfidelity )
   else
      scom.addPilot( pilots, spreacher )
      scom.addPilot( pilots, sshaman )
      scom.addPilot( pilots, sfidelity )
   end

   return pilots
end

local fsirius = faction.get("Sirius")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 300
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)
   weights[ spawn_capship ] = math.max(1, -500 + 1.70 * max)

   return scom.init( fsirius, weights, max, {patrol=true} )
end
