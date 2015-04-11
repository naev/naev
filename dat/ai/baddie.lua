--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include("dat/ai/tpl/generic.lua")
include("dat/ai/personality/patrol.lua")

--[[

   Generic Mission baddie AI

]]--

-- Settings
mem.aggressive     = true
mem.safe_distance  = 500
mem.armour_run     = 80
mem.armour_return  = 100
mem.atk_board      = true
mem.atk_kill       = true
mem.atk_board      = false
mem.bribe_no       = "You can't bribe me!"
mem.refuel_no      = "I won't give you fuel!"


function create ()

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Choose attack format
   attack_choose()
end
