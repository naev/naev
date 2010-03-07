

-- @brief Calculates when next spawn should occur
function calcNextSpawn( presence )
   return 5
end


--[[
   @brief Creates the spawn table based on a weighted spawn function table.
      @param weights Weighted spawn function table to use to generate the spawn table.
      @return The matching spawn table.
--]]
function createSpawnTable( weights )
   local spawn_table = {}
   local max = 0

   -- Create spawn table
   for k,v in pairs(weights) do
      max = max + v
      spawn_table[ #spawn_table+1 ] = { max, k }
   end

   -- Normalize
   for k,v in ipairs(spawn_table) do
      v[1] = v[1] / max
   end

   -- Job done
   return spawn_table
end


-- @brief Chooses what to spawn
function spawn_choose( spawn )
   local r = rnd.rnd()
   for k,v in ipairs(spawn) do
      if v[1] < r then
         return v[2]()
      end
   end
end


-- @brief Actually spawns the pilots
function spawn_spawn( pilots )
   local spawned = {}
   for k,v in ipairs(pilots) do
      local p = pilot.add( v )
      for _,vv in ipairs(p) do
         spawned[ #spawned+1 ] = vv
      end
   end
   return spawned
end


-- @brief adds a pilot to the table
function spawn_addPilot( pilots, name )
   pilots[ #pilots+1 ] = name
end


-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
   local pilots = {}
   local r = rnd.rnd()
   local p = 0

   if r < 0.5 then
      spawn_addPilot( pilots, "Empire Lancelot" );
      p = 15
   elseif r < 0.8 then
      spawn_addPilot( pilots, "Empire Lancelot" );
      spawn_addPilot( pilots, "Empire Lancelot" );
      p = 30
   else
      spawn_addPilot( pilots, "Empire Pacifier" );
      p = 45
   end

   return pilots, p
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
   local pilots = {}
   local r = rnd.rnd()
   local p = 0

   if r < 0.5 then
      spawn_addPilot( pilots, "Empire Lancelot" );
      spawn_addPilot( pilots, "Empire Lancelot" );
      spawn_addPilot( pilots, "Empire Admonisher" );
      p = 70
   elseif r < 0.8 then
      spawn_addPilot( pilots, "Empire Admonisher" );
      spawn_addPilot( pilots, "Empire Admonisher" );
      p = 80
   else
      spawn_addPilot( pilots, "Empire Lancelot" );
      spawn_addPilot( pilots, "Empire Lancelot" );
      spawn_addPilot( pilots, "Empire Pacifier" );
      p = 100
   end

   return pilots, p
end


-- @brief Spawns a capship with escorts.
function spawn_capship ()
   local pilots = {}
   local r = rnd.rnd()
   local p = 0
   local capship

   -- Generate the capship
   if r < 0.7 then
      spawn_addPilots( pilots, "Empire Hawking" )
      p = 100
   else
      spawn_addPilots( pilots, "Empire Peacemaker" )
      p = 150
   end

   -- Generate the escorts
   r = rnd.rnd()
   if r < 0.5 then
      spawn_addPilot( pilots, "Empire Lancelot" );
      spawn_addPilot( pilots, "Empire Lancelot" );
      spawn_addPilot( pilots, "Empire Lancelot" );
      p = p + 60
   elseif r < 0.8 then
      spawn_addPilot( pilots, "Empire Lancelot" );
      spawn_addPilot( pilots, "Empire Admonisher" );
      p = p + 60
   else
      spawn_addPilot( pilots, "Empire Lancelot" );
      spawn_addPilot( pilots, "Empire Pacifier" );
      p = p + 70
   end

   return pilots, p
end


-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_squad   ] = 0.33*max
   weights[ spawn_capship ] = 100*math.exp( -presence / 1000 )
   
   -- Create spawn table base on weights
   spawn_table = createSpawnTable( weights )

   -- Calculate spawn data
   spawn_data, spawn_presence = spawn_choose( spawn_table )

   return 0, 0
end


-- @brief Spawning hook
function spawn ( presence, max )
   local pilots
  
   -- Actually spawn the pilots
   pilots = spawn_spawn( spawn_data )

   return spawn_presence, calcNextSpawn( spawn_presence, max ), pilots
end


