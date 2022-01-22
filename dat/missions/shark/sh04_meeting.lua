--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Meeting">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>3</priority>
  <done>Invitation</done>
  <chance>100</chance>
  <location>Bar</location>
  <spob>Darkshed</spob>
 </avail>
 <notes>
  <campaign>Nexus show their teeth</campaign>
 </notes>
</mission>
--]]
--[[
   This is the fifth mission of the Shark's teeth campaign. The player has to go to a planet in Za'lek space.

   Stages :
   0) Way to Za'lek system
   1) Way back to Darkshed

   TODO: I'm not really happy with the drone's behaviour: it's quite too obvious
--]]
local pir = require "common.pirate"
require "proximity"
local fmt = require "format"
local fleet = require "fleet"
local shark = require "common.shark"

local badguy, badguyprox -- Non-persistent state
local ambush -- Forward-declared functions
-- luacheck: globals enabling enter land reveal (Hook functions passed by name)

--Change here to change the planets and the systems
local mispla, missys = spob.getS("Curie")
local paypla, paysys = spob.getS("Darkshed")

function create ()
   if not misn.claim(missys) then
      misn.finish(false)
   end

   misn.setNPC(_("Arnold Smith"), "neutral/unique/arnoldsmith.webp", _([[He is waiting for you.]]))
end

function accept()
   mem.stage = 0
   mem.proba = 0.3  --the probability of ambushes will change
   mem.firstambush = true  --In the first ambush, there will be a little surprise text

   if tk.yesno(_("Travel"), fmt.f(_([["OK, are you ready for the travel to {pnt} in the {sys} system?"]]), {pnt=mispla, sys=missys})) then
      misn.accept()
      tk.msg(_("Time to go"), _([["Let's go, then."]]))

      misn.setTitle(_("The Meeting"))
      misn.setReward(fmt.credits(shark.rewards.sh04))
      misn.setDesc(_("Nexus Shipyards asks you to take part in a secret meeting"))
      misn.osdCreate(_("The Meeting"), {
         fmt.f(_("Go to the {sys} system and land on {pnt}"), {sys=missys, pnt=mispla}),
         fmt.f(_("Bring Smith back to {pnt} in the {sys} system"), {pnt=paypla, sys=paysys}),
      })
      misn.osdActive(1)

      mem.marker = misn.markerAdd(mispla, "low")

      local c = commodity.new( N_("Smith"), N_("Arnold Smith of Nexus Shipyards.") )
      mem.smith = misn.cargoAdd( c, 0 )

      mem.landhook = hook.land("land")
      mem.enterhook = hook.enter("enter")
      else
      tk.msg(_("Sorry, not interested"), _([["OK, come back when you are ready."]]))
      misn.finish(false)
   end
end

function land()
   --The player is landing on the mission planet
   if mem.stage == 0 and spob.cur() == mispla then
      tk.msg(_("The meeting"), _([[As you land, you see a group of people that were waiting for your ship. Smith hails them and tells you to wait in the ship while he goes to a private part of the bar.
    A few periods later, he comes back and explains that he wasn't able to improve Nexus sales in the Frontier, but he was able to stop House Sirius from entering the picture, at least.]]))
      mem.stage = 1
      misn.osdActive(2)
      misn.markerRm(mem.marker)
      mem.marker2 = misn.markerAdd(paypla, "low")
   end

   --Job is done
   if mem.stage == 1 and spob.cur() == paypla then
      if misn.cargoRm(mem.smith) then
         tk.msg(_("End of mission"), _([[Smith gets out of your ship and looks at you, smiling. "You know, it's like that in our kind of job. Sometimes it works and sometimes it fails. It's not our fault. Anyway, here is your pay."]]))
         player.pay(shark.rewards.sh04)
         pir.reputationNormalMission(rnd.rnd(2,3))
         misn.osdDestroy()
         hook.rm(mem.enterhook)
         hook.rm(mem.landhook)
         shark.addLog( _([[You transported Arnold Smith to a secret meeting for Nexus Shipyards. The meeting supposedly did not go as well as he hoped, but was a partial success.]]) )
         misn.finish(true)
      end
   end
end

function enter()
   --This timer will ensure that the hacked drones don't reveal themselves during the jumping
   mem.enable = false
   hook.timer(5.0,"enabling")
   -- Ambush !
   if system.cur():presence(faction.get("Za'lek")) > 50 then  -- Only in Za'lek space
      if mem.stage == 0 and rnd.rnd() < mem.proba then
         ambush()
      else
         --the probability of an ambush grows up when you cross a system without meeting any enemy
         mem.proba = mem.proba + 0.2
      end
   end
end

function ambush()
   -- Adds the drones.
   badguy = fleet.add(4, {"Za'lek Light Drone", "Za'lek Heavy Drone", "Za'lek Bomber Drone"}, "Za'lek", nil, "collective")
   badguyprox = {}

   for i, j in ipairs(badguy) do
      --Makes the drones follow the player
      j:control()
      j:follow(player.pilot())

      --as the player approaches, the drones reveal to be bad guys!
      badguyprox[i] = hook.timer(0.5, "proximity", {anchor = j, radius = 1000, funcname = "reveal"})
   end
end

function abort()
   misn.cargoRm(mem.smith)
   misn.finish(false)
end

function reveal()  --transforms the spawn drones into baddies
   if mem.enable == true then  --only if this happens a few time after the jumping/taking off
      for i, j in ipairs(badguy) do
         if j:exists() then
            j:rename(_("Hacked Drone"))
            j:setHostile()
            j:setFaction("Mercenary")
            j:control(false)
            hook.rm(badguyprox[i]) -- Only one drone needs to trigger this function.
         end
      end
      if mem.firstambush == true then
         --Surprise message
         tk.msg(_("What is going on?"), _([[Suddenly, a Za'lek drone starts attacking you! As you wonder what to do, you hear a broadcast from a remote Za'lek ship. "Attention please, it seems some of our drones have gone haywire. If a drone attacks you and you aren't wanted by the authorities, you are hereby granted authorization to destroy it."]]))
         mem.firstambush = false
      end
      mem.proba = mem.proba - 0.1 * #badguy --processing the probability change
   end
end

function enabling()
   mem.enable = true
end
