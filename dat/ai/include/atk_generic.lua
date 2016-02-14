--[[
--    Generic attack functions
--]]


--[[
-- Required initialization function
--]]
function atk_generic_init ()
   mem.atk_think  = atk_generic_think
   mem.atk        = atk_generic
end


--[[
-- Mainly manages targetting nearest enemy.
--]]
function atk_generic_think ()
   local enemy  = ai.getenemy()
   local target = ai.target()

   -- Stop attacking if it doesn't exist
   if not target:exists() then
      ai.poptask()
      return
   end

   -- Get new target if it's closer
   if enemy ~= target and enemy ~= nil then
      local dist  = ai.dist( target )
      local range = ai.getweaprange( 3 )

      -- Shouldn't switch targets if close
      if dist > range * mem.atk_changetarget then
         ai.pushtask( "attack", enemy )
      end
   end
end


--[[
-- Attacked function.
--]]
function atk_generic_attacked( attacker )
   local target = ai.target()

   if mem.recharge then
      mem.recharge = false
   end

   -- If no target automatically choose it
   if not target:exists() then
      ai.pushtask("attack", attacker)
      return
   end

   local tdist  = ai.dist(target)
   local dist   = ai.dist(attacker)
   local range  = ai.getweaprange( 0 )

   if target ~= attacker and dist < tdist and
         dist < range * mem.atk_changetarget then
      ai.pushtask("attack", attacker)
   end
end


--[[
-- Generic "brute force" attack.  Doesn't really do anything interesting.
--]]
function atk_generic ()
   local target = _atk_com_think()
   if target == nil then return end

   -- Targetting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange( 3 )

   -- We first bias towards range
   if dist > range * mem.atk_approach then
      _atk_g_ranged( target, dist )

   -- Now we do an approach
   elseif dist > range * mem.atk_aim then
      _atk_g_approach( target, dist )

   -- Close enough to melee
   else
      _atk_g_melee( target, dist )
   end
end


--[[
-- Enters ranged combat with the target
--]]
function _atk_g_ranged( target, dist )

   -- Pilot thinks dogfight is the best
   if ai.relhp(target)*ai.reldps(target) >= 0.25 
         or ai.getweapspeed(4) < target:stats().speed_max*1.2 
         or ai.getweaprange(4) < ai.getweaprange(1)*1.5 then

      local dir
      if not mem.careful or dist < 3 * ai.getweaprange(3, 0) * mem.atk_approach then
         dir = ai.face(target) -- Normal face the target
      else
         dir = ai.careful_face(target) -- Careful method
      end

      -- Check if in range to shoot missiles
      if dist < ai.getweaprange( 4 ) and dir < 30 then
         ai.weapset( 4 )
      end

      -- Approach for melee
      if dir < 10 then
         ai.accel()
      end

   else   --Pilot fears his enemy

   --[[ The pilot tries first to place himself at range and at constant velocity.
        When he is stabilized, he starts shooting until he has to correct his trajectory again

        If he doesn't manage to shoot missiles after a few seconds 
        (because the target dodges),
        he gives up and just faces the target and shoot (provided he is in range)
   ]]

      local p = ai.pilot()

      -- Estimate the range
      local radial_vel = ai.relvel(target, true)
      local range = ai.getweaprange( 4 )
      range = math.min ( range - dist * radial_vel / ( ai.getweapspeed( 4 ) - radial_vel ), range )

      local goal = ai.follow_accurate(target, range * 0.8, 0, 10, 20, "keepangle")
      local mod = vec2.mod(goal - p:pos())

      --Must approach or stabilize
      if mod > 3000 then
         -- mustapproach allows a hysteretic behaviour
         mem.mustapproach = true
      end
      if dist > range*0.95 then
         mem.outofrange = true
      end

      if (mem.mustapproach and not ai.timeup(1) ) or mem.outofrange then
         local dir   = ai.face(goal)
         if dir < 10 and mod > 300 then
            ai.accel()
            --mem.stabilized = false
         -- ship must be stabilized since 2 secs
         elseif ai.relvel(target) < 5 and not ai.timeup(1) then--[[if not mem.stabilized then
            mem.stabilized = true
            ai.settimer(0, 2000)
         elseif not ai.timeup(1) and ai.timeup(0) then
            -- If the ship manages to catch its mark, reset the timer]]
            --ai.settimer(1, 10000)
            mem.mustapproach = false
         end
         if dist < range*0.85 then
            mem.outofrange = false
         end

      else -- In range
         local dir  = ai.face(target)
         if dir < 30 then
            mem.totmass = p:stats().mass
            ai.weapset( 4 )
            -- If he managed to shoot, the mass decreased
            if p:stats().mass < mem.totmass - 0.01 and not ai.timeup(1) then
               ai.settimer(1, 13000)
            end
         end
      end

      --The pilot just arrived in the good zone : 
      --From now, if ship doesn't manage to stabilize within a few seconds, shoot anyway
      if dist < 1.5*range and not mem.inzone then
         mem.inzone = true
         ai.settimer(1, mod/p:stats().speed*700 )
      end

   end

   -- Always launch fighters for now
   ai.weapset( 5 )
end


--[[
-- Approaches the target
--]]
function _atk_g_approach( target, dist )
   dir = ai.idir(target)
   if dir < 10 and dir > -10 then
      _atk_keep_distance()
   else
      dir = ai.iface(target)
   end
   if dir < 10 then
      ai.accel()
   end
end


--[[
-- Melees the target
--]]
function _atk_g_melee( target, dist )
   local dir   = ai.aim(target) -- We aim instead of face
   local range = ai.getweaprange( 3 )
   ai.weapset( 3 ) -- Set turret/forward weaponset.

   -- Drifting away we'll want to get closer
   if dir < 10 and dist > 0.5*range and ai.relvel(target) > -10 then
      ai.accel()
   end

   -- Shoot if should be shooting.
   if dir < 10 then
      ai.shoot()
   end
   ai.shoot(true)
end
