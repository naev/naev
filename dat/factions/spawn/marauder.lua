local spir = require "factions.spawn.lib.pirate"

local fmarauder = faction.get("Marauder")
-- @brief Creation hook. (May be fine-tuned by reusing only spir.spawn_* and writing a custom create().)
function create ( max )
   return spir.create( fmarauder, max, { reweight = function( weights )
      weights[ spir.spawn_loner_strong ] = weights[ spir.spawn_loner_strong ] - 100
      weights[ spir.spawn_squad ]        = weights[ spir.spawn_squad ] - 100
      weights[ spir.spawn_capship ]      = weights[ spir.spawn_capship ] - 200
      return weights
   end } )
end
