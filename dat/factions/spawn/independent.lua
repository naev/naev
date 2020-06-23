require("dat/factions/spawn/common.lua")


-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.5 then
       scom.addPilot( pilots, "Independent Schroedinger", 12 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Independent Schroedinger", 12 );
       scom.addPilot( pilots, "Independent Gawain", 7 );
    else
       scom.addPilot( pilots, "Independent Schroedinger", 12 );
       scom.addPilot( pilots, "Independent Schroedinger", 12 );
       scom.addPilot( pilots, "Independent Gawain", 7 );
    end

    return pilots
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.5 then
       scom.addPilot( pilots, "Independent Schroedinger", 5 );
       scom.addPilot( pilots, "Independent Gawain", 5 );
       scom.addPilot( pilots, "Independent Gawain", 5 );
       scom.addPilot( pilots, "Independent Hyena", 10 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Independent Gawain", 5 );
       scom.addPilot( pilots, "Independent Gawain", 5 );
       scom.addPilot( pilots, "Independent Gawain", 5 );
       scom.addPilot( pilots, "Independent Hyena", 10 );
       scom.addPilot( pilots, "Independent Hyena", 10 );
    else
       scom.addPilot( pilots, "Independent Hyena", 10 );
       scom.addPilot( pilots, "Independent Hyena", 10 );
       scom.addPilot( pilots, "Independent Hyena", 10 );
       scom.addPilot( pilots, "Independent Hyena", 10 );
    end

    return pilots
end


-- @brief Creation hook.
function create ( max )
    local weights = {}

    -- Create weights for spawn table
    weights[ spawn_patrol  ] = 100
    weights[ spawn_squad   ] = math.max(1, -100 + 1.00 * max)
   
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
    pilots = scom.spawn( spawn_data, "Independent" )

    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )

    return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
