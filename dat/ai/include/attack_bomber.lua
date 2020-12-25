--[[
--    Attack functions for bombers
--]]


--[[
-- Bombers don't really think, they lock on until target is dead.
--]]
function atk_b_think ()
   -- No thinking atm
end



--[[
-- Attacks the current target, task pops when target is dead.
--
-- Specialized for bomber type craft.  AI will try to shoot missiles and such
--  until out and then will melee.
--]]
function atk_b ()
   target = ai.target()
   ai.hostile(target) -- Mark as hostile

   -- make sure pilot exists
   if not target:exists() then
      ai.poptask()
      return
   end
   ai.settarget(target)

   -- Get stats about enemy
   dist = ai.dist( target ) -- get distance

   -- Get bombing tool
   secondary, special = ai.secondary("ranged")
   if secondary ~= "Launcher" or special == "Unguided" then -- No launcher, must melee
      atk_b_melee( target, dist, secondary, special )
   end

   atk_b_ranged( target, dist )
end


--[[
--   Melee combat.
--]]
function atk_b_melee( target, dist, secondary, special )
   range = ai.getweaprange()

   -- Must approach
   if dist > range then
      dir = ai.face(target)
      if dir < 10 then
         ai.accel()
      end

   -- Time to shoot
   else
      dir = ai.aim(target) -- We aim instead of face
      
      -- Fire secondary
      if dir < 10 or special == "Turret" then
         ai.shoot(true)
      end

      -- Fire primary
      if dir < 10 then
         ai.shoot(false)
      elseif ai.hasturrets() then
         ai.shoot(false, 1)
      end
   end
end


--[[
--   The heart and soul of the bomber.
--]]
function atk_b_ranged( target, dist )

   -- Get ranges relative to bombing weapon of choice
   bombrange = ai.getweaprange(true)
   backoff = bombrange / 4

   -- Must get closer to be able to bomb
   if dist > bombrange then
      dir = ai.face(target)
      if dir < 10 then
         ai.accel()
      end

   -- In bombing range
   elseif dist > backoff then
      dir = ai.face(target)

      -- Shoot missiles if in range
      if secondary == "Launcher" and
            dist < bombrange then

         -- More lenient with aiming
         if special == "Smart" and dir < 30 then
            ai.shoot(true)

         -- Non-smart miss more
         elseif dir < 10 then
            ai.shoot(true)
         end
      end

      -- We don't approach, we try to stay away from melee

   -- Time to break attack and get back to bomb
   else
      ai.pushtask( "atk_b_backoff" )
   end
end


--[[
--   Back off and come back when ready to kill
--]]
function atk_b_backoff ()
   range = ai.getweaprange()

   -- Flee
   ai.face(target, true)
   ai.accel()

   -- Fire turret if being chased
   if dist < range and ai.hasturrets() then
      ai.shoot(false, 1)
   end
end

