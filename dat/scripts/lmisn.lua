--[[
   Mission helper stuff
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
      victory = audio.newSource( 'snd/sounds/jingles/victory.ogg' ),
   }
end

function lmisn.sfxVictory ()
   if not _sfx then _sfx_load() end

   local sfx = _sfx.victory:clone()
   sfx:play()
end

function lmisn.sfxMoney ()
end


-- Choose the next system to jump to on the route from system nowsys to system finalsys.
function lmisn.getNextSystem( nowsys, finalsys, hidden )
   if nowsys == finalsys or finalsys == nil then
       return nowsys
   end

   path = nowsys:jumpPath( finalsys, hidden )
   if not path then
      return nowsys
   end

   return path[1]:dest()
end

--[[
-- @brief Fetches an array of systems from min to max jumps away from the given
--       system sys.
--
-- The following example gets a random Sirius M class planet between 1 to 6 jumps away.
--
-- @code
-- local planets = {}
-- lmisn.getSysAtDistance( system.cur(), 1, 6,
--     function(s)
--         for i, v in ipairs(s:planets()) do
--             if v:faction() == faction.get("Sirius") and v:class() == "M" then
--                 return true
--             end
--         end
--         return false
--     end )
--
-- if #planets == 0 then abort() end -- In case no suitable planets are in range.
--
-- local index = rnd.rnd(1, #planets)
-- destplanet = planets[index][1]
-- destsys = planets[index][2]
-- @endcode
--
--    @param sys System to calculate distance from or nil to use current system
--    @param min Min distance to check for.
--    @param max Maximum distance to check for.
--    @param filter Optional filter function to use for more details.
--    @param data Data to pass to filter
--    @param hidden Whether or not to consider hidden jumps (off by default)
--    @return The table of systems n jumps away from sys
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
      for _,s in ipairs(open) do
         adjsys = s:adjacentSystems( hidden ) -- Get them all
         for _,a in ipairs(adjsys) do
            -- Must not have been explored previously
            if close[ a:nameRaw() ] == nil then
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

return lmisn
