local spir = require "factions.spawn.lib.pirate"

local fmarauder = faction.get("Marauder")
-- @brief Creation hook. (May be fine-tuned by reusing only spir.spawn_* and writing a custom create().)
function create ( max )
   return spir.create( fmarauder, max, { reweight = function( weights )
      weights[ spir.weights_loner_strong ] = weights[ spir.weights_loner_strong ] - 100
      weights[ spir.weights_squad ] = weights[ spir.weights_squad ] - 100
      weights[ spir.weights_capship ] = weights[ spir.weights_capship ] - 200
   end } )
end
