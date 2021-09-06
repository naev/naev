local scom = require "factions.spawn.lib.common"
local formation = require "scripts.formation"

local slancelot   = ship.get("Empire Lancelot")
local sshark      = ship.get("Empire Shark")
local sadmonisher = ship.get("Empire Admonisher")
local spacifier   = ship.get("Empire Pacifier")
local shawking    = ship.get("Empire Hawking")
local speacemaker = ship.get("Empire Peacemaker")

-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
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
function spawn_squad ()
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
function spawn_capship ()
   local pilots = {}
   local r = rnd.rnd()

   -- Generate the capship
   if r < 0.7 then
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


-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 300
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)
   weights[ spawn_capship ] = math.max(1, -500 + 1.70 * max)

   -- Create spawn table base on weights
   spawn_table = scom.createSpawnTable( weights )

   -- Calculate spawn data
   spawn_data = scom.choose( spawn_table )

   return scom.calcNextSpawn( 0, scom.presence(spawn_data), max )
end


-- @brief Spawning hook
function spawn ( presence, max )
   -- Over limit
   if presence > max then
      return 5
   end

   -- Actually spawn the pilots
   local pilots = scom.spawn( spawn_data, "Empire" )

   -- Calculate spawn data
   spawn_data = scom.choose( spawn_table )

   return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end


