--[[
--    Generic attack functions
--]]

mem.atk_changetarget  = 1.8 -- Distance at which target changes
mem.atk_approach      = 1.4 -- Distance that marks approach
mem.atk_aim           = 1.0 -- Distance that marks aim
mem.atk_board         = false -- Whether or not to board the target
mem.atk_kill          = true -- Whether or not to finish off the target


--[[
-- Mainly manages targetting nearest enemy.
--]]
function atk_g_think ()
   local enemy = ai.getenemy()
   local target = ai.target()

   -- Stop attacking if it doesn't exist
	if not ai.exists(target) then
		ai.poptask()
		return
	end

   -- Get new target if it's closer
   if enemy ~= target and enemy ~= nil then
      local dist = ai.dist( target )
      local range = ai.getweaprange()

      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask( "attack", enemy )
      end
   end
end


--[[
-- Attacked function.
--]]
function atk_g_attacked( attacker )
   local target = ai.target()

   -- If no target automatically choose it
   if not ai.exists(target) then
      ai.pushtask("attack", attacker)
      return
   end

   local tdist  = ai.dist(target)
   local dist   = ai.dist(attacker)
   local range  = ai.getweaprange()

   if target ~= attacker and dist < tdist and
         dist < range * mem.atk_changetarget then
      ai.pushtask("attack", attacker)
   end
end


--[[
-- Generic "brute force" attack.  Doesn't really do anything interesting.
--]]
function atk_g ()
	local target = ai.target()

	-- make sure pilot exists
	if not ai.exists(target) then
		ai.poptask()
		return
	end

   -- Check if is bribed by target
   if ai.isbribed(target) then
      ai.poptask()
      return
   end

   -- Check if we want to board
   if mem.atk_board and ai.canboard(target) then
      ai.pushtask( "board", target );
      return
   end

   -- Check to see if target is disabled
   if not mem.atk_kill and ai.isdisabled(target) then
      ai.poptask()
      return
   end

   -- Targetting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
	local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange()

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      atk_g_ranged( target, dist )

   -- Now we do an approach
   elseif dist > range * mem.atk_aim then
      atk_g_approach( target, dist )

   -- Close enough to melee
   else
      atk_g_melee( target, dist )
   end
end


--[[
-- Enters ranged combat with the target
--]]
function atk_g_ranged( target, dist )
   local dir = ai.face(target) -- Normal face the target
   local secondary, special, ammo = ai.secondary("ranged")

   -- Always use fighter bay
   if secondary == "" then

   -- Shoot missiles if in range
   elseif secondary == "Launcher" and
         dist < ai.getweaprange(true) then

      -- More lenient with aiming
      if special == "Smart" and dir < 30 then
         ai.shoot(true)

      -- Non-smart miss more
      elseif dir < 10 then
         ai.shoot(true)
      end
   end

   -- Approach for melee
   if dir < 10 then
      ai.accel()
   end
end


--[[
-- Approaches the target
--]]
function atk_g_approach( target, dist )
   local dir = ai.aim(target)
   if dir < 10 then
      ai.accel()
   end
end


--[[
-- Melees the target
--]]
function atk_g_melee( target, dist )
   local secondary, special = ai.secondary("melee")
   local dir = ai.aim(target) -- We aim instead of face
   local range = ai.getweaprange()

   -- Fire non-smart secondary weapons
   if (secondary == "Launcher" and special ~= "Smart") or
         secondary == "Beam Weapon" then
      if dir < 10 or special == "Turret" then -- Need good acuracy
         ai.shoot(true)
      end
   end

   -- Drifting away we'll want to get closer
   if dir < 10  and dist > 0.5*range and ai.relvel(target) > -10 then
      ai.accel()
   end

   -- Shoot if should be shooting.
   if dist < range then
      if dir < 10 then
         ai.shoot(false)
      elseif ai.hasturrets() then
         ai.shoot(false, 1)
      end
   end
end
