local spir = require "factions.spawn.lib.pirate"

local fpirate = faction.get("Pirate")
-- @brief Creation hook. (May be fine-tuned by reusing only spir.spawn_* and writing a custom create().)
function create ( max )
   return spir.create( fpirate, max )
end
