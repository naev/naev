--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--

   Template for tutorial modules.
   Each module should start by setting up the tutorial environment and enforcing rules.
   Each module should clean up, set mission variable "var_next" to next tutorial in sequence, and return to the main tutorial menu when ending or aborting.

--]]

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    
end

function create()
    -- Set up the player here.
    player.teleport("Mohawk")
    player.pilot():setPos(vec2.new(0, 0))
    player.msgClear()
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end
