--[[--
   Mission helper utilities.
   @module lmisn
--]]
local fmt = require "format"
local luaspfx = require "luaspfx"
local lmisn = {}

--[[
   Sound Effects
--]]
local audio
local _sfx
local function _sfx_load ()
   audio = require 'love.audio'
   _sfx = {
      bingo = audio.newSoundData( 'snd/sounds/jingles/success' ),
      money = audio.newSoundData( 'snd/sounds/jingles/money' ),
      victory = audio.newSoundData( 'snd/sounds/jingles/victory' ),
      eerie = audio.newSoundData( 'snd/sounds/jingles/eerie' ),
   }
end

--[[--
   Plays a happy victory sound. Should be used for unique mission completion.
--]]
function lmisn.sfxVictory ()
   if not _sfx then _sfx_load() end

   luaspfx.sfx( false, nil, _sfx.victory )
end

--[[--
   Plays a money indicating money made. Should be used when the player receives payment for generic repeatable missions.
--]]
function lmisn.sfxMoney ()
   if not _sfx then _sfx_load() end

   luaspfx.sfx( false, nil, _sfx.money )
end

--[[--
   Plays a jingle indicating success. Meant more for small good things advancing missions rather than completion.
--]]
function lmisn.sfxBingo()
   if not _sfx then _sfx_load() end

   luaspfx.sfx( false, nil, _sfx.bingo )
end

--[[--
   Plays a weird eerie sfx.
--]]
function lmisn.sfxEerie ()
   if not _sfx then _sfx_load() end

   luaspfx.sfx( false, nil, _sfx.eerie )
end

--[[--
   Returns a complete or filtered table of landable spobs (that is, landable, inhabitable, and not restricted)

   @tparam[opt] System sys The system to search, defaulting to the current one.
   @tparam[opt] Faction fct If nil, return all landable spobs in the system meeting the landable criteria, otherwise...
   @tparam[opt=false] boolean fctmatch If true, restrict results to the given faction; if false, restricts it to factions not hostile with fct.
   @treturn table All matching spobs in a list.
--]]
function lmisn.getLandableSpobs( sys, fct, fctmatch )
   sys = sys or system.cur()
   fct = fct and faction.get(fct)
   local pnt_candidates = {}
   for _k,p in ipairs(sys:spobs()) do
      local s = p:services()
      if s.land and s.inhabited and not p:tags().restricted then
         if not fct then
            table.insert( pnt_candidates, p )
         else
            local f = p:faction()
            if (fctmatch and f==fct) or (not fctmatch and f and not f:areEnemies(fct)) then
               table.insert( pnt_candidates, p )
            end
         end
      end
   end
   return pnt_candidates
end


--[[--
   Choose the next system to jump to on the route from system nowsys to system finalsys.

   @tparam System nowsys Start point.
   @tparam System finalsys End point.
   @tparam[opt=false] boolean hidden Whether the path may include hidden systems.
   @treturn System The next system to jump to, defaulting to nowsys if there's no path or nothing to do.
--]]
function lmisn.getNextSystem( nowsys, finalsys, hidden )
   if nowsys == finalsys or finalsys == nil then
      return nowsys
   end

   local path = nowsys:jumpPath( finalsys, hidden )
   if not path then
      return nowsys
   end

   return path[1]:dest()
end

--[[--
   Gets the route between nowsys and finalsys.

   @treturn table A table of systems including the nowsys and finalsys.
--]]
function lmisn.getRoute( nowsys, finalsys, hidden )
   local route = {nowsys}
   if nowsys == finalsys or finalsys == nil then
      return route
   end

   local path = nowsys:jumpPath( finalsys, hidden )
   if not path then
      return route
   end

   for k,v in ipairs(path) do
      route[ k+1 ] = v:dest()
   end
   return route
end

lmisn.sysFilters = {}
--[[--
   Provides the default system filter. Always true regardless of input.

   @treturn function The filter function.
--]]
function lmisn.sysFilters.default ()
   return function( _sys )
      return true
   end
end
--[[--
   Provides the faction system filter. Makes sure each system hasa minimum amount of presence of a certain faction.

   @usage sys = lmisn.getSysAtDistance( system.cur(), 1, 3, lmisn.sysFilters.faction( "Dvaered", 50 ) ) -- Gets all systems within 1 to 3 jumps that have >= 50 Dvaered presence

   @tparam Faction fct Faction to ensure minimum presence.
   @tparam number threshold Minimum amount of presence necessary to return true.
   @treturn function The filter function.
--]]
function lmisn.sysFilters.faction( fct, threshold )
   fct = fct and faction.get(fct)
   threshold = threshold or 0
   local fctname = fct:nameRaw()
   return function( sys )
      local f = sys:presences()[fctname] or 0
      return (f >= threshold)
   end
end
--[[--
   Provides the landable system filter. Makes sure each system has at least on landable, inhabited, non-restricted spob and is landable by a certain faction.

   @tparam Faction fct Faction to make sure can land.
   @tparam[opt=false] boolean samefact Whether or not being non-hostile is sufficient, or we enforce exact match (true).
   @treturn function The filter function.
--]]
function lmisn.sysFilters.factionLandable( fct, samefact )
   fct = fct and faction.get(fct)
   return function( sys )
      for k,p in ipairs(sys:spobs()) do
         local s = p:services()
         if s.land and s.inhabited and not p:tags().restricted then
            local f = p:faction()
            if (samefact and f==fct) or (not samefact and f and not f:areEnemies(fct)) then
               return true
            end
         end
      end
      return false
   end
end

--[[--
   Fetches an array of systems from min to max jumps away from the given
         system sys.

   The following example gets a random Sirius M class spob between 1 to 6 jumps away.

   @usage
   local spobs = {}
   lmisn.getSysAtDistance( system.cur(), 1, 6,
      function(s)
         for i, v in ipairs(s:spobs()) do
            if v:faction() == faction.get("Sirius") and v:class() == "M" then
               return true
            end
         end
         return false
      end )

   if #spobs == 0 then misn.finish(false) end -- In case no suitable spobs are in range.

   local index = rnd.rnd(1, #spobs)
   destspob = spobs[index][1]
   destsys = spobs[index][2]

      @param sys System to calculate distance from or nil to use current system
      @param min Min distance to check for.
      @param max Maximum distance to check for.
      @param filter Optional filter function to use for more details.
      @param data Data to pass to filter
      @param hidden Whether or not to consider hidden jumps (off by default)
      @return The table of systems n jumps away from sys
--]]
function lmisn.getSysAtDistance( sys, min, max, filter, data, hidden )
   -- Get default parameters
   sys = sys or system.cur ()
   max = max or min

   local open  = { sys }
   local close = { [sys:nameRaw()]=sys }
   local dist  = { [sys:nameRaw()]=0 }

   -- Run max times
   for i=1,max do
      local added = 0
      local nopen = {}
      -- Get all the adjacent system of the current set
      for _j,s in ipairs(open) do
         local adjsys = s:adjacentSystems( hidden ) -- Get them all
         for _k,a in ipairs(adjsys) do
            -- Must not have been explored previously
            if not a:tags().restricted and close[ a:nameRaw() ] == nil then
               nopen[ #nopen+1 ] = a
               close[ a:nameRaw() ] = a
               dist[  a:nameRaw() ] = i
               added = added+1
            end
         end
      end
      -- Found all systems
      if added==0 then
         break
      end
      open = nopen -- New table becomes the old
   end

   -- Now we filter the solutions
   local finalset = {}
   for i,s in pairs(close) do
      if dist[i] >= min and dist[i] <= max and
            (filter == nil or filter(s,data)) then
         finalset[ #finalset+1 ] = s
      end
   end
   return finalset
end

--[[--
   Works the same as lmisn.getSysAtDistance, but for spobs.

   Filter is applied on a spob level.

   @usage
   -- Get random spob within 3 to 5 jumps of current system that is landable by independent pilots.
   local candidates = lmisn.getSpobAtDistance( system.cur(), 3, 5, "Independent", false )
   -- Make sure there are candidates
   if #candidates==0 then
      error("There are no spobs meeting the criteria!")
   end
   -- Sort candidates by some criteria
   table.sort( candidates, my_spob_sort_function )
   -- Get best by sorting criteria
   local destpnt = candidates[1]

   @tparam[opt=system.cur()] System sys System to base distance calculations off of.
   @tparam number min Minimum jump distance to get spob at.
   @tparam number max Maximum jump distance to get spob at.
   @tparam[opt=faction.get("Player")] Faction fct What faction to do landing checks with.
   @tparam[opt=false] boolean samefct Whether or not to only allow spobs to belong exactly to fct.
   @tparam[opt=nil] function filter Filtering function that returns a boolean and takes a spob being tested as a parameter.
   @param[opt=nil] data Custom data that will be passed to filter.
   @tparam[opt=false] boolean hidden Whether or not to consider hidden jumps when computing system distance.
   @treturn table A table containing all the spobs matching the criteria. Can be empty if no matches found.
--]]
function lmisn.getSpobAtDistance( sys, min, max, fct, samefct, filter, data, hidden )
   fct = fct or faction.get("Player")
   local pnts = {}
   local candidates = lmisn.getSysAtDistance( sys, min, max, lmisn.sysFilters.factionLandable( fct ), nil, hidden )
   if #candidates == 0 then
      return pnts
   end
   for _k,s in ipairs(candidates) do
      local lp = lmisn.getLandableSpobs( s, fct, samefct )
      for _i,p in ipairs(lp) do
         if not filter or filter( p, data ) then
            table.insert( pnts, p )
         end
      end
   end
   return pnts
end

--[[--
   Gets a random spob at a distance. Only checks for inhabited, landable, and non-restricted spobs.

   @usage destpnt = lmisn.getRandomSpobAtDistance( system.cur(), 3, 5, "Independent", false ) -- Get random spob within 3 to 5 jumps of current system that is landable by independent pilots.

   @tparam[opt=system.cur()] System sys System to base distance calculations off of.
   @tparam number min Minimum jump distance to get spob at.
   @tparam number max Maximum jump distance to get spob at.
   @tparam[opt=nil] Faction fct What faction to do landing checks with.
   @tparam[opt=false] boolean samefct Whether or not to only allow spobs to belong exactly to fct.
   @tparam[opt=nil] function filter Filtering function that returns a boolean and takes a spob being tested as a parameter.
   @param[opt=nil] data Custom data that will be passed to filter.
   @tparam[opt=false] boolean hidden Whether or not to consider hidden jumps when computing system distance.
   @treturn Spob A single spob matching all the criteria.
--]]
function lmisn.getRandomSpobAtDistance( sys, min, max, fct, samefct, filter, data, hidden )
   local candidates = lmisn.getSpobAtDistance( sys, min, max, fct, samefct, filter, data, hidden )
   if #candidates == 0 then
      return nil, nil
   end
   return spob.getS( candidates[ rnd.rnd(1,#candidates) ] )
end

--[[--
Calculates the distance (in pixels) from a position in a system to a position in another system.

   @tparam System origin_sys System to calculate distance from.
   @tparam Vec2 origin_pos Position to calculate distance from.
   @tparam System dest_sys Target system to calculate distance to.
   @tparam Vec2 dest_pos Target position to calculate distance to.
            (might be nil, meaning (any) dest_sys entry point.)
   @tparam table params Table of parameters. Currently supported are "use_hidden".
   @return The distance travelled
   @return The jumpPath leading to the target system
--]]
function lmisn.calculateDistance( origin_sys, origin_pos, dest_sys, dest_pos, params )
   params = params or {}
   local traveldist = 0
   local pos = origin_pos

   local jumps = origin_sys:jumpPath( dest_sys, params.use_hidden )
   if jumps then
      for k, v in ipairs(jumps) do
         -- We're not in the destination system yet.
         -- So, get the next system on the route, and the distance between
         -- our entry point and the jump point to the next system.
         -- Then, set the exit jump point as the next entry point.
         local j, r = jump.get( v:system(), v:dest() )
         traveldist = traveldist + vec2.dist(pos, j:pos())
         pos = r:pos()
      end
   else
      jumps = {}
   end

   -- We ARE in the destination system now, so route from the entry point to the destination planet.
   if dest_pos then
      traveldist = traveldist + vec2.dist( pos, dest_pos )
   end
   return traveldist, jumps
end


local function isSpobOf(sp, sys)
   for _, spob in ipairs( sys:spobs() ) do
      if spob == sp then
         return true
      end
   end
   return false
end

local function angle_with_dest( p, dst_sys, dst_pos )
   local nxt
   if dst_sys == system.cur() then
      nxt = dst_pos
   else
      nxt = system.cur():jumpPath( dst_sys )[1]:pos()
   end
   local v = nxt - p:pos()
   local a = v:angle() - p:dir()
   a = a * 180 / math.pi
   if a <= -180 then
      a = a + 360
   elseif a > 180 then
      a = a - 360
   end
   return math.abs(a)
end

-- jumpin / jumpout animations not taken into account
-- ship assumed to jumpin at jump point with speed_max
-- ship assumed to be originally motionless (and, if landed, facing the opposite direction)
-- ship assumed to have enough space to reach max_speed between "checkpoints"
local const = require 'constants'
-- This does not correspond to the constant, but makes the correct predictions.

--[[--
Calculates the in-game time from a position/spob in a system to a position/spob/nil in another system. Spobs and systems can be given as strings.

   @tparam Pilot p Pilot to compute travel time for.
   @tparam System src_sys System to calculate distance from. If nil, current system.
   @tparam System dst_sys or nil Target system to calculate distance to. If omitted, use src_sys.
   @tparam Vec2|Spob _src_pos Position to calculate distance from. If the argument is a spob, taking off time is accounted for. If nil, current pilot position.
   @tparam Vec2|Spob|nil dst_pos Target position to calculate distance to. If target is nil, just aim system entry point. If target is a spob, braking time is accounted for.
   @treturn Time The in-game time needed for this travel.
--]]
function lmisn.travel_time( p, src_sys, dst_sys, src_pos, dst_pos )
   local pstats = p:stats()
   local delays = 0 -- land/jump times (everything not affected by action_speed)
   local total = 0
   local stops = 0
   local angle = 180

   if src_sys == nil then
      src_sys = system.cur()
   end
   src_sys = system.get(src_sys)
   dst_sys = dst_sys and system.get(dst_sys) or src_sys

   if type(src_pos) == "string" then
      src_pos = spob.get(src_pos)
   end
   if type(dst_pos) == "string" then
      dst_pos = spob.get(dst_pos)
   end

   if src_sys == dst_sys and (src_pos == dst_pos or dst_pos == nil) then
      return time.new(0, 0, 0)
   end

   if src_pos == nil then
      if src_sys ~= system.cur() then
         warn('travel_time: nil src_pos with non-current src sys makes no sense.')
      end
      src_pos = p:pos()
      angle = angle_with_dest( p, dst_sys, dst_pos:pos() )
   end

   if isSpobOf(src_pos, src_sys) then
      delays = delays + p:shipstat("land_delay",true) * const.TIMEDATE_LAND_INCREMENTS
      total = total + const.PILOT_TAKEOFF_DELAY * const.TIMEDATE_INCREMENTS_PER_SECOND
      src_pos = src_pos:pos()
   end
   if isSpobOf(dst_pos, dst_sys) then
      stops = stops + 1
      total = total + const.PILOT_LANDING_DELAY * const.TIMEDATE_INCREMENTS_PER_SECOND
      dst_pos = dst_pos:pos()
   end
   local dist = lmisn.calculateDistance( src_sys, src_pos, dst_sys, dst_pos )
   local jumps = src_sys:jumpDist(dst_sys)
   local warmup = p:shipstat("jump_warmup",true)
   if not p:shipstat("misc_instant_jump") then
      stops = stops + jumps
      total = total + (warmup*const.HYPERSPACE_ENGINE_DELAY + 2) * jumps * const.TIMEDATE_INCREMENTS_PER_SECOND
   end
   total = total + jumps * (warmup*const.HYPERSPACE_FLY_DELAY + 0.3) * const.TIMEDATE_INCREMENTS_PER_SECOND
   delays = delays + pstats.jump_delay*jumps

   -- approximation: we assume the ship instantly get to drift speed when stopping thrust
   local turn_time = 180 / pstats.turn
   local start_time = pstats.speed_max / pstats.accel + angle / pstats.turn
   local stop_time = pstats.speed / pstats.accel + turn_time
   local start_dist = pstats.speed_max * pstats.speed_max / 2 / pstats.accel
   local stop_dist = pstats.speed*pstats.speed/2/pstats.accel + turn_time*pstats.speed

   total = total + (stops * stop_time + start_time) * const.TIMEDATE_INCREMENTS_PER_SECOND
   dist = dist - (stops * stop_dist + start_dist)
   total = total + dist / pstats.speed_max * const.TIMEDATE_INCREMENTS_PER_SECOND
   --print ('delays / other '..tostring(delays)..' / '..tostring(total))
   return time.new(0, 0, delays + total / (p:shipstat("action_speed",true)))
end

--[[--
   Same as lmisn.travel_time, but automatically computes it from the player's current location.

   @tparam System dst_sys or nil Target system to calculate distance to. If omitted, use src_sys.
   @tparam Vec2|Spob|nil dst_pos Target position to calculate distance to. If target is nil, just aim system entry point. If target is a spob, braking time is accounted for.
   @treturn Time The in-game time needed for this travel.
--]]
function lmisn.player_travel_time(dst_sys, dst_pos)
   local where
   if player.isLanded() then
      where = spob.cur()
   end
   return lmisn.travel_time( player.pilot(), nil, dst_sys, where, dst_pos)
end

--[[--
Gets the closest jump to a position.

   @tparam[opt=player.pos()] Vec2 pos Position to get closest jump to.
   @tparam[opt=system.cur()] System sys System to get closest jump from location.
--]]
function lmisn.nearestJump( pos, sys )
   pos = pos or player.pos()
   sys = sys or system.cur()
   local closest_j = nil
   local closest = math.huge
   for k,j in ipairs(sys:jumps(true)) do
      local d = j:pos():dist2(pos)
      if d < closest then
         closest_j = j
         closest = d
      end
   end
   return closest_j
end

--[[--
   Wrapper for player.misnActive that works on a table of missions.

   @usage if anyMissionActive( { "Cargo", "Cargo Rush" } ) then -- at least one Cargo or Cargo Rush is active

   @tparam table names Table of names (strings) of missions to check
   @return true if any of the listed missions are active
--]]
function lmisn.anyMissionActive( names )
   for i, j in ipairs( names ) do
      if player.misnActive( j ) then
         return true
      end
   end
   return false
end

--[[--
   Wrapper for player.msg + misn.finish for when the player fails a mission.

   @tparam string reason Reason the mission failed.
--]]
function lmisn.fail( reason )
   player.msg("#r"..fmt.f(_("MISSION FAILED: {reason}"),{reason=reason}))
   misn.finish(false)
end

local function check_player_tag( tag )
   local pp = player.pilot()
   if pp:ship():tags()[tag] then
      return true
   end
   for k,o in ipairs(pp:outfitsList("all")) do
      if o:tags()[tag] then
         return true
      end
   end
   return false
end

--[[--
Returns whether or not the player is lucky.
--]]
function lmisn.is_lucky()
   return check_player_tag( "lucky" )
end

--[[--
Gets whether or not a ship is a luxury vessel.
--]]
function lmisn.is_luxury()
   return check_player_tag( "luxury" )
end

return lmisn
