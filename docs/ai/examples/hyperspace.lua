--[[
-- we check to see if we can hyperspace in control to not spam
--]]

-- add this to control function
function control ()
   task = ai.taskname()
   if task == "hyperspace" then
      ai.hyperspace() -- try to hyperspace
   end
end


-- goes hyperspace
function hyperspace ()
   dir = ai.face(-1) -- face away from (0,0)
   if (dir < 10) then -- try to go straight
      ai.accel()
   end
end

