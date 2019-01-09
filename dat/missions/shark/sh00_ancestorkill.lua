--[[

   This is the first mission of the Shark's teeth campaign. The player has to kill a pirate ancestor with a shark.

   Stages :
   0) Way to Ulios in Ingot
   1) Taking off from Ulios and going to Toaxis
   2) Fight in Toaxis
   3) Pirate ran away
   4) Pirate was killed

--]]

--Needed scripts
include("dat/scripts/pilot/pirate.lua")
include("dat/scripts/numstring.lua")

title = {}
text = {}
osd_msg = {}
npc_desc = {}
bar_desc = {}

title[1] = _("Nexus Shipyards needs you")
text[1] = _([[You approach the man and he introduces himself. "Hello, my name is Arnold Smith; I work for Nexus Shipyards. I'm looking for a talented pilot to make a demonstration to one of our potential customers.
    "Pretty simple, really: we want someone to show how great Nexus ship designs are by destroying a Pirate Ancestor with our lowest-grade ship, the Shark. Of course, the pilot of the Ancestor has a bounty on his head, so it won't be illegal. The sum of the bounty will be paid to you and Nexus will add a little extra. Would you be interested?"]])

refusetitle = _("Sorry, not interested")
refusetext = _([["That's your choice," the man says. "Don't hesitate to tell me if you change your mind."]])

title[2] = _("Wonderful")
text[2] = _([["Great! I knew I could trust you. I'll meet you on %s in the %s system. I'll be with my boss and our customer, Baron Sauterfeldt."]])

title[3] = _("Ready for action")
text[3] = _([["Nice to see you again," the man says with a smile. "I hope you are ready to kick that pirate's ass! Please follow me. I will introduce you to my boss, the sales manager of Nexus Shipyards. Oh, and the Baron, too."
    The man guides you to some kind of control room where you see some important-looking people. After introducing you to some of them, he goes over the mission, rather over-emphasizing the threat involved; it's just a Pirate Ancestor, after all. Nonetheless, the Baron is intrigued.
    The man gets a call. After answering, he turns to you. "Perfect timing! The pirate has just arrived at %s. Now go show them what your ship can do!" Time to head back to the ship, then.]])

title[4] = _("Congratulations!")
text[4] = _([[As you step on the ground, Arnold Smith greets you. "That was a great demonstration! Thank you. I haven't been able to speak to the Baron about the results yet, but I am confident he will be impressed." He hands you your pay. "I may have another mission for you later. Be sure to check back!"]])

-- Mission details
misn_title = _("A Shark Bites")
misn_reward = _("%s credits")
misn_desc = _("Nexus Shipyards needs you to demonstrate to Baron Sauterfeldt the capabilities of Nexus designs.")

-- NPC
npc_desc[1] = _("A honest-looking man")
bar_desc[1] = _("This man looks like a honest citizen. He glances in your direction.")

npc_desc[2] = _("Arnold Smith")
bar_desc[2] = _([[The Nexus employee who recruited you for a very special demo of the "Shark" fighter.]])

-- OSD
osd_title = _("A Shark Bites")
osd_msg[1] = _("Buy a Shark (but not a Pirate Shark), then fly to the %s system and land on %s")
osd_msg[2] = _("Go to %s and kill the pirate with your Shark")
osd_msg[3] = _("Land on %s and collect your fee")

leave_msg = _("MISSION FAILED: You left the pirate.")
piratejump_msg = _("MISSION FAILED: The pirate ran away.")
noshark_msg = _("MISSION FAILED: You were supposed to use a Shark.")

function create ()

   --Change here to change the planet and the system
   sysname = "Ingot"
   planame = "Ulios"
   bsyname = "Toaxis"
   missys = system.get(sysname)
   mispla = planet.get(planame)
   battlesys = system.get(bsyname)

   if not misn.claim(battlesys) then
      misn.finish(false)
   end

   misn.setNPC(npc_desc[1], "neutral/male1")
   misn.setDesc(bar_desc[1])
end

function accept()

   stage = 0
   reward = 500000

   if tk.yesno(title[1], text[1]) then
      misn.accept()
      piratename = pirate_name()    --for now, we only need his name
      tk.msg(title[2], text[2]:format(missys:name(),mispla:name()))

      osd_msg[1] = osd_msg[1]:format(missys:name(), mispla:name())
      osd_msg[2] = osd_msg[2]:format(battlesys:name())
      osd_msg[3] = osd_msg[3]:format(mispla:name())

      misn.setTitle(misn_title)
      misn.setReward(misn_reward:format(numstring(reward)))
      misn.setDesc(misn_desc)
      osd = misn.osdCreate(osd_title, osd_msg)
      misn.osdActive(1)

      markeri = misn.markerAdd(missys, "low")

      jumpouthook = hook.jumpout("jumpout")
      landhook = hook.land("land")
      enterhook = hook.enter("enter")
   else
      tk.msg(refusetitle, refusetext)
      misn.finish(false)
   end
end

-- landing on any planet system
function land()

   -- Did the player reach Ulios ?
   if planet.cur() == mispla and stage == 0 then
      smith = misn.npcAdd("beginbattle", npc_desc[2], "neutral/male1", bar_desc[2])
   end

   -- Did the player land again on Ulios after having killed the pirate
   if planet.cur() == mispla and stage == 4 then
      tk.msg(title[4], text[4])
      player.pay(reward)
      misn.osdDestroy(osd)
      hook.rm(enterhook)
      hook.rm(landhook)
      hook.rm(jumpouthook)
      misn.finish(true)
   end
end

--jumping out the system
function jumpout()
   if stage == 2 then   --You were supposed to kill him, not to go away !
      player.msg( "\ar" .. leave_msg .. "\a0" )
      misn.finish(false)
   end
end

function enter()
   --Jumping in Toaxis for the battle
   if system.cur() == battlesys and stage == 1 then

      --Check if the player uses a Shark
      playership = player.pilot():ship()
      playershipname = playership:name()

      if playershipname ~= "Shark" and playershipname ~= "Empire Shark" then
         player.msg( "\ar" .. noshark_msg .. "\a0" )
         misn.finish(false)
      end

      --Be sure that nobody unexpected will take part in our epic battle
      pilot.clear()
      pilot.toggleSpawn(false)

      -- spawns the bad guy
      badboy = pilot.add( "Pirate Ancestor", nil, 0 )[1]
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

   tk.msg(title[3], text[3]:format(battlesys:name()))
   misn.osdActive(2)
   stage = 1

   marker1 = misn.markerAdd(battlesys, "low")
   player.takeoff()
end

function pirate_jump()  --he went away
   player.msg( "\ar" .. piratejump_msg .. "\a0" )
   misn.finish( false )
end

function pirate_dead()  --wou win
   stage = 4
   misn.markerRm(marker1)
   marker2 = misn.markerAdd(missys, "low")
   misn.osdActive(3)
end
