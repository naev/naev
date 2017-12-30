include("dat/factions/spawn/common.lua")


-- @brief Spawns a capship with escorts.
function spawn_capship ()
    local pilots = {}
    pilots.__fleet = true
    local r = rnd.rnd()

    -- Generate the capship
    scom.addPilot( pilots, "Goddard Goddard", 145 )

    -- Generate the escorts
    r = rnd.rnd()
    if r < 0.5 then
       scom.addPilot( pilots, "Goddard Lancelot", 25 );
       scom.addPilot( pilots, "Goddard Lancelot", 25 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Goddard Lancelot", 25 );
       scom.addPilot( pilots, "Goddard Lancelot", 25 );
       scom.addPilot( pilots, "Goddard Lancelot", 25 );
    else
       scom.addPilot( pilots, "Goddard Lancelot", 25 );
       scom.addPilot( pilots, "Goddard Lancelot", 25 );
       scom.addPilot( pilots, "Goddard Lancelot", 25 );
       scom.addPilot( pilots, "Goddard Lancelot", 25 );
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
    local pilots

    -- Over limit
    if presence > max then
       return 5
    end
  
    -- Actually spawn the pilots
    pilots = scom.spawn( spawn_data )

    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )

    return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
