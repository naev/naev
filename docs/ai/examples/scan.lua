--[[
-- typical usage would be:
--    ai.pushtask(0, "scan", ai.rndpilot())
--]]

function scan ()
   target = ai.targetid()
   if not ai.exists(target) then
      ai.poptask()
      return
   end
   dir = ai.face(target)
   dist = ai.dist( ai.pos(target) )
   if dir < 10 and dist > 300 then
      ai.accel()
   elseif dist < 300 then
      -- scan the target
      ai.poptask()
   end
end

