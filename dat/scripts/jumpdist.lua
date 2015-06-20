--[[
-- @brief Fetches an array of systems from min to max jumps away from the given
--       system sys.
--
-- The following example gets a random Sirius M class planet between 1 to 6 jumps away.
--
-- @code
-- local planets = {} 
-- getsysatdistance( system.cur(), 1, 6,
--     function(s)
--         for i, v in ipairs(s:planets()) do
--             if v:faction() == faction.get("Sirius") and v:class() == "M" then
--                 return true
--             end
--         end 
--         return false
--     end )
-- 
-- if #planets == 0 then abort() end -- Sanity in case no suitable planets are in range.
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
function getsysatdistance( sys, min, max, filter, data, hidden )
   -- Get default parameters
   if sys == nil then
      sys = system.cur()
   end
   if max == nil then
      max = min
   end

   open  = { sys }
   close = { [sys:name()]=sys }
   dist  = { [sys:name()]=0 }

   -- Run max times
   for i=1,max do
      nopen = {}
      -- Get all the adjacent system of the current set
      for _,s in ipairs(open) do
         adjsys = s:adjacentSystems( hidden ) -- Get them all
         for _,a in ipairs(adjsys) do
            -- Must not have been explored previously
            if close[ a:name() ] == nil then
               nopen[ #nopen+1 ] = a
               close[ a:name() ] = a
               dist[  a:name() ] = i
            end
         end
      end
      open = nopen -- New table becomes the old
   end

   -- Now we filter the solutions
   finalset = {}
   for i,s in pairs(close) do
      if dist[i] >= min and dist[i] <= max and
            (filter == nil or filter(s,data)) then
         finalset[ #finalset+1 ] = s
      end
   end
   return finalset
end


