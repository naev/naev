require("factions/spawn/common.lua")


-- @brief Spawns a single small ship.
function spawn_patrol ()
    local pilots = {}

    -- Compute the hostile presence
    local host = 0
    for k,fact in pairs(faction.get("Civilian"):enemies()) do 
       host = host + system.cur():presence(fact)
    end
    host = host / system.cur():presence(faction.get("Civilian"))

    -- The more hostiles, the less advertisers
    local prop = .25 -- Advertisers proportion at host = 0
    local h0   = 3  -- At this hostile presence, advertiser prop is 5% of original proportion
    local r = rnd.rnd() + prop*(1-math.exp(-3*host/h0))

    if r < prop then
       local civships = {{"Advertiser Schroedinger", 8},
                         {"Advertiser Llama", 8},
                         {"Advertiser Gawain", 8},
                         {"Advertiser Hyena", 13}
                        }
       local select = rnd.rnd(1, #civships)
       scom.addPilot( pilots, civships[select][1], civships[select][2] );
    else
       local adships = {{"Civilian Schroedinger", 8},
                         {"Civilian Llama", 8},
                         {"Civilian Gawain", 8},
                         {"Civilian Hyena", 13}
                        }
       local select = rnd.rnd(1, #adships)
       scom.addPilot( pilots, adships[select][1], adships[select][2] );
    end
    


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
    pilots = scom.spawn( spawn_data, "Civilian" )
    
    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )
    
    return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end


