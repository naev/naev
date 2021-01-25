--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Baroncomm_baron">
  <trigger>enter</trigger>
  <chance>4</chance>
  <cond>
   not var.peek("baron_hated") and
   not player.misnDone("Baron") and
   not player.misnActive("Baron") and
   (
      system.cur():faction() == faction.get("Empire") or
      system.cur():faction() == faction.get("Dvaered") or
      system.cur():faction() == faction.get("Sirius")
   )
  </cond>
  <flags>
  </flags>
  <notes>
   <campaign>Baron Sauterfeldt</campaign>
   <tier>2</tier>
  </notes>
 </event>
 --]]
--[[
-- Comm Event for the Baron mission string
--]]


function create ()
    if not evt.claim(system.cur()) then
      evt.finish()
    end

    hyena = pilot.addFleet("Civilian Hyena", true)[1]
    
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
