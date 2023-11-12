local spir = require "factions.spawn.lib.pirate"

local fblacklotus = faction.get("Black Lotus")
-- @brief Creation hook. (May be fine-tuned by reusing only spir.spawn_* and writing a custom create().)
function create ( max )
   return spir.create( fblacklotus, max, {
      prefer_fleets = true,
   } )
end
