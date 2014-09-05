--[[ 

   Event for kidnapped mission.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

--Create Mom and Dad in their spaceship, and have them come from the planet Brooks in Arcturus system, following the player.
function create ()
    panma = pilot.add("Civilian Llama", "civilian", planet.get("Brooks"))[1]
    panma:control()
    panma:follow(player.pilot())
    hook.pilot(panma, "jump", "finish")
    hook.pilot(panma, "death", "finish")
    hook.land("finish")
    hook.jumpout("finish")
    
    yohail = hook.timer( 2000., "hailme" );
end

--Pa and Ma are hailing the player!
function hailme()
    panma:hailPlayer()
    hook.pilot(panma, "hail", "hail")
end

--Pa and Ma have been hailed. The mission can begin, and panma should land on the planet Brooks
function hail()
    panma:control(false)
    player.commClose()
    naev.missionStart("Kidnapped")
    evt.finish(true)
end

function finish()
    hook.rm(yohail)
    evt.finish()
end
