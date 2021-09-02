local scom = require "factions.spawn.lib.common"
require "factions.spawn.pirate"

local fpir = faction.get("Wild Ones")
-- @brief Spawning hook
function spawn ( presence, max )
   -- Over limit
   if presence > max then
      return 5
   end

   -- Actually spawn the pilots
   local pilots = scom.spawn( spawn_data, fpir )

   -- Calculate spawn data
   spawn_data = scom.choose( spawn_table )

   return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
