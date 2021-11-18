local spir = require "factions.spawn.lib.pirate"

local fwildones = faction.get("Wild Ones")
-- @brief Creation hook. (May be fine-tuned by reusing only spir.spawn_* and writing a custom create().)
function create ( max )
   return spir.create( fwildones, max )
end
