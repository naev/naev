--[[
--    Attack utilitiesGeneric attack functions
--]]


--[[
--Attempts to maintain a constant distance from nearby things
--This modulates the distance between the current pilot and its nearest neighbor
--]]
function __atk_keep_distance()
   --anticipate this will be added to eliminate potentially silly behavior if it becomes a problem
   --local flight_offset = ai.drift_facing()

   --find nearest thing
   local neighbor = ai.nearestpilot()
   if not neighbor or not neighbor:exists() then
      return
   end

   --find the distance based on the direction I'm travelling
   local perp_distance = ai.flyby_dist(neighbor)
   -- adjust my direction of flight to account for this
   -- if pilot is too close, turn away
   if perp_distance < 0 and perp_distance > -50 then
      ai.turn(1)
   elseif perp_distance > 0 and perp_distance < 50 then
      ai.turn(-1)
   end
end

--[[
-- Tests if the target is seeable (even fuzzy). If no, try to go where it was last seen
-- This is not supra-clean as we are not supposed to know [target:pos()]
-- But the clean way would require to have stored the target position into memory
-- This test should be put in any subtask of the attack task.
--]]
function __atk_check_seeable( target )
   if __check_seeable( target ) then
      return true
   end
   __investigate_target( target )
   return false
end


--[[
-- Decides if zigzag is a good option
--]]
function __atk_decide_zz( target, dist )
   -- Some AI will not do fancy maneuvers
   if mem.simplecombat then return false end
   -- The situation is the following: we're out of range, facing the target,
   -- going towards the target, and someone is shooting on us.
   local pilot  = ai.pilot()
   local range  = ai.getweaprange(3)
   local dir    = ai.idir(target)

   local m1, d1 = vec2.polar( pilot:vel() )
   local m2, d2 = vec2.polar( target:pos() - pilot:pos() )
   local d = d1-d2

   return ( (dist > range) and (ai.hasprojectile())
           and (dir < 10) and (dir > -10) and (d < 10) and (d > -10) )
end


--[[
-- Zig zags towards the target
--]]
function _atk_zigzag( target )
   target = __atk_com_think( target )
   if target == nil then return end

   -- See if the enemy is still seeable
   if not __atk_check_seeable( target ) then return end

   ai.settarget(target)

   -- Is there something to dodge?
   if not ai.hasprojectile() then
      ai.popsubtask()
      return
   end

   -- Are we ready to shoot?
   local dist = ai.dist( target )
   local range = ai.getweaprange(3)
   if dist < range then
      ai.popsubtask()
      return
   end

   local dir = ai.dir( target )
   __zigzag(dir, 30)
end


--[[
-- Tries to shoot seekers at close range
--]]
function __atk_dogfight_seekers( dist, dir )
   if dist > 100 then
      if dist < ai.getweaprange( 4 ) and dir < 20  then
         ai.weapset( 4 )
      elseif dist < ai.getweaprange( 9 ) then
         ai.weapset( 9 )
      end
   end
end


--[[
-- Common control stuff
--]]
function __atk_com_think( target, dokill )
   -- make sure pilot exists
   if not target or not target:exists() then
      ai.poptask()
      return
   end

   -- Check if is bribed by target
   if ai.isbribed(target) then
      ai.poptask()
      return
   end

   -- Kill the target
   if dokill then
      return target
   end

   -- Check if we want to board
   if mem.atk_board and ai.canboard(target) then
      ai.pushtask("board", target )
      return
   end

   -- Check to see if target is disabled
   if not mem.atk_kill and target:flags("disabled") then
      ai.poptask()
      return
   end

   return target
end


