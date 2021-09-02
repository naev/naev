local lanes = require 'ai.core.misc.lanes'

local scom = {}

-- @brief Calculates when next spawn should occur
function scom.calcNextSpawn( cur, new, max )
   if cur == 0 then return rnd.rnd(0, 10) end -- Kickstart spawning.

   local stddelay = 10 -- seconds
   local maxdelay = 60 -- seconds. No fleet can ever take more than this to show up.
   local stdfleetsize = 1/4 -- The fraction of "max" that gets the full standard delay. Fleets bigger than this portion of max will have longer delays, fleets smaller, shorter.
   local delayweight = 1 -- A scalar for tweaking the delay differences. A bigger number means bigger differences.
   local percent = (cur + new) / max
   local penaltyweight = 1 -- Further delays fleets that go over the presence limit.
   if percent > 1 then
      penaltyweight = 1 + 10 * (percent - 1)
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
   -- Create spawn table
   local spawn_table = {}
   local max = 0
   for k,v in pairs(weights) do
      max = max + v
      spawn_table[ #spawn_table+1 ] = { chance = max, func = k }
   end

   -- Safety check
   if max == 0 then
      error(_("No weight specified"))
   end

   -- Normalize
   for k,v in ipairs(spawn_table) do
      v.chance = v.chance / max
   end

   -- Job done
   return spawn_table
end


-- @brief Chooses what to spawn
function scom.choose( stable )
   local r = rnd.rnd()
   for k,v in ipairs( stable ) do
      if r < v.chance then
         return v.func()
      end
   end
   error(_("No spawn function found"))
end


-- @brief Actually spawns the pilots
function scom.spawn( pilots, faction )
   local spawned = {}

   -- Case no pilots
   if pilots == nil then
      return nil
   end

   local leader
   local origin
   if pilots.__stealth and naev.isSimulation() then
      -- Try to random sample a good point
      local r = system.cur():radius() * 0.8
      local p = vec2.newP( rnd.rnd() * r, rnd.rnd() * 360 )
      local m = 3000 -- margin
      local L = lanes.get(faction, "non-friendly")
      for i = 1,20 do
         local np = lanes.getNonPoint( L, p, r, m )
         if np and #pilot.getHostiles( faction, m, np ) == 0 then
            origin = np
            break
         end
      end
   end
   if not origin then
      origin = pilot.choosePoint( faction, false, pilots.__stealth ) -- Find a suitable spawn point
   end
   for k,v in ipairs(pilots) do
      local params = v.params or {}
      if not leader then
         if pilots.__formation ~= nil then
            leader:memory().formation = pilots.__formation
         end
      end
      if params.stealth==nil and pilots.__stealth then
         params.stealth= true
      end
      if params.ai==nil and pilots.__ai then
         params.ai = pilots.__ai
      end
      local pfact = params.faction or faction
      local p = pilot.add( v.ship, pfact, origin, params.name, params )
      local mem = p:memory()
      mem.natural = true -- mark that it was spawned naturally and not as part of a mission
      local presence = v.presence
      if not pilots.__nofleet then
         if leader == nil then
            leader = p
         else
            p:setLeader(leader)
         end
      end
      if pilots.__doscans then
         mem.doscans = true
      end
      spawned[ #spawned+1 ] = { pilot=p, presence=presence }
   end
   return spawned
end


-- @brief adds a pilot to the table
function scom.addPilot( pilots, s, params )
   local presence = s:points()
   pilots[ #pilots+1 ] = { ship=s, presence=presence, params=params }
   pilots.__presence = (pilots.__presence or 0) + presence
end


-- @brief Gets the presence value of a group of pilots
function scom.presence( pilots )
   return pilots.__presence or 0
end


-- @brief Default decrease function
function scom.decrease( cur, max, timer )
   return timer
end

return scom
