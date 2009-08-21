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
   local task  = ai.taskname()

   if task == "hyperspace" then
      ai.hyperspace() -- try to hyperspace 

   elseif task ~= "runaway" then
      local enemy = ai.getenemy()

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

      ai.hyperspace()
   end
end

-- Required "attacked" function
function attacked ( attacker )
   local task = ai.taskname()

   if task == "runaway" then
      local target = ai.target()
      if target == nil or ai.target() ~= attacker then
         ai.poptask()
         ai.pushtask(0, "runaway", attacker)
      end
   else
      ai.pushtask(0, "runaway", attacker)
   end
end

