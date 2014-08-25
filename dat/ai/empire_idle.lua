--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include("dat/ai/empire.lua")

--[[
    This AI is for Empire ships that should initially just sit stationary in space. Good for guards.
--]]
 

-- Just stays still
function stay_still ()
   if ai.isstopped() then
      return
   end
 
   ai.brake()
end
 
 
-- By default stay still
function idle ()
   ai.pushtask( "stay_still" )
end
