local scom = require "factions.spawn.lib.common"


-- @brief Spawns a capship with escorts.
function spawn_capship ()
    local pilots = {}

    -- Generate the capship
    scom.addPilot( pilots, "Goddard", 145 )

    -- Generate the escorts
    local r = rnd.rnd()
    if r < 0.5 then
       scom.addPilot( pilots, "Lancelot", 25 )
       scom.addPilot( pilots, "Lancelot", 25 )
    elseif r < 0.8 then
       scom.addPilot( pilots, "Lancelot", 25 )
       scom.addPilot( pilots, "Lancelot", 25 )
       scom.addPilot( pilots, "Lancelot", 25 )
    else
       scom.addPilot( pilots, "Lancelot", 25 )
       scom.addPilot( pilots, "Lancelot", 25 )
       scom.addPilot( pilots, "Lancelot", 25 )
       scom.addPilot( pilots, "Lancelot", 25 )
    end

    return pilots
end


-- @brief Creation hook.
function create ( max )
    local weights = {}

    -- Create weights for spawn table
    weights[ spawn_capship ] = 100

    -- Create spawn table base on weights
    spawn_table = scom.createSpawnTable( weights )

    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )

    return scom.calcNextSpawn( 0, scom.presence(spawn_data), max )
end


-- @brief Spawning hook
function spawn ( presence, max )
    -- Over limit
    if presence > max then
       return 5
    end

    -- Actually spawn the pilots
    local pilots = scom.spawn( spawn_data, "Goddard" )

    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )

    return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
