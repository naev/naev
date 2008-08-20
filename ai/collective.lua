include("ai/include/basic.lua")

-- Required control rate
control_rate = 0.5
-- Required "control" function
function control ()
   local task = ai.taskname()

   -- Think function for attack
   if task == "attack" then
      attack_think()

   elseif task == "none" then
      local enemy = ai.getenemy()

      if enemy ~= nil then
         ai.pushtask(0, "attack", enemy)
      else
         ai.pushtask(0, "hyperspace")
      end

   elseif task == "hyperspace" then
      ai.hyperspace()
   end
end
-- Required "attacked" function
function attacked ( attacker )
   task = ai.taskname()
   if task ~= "attack" then

      -- now pilot fights back
      ai.pushtask(0, "attack", attacker)

   elseif task == "attack" then
         if ai.targetid() ~= attacker then
            ai.pushtask(0, "attack", attacker)
         end
   end
end
function create ()
end
