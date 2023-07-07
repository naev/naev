--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="A Shark Bites">
 <unique />
 <priority>3</priority>
 <cond>
   if spob.cur() == spob.get("Ulios") then
      return false
   end
   local misn_test = require "misn_test"
   if not misn_test.mercenary() then
      return false
   end
   return misn_test.reweight_active()
 </cond>
 <chance>5</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Traders Society</faction>
 <faction>Za'lek</faction>
 <notes>
  <campaign>Nexus show their teeth</campaign>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[
   This is the first mission of the Shark's teeth campaign. The player has to kill a pirate ancestor with a shark.

   Stages :
   0) Way to Ulios in Ingot
   1) Taking off from Ulios and going to Toaxis
   2) Fight in Toaxis
   3) Pirate ran away
   4) Pirate was killed
--]]
local pir = require "common.pirate"
local pilotname = require "pilotname"
local fmt = require "format"
local lmisn = require "lmisn"
local shark = require "common.shark"
local vn = require "vn"

--Change here to change the planet and the system
local missys = system.get("Ingot")
local mispla = spob.get("Ulios")
local battlesys = system.get("Toaxis")

function create ()
   if not misn.claim(battlesys) then
      misn.finish(false)
   end

   misn.setNPC(_("An honest-looking man"), shark.arnold.portrait, _("This man looks like a honest citizen. He glances in your direction."))
end

function accept()
   mem.stage = 0
   local accepted = false

   vn.clear()
   vn.scene()
   local arnold = vn.newCharacter( shark.vn_arnold() )
   vn.transition( shark.arnold.transition )

   arnold(_([[You approach the man and he introduces himself. "Hello, my name is Arnold Smith; I work for Nexus Shipyards. I'm looking for a talented pilot to make a demonstration to one of our potential customers.]]))
   arnold(_([["Pretty simple, really: we want someone to show how great Nexus ship designs are by destroying a Pirate Ancestor with our lowest-grade ship, the Shark. Of course, the pilot of the Ancestor has a bounty on his head, so it won't be illegal. The sum of the bounty will be paid to you and Nexus will add a little extra. Would you be interested?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   arnold(_([["That's your choice," the man says. "Don't hesitate to tell me if you change your mind."]]))
   vn.done( shark.arnold.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   arnold(fmt.f(_([["Great! I knew I could trust you. I'll meet you on {pnt} in the {sys} system. I'll be with my boss and our customer, Baron Sauterfeldt."]]),
      {sys=missys, pnt=mispla}))

   vn.done( shark.arnold.transition )
   vn.run()

   if not accepted then return end

   misn.accept()
   mem.piratename = pilotname.pirate() --for now, we only need his name

   misn.setTitle(_("A Shark Bites"))
   misn.setReward(shark.rewards.sh00)
   misn.setDesc(_("Nexus Shipyards needs you to demonstrate to Baron Sauterfeldt the capabilities of Nexus designs."))
   misn.osdCreate(_("A Shark Bites"), {
      fmt.f(_("Buy a Shark (but not a Pirate Shark), then fly to the {sys} system and land on {pnt}"), {sys=missys, pnt=mispla}),
      fmt.f(_("Go to {sys} and kill the pirate with your Shark"), {sys=battlesys}),
      fmt.f(_("Land on {pnt} and collect your fee"), {pnt=mispla}),
   })
   misn.osdActive(1)

   mem.markeri = misn.markerAdd(mispla, "low")

   hook.jumpout("jumpout")
   hook.land("land")
   hook.enter("enter")
end

-- landing on any planet system
function land()

   -- Did the player reach Ulios ?
   if spob.cur() == mispla and mem.stage == 0 then
      misn.npcAdd("beginbattle", shark.arnold.name, shark.arnold.portrait, _([[The Nexus employee who recruited you for a very special demo of the "Shark" fighter.]]))
   end

   -- Did the player land again on Ulios after having killed the pirate
   if spob.cur() == mispla and mem.stage == 4 then
      vn.clear()
      vn.scene()
      local arnold = vn.newCharacter( shark.vn_arnold() )
      vn.transition( shark.arnold.transition )

      arnold(_([[As you step on the ground, Arnold Smith greets you. "That was a great demonstration! Thank you. I haven't been able to speak to the Baron about the results yet, but I am confident he will be impressed." He hands you your pay. "I may have another mission for you later. Be sure to check back!"]]))

      vn.func( function ()
         player.pay(shark.rewards.sh00)
         pir.reputationNormalMission(rnd.rnd(2,3))
      end )
      vn.sfxVictory()
      vn.na(fmt.reward(shark.rewards.sh00))

      vn.done( shark.arnold.transition )
      vn.run()

      shark.addLog( _([[You helped Nexus Shipyards demonstrate the capabilities of their ships by destroying a Pirate Ancestor.]]) )
      misn.finish(true)
   end
end

--jumping out the system
function jumpout()
   if mem.stage == 2 then   --You were supposed to kill him, not to go away !
      lmisn.fail( _("You left the pirate.") )
   end
end

function enter()
   --Jumping in Toaxis for the battle
   if system.cur() == battlesys and mem.stage == 1 then
      --Check if the player uses a Shark
      local playership = player.pilot():ship()
      local playershipname = playership:nameRaw()

      if playershipname ~= "Shark" and playershipname ~= "Empire Shark" then
         lmisn.fail( _("You were supposed to use a Shark.") )
      end

      --Be sure that nobody unexpected will take part in our epic battle
      pilot.clear()
      pilot.toggleSpawn(false)

      -- spawns the bad guy
      local badboy = pilot.add( "Pirate Ancestor", shark.pirateFaction(), system.get("Zacron"), mem.piratename )
      badboy:setHostile()
      badboy:setVisplayer()
      badboy:setHilight()

      hook.pilot( badboy, "death", "pirate_dead" )
      hook.pilot( badboy, "jump", "pirate_jump" )
   end
end

--Chatting with Smith and begin the battle
function beginbattle()
   misn.markerRm(mem.markeri)

   vn.clear()
   vn.scene()
   local arnold = vn.newCharacter( shark.vn_arnold() )
   vn.transition( shark.arnold.transition )

   arnold(_([["Nice to see you again," he says with a smile. "I hope you are ready to kick that pirate's ass! Please follow me. I will introduce you to my boss, the sales manager of Nexus Shipyards. Oh, and the Baron, too."]]))
   vn.na(_([[Arnold Smith guides you to some kind of control room where you see some important-looking people. After introducing you to some of them, he goes over the mission, rather over-emphasizing the threat involved; it's just a Pirate Ancestor, after all. Nonetheless, the Baron is intrigued.]]))
   arnold(fmt.f(_([[Arnold Smith gets a call. After answering, he turns to you. "Perfect timing! The pirate has just arrived at {sys}. Now go show them what your ship can do!" Time to head back to the ship, then.]]),
      {sys=battlesys}))

   vn.done( shark.arnold.transition )
   vn.run()

   misn.osdActive(2)
   mem.stage = 1

   mem.marker1 = misn.markerAdd(battlesys, "low")
   player.takeoff()
end

function pirate_jump()  --he went away
   lmisn.fail( _("The pirate ran away.") )
end

function pirate_dead()  --wou win
   mem.stage = 4
   misn.markerRm(mem.marker1)
   mem.marker2 = misn.markerAdd(mispla, "low")
   misn.osdActive(3)
   pilot.toggleSpawn()
end
