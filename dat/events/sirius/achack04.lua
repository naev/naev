--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Achack04 Helper">
 <location>enter</location>
 <chance>10</chance>
 <cond>
   if player.misnActive("Sirian Truce") or player.misnDone("Sirian Truce") then
      return false
   end
   if not player.misnDone("Joanne's Doubt") then
      return false
   end
   local pres = system.cur():presences()["Sirius"] or 0
   if pres &lt; 50 then
      return false
   end
   return true
 </cond>
 <notes>
  <done_misn name="Joanne's Doubt"/>
  <campaign>Academy Hack</campaign>
 </notes>
</event>
--]]
--[[
   This is a helper event for the fourth mission in the Academy Hack minor campaign.
--]]
function create()
   -- Don't want to interrupt anything important
   if not naev.claimTest{system.cur(),true} then
      evt.finish()
   end

   local delay = rnd.uniform(10, 40)
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
