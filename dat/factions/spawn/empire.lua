include("dat/factions/spawn/common.lua")
include("dat/factions/spawn/mercenary_helper.lua")

include("pilot/empire.lua") -- Uniques
function empire_unique()
   local p = empire_create( true )
   return {p}
end

-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
   local pilots = {}
   local r = rnd.rnd()

   if r < pbm then
      pilots = spawnLtMerc("Empire")
   elseif r < 0.5 then
      scom.addPilot( pilots, "Empire Lancelot", 25 );
   elseif r < 0.8 then
      scom.addPilot( pilots, "Empire Shark", 20 );
      scom.addPilot( pilots, "Empire Lancelot", 25 );
   else
      scom.addPilot( pilots, "Empire Pacifier", 75 );
   end

   return pilots
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
   local pilots = {}
   local r = rnd.rnd()

   if r < pbm then
      pilots = spawnMdMerc("Empire")
   elseif r < 0.5 then
      scom.addPilot( pilots, "Empire Shark", 20 );
      scom.addPilot( pilots, "Empire Lancelot", 25 );
      scom.addPilot( pilots, "Empire Admonisher", 45 );
   elseif r < 0.8 then
      scom.addPilot( pilots, "Empire Lancelot", 25 );
      scom.addPilot( pilots, "Empire Admonisher", 45 );
   else
      scom.addPilot( pilots, "Empire Shark", 20 );
      scom.addPilot( pilots, "Empire Lancelot", 25 );
      scom.addPilot( pilots, "Empire Pacifier", 75 );
   end

   return pilots
end


-- @brief Spawns a capship with escorts.
function spawn_capship ()
   local pilots = {}
   pilots.__fleet = true

   if rnd.rnd() < pbm then
      pilots = spawnBgMerc("Empire")
   else
      local r = rnd.rnd()

      -- Generate the capship
      if r < 0.7 then
         scom.addPilot( pilots, "Empire Hawking", 140 )
      elseif r < 0.97 then
         scom.addPilot( pilots, "Empire Peacemaker", 165 )
      else
         scom.addPilot( pilots, empire_unique, 200 )
      end

      -- Generate the escorts
      r = rnd.rnd()
      if r < 0.5 then
         scom.addPilot( pilots, "Empire Shark", 20 );
         scom.addPilot( pilots, "Empire Lancelot", 25 );
         scom.addPilot( pilots, "Empire Lancelot", 25 );
      elseif r < 0.8 then
         scom.addPilot( pilots, "Empire Lancelot", 25 );
         scom.addPilot( pilots, "Empire Admonisher", 45 );
      else
         scom.addPilot( pilots, "Empire Lancelot", 25 );
         scom.addPilot( pilots, "Empire Pacifier", 75 );
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


