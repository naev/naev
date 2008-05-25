--[[
-- Based on when pilot is hit by something, then will attempt to retaliate
--]]

-- triggered when pilot is hit by something
function attacked ( attacker )
   task = ai.taskname()
   if task ~= "attack" and task ~= "runaway" then

      -- some taunting
      taunt( attacker )

      -- now pilot fights back
      ai.pushtask(0, "attack", attacker)

   elseif task == "attack" then
      if ai.targetid() ~= attacker then
            ai.pushtask(0, "attack", attacker)
      end
   end
end

-- taunts
function taunt ( target )
   num = ai.rnd(0,4)
   if num == 0 then msg = "You dare attack me!"
   elseif num == 1 then msg = "You are no match for the Empire!"
   elseif num == 2 then msg = "The Empire will have your head!"
   elseif num == 3 then msg = "You'll regret this!"
   end
   if msg then ai.comm(attacker, msg) end
end

-- attacks
function attack ()
   target = ai.targetid()

   -- make sure pilot exists
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   dir = ai.face( target )
   dist = ai.dist( ai.pos(target) )
   second = ai.secondary()

   if ai.secondary() == "Launcher" then
      ai.settarget(target)
      ai.shoot(2)
   end

   if dir < 10 and dist > 300 then
      ai.accel()
   elseif (dir < 10 or ai.hasturrets()) and dist < 300 then
      ai.shoot()
   end
end


