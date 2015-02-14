--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include("dat/ai/empire_idle.lua")
include("dat/ai/personality/patrol.lua")

-- Settings
mem.aggressive = false
mem.defensive  = false
mem.distressmsg = "Empire refuel ship under attack!"

function create ()

   -- Broke
   ai.setcredits( 0 )

   -- Get refuel chance
   p = ai.getPlayer()
   if ai.exists(p) then
      mem.refuel = 0
      -- Most likely no chance to refuel
      mem.refuel_msg = "\"Sure thing.\""
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   bribe_no = "I'm out of here."
end



