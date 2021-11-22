local scom = require "factions.spawn.lib.common"

local slancelot   = ship.get("Empire Lancelot")
local sshark      = ship.get("Empire Shark")
local sadmonisher = ship.get("Empire Admonisher")
local spacifier   = ship.get("Empire Pacifier")
local shawking    = ship.get("Empire Hawking")
local speacemaker = ship.get("Empire Peacemaker")
local srainmaker  = ship.get("Empire Rainmaker")

-- @brief Spawns a small patrol fleet.
local function spawn_patrol ()
   local pilots = { __doscans = true }
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, slancelot )
   elseif r < 0.8 then
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, sshark )
   else
      scom.addPilot( pilots, spacifier )
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
      scom.addPilot( pilots, sadmonisher )
      scom.addPilot( pilots, slancelot )
   elseif r < 0.8 then
      scom.addPilot( pilots, sadmonisher )
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, sshark )
   else
      scom.addPilot( pilots, spacifier )
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, sshark )
   end

   return pilots
end


-- @brief Spawns a capship with escorts.
local function spawn_capship ()
   local pilots = {}
   local r = rnd.rnd()

   -- Generate the capship
   if r < 0.1 then
      scom.addPilot( pilots, srainmaker )
   elseif r < 0.7 then
      scom.addPilot( pilots, shawking )
   else
      scom.addPilot( pilots, speacemaker )
   end

   -- Generate the escorts
   r = rnd.rnd()
   if r < 0.5 then
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, slancelot )
      scom.addPilot( pilots, sshark )
   elseif r < 0.8 then
      scom.addPilot( pilots, sadmonisher )
      scom.addPilot( pilots, slancelot )
   else
      scom.addPilot( pilots, spacifier )
      scom.addPilot( pilots, slancelot )
   end

   return pilots
end


local fempire = faction.get("Empire")
-- @brief Creation hook.
function create( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 300
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)
   weights[ spawn_capship ] = math.max(1, -500 + 1.70 * max)

   -- Initialize spawn stuff
   return scom.init( fempire, weights, max, {patrol=true} )
end
