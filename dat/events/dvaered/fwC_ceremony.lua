--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Strafer's Ceremony">
 <location>enter</location>
 <chance>50</chance>
 <cond>
  system.cur():faction() == faction.get("Dvaered") and
  player.misnDone("Dvaered Meeting") and
  var.peek("invasion_time") and
  not (player.misnDone("Dvaered Ballet") or player.misnActive("Dvaered Ballet")) and
  not (system.cur() == system.get("Dvaer"))
 </cond>
 <notes>
  <done_misn name="Dvaered Meeting"/>
  <campaign>Frontier Invasion</campaign>
 </notes>
</event>
--]]
--[[
-- Player receive an invitation for Strafer's ceremony
--]]

local timer

function create ()
   local invasion_time = var.peek("invasion_time")
   if time.get() < time.fromnumber(invasion_time) + time.new(0, 10, 0) then
      evt.finish()
   end

   timer      = hook.timer(20.0, "msgme")
   hook.land("finish")
   hook.jumpout("finish")
end

-- Player receives the message
function msgme()
   naev.missionStart("Dvaered Ballet")
   evt.finish()
end

function finish()
   hook.rm(timer)
   evt.finish()
end
