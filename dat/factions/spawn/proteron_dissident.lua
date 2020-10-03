require("factions/spawn/common.lua")


-- @brief Spawns a single small ship.
function spawn_patrol ()
    local pilots = {}
    local civships = {{"Proteron Dissident Schroedinger", 8},
                      {"Proteron Dissident Llama", 8},
                      {"Proteron Dissident Gawain", 8},
                      {"Proteron Dissident Hyena", 13}
                     }
    
    local select = rnd.rnd(1, #civships)
    
    scom.addPilot( pilots, civships[select][1], civships[select][2] );

    return pilots
end

-- @brief Creation hook.
function create ( max )
    local weights = {}
    
    -- Create weights for spawn table
    weights[ spawn_patrol  ] = 100
    
    -- Create spawn table base on weights
    spawn_table = scom.createSpawnTable( weights )
    
    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )
    
    return scom.calcNextSpawn( 0, scom.presence(spawn_data), max )
end


-- @brief Spawning hook
function spawn ( presence, max )
    local pilots
    
    -- Over limit
    if presence > max then
        return 5
    end
    
    -- Actually spawn the pilots
    pilots = scom.spawn( spawn_data, "Proteron Dissident" )
    
    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )
    
    return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end


