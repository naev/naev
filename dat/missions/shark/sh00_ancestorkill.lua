--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="A Shark Bites">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>3</priority>
  <cond>planet.cur() ~= planet.getS("Ulios") and player.numOutfit("Mercenary License") &gt; 0</cond>
  <chance>5</chance>
  <location>Bar</location>
  <faction>Dvaered</faction>
  <faction>Empire</faction>
  <faction>Frontier</faction>
  <faction>Goddard</faction>
  <faction>Independent</faction>
  <faction>Sirius</faction>
  <faction>Soromid</faction>
  <faction>Traders Guild</faction>
  <faction>Za'lek</faction>
 </avail>
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
local shark = require "common.shark"

function create ()

   --Change here to change the planet and the system
   local sysname = "Ingot"
   local planame = "Ulios"
   local bsyname = "Toaxis"
   missys = system.get(sysname)
   mispla = planet.getS(planame)
   battlesys = system.get(bsyname)

   if not misn.claim(battlesys) then
      misn.finish(false)
   end

   misn.setNPC(_("An honest-looking man"), "neutral/unique/arnoldsmith.webp", _("This man looks like a honest citizen. He glances in your direction."))
end

function accept()

   stage = 0
   reward = 500e3

   if tk.yesno(_("Nexus Shipyards needs you"), _([[You approach the man and he introduces himself. "Hello, my name is Arnold Smith; I work for Nexus Shipyards. I'm looking for a talented pilot to make a demonstration to one of our potential customers.
    "Pretty simple, really: we want someone to show how great Nexus ship designs are by destroying a Pirate Ancestor with our lowest-grade ship, the Shark. Of course, the pilot of the Ancestor has a bounty on his head, so it won't be illegal. The sum of the bounty will be paid to you and Nexus will add a little extra. Would you be interested?"]])) then
      misn.accept()
      piratename = pilotname.pirate() --for now, we only need his name
      tk.msg(_("Wonderful"), _([["Great! I knew I could trust you. I'll meet you on %s in the %s system. I'll be with my boss and our customer, Baron Sauterfeldt."]]):format(missys:name(),mispla:name()))

      misn.setTitle(_("A Shark Bites"))
      misn.setReward(fmt.credits(reward))
      misn.setDesc(_("Nexus Shipyards needs you to demonstrate to Baron Sauterfeldt the capabilities of Nexus designs."))
      misn.osdCreate(_("A Shark Bites"), {
         _("Buy a Shark (but not a Pirate Shark), then fly to the %s system and land on %s"):format(missys:name(), mispla:name()),
         _("Go to %s and kill the pirate with your Shark"):format(battlesys:name()),
         _("Land on %s and collect your fee"):format(mispla:name()),
      })
      misn.osdActive(1)

      markeri = misn.markerAdd(missys, "low")

      jumpouthook = hook.jumpout("jumpout")
      landhook = hook.land("land")
      enterhook = hook.enter("enter")
   else
      tk.msg(_("Sorry, not interested"), _([["That's your choice," the man says. "Don't hesitate to tell me if you change your mind."]]))
      misn.finish(false)
   end
end

-- landing on any planet system
function land()

   -- Did the player reach Ulios ?
   if planet.cur() == mispla and stage == 0 then
      misn.npcAdd("beginbattle", _("Arnold Smith"), "neutral/unique/arnoldsmith.webp", _([[The Nexus employee who recruited you for a very special demo of the "Shark" fighter.]]))
   end

   -- Did the player land again on Ulios after having killed the pirate
   if planet.cur() == mispla and stage == 4 then
      tk.msg(_("Congratulations!"), _([[As you step on the ground, Arnold Smith greets you. "That was a great demonstration! Thank you. I haven't been able to speak to the Baron about the results yet, but I am confident he will be impressed." He hands you your pay. "I may have another mission for you later. Be sure to check back!"]]))
      player.pay(reward)
      misn.osdDestroy()
      hook.rm(enterhook)
      hook.rm(landhook)
      hook.rm(jumpouthook)
      shark.addLog( _([[You helped Nexus Shipyards demonstrate the capabilities of their ships by destroying a Pirate Ancestor.]]) )
      pir.reputationNormalMission(rnd.rnd(2,3))
      misn.finish(true)
   end
end

--jumping out the system
function jumpout()
   if stage == 2 then   --You were supposed to kill him, not to go away !
      player.msg( "#r" .. _("MISSION FAILED: You left the pirate.") .. "#0" )
      misn.finish(false)
   end
end

function enter()
   --Jumping in Toaxis for the battle
   if system.cur() == battlesys and stage == 1 then

      --Check if the player uses a Shark
      playership = player.pilot():ship()
      playershipname = playership:nameRaw()

      if playershipname ~= "Shark" and playershipname ~= "Empire Shark" then
         player.msg( "#r" .. _("MISSION FAILED: You were supposed to use a Shark.") .. "#0" )
         misn.finish(false)
      end

      --Be sure that nobody unexpected will take part in our epic battle
      pilot.clear()
      pilot.toggleSpawn(false)

      -- spawns the bad guy
      badboy = pilot.add( "Pirate Ancestor", shark.pirateFaction(), system.get("Raelid") )
      badboy:rename(piratename)
      badboy:setHostile()
      badboy:setVisplayer()
      badboy:setHilight()

      hook.pilot( badboy, "death", "pirate_dead" )
      hook.pilot( badboy, "jump", "pirate_jump" )
   end
end

--Chatting with Smith and begin the battle
function beginbattle()

   misn.markerRm(markeri)

   tk.msg(_("Ready for action"), _([["Nice to see you again," he says with a smile. "I hope you are ready to kick that pirate's ass! Please follow me. I will introduce you to my boss, the sales manager of Nexus Shipyards. Oh, and the Baron, too."
    Arnold Smith guides you to some kind of control room where you see some important-looking people. After introducing you to some of them, he goes over the mission, rather over-emphasizing the threat involved; it's just a Pirate Ancestor, after all. Nonetheless, the Baron is intrigued.
    Arnold Smith gets a call. After answering, he turns to you. "Perfect timing! The pirate has just arrived at %s. Now go show them what your ship can do!" Time to head back to the ship, then.]]):format(battlesys:name()))
   misn.osdActive(2)
   stage = 1

   marker1 = misn.markerAdd(battlesys, "low")
   player.takeoff()
end

function pirate_jump()  --he went away
   player.msg( "#r" .. _("MISSION FAILED: The pirate ran away.") .. "#0" )
   misn.finish( false )
end

function pirate_dead()  --wou win
   stage = 4
   misn.markerRm(marker1)
   marker2 = misn.markerAdd(missys, "low")
   misn.osdActive(3)
end
