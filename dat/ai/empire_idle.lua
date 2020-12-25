require("ai/empire")

--[[
    This AI is for Empire ships that should initially just sit stationary in space. Good for guards.
--]]
 

-- Just stays still
function stay_still ()
   if ai.isstopped() then
      return
   end
 
   ai.brake()
end
 
 
-- By default stay still
function idle ()
   ai.pushtask( "stay_still" )
end
