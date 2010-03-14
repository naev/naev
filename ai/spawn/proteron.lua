include("ai/spawn/common.lua")


-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.5 then
       scom.addPilot( pilots, "Proteron Derivative", 15 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
    else
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
    end

    return pilots
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.5 then
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Kahan", 50 );
    else
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Kahan", 50 );
       scom.addPilot( pilots, "Proteron Kahan", 50 );
    end

    return pilots
end


-- @brief Spawns a capship with escorts.
function spawn_capship ()
    local pilots = {}
    local r = rnd.rnd()

    -- Generate the capship
    scom.addPilot( pilots, "Proteron Archimedes", 100 )

    -- Generate the escorts
    r = rnd.rnd()
    if r < 0.5 then
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Kahan", 50 );
    else
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Derivative", 15 );
       scom.addPilot( pilots, "Proteron Kahan", 50 );
       scom.addPilot( pilots, "Proteron Kahan", 50 );
    end

    return pilots
end


-- @brief Creation hook.
function create ( max )
    local weights = {}

    -- Create weights for spawn table
    weights[ spawn_patrol  ] = 100
    weights[ spawn_squad   ] = 0.33*max
    weights[ spawn_capship ] = 100*math.exp( -max / 1000 )
   
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