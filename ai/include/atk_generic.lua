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
   if not ai.exists(target) then
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

   if mem.cooldown then
      mem.cooldown = false
      ai.getPilot():setCooldown( false )
   end

   -- If no target automatically choose it
   if not ai.exists(target) then
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
   local dir = ai.face(target) -- Normal face the target

   -- Check if in range to shoot missiles
   if dist < ai.getweaprange( 4 ) and dir < 30 then
      ai.weapset( 4 )
   end

   -- Always launch fighters for now
   ai.weapset( 5 )

   -- Approach for melee
   if dir < 10 then
      ai.accel()
   end
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
