--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Baroncomm_baron">
 <location>enter</location>
 <chance>4</chance>
 <cond>
   if var.peek("baron_hated") or
      player.misnDone("Baron") or
      player.misnActive("Baron") then
      return false
   end
   local sf = system.cur():faction()
   if not inlist( {
      faction.get("Empire"),
      faction.get("Dvaered"),
      faction.get("Sirius"),
   }, sf )
      return false
   end
   if player.wealth() &lt; 1e6 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <notes>
  <campaign>Baron Sauterfeldt</campaign>
  <tier>2</tier>
 </notes>
</event>
--]]
--[[
-- Comm Event for the Baron mission string
--]]

local hyena, hailie

function create ()
    if not evt.claim(system.cur()) then
      evt.finish()
    end

    hyena = pilot.add( "Hyena", "Independent" )

    hook.pilot(hyena, "jump", "finish")
    hook.pilot(hyena, "death", "finish")
    hook.land("finish")
    hook.jumpout("finish")

    hailie = hook.timer( 3.0, "hailme" );
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
