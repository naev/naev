

scom = {}


-- @brief Calculates when next spawn should occur
function scom.calcNextSpawn( cur, new, max )
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
function scom.createSpawnTable( weights )
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
function scom.choose( stable )
   local r = rnd.rnd()
   for k,v in ipairs( stable ) do
      if r < v["chance"] then
         return v["func"]()
      end
   end
   error("No spawn function found")
end


-- @brief Actually spawns the pilots
function scom.spawn( pilots )
   local spawned = {}
   local leader = nil
   for k,v in ipairs(pilots) do
      local p
      if type(v["pilot"])=='function' then
         p = v["pilot"]() -- Call function
      elseif not v["pilot"][1] then
         local pos = nil
         if leader ~= nil then
            pos = leader:pos()
         end
         p = pilot.add( v["pilot"], nil, pos )
      else
         p = scom.spawnRaw( v["pilot"][1], v["pilot"][2], v["pilot"][3], v["pilot"][4], v["pilot"][5])
      end
      if #p == 0 then
         error("No pilots added")
      end
      local presence = v["presence"] / #p
      for _,vv in ipairs(p) do
         if pilots.__fleet then
            if leader == nil then
               leader = vv
            else
               vv:setLeader(leader)
            end
         end
         spawned[ #spawned+1 ] = { pilot = vv, presence = presence }
      end
   end
   return spawned
end


-- @brief spawn a pilot with addRaw
function scom.spawnRaw( ship, name, ai, equip, faction)
   local p = {pilot.addRaw( ship, ai, nil, equip )}
   p[1]:rename(name)
   p[1]:setFaction(faction)
   return p
end


-- @brief adds a pilot to the table
function scom.addPilot( pilots, name, presence )
   pilots[ #pilots+1 ] = { pilot = name, presence = presence }
   if pilots[ "__presence" ] then
      pilots[ "__presence" ] = pilots[ "__presence" ] + presence
   else
      pilots[ "__presence" ] = presence
   end
end


-- @brief Gets the presence value of a group of pilots
function scom.presence( pilots )
   if pilots[ "__presence" ] then
      return pilots[ "__presence" ]
   else
      return 0
   end
end


-- @brief Default decrease function
function scom.decrease( cur, max, timer )
   return timer
end


