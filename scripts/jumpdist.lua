

--[[
-- @brief Fetches an array of systems from min to max jumps away from the given
--       system sys.
--
-- A following example to get closest system with shipyard (to max of 10):
--
-- @code
-- local i, t
-- while i < 10 do
--    t = getsysatdistance( target, i, i,
--          function(s)
--             for k,v in s:planets() do
--                if v:hasShipyard()
--                   return true
--                end
--             end
--             return false
--          end )
-- end
-- local target_system = t[ rnd.rnd(1,#t) ]
-- @endcode
--
--    @param sys System to calculate distance from or nil to use current system
--    @param min Min distance to check for.
--    @param max Maximum distance to check for.
--    @param filter Optional filter function to use for more details.
--    @param data Data to pass to filter
--    @return The table of systems n jumps away from sys
--]]
function getsysatdistance( sys, min, max, filter, data )
   -- Get default parameters
   if sys == nil then
      sys = system.cur()
   end
   if max == nil then
      max = min
   end
   -- Begin iteration
   return _getsysatdistance( sys, min, max, sys, max, {}, filter, data )
end


-- The first call to this function should always have n >= max
function _getsysatdistance( target, min, max, sys, n, t, filter, data )
   if n == 0 then -- This is a leaf call - perform checks and add if appropriate
      local d = target:jumpDist(sys)

      -- Check bounds
      if d < min or d > max then
         return t
      end

      -- Case filter function is available
      if filter ~= nil and not filter(sys, data) then
         return t
      end

      -- Add to table
      local seen = false
      for i, j in ipairs(t) do -- Check if the system is already in our array
         if j == sys then
            seen = true
            break
         end
      end
      if not seen then -- Don't add a system we've already tagged.
         t[#t+1] = sys
      end
      return t
   else -- This is a branch call - recursively call over all adjacent systems
      for i, j in pairs( sys:adjacentSystems() ) do
         t = _getsysatdistance(target, min, max, i, n-1, t, filter, data)
      end
      return t
   end
end


