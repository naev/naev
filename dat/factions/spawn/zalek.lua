include("dat/factions/spawn/common.lua")
include("dat/factions/spawn/mercenary_helper.lua")


-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
    local pilots = {}
    local r = rnd.rnd()

    if r < pbm then
       pilots = spawnLtMerc("Za'lek")
    elseif r < 0.5 then
       scom.addPilot( pilots, "Za'lek Light Drone", 10 );
       scom.addPilot( pilots, "Za'lek Heavy Drone", 15 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Za'lek Light Drone", 15 );
       scom.addPilot( pilots, "Za'lek Heavy Drone", 15 );
       scom.addPilot( pilots, "Za'lek Heavy Drone", 20 );
    elseif r < 0.9 then
       scom.addPilot( pilots, "Za'lek Light Drone", 10 );
       scom.addPilot( pilots, "Za'lek Heavy Drone", 20 );
       scom.addPilot( pilots, "Za'lek Sting", 45 );
    else
       scom.addPilot( pilots, "Za'lek Demon", 75 );
    end

    return pilots
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
    local pilots = {}
    local r = rnd.rnd()

    if r < pbm then
       pilots = spawnMdMerc("Za'lek")
    elseif r < 0.5 then
       scom.addPilot( pilots, "Za'lek Light Drone", 10 );
       scom.addPilot( pilots, "Za'lek Light Drone", 10 );
       scom.addPilot( pilots, "Za'lek Heavy Drone", 20 );
       scom.addPilot( pilots, "Za'lek Sting", 45 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Za'lek Heavy Drone", 20 );
       scom.addPilot( pilots, "Za'lek Heavy Drone", 20 );
       scom.addPilot( pilots, "Za'lek Sting", 45 );
    else
       scom.addPilot( pilots, "Za'lek Light Drone", 10 );
       scom.addPilot( pilots, "Za'lek Light Drone", 10 );
       scom.addPilot( pilots, "Za'lek Demon", 75 );
    end

    return pilots
end


-- @brief Spawns a capship with escorts.
function spawn_capship ()
    local pilots = {}
    pilots.__fleet = true
    local r = rnd.rnd()

    if rnd.rnd() < pbm then
       pilots = spawnBgMerc("Za'lek")
    else
       -- Generate the capship
       if r < 0.7 then
          scom.addPilot( pilots, "Za'lek Mephisto", 140 )
       else
          scom.addPilot( pilots, "Za'lek Diablo", 150 )
       end

       -- Generate the escorts
       r = rnd.rnd()
       if r < 0.5 then
          scom.addPilot( pilots, "Za'lek Light Drone", 10 );
          scom.addPilot( pilots, "Za'lek Light Drone", 10 );
          scom.addPilot( pilots, "Za'lek Heavy Drone", 20 );
          scom.addPilot( pilots, "Za'lek Heavy Drone", 20 );
          scom.addPilot( pilots, "Za'lek Bomber Drone", 25 );
       elseif r < 0.8 then
          scom.addPilot( pilots, "Za'lek Bomber Drone", 25 );
          scom.addPilot( pilots, "Za'lek Sting", 45 );
       else
          scom.addPilot( pilots, "Za'lek Light Drone", 10 );
          scom.addPilot( pilots, "Za'lek Bomber Drone", 25 );
          scom.addPilot( pilots, "Za'lek Demon", 75 );
       end
    end

    return pilots
end


-- @brief Creation hook.
function create ( max )
    local weights = {}

    -- Create weights for spawn table
    weights[ spawn_patrol  ] = 100
    weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max)
    weights[ spawn_capship ] = math.max(1, -500 + 1.70 * max)
   
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
