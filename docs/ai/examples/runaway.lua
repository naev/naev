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
function runaway( target )
   if not target:exists() then
      ai.poptask()
      ai.pushtask("hyperspace")
      return
   end

   ai.face( target, 1 )
   ai.accel()
   if ai.hasturrets() then
      local dist = ai.dist( target:pos() )
      if dist < 300 then
         ai.settarget(target)
         ai.shoot()
      end
   end
end
