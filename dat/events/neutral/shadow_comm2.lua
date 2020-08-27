--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Shadowcomm2">
  <trigger>enter</trigger>
  <chance>3</chance>
  <cond>system.cur():presence("hostile") &lt; 300 and player.misnDone("Shadow Vigil") and not (player.misnDone("Dark Shadow") or var.peek("darkshadow_active") == true)</cond>
  <flags>
  </flags>
 </event>
 --]]
--[[
-- Comm Event for the Shadow missions
--]]

require ("proximity.lua") 

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