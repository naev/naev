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
   if not ai.exists(neighbor) then
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
-- Common control stuff
--]]
function _atk_com_think ()
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
      ai.pushtask("board", target );
      return
   end

   -- Check to see if target is disabled
   if not mem.atk_kill and ai.isdisabled(target) then
      ai.poptask()
      return
   end
   
   return target
end


