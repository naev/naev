--[[ 
--Event for kidnapped mission.
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