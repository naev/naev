--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Strafer's Ceremony">
  <trigger>enter</trigger>
  <chance>50</chance>
  <cond>system.cur():faction() == faction.get("Dvaered") and player.misnDone("Dvaered Meeting") == true and (var.peek("invasion_time") == nil or time.get() &gt;= time.fromnumber(var.peek("invasion_time")) + time.create(0, 10, 0)) and not (player.misnDone("Dvaered Triathlon") or player.misnActive("Dvaered Triathlon")) and not (system.cur() == system.get("Dvaer"))</cond>
  <notes>
   <done_misn name="Dvaered Meeting"/>
   <campaign>Frontier Invasion</campaign>
  </notes>
 </event>
 --]]
--[[
-- Player recieve an invitation for Strafer's ceremony
--]]

function create ()
    timer = hook.timer(20000, "msgme")
    landhook = hook.land("finish")
    jumpouthook = hook.jumpout("finish")
end

-- Player recieves the message
function msgme()
    naev.missionStart("Dvaered Ballet")
    evt.finish()
end

function finish()
    hook.rm(timer)
    evt.finish()
end
