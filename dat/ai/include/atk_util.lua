--[[
--    Attack utilitiesGeneric attack functions
--]]


--[[
--Attempts to maintain a constant distance from nearby things
--This modulates the distance between the current pilot and its nearest neighbor
--]]
function _atk_keep_distance()
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
function _atk_check_seeable()
   local self   = ai.pilot()
   local target = ai.target()

   -- Pilot still sees the target: continue attack
   if self:inrange( target ) then
      return true
   end

   -- Pilots on manual control (in missions or events) never loose target
   -- /!\ This is not necessary desirable all the time /!\
   -- TODO: there should probably be a flag settable to allow to outwit pilots under manual control 
   if self:flags().manualcontrol then
      return true
   end

   ai.settarget(self) -- Un-target
   ai.poptask()
   ai.pushtask("moveto", target:pos() )
   return false
end


--[[
-- Decides if zigzag is a good option
--]]
function _atk_decide_zz()
   -- The situation is the following: we're out of range, facing the target,
   -- going towards the target, and someone is shooting on us.

   local target = ai.target()
   local pilot  = ai.pilot()
   local range  = ai.getweaprange(3)
   local dir = ai.idir(target)
   local dist  = ai.dist( target )

   local m, d1 = vec2.polar( pilot:vel() )
   local m, d2 = vec2.polar( target:pos() - pilot:pos() )
   local d = d1-d2

   return ( (dist > (1.1*range)) and (ai.hasprojectile())
           and (dir < 10) and (dir > -10) and (d < 10) and (d > -10) )
end


--[[
-- Zig zags towards the target
--]]
function _atk_zigzag()
   local target = ai.target()
   local range  = ai.getweaprange(3)

   if (not target:exists()) then
      ai.poptask()
      return
   end

   -- See if the enemy is still seeable
   if not _atk_check_seeable() then return end

   -- Is there something to dodge?
   if (not ai.hasprojectile()) then
      ai.popsubtask()
      return
   end

   local dist  = ai.dist( target )

   -- Are we ready to shoot?
   if dist < (1.1*range) then
      ai.popsubtask()
      return
   end

   local dir = ai.dir(ai.target())
   __zigzag(dir, 30)
end


--[[
-- Common control stuff
--]]
function _atk_com_think ()
   local target = ai.target()

   -- make sure pilot exists
   if not target:exists() then
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
      ai.pushtask("board", target );
      return
   end

   -- Check to see if target is disabled
   if not mem.atk_kill and target:flags().disabled then
      ai.poptask()
      return
   end
   
   return target
end


