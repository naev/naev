--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Meeting">
 <unique />
 <priority>3</priority>
 <done>Invitation</done>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Darkshed</spob>
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
local vn = require "vn"
local vntk = require "vntk"

local badguy, badguyprox -- Non-persistent state
local ambush -- Forward-declared functions

--Change here to change the planets and the systems
local mispla, missys = spob.getS("Curie")
local paypla, paysys = spob.getS("Darkshed")

function create ()
   if not misn.claim(missys) then
      misn.finish(false)
   end

   misn.setNPC(shark.arnold.name, shark.arnold.portrait, _([[He is waiting for you.]]))
end

function accept()
   mem.stage = 0
   mem.proba = 0.3  --the probability of ambushes will change
   mem.firstambush = true  --In the first ambush, there will be a little surprise text
   local accepted = false

   vn.clear()
   vn.scene()
   local arnold = vn.newCharacter( shark.vn_arnold() )
   vn.transition( shark.arnold.transition )

   arnold(fmt.f(_([["OK, are you ready for the travel to {pnt} in the {sys} system?"]]),
      {pnt=mispla, sys=missys}))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   arnold(_([["OK, come back when you are interested."]]))
   vn.done( shark.arnold.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   arnold(_([["Fantastic! I am known as Donald Ulnish to the Council member. Good luck."]]))

   vn.done( shark.arnold.transition )
   vn.run()

   if not accepted then return end

   misn.accept()

   misn.setTitle(_("The Meeting"))
   misn.setReward(shark.rewards.sh04)
   misn.setDesc(_("Nexus Shipyards asks you to take part in a secret meeting"))
   misn.osdCreate(_("The Meeting"), {
      fmt.f(_("Go to the {sys} system and land on {pnt}"), {sys=missys, pnt=mispla}),
      fmt.f(_("Bring Smith back to {pnt} in the {sys} system"), {pnt=paypla, sys=paysys}),
   })
   misn.osdActive(1)

   mem.marker = misn.markerAdd(mispla, "low")

   local c = commodity.new( N_("Smith"), N_("Arnold Smith of Nexus Shipyards.") )
   mem.smith = misn.cargoAdd( c, 0 )

   hook.land("land")
   hook.enter("enter")
end

function land()
   --The player is landing on the mission planet
   if mem.stage == 0 and spob.cur() == mispla then
      vn.clear()
      vn.scene()
      local arnold = vn.newCharacter( shark.vn_arnold() )
      vn.transition( shark.arnold.transition )
      arnold(_([[As you land, you see a group of people that were waiting for your ship. Smith hails them and tells you to wait in the ship while he goes to a private part of the bar.]]))
      arnold(_([[A few periods later, he comes back and explains that he wasn't able to improve Nexus sales in the Frontier, but he was able to stop House Sirius from entering the picture, at least.]]))
      vn.done( shark.arnold.transition )
      vn.run()
      mem.stage = 1
      misn.osdActive(2)
      misn.markerRm(mem.marker)
      mem.marker2 = misn.markerAdd(paypla, "low")

   --Job is done
   elseif mem.stage == 1 and spob.cur() == paypla then
      vn.clear()
      vn.scene()
      local arnold = vn.newCharacter( shark.vn_arnold() )
      vn.transition( shark.arnold.transition )
      arnold(_([[Smith gets out of your ship and looks at you, smiling. "You know, it's like that in our kind of job. Sometimes it works and sometimes it fails. It's not our fault. Anyway, here is your pay."]]))
      vn.func( function ()
         player.pay(shark.rewards.sh04)
         pir.reputationNormalMission(rnd.rnd(2,3))
      end )
      vn.sfxVictory()
      vn.na(fmt.reward(shark.rewards.sh04))
      vn.done( shark.arnold.transition )
      vn.run()

      shark.addLog( _([[You transported Arnold Smith to a secret meeting for Nexus Shipyards. The meeting supposedly did not go as well as he hoped, but was a partial success.]]) )
      misn.finish(true)
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
         vntk.msg(_("What is going on?"), _([[Suddenly, a Za'lek drone starts attacking you! As you wonder what to do, you hear a broadcast from a remote Za'lek ship. "Attention please, it seems some of our drones have gone haywire. If a drone attacks you and you aren't wanted by the authorities, you are hereby granted authorization to destroy it."]]))
         mem.firstambush = false
      end
      mem.proba = mem.proba - 0.1 * #badguy --processing the probability change
   end
end

function enabling()
   mem.enable = true
end
