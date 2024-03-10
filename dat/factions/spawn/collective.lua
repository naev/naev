local scom = require "factions.spawn.lib.common"

local sdrone = ship.get("Drone")
local sheavy = ship.get("Heavy Drone")

-- @brief Spawns a small swarm.
local function spawn_patrol ()
   return scom.doTable( {}, {
      { sdrone },
   } )
end

-- @brief Spawns a medium sized squadron.
local function spawn_squad ()
   return scom.doTable( {}, {
      { w=0.5, sdrone, sdrone },
      { w=0.8, sheavy, sdrone, sdrone },
      { sheavy, sheavy, sdrone, sdrone },
   } )
end

-- @brief Spawns a large swarm.
local function spawn_capship ()
   -- TODO use mothership when redoing collective stuff
   return scom.doTable( {}, {
      { w=0.5, sheavy, sdrone, sdrone, sdrone },
      { w=0.8, sheavy, sheavy, sdrone, sdrone, sdrone },
      { sheavy, sheavy, sheavy, sdrone, sdrone, sdrone  },
   } )
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
