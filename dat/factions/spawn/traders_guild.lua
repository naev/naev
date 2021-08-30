local scom = require "factions.spawn.lib.common"

local sllama      = ship.get("Llama")
local sgawain     = ship.get("Gawain")
local skoala      = ship.get("Koala")
local squicksilver= ship.get("Quicksilver")
local smule       = ship.get("Mule")
local srhino      = ship.get("Rhino")
--local sshark      = ship.get("Shark")

-- @brief Spawns a small trade fleet.
function spawn_patrol ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, sllama )
   elseif r < 0.8 then
      scom.addPilot( pilots, sllama )
      scom.addPilot( pilots, sllama )
   else
      scom.addPilot( pilots, skoala )
      scom.addPilot( pilots, sllama )
   end

   return pilots
end


-- @brief Spawns a larger trade fleet.
function spawn_squad ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, skoala )
      if rnd.rnd() < 0.6 then
         scom.addPilot( pilots, sllama )
         scom.addPilot( pilots, sllama )
      else
         scom.addPilot( pilots, sgawain )
         scom.addPilot( pilots, sgawain )
      end
   else
      if rnd.rnd() < 0.7 then
         scom.addPilot( pilots, smule )
      else
         scom.addPilot( pilots, srhino )
      end
      if rnd.rnd() < 0.4 then
         scom.addPilot( pilots, sllama )
         scom.addPilot( pilots, sllama )
      elseif rnd.rnd() < 0.7 then
         scom.addPilot( pilots, skoala )
         scom.addPilot( pilots, skoala )
      else
         scom.addPilot( pilots, squicksilver )
         scom.addPilot( pilots, squicksilver )
      end
   end

   return pilots
end


-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)

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
   local pilots = scom.spawn( spawn_data, "Traders Guild" )

   -- Calculate spawn data
   spawn_data = scom.choose( spawn_table )

   return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
