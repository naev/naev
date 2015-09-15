--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include("dat/ai/include/basic.lua")
include("dat/ai/personality/trader.lua")

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

   if task ~= "runaway" then
      local enemy = ai.getenemy()

      if enemy ~= nil then
         ai.pushtask("runaway", enemy)
      else
         ai.pushtask("hyperspace" )
      end
   
   elseif task == "runaway" then
      target = ai.target()

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
      local target = ai.target()
      if target == nil or ai.target() ~= attacker then
         ai.poptask()
         ai.pushtask("runaway", attacker)
      end
   else
      ai.pushtask("runaway", attacker)
   end
end

