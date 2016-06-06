--more of a dummy file to help avoid errors. Nasin should not actually spawn.

scom = {}


-- @brief Calculates when next spawn should occur
scom.calcNextSpawn = function( cur, new, max )
    if cur == 0 then return rnd.rnd(0, 10) end -- Kickstart spawning.
    
    local stddelay = 10 -- seconds
    local maxdelay = 60 -- seconds. No fleet can ever take more than this to show up.
    local stdfleetsize = 1/4 -- The fraction of "max" that gets the full standard delay. Fleets bigger than this portion of max will have longer delays, fleets smaller, shorter.
    local delayweight = 1. -- A scalar for tweaking the delay differences. A bigger number means bigger differences.
    local percent = (cur + new) / max
    local penaltyweight = 1. -- Further delays fleets that go over the presence limit.
    if percent > 1. then
        penaltyweight = 1. + 10. * (percent - 1.)
    end
        
    local fleetratio = (new/max)/stdfleetsize -- This turns into the base delay multiplier for the next fleet.
    
    return math.min(stddelay * fleetratio * delayweight * penaltyweight, maxdelay)
end


--[[
   @brief Creates the spawn table based on a weighted spawn function table.
      @param weights Weighted spawn function table to use to generate the spawn table.
      @return The matching spawn table.
--]]
scom.createSpawnTable = function( weights )
   local spawn_table = {}
   local max = 0

   -- Create spawn table
   for k,v in pairs(weights) do
      max = max + v
      spawn_table[ #spawn_table+1 ] = { chance = max, func = k }
   end

   -- Sanity check
   if max == 0 then
      error("No weight specified")
   end

   -- Normalize
   for k,v in ipairs(spawn_table) do
      v["chance"] = v["chance"] / max
   end

   -- Job done
   return spawn_table
end


-- @brief Chooses what to spawn
scom.choose = function( stable )
   local r = rnd.rnd()
   for k,v in ipairs( stable ) do
      if r < v["chance"] then
         return v["func"]()
      end
   end
   error("No spawn function found")
end


-- @brief Actually spawns the pilots
scom.spawn = function( pilots )
   local spawned = {}
   for k,v in ipairs(pilots) do
      local p = pilot.add( v["pilot"] )
      if #p == 0 then
         error("No pilots added")
      end
      local presence = v["presence"] / #p
      for _,vv in ipairs(p) do
         spawned[ #spawned+1 ] = { pilot = vv, presence = presence }
      end
   end
   return spawned
end


-- @brief adds a pilot to the table
scom.addPilot = function( pilots, name, presence )
   pilots[ #pilots+1 ] = { pilot = name, presence = presence }
   if pilots[ "__presence" ] then
      pilots[ "__presence" ] = pilots[ "__presence" ] + presence
   else
      pilots[ "__presence" ] = presence
   end
end


-- @brief Gets the presence value of a group of pilots
scom.presence = function( pilots )
   if pilots[ "__presence" ] then
      return pilots[ "__presence" ]
   else
      return 0
   end
end


-- @brief Default decrease function
scom.decrease = function( cur, max, timer )
   return timer
end


