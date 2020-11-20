--[[This file contains the attack profiles by ship type.
--commonly used range and condition-based attack patterns are found in another file
--Think functions for determining who to attack are found in another file
--]]


--[[
-- Main control function for bomber behavior.
-- Bombers are expected to have heavy weapons and target
--ships bigger than they are
--]]
function atk_bomber ()
   local target = _atk_com_think()
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 0)

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      atk_g_ranged( target, dist )

   -- Now we do an approach
   --elseif dist > 10 * range * mem.atk_aim then
   --   atk_spiral_approach( target, dist )

   -- Close enough to melee
   else
        mem.aggressive = false
        atk_g_flyby( target, dist )   
   end
end

--[[
-- Main control function for corvette behavior.
--]]
function atk_corvette ()
   local target = _atk_com_think()
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3, 0)
   local range2 = ai.getweaprange(3, 1)
   
   if range2 > range then
    range = range2
   end

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      atk_g_ranged( target, dist )


   -- Close enough to melee
   else
      if target:stats().mass < 500 then
        atk_g_space_sup(target, dist)
      else
        mem.aggressive = true
        atk_g_flyby( target, dist )
      end
   end
end


--[[
-- Main control function for capital ship behavior.
--]]
function atk_capital ()
   local target = _atk_com_think()
   if target == nil then return end

   -- Targeting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange(3)

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      
      atk_g_ranged( target, dist )

   -- Now we do an approach
   --elseif dist > 10 * range * mem.atk_aim then
   --   atk_spiral_approach( target, dist )

   -- Close enough to melee
   else   
     atk_g_capital(target, dist)
   end
end --end capship attack

