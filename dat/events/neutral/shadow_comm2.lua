--[[

   Comm Event for the Shadow missions

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include ("proximity.lua") 

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
end

function create ()
    hailie = hook.timer(20000, "hailme")
    
    landhook = hook.land("finish")
    jumpouthook = hook.jumpout("finish")
end

-- Make the ship hail the player
function hailme()
    naev.missionStart("Dark Shadow")
    player.commClose()
    evt.finish()
end

function finish()
    hook.rm(hailie)
    evt.finish()
end
