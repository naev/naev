--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Achack04 Helper">
  <trigger>enter</trigger>
  <chance>5</chance>
  <cond>not player.misnActive("Sirian Truce") and not player.misnDone("Sirian Truce") and player.misnDone("Joanne's Doubt") and system.cur():presences()["Sirius"] ~= nil and (var.peek("achack04repeat") == nil or time.get() - time.fromnumber(var.peek("achack04repeat")) &gt; time.create(0, 30, 0))</cond>
  <flags>
  </flags>
 </event>
 --]]
--[[
-- This is a helper event for the fourth mission in the Academy Hack minor campaign.
--]]

function create()
   delay = rnd.rnd(10000, 40000) -- 10-40s
   hook.timer(delay, "startMission")
   hook.land("cleanup")
   hook.jumpout("cleanup")
end

function startMission()
   naev.missionStart("Sirian Truce")
   cleanup()
end

function cleanup()
   evt.finish()
end