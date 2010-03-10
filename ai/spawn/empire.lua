

-- @brief Calculates when next spawn should occur
function calcNextSpawn( presence, max )
   return 5.
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
      local p = pilot.add( v["pilot"] )
      local presence = v["presence"] / #p
      for _,vv in ipairs(p) do
         spawned[ #spawned+1 ] = { pilot = vv, presence = presence }
      end
   end
   return spawned
end


-- @brief adds a pilot to the table
function spawn_addPilot( pilots, name, presence )
   pilots[ #pilots+1 ] = { pilot = name, presence = presence }
   if pilots[ "__presence" ] then
      pilots[ "__presence" ] = pilots[ "__presence" ] + presence
   else
      pilots[ "__presence" ] = presence
   end
end


-- @brief Gets the presence value of a group of pilots
function spawn_presence( pilots )
   if pilots[ "__presence" ] then
      return pilots[ "__presence" ]
   else
      return 0
   end
end


-- @brief Spawns a small patrol fleet.
function spawn_patrol ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      spawn_addPilot( pilots, "Empire Lancelot", 15 );
   elseif r < 0.8 then
      spawn_addPilot( pilots, "Empire Lancelot", 15 );
      spawn_addPilot( pilots, "Empire Lancelot", 15 );
   else
      spawn_addPilot( pilots, "Empire Pacifier", 45 );
   end

   return pilots
end


-- @brief Spawns a medium sized squadron.
function spawn_squad ()
   local pilots = {}
   local r = rnd.rnd()

   if r < 0.5 then
      spawn_addPilot( pilots, "Empire Lancelot", 20 );
      spawn_addPilot( pilots, "Empire Lancelot", 20 );
      spawn_addPilot( pilots, "Empire Admonisher", 30 );
   elseif r < 0.8 then
      spawn_addPilot( pilots, "Empire Admonisher", 40 );
      spawn_addPilot( pilots, "Empire Admonisher", 40 );
   else
      spawn_addPilot( pilots, "Empire Lancelot", 20 );
      spawn_addPilot( pilots, "Empire Lancelot", 20 );
      spawn_addPilot( pilots, "Empire Pacifier", 60 );
   end

   return pilots
end


-- @brief Spawns a capship with escorts.
function spawn_capship ()
   local pilots = {}
   local r = rnd.rnd()
   local capship

   -- Generate the capship
   if r < 0.7 then
      spawn_addPilots( pilots, "Empire Hawking", 100 )
   else
      spawn_addPilots( pilots, "Empire Peacemaker", 150 )
   end

   -- Generate the escorts
   r = rnd.rnd()
   if r < 0.5 then
      spawn_addPilot( pilots, "Empire Lancelot", 20 );
      spawn_addPilot( pilots, "Empire Lancelot", 20 );
      spawn_addPilot( pilots, "Empire Lancelot", 20 );
   elseif r < 0.8 then
      spawn_addPilot( pilots, "Empire Lancelot", 20 );
      spawn_addPilot( pilots, "Empire Admonisher", 40 );
   else
      spawn_addPilot( pilots, "Empire Lancelot", 20 );
      spawn_addPilot( pilots, "Empire Pacifier", 50 );
   end

   return pilots, p
end


-- @brief Creation hook.
function create ( max )
   local weights = {}

   -- Create weights for spawn table
   weights[ spawn_patrol  ] = 100
   weights[ spawn_squad   ] = 0.33*max
   weights[ spawn_capship ] = 100*math.exp( -max / 1000 )
   
   -- Create spawn table base on weights
   spawn_table = createSpawnTable( weights )

   -- Calculate spawn data
   spawn_data = spawn_choose( spawn_table )

   return 0
end


-- @brief Spawning hook
function spawn ( presence, max )
   local pilots
  
   -- Actually spawn the pilots
   pilots = spawn_spawn( spawn_data )

   return calcNextSpawn( spawn_presence(pilots), max ), pilots
end


