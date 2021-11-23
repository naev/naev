require 'ai.empire'

--[[
    This AI is for Empire ships that should initially just sit stationary in space. Good for guards.
--]]

-- Just stays still
-- luacheck: globals stay_still (AI Task functions passed by name)
function stay_still ()
   if ai.isstopped() then
      return
   end
   ai.brake()
end


-- By default stay still
-- luacheck: globals idle (AI Task functions passed by name)
function idle ()
   ai.pushtask( "stay_still" )
end
