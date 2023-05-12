--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Shadowcomm2">
 <location>enter</location>
 <chance>3</chance>
 <cond>system.cur():presence("hostile") &lt; 300 and player.misnDone("Shadow Vigil") and not (player.misnDone("Dark Shadow") or var.peek("darkshadow_active") == true) and system.cur():presence("Independent") &gt; 100</cond>
 <notes>
  <done_misn name="Shadow Vigil"/>
  <campaign>Shadow</campaign>
 </notes>
</event>
--]]
--[[
   Comm Event for the Shadow missions
--]]
require "proximity"


function create ()
   -- Make sure system isn't claimed, but we don't claim it
   if not naev.claimTest( system.cur() ) then evt.finish() end

   hook.timer(20.0, "hailme")
   hook.land("finish")
   hook.jumpout("finish")
end

-- Make the ship hail the player
function hailme()
   naev.missionStart("Dark Shadow")
   player.commClose()
   evt.finish()
end

function finish()
   evt.finish()
end
