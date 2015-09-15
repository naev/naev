--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include("dat/ai/tpl/escort.lua")
include("dat/ai/personality/patrol.lua")

-- Settings
mem.aggressive = true
mem.command = false


function create ()
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   mem.escort = player.pilot()
end

