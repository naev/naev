--more of a dummy file to help avoid errors. Nasin should not actually spawn.
local scom = require "factions.spawn.lib.common"


-- @brief Dummy spawn function.
function spawn_dummy ()
   return {}
end


-- @brief Dummy creation hook.
function create ( max )
   local weights = {}
   weights[ spawn_dummy ] = 100
   spawn_table = scom.createSpawnTable( weights )
   spawn_data = scom.choose( spawn_table )
   return scom.calcNextSpawn( 0, scom.presence(spawn_data), max )
end


-- @brief Dummy spawning hook (returning a timer).
function spawn ( presence, max )
   return 5
end
