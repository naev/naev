--[[

   Comm Event for the Crazy Baron mission string

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]


function create ()
    hyena = pilot.add("Civilian Hyena", nil, true)[1]
    
    hook.pilot(hyena, "jump", "finish")
    hook.pilot(hyena, "death", "finish")
    hook.land("finish")
    hook.jumpout("finish")
    
    hailie = hook.timer( 3000., "hailme" );
end

-- Make the ship hail the player
function hailme()
    hyena:hailPlayer()
    hook.pilot(hyena, "hail", "hail")
end

-- Triggered when the player hails the ship
function hail()
    naev.missionStart("Baron")
    player.commClose()
    evt.finish(true)
end

function finish()
    hook.rm(hailie)
    evt.finish()
end
