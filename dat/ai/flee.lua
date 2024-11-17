require 'ai.core.core'
require 'ai.core.idle.trader'

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
function control( dt )
   mem.elapsed = mem.elapsed + dt
   local task  = ai.taskname()

   if task ~= "runaway" then
      local enemy = ai.getenemy()

      if enemy ~= nil then
         ai.pushtask("runaway", enemy)
      else
         ai.pushtask("hyperspace" )
      end

   elseif task == "runaway" then
      local target = ai.taskdata()

      if not target:exists() then
         ai.poptask()
         return
      end
   end
end

-- Required "attacked" function
function attacked ( attacker )
   local task = ai.taskname()

   if task == "runaway" then
      local target = ai.taskdata()
      if target == nil or ai.taskdata() ~= attacker then
         ai.poptask()
         ai.pushtask("runaway", attacker)
      end
   else
      ai.pushtask("runaway", attacker)
   end
end
