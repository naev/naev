local scom = require "factions.spawn.lib.common"

local sdrone = ship.get("Drone")
local sheavy = ship.get("Heavy Drone")

-- @brief Spawns a small swarm.
local function spawn_patrol ()
   local pilots = {}

   scom.addPilot( pilots, sdrone )

   return pilots
end


-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, sdrone )
      scom.addPilot( pilots, sdrone )
   elseif r < 0.8 then
      scom.addPilot( pilots, sheavy )
      scom.addPilot( pilots, sdrone )
      scom.addPilot( pilots, sdrone )
   else
      scom.addPilot( pilots, sheavy )
      scom.addPilot( pilots, sheavy )
      scom.addPilot( pilots, sdrone )
      scom.addPilot( pilots, sdrone )
   end

   return pilots
end


-- @brief Spawns a large swarm.
local function spawn_capship ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      scom.addPilot( pilots, sheavy )
      scom.addPilot( pilots, sdrone )
      scom.addPilot( pilots, sdrone )
      scom.addPilot( pilots, sdrone )
   elseif r < 0.8 then
      scom.addPilot( pilots, sheavy )
      scom.addPilot( pilots, sheavy )
      scom.addPilot( pilots, sdrone )
      scom.addPilot( pilots, sdrone )
      scom.addPilot( pilots, sdrone )
   else
      scom.addPilot( pilots, sheavy )
      scom.addPilot( pilots, sheavy )
      scom.addPilot( pilots, sheavy )
      scom.addPilot( pilots, sdrone )
      scom.addPilot( pilots, sdrone )
      scom.addPilot( pilots, sdrone )
   end

   return pilots
end


local fcollective = faction.get("Collective")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_squad   ] = math.max(1, -100 + 1.00 * max)
   weights[ spawn_capship ] = math.max(1, -100 + 0.50 * max)

   return scom.init( fcollective, weights, max )
end

