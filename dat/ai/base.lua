include("dat/ai/include/basic.lua")


-- Required control rate
control_rate   = 5


function create ()
   mem.comm_no = "No response."
end


-- Required "control" function
function control ()
   local task = ai.taskname()
   local enemy = ai.getenemy()

   -- Get new task
   if task ~= "attack" then
      -- We'll first check enemy.
      if enemy ~= nil and mem.aggressive then
         taunt(enemy, true)
         ai.pushtask("attack", enemy)
      else
         ai.pushtask("idle")
      end
   end
end


function attacked( attacker )
   if ai.taskname() ~= "attack" then
      ai.pushtask("attack", attacker)
   end
end


function attack ()
   -- make sure pilot exists
   if not target:exists() then
      ai.poptask()
      return
   end

   -- Targetting stuff
   ai.hostile(target) -- Mark as hostile
   ai.settarget(target)

   -- Get stats about enemy
   local dist  = ai.dist( target ) -- get distance
   local range = ai.getweaprange()
   local dir   = ai.aim(target)

   if dist < range then
      if dir < 10 then
         ai.shoot(false)
      elseif ai.hasturrets() then
         ai.shoot(false,1)
      end
   else
      ai.poptask() -- change targets
   end

end


function idle ()
end
