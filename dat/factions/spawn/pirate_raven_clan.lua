local spir = require "factions.spawn.lib.pirate"

local fravenclan = faction.get("Raven Clan")
-- @brief Creation hook. (May be fine-tuned by reusing only spir.spawn_* and writing a custom create().)
function create ( max )
   return spir.create( fravenclan, max )
end
