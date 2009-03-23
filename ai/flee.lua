include("ai/include/basic.lua")
--[[
 ===FLEE AI===
Mission specific AI to make pilots leave the system immediately.

  Settings
--]]
-- Required control rate
control_rate   = 2


-- Required "flee" function
function create ()
end

-- Required "control" function
function control ()
   task = ai.taskname()
   enemy = ai.getenemy()

   if task ~= "runaway" then

      if enemy ~= nil then
         ai.pushtask(0, "runaway", enemy)
      else
         ai.pushtask(0, "hyperspace" )
      end
   
   elseif task == "runaway" then
      target = ai.target()

      if not ai.exists(target) then
         ai.poptask()
         return
      end

      if ai.dist(target) > 300 then
         ai.hyperspace()
      end

   -- Enter hyperspace if possible
   elseif task == "hyperspace" then 
      ai.hyperspace() -- try to hyperspace 

   end
end

-- Required "attacked" function
function attacked ( attacker )
   task = ai.taskname()
   target = ai.target()

   if task == "runaway" then
      if ai.target() ~= attacker then
         ai.poptask()
         ai.pushtask(0, "runaway", attacker)
      end
   end
end

