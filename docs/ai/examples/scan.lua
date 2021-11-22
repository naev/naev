--[[
-- typical usage would be:
--    ai.pushtask(0, "scan", ai.rndpilot())
--]]

function scan( target )
   if not target:exists() then
      ai.poptask()
      return
   end
   dir = ai.face(target)
   dist = ai.dist( ai.pos(target) )
   if dir < math.rad(10) and dist > 300 then
      ai.accel()
   elseif dist < 300 then
      -- scan the target
      ai.poptask()
   end
end
