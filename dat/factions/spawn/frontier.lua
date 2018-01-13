include("dat/factions/spawn/common.lua")
include("dat/factions/spawn/mercenary_helper.lua")


-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
    local pilots = {}
    local r = rnd.rnd()

    if r < pbm then
       pilots = spawnLtMerc("Frontier")
    elseif r < 0.5 then
       scom.addPilot( pilots, "Frontier Lancelot", 30 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Frontier Hyena", 20 );
       scom.addPilot( pilots, "Frontier Lancelot", 30 );
    else
       scom.addPilot( pilots, "Frontier Hyena", 20 );
       scom.addPilot( pilots, "Frontier Ancestor", 25 );
    end

    return pilots
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
    local pilots = {}
    local r = rnd.rnd()

    if r < pbm then
        pilots = spawnMdMerc("Frontier")
    elseif r < 0.5 then
       scom.addPilot( pilots, "Frontier Lancelot", 30 );
       scom.addPilot( pilots, "Frontier Phalanx", 55 );
    else
       scom.addPilot( pilots, "Frontier Lancelot", 30 );
       scom.addPilot( pilots, "Frontier Lancelot", 30 );
       scom.addPilot( pilots, "Frontier Ancestor", 25 );
    end

    return pilots
end


-- @brief Creation hook.
function create ( max )
    local weights = {}

    -- Create weights for spawn table
    weights[ spawn_patrol  ] = 100
    weights[ spawn_squad   ] = 0.33*max
   
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
    pilots = scom.spawn( spawn_data, "Frontier" )

    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )

    return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
