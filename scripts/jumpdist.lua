-- @brief Fetches an array of systems exactly n jumps away from the given system sys
--    @param sys System to calculate distance from or nil to use current system
--    @param n Number of jumps to get systems
--    @return The table of systems n jumps away from sys
function getsysatdistance( sys, min, max )
   -- Get default parameters
   if sys == nil then
      sys = system.get()
   end
   if max == nil then
      max = min
   end
   -- Begin iteration
   return _getsysatdistance( sys, min, max, sys, max, {} )
end

-- The first call to this function should always have n >= max
function _getsysatdistance( target, min, max, sys, n, t )
   if n == 0 then -- This is a leaf call - perform checks and add if appropriate
      local d
      d = target:jumpDist(sys)
      if d >= min and d <= max then
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
      end
      return t
   else -- This is a branch call - recursively call over all adjacent systems
      for i, j in pairs( sys:adjacentSystems() ) do
         t = _getsysatdistance(target, min, max, j, n-1, t)
      end
      return t
   end
end
