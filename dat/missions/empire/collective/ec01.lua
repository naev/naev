--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Collective Espionage 1">
 <unique />
 <priority>2</priority>
 <cond>faction.reputationGlobal("Empire") &gt; 5 and var.peek("collective_fail") ~= true</cond>
 <done>Collective Scouting</done>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Omega Enclave</spob>
 <notes>
  <campaign>Collective</campaign>
 </notes>
</mission>
--]]
--[[

   Collective Espionage I

   Author: bobbens
      minor edits by Infiltrator
      converted to VN framework by MageKing17

   Second mission in the mini collective campaign.

   You must inspect a system for wireless communications.

]]--
local fmt = require "format"
local emp = require "common.empire"

local vn = require "vn"

-- Mission constants
local misn_base, misn_base_sys = spob.getS("Omega Enclave")
local targsys1 = system.get("C-43")
local targsys2 = system.get("C-59")


function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( emp.dimitri.barname, emp.dimitri.portrait, _("You notice Lt. Commander Dimitri motioning for you to come over to him.") )
end


function accept ()
   local accepted = false

   vn.reset()
   vn.scene()
   local dimitri = vn.newCharacter(emp.vn_dimitri())
   vn.transition(emp.dimitri.transition)
   dimitri(_([[You meet up with Lt. Commander Dimitri.
"We managed to capture the drone after you located it. It didn't seem to be in good health. Our scientists are studying it as we speak, but we've found something strange in it. Some sort of weird wireless module. We'd like you to do a deep scan of the nearby Collective systems to see if you can pick up any strange wireless communications."]]))
   dimitri(_([[This will be a dangerous mission, because you'll need to stay in the system long enough for the scan to complete. I recommend a fast ship to outrun the drones. Are you interested in doing this now?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done(emp.dimitri.transition)

   vn.label("accept")
   vn.func(function () accepted = true end)
   dimitri(_([["You need to jump to each of the systems indicated on your map, and stay in the system until the scan finishes. If you jump out prematurely, you'll have to restart the scan from scratch when you return."]]))
   dimitri(_([["Of course, we're not sending you in unprepared. I have updated your ship's computer with a map of the Collective systems, at least the part we know about. I'm afraid it's not very complete intel, but it should be enough."]]))
   dimitri(_([["Like I said, it's best if you tried to avoid the drones, but if you think you can take them, go for it! Good luck."]]))
   vn.done(emp.dimitri.transition)
   vn.run()

   if not accepted then return end

   -- Accept mission
   misn.accept()

   mem.credits = emp.rewards.ec01

   mem.misn_stage = 0
   mem.systems_visited = 0 -- Number of Collective systems visited

   -- Mission details
   misn.setTitle(_("Collective Espionage"))
   misn.setReward( mem.credits )
   misn.setDesc(_("Scan the Collective systems for wireless communications"))
   mem.misn_marker1 = misn.markerAdd(targsys1, "low")
   mem.misn_marker2 = misn.markerAdd(targsys2, "low")
   misn.osdCreate(_("Collective Espionage"), {
      _("Scan the Collective systems for wireless communications"),
      fmt.f(_("Travel back to {pnt} in {sys}"), {pnt=misn_base, sys=misn_base_sys}),
   })

   player.outfitAdd("Map: Collective Space")

   hook.enter("enter")
   hook.land("land")
end


function enter()
    -- End any ongoing scans.
   if mem.scanning then
      hook.rm(mem.timerhook)
      player.omsgRm(mem.omsg)
      mem.scanning = false
   end

   if (system.cur() == targsys1 and not mem.sysdone1) or (system.cur() == targsys2 and not mem.sysdone2) then
      mem.scantime = 90 -- seconds
      mem.omsg = player.omsgAdd(fmt.f(_("Scanning... {seconds}s remaining."), {seconds=mem.scantime}), 0)
      mem.timerhook = hook.timer(1.0, "scantimer")
      mem.scanning = true
   end
end

function scantimer()
   mem.scantime = mem.scantime - 1
   if mem.scantime == 0 then
      player.omsgRm(mem.omsg)
      if system.cur() == targsys1 then
         misn.markerRm(mem.misn_marker1)
         mem.sysdone1 = true
      elseif system.cur() == targsys2 then
         misn.markerRm(mem.misn_marker2)
         mem.sysdone2 = true
      end
      mem.scanning = false

      if mem.sysdone1 and mem.sysdone2 then
         misn.osdActive(2)
         misn.markerAdd(misn_base, "low")
      end

      return
   end
   player.omsgChange(mem.omsg, fmt.f(_("Scanning... {seconds}s remaining."), {seconds=mem.scantime}), 0)
   mem.timerhook = hook.timer(1.0, "scantimer")
end

function land()
   if spob.cur() == misn_base and mem.sysdone1 and mem.sysdone2 then
      vn.reset()
      vn.scene()
      local dimitri = vn.newCharacter(emp.vn_dimitri())
      vn.transition(emp.dimitri.transition)
      vn.na(_([[After landing, Lt. Commander Dimitri greets you on the landing pad.]]))
      dimitri(_([["I suppose all went well? Those drones can really give a beating. We'll have the researchers start looking at your logs right away. Meet me in the bar again in a while."]]))
      vn.func(function ()
         faction.hit("Empire",35)
         player.pay(mem.credits)
      end)
      vn.sfxVictory()
      vn.na(fmt.reward(mem.credits))
      vn.done(emp.dimitri.transition)
      vn.run()

      emp.addCollectiveLog(fmt.f(_([[You helped gather intel on the Collective by scanning Collective systems. Lt. Commander Dimitri told you to meet him in the bar again on {pnt}.]]), {pnt=misn_base}))
      misn.finish(true)
   end
end
