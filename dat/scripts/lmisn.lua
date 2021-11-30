--[[--
   Mission helper utilities.
   @module lmisn
--]]
local lmisn = {}

--[[
   Sound Effects
--]]

local audio
local _sfx
local function _sfx_load ()
   audio = require 'love.audio'
   _sfx = {
      money = audio.newSource( 'snd/sounds/jingles/money.ogg' ),
      victory = audio.newSource( 'snd/sounds/jingles/victory.ogg' ),
   }
end

--[[--
   Plays a happy sound.
--]]
function lmisn.sfxVictory ()
   if not _sfx then _sfx_load() end

   local sfx = _sfx.victory:clone()
   sfx:play()
end

--[[--
   Plays another happy sound.
--]]
function lmisn.sfxMoney ()
   if not _sfx then _sfx_load() end

   local sfx = _sfx.money:clone()
   sfx:play()
end


--[[--
   Returns a complete or filtered table of landable planets.

   @tparam[opt] System sys The system to search, defaulting to the current one.
   @tparam[opt] Faction fct If nil, return all landable planets in the system, otherwise...
   @tparam[opt=false] boolean fctmatch If true, restrict results to the given faction; if false, restrict to its enemies.
   @treturn table All matching planets in a list.
--]]
function lmisn.getLandablePlanets( sys, fct, fctmatch )
   sys = sys or system.cur()
   if fct ~= nil then
      fct = faction.get(fct)
   end
   local pnt_candidates = {}
   for _k,p in ipairs(sys:planets()) do
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
   @tparam boolean hidden Whether the path may include hidden systems.
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

lmisn.sysFilters = {}
function lmisn.sysFilters.default ()
   return function( _sys )
      return true
   end
end
function lmisn.sysFilters.faction( fct, threshold )
   if fct ~= nil then
      fct = faction.get(fct)
   end
   threshold = threshold or 0
   local fctname = fct:nameRaw()
   return function( sys )
      local f = sys:presences()[fctname] or 0
      return (f > threshold)
   end
end
function lmisn.sysFilters.factionLandable( fct, samefact )
   if fct ~= nil then
      fct = faction.get(fct)
   end
   return function( sys )
      for k,p in ipairs(sys:planets()) do
         local s = p:services()
         if s.land and s.inhabited then
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

   The following example gets a random Sirius M class planet between 1 to 6 jumps away.

   @usage
   local planets = {}
   lmisn.getSysAtDistance( system.cur(), 1, 6,
       function(s)
           for i, v in ipairs(s:planets()) do
               if v:faction() == faction.get("Sirius") and v:class() == "M" then
                   return true
               end
           end
           return false
       end )

   if #planets == 0 then abort() end -- In case no suitable planets are in range.

   local index = rnd.rnd(1, #planets)
   destplanet = planets[index][1]
   destsys = planets[index][2]

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
   if sys == nil then
      sys = system.cur()
   end
   if max == nil then
      max = min
   end

   local open  = { sys }
   local close = { [sys:nameRaw()]=sys }
   local dist  = { [sys:nameRaw()]=0 }

   -- Run max times
   for i=1,max do
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
            end
         end
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
   Works the same as lmisn.getSysAtDistance, but for planets.

   Filter is applied on a planet level.
--]]
function lmisn.getPlanetAtDistance( sys, min, max, fct, samefct, filter, data, hidden )
   local pnts = {}
   local candidates = lmisn.getSysAtDistance( sys, min, max, lmisn.sysFilters.factionLandable( fct ), nil, hidden )
   if #candidates == 0 then
      return pnts
   end
   for _k,s in ipairs(candidates) do
      local lp = lmisn.getLandablePlanets( s, fct, samefct )
      for _i,p in ipairs(lp) do
         if not filter or filter( p, data ) then
            table.insert( pnts, p )
         end
      end
   end
   return pnts
end

--[[--
   Gets a random planet at a distance. Only checks for inhabited, landable, and non-restricted planets.

   @usage lmisn.getRandomPlanetAtDistance( system.cur(), 3, 5, "Independent", false ) -- Get random planet within 3 to 5 jumps of current system that is landable by independent pilots.

      @tparam[opt=system.cur()] System sys System to base distance calculations off of.
      @tparam number min Minimum jump distance to get planet at.
      @tparam number max Maximum jump distance to get planet at.
      @tparam[opt=nil] Faction fct What faction to do landing checks with.
      @tparam[opt=false] boolean samefct Whether or not to only allow planets to belong exactly to fct.
      @tparam[opt=nil] function filter Filtering function that returns a boolean and takes a planet being tested as a parameter.
      @param[opt=nil] data Custom data that will be passed to filter.
      @tparam[opt=false] boolean hidden
      @treturn Planet A single planet matching all the criteria.
--]]
function lmisn.getRandomPlanetAtDistance( sys, min, max, fct, samefct, filter, data, hidden )
   local candidates = lmisn.getPlanetAtDistance( sys, min, max, fct, samefct, filter, data, hidden )
   if #candidates == 0 then
      return nil, nil
   end
   return planet.getS( candidates[ rnd.rnd(1,#candidates) ] )
end

--[[--
   Wrapper for player.misnActive that works on a table of missions.

   @usage if anyMissionActive( { "Cargo", "Cargo Rush" } ) then -- at least one Cargo or Cargo Rush is active

      @param names Table of names of missions to check
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

return lmisn
