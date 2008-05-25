--[[
-- Pilot runs away, could be at low health, or at signs of fighting
--]]


-- Required "attacked" function
function attacked ( attacker )
   if ai.taskname() ~= "runaway" then
      -- Sir Robin bravely ran away
      ai.pushtask(0, "runaway", attacker)
   else -- run away from the new baddie
      ai.poptask()
      ai.pushtask(0, "runaway", attacker)
   end
end

-- runs away
function runaway ()
   target = ai.targetid()

   if not ai.exists(target) then
      ai.pushtask()
      ai.pushtask(0,"hyperspace")
      return
   end

   dir = ai.face( target, 1 )
   ai.accel()
   if ai.hasturrets() then
      dist = ai.dist( ai.pos(target) )
      if dist < 300 then
         ai.settarget(target)
         ai.shoot()
      end
   end
end


