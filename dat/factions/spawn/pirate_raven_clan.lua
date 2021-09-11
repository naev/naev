local scom = require "factions.spawn.lib.common"
require "factions.spawn.pirate"

local fravenclan = faction.get("Raven Clan")
-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_loner   ] = 100
   weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)
   weights[ spawn_capship ] = math.max(1, -500 + 1.70 * max)

   return scom.init( fravenclan, weights, max )
end
