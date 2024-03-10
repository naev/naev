local lanes = require 'ai.core.misc.lanes'

local scom = {}

local function _normalize_presence( max )
   local r = system.cur():radius()
   --[[
   -- Strictly speaking this should be quadratic, but I find it's a bit too strong of an effect
   -- No need for math.pi as it cancels out
   local norm = math.pow( 15e3, 2 )
   local area = math.pow( r, 2 )
   return max * area / norm
   --]]
   return max * math.min( 1, r / 15e3 )
end

--[[
   @brief Initializes the common spawn scheduler framework.
      @tparam Faction fct Faction to initialize for.
      @tparam table weights Weights to use.
      @tparam number max Maximum presence allowed for the current system.
      @tparam table params Custom parameters.
--]]
function scom.init( fct, weights, max, params )
   params = params or {}
   scom._faction     = faction.get(fct)
   scom._weight_table= scom.createSpawnTable( weights )
   scom._max         = _normalize_presence( max )
   scom._spawn_data  = nil
   scom._params      = params
   if not params.nospawnfunc then
      spawn = scom.spawn_handler -- Global!
   end

   scom.choose()
   return scom.calcNextSpawn( 0 )
end
function scom.spawn_handler( presence, max )
   scom._max = _normalize_presence( max ) -- just in case it changed
   -- Over limit so do a short delay
   if presence > max then
      return 5
   end
   -- Spawn chosen pilots
   local pilots = scom.spawn()
   -- Choose next spawn and time to spawn
   scom.choose()
   return scom.calcNextSpawn( presence ), pilots
end

-- @brief Calculates when next spawn should occur
function scom.calcNextSpawn( cur )
   local new = scom.presence( scom._spawn_data )
   local max = scom._max

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

   local fleetratio = (new/max) / stdfleetsize -- This turns into the base delay multiplier for the next fleet.

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
      if v > 0 then
         max = max + v
         table.insert( spawn_table, { w = v, func = k } )
      end
   end

   -- Safety check
   if max == 0 then
      error(_("No weight specified"))
   end
   spawn_table._maxw = max

   -- Sort so it's a wee bit faster
   table.sort( spawn_table, function( a, b )
      return a.w > b.w
   end )

   -- Job done
   return spawn_table
end

-- @brief Chooses what to spawn
function scom.choose ()
   local r = rnd.rnd() * scom._weight_table._maxw
   local m = 0
   for _k,v in ipairs( scom._weight_table ) do
      m = m + v.w
      if r < m then
         scom._spawn_data = v.func()
         return true
      end
   end
   error(_("No spawn function found"))
end

-- @brief Actually spawns the pilots
function scom.spawn( pilots )
   pilots = pilots or scom._spawn_data
   local fct = scom._faction
   local spawned = {}
   local issim = naev.isSimulation()

   -- Case no pilots
   if pilots == nil then
      return nil
   end

   -- Useful later
   local _nebu_dens, nebu_vol = system.cur():nebula()

   -- When simulating we try to find good points
   local leader
   local origin
   if issim then
      -- Stealth should avoid enemies nearby
      if pilots.__stealth then
         local r = system.cur():radius() * 0.8
         local p = vec2.newP( rnd.rnd() * r, rnd.angle() )
         local m = 3000 -- margin
         local L = lanes.get(fct, "non-friendly")
         for i = 1,20 do -- Just brute force sampling
            local np = lanes.getNonPoint( L, p, r, m )
            if np and #pilot.getEnemies( fct, m, np ) == 0 then
               origin = np
               break
            end
         end
      -- Spawn near patrol points in the system
      elseif scom._params.patrol then
         local L = lanes.get(fct, "friendly")
         origin = lanes.getPoint( L )
      end
   end
   if not origin then
      origin = pilot.choosePoint( fct, false, pilots.__stealth ) -- Find a suitable spawn point
   end
   for _k,v in ipairs(pilots) do
      local params = v.params or {}
      if params.stealth==nil and pilots.__stealth then
         params.stealth = true
      end
      if params.ai==nil and pilots.__ai then
         params.ai = pilots.__ai
      end
      local pfact = params.faction or fct
      local p = pilot.add( v.ship, pfact, origin, params.name, params )
      local mem = p:memory()
      mem.natural = true -- mark that it was spawned naturally and not as part of a mission
      local presence = v.presence
      if not pilots.__nofleet then
         if leader == nil then
            leader = p
            if pilots.__formation ~= nil then
               mem.formation = pilots.__formation
            end
         else
            p:setLeader(leader)
            if #pilots > 1 then
               mem.autoleader = true
            end
         end
      end
      if pilots.__doscans then
         mem.doscans = true
      end
      if not pfact:known() then
         p:rename(_("Unknown"))
      end
      if params.postprocess then
         params.postprocess( p )
      end
      -- Make sure they survive the nebula
      if nebu_vol > 0 then
         local dmg = nebu_vol * (1-p:shipstat("nebu_absorb",true))
         if p:stats().shield_regen <= dmg then
            if leader==p then
               leader = nil
            end
            p:rm()
            p = nil
         end
      end

      -- Add pilot as spawned
      if p ~= nil then
         table.insert( spawned, { pilot=p, presence=presence } )
      end
   end

   return spawned
end

-- @brief Probabilistically adds a table of pilots
function scom.doTable( pilots, tbl )
   local r = rnd.rnd()
   local n = #tbl
   for k,t in ipairs(tbl) do
      if not t.w and k<n then
         warn(_("Spawn table is missing 'w' fields!"))
      end
      if not t.w or t.w <= r then
         for i,p in ipairs(t) do
            scom.addPilot( pilots, p )
         end
         break
      end
   end
   return pilots
end

-- @brief adds a pilot to the table
function scom.addPilot( pilots, s, params )
   local presence = s:points()
   table.insert(pilots, { ship=s, presence=presence, params=params })
   pilots.__presence = (pilots.__presence or 0) + presence
end

-- @brief Gets the presence value of a group of pilots
function scom.presence( pilots )
   return (pilots and pilots.__presence) or 0
end

return scom
