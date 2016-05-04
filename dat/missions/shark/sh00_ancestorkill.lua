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

lang = naev.lang()
if lang == "es" then
   else -- default english
   title = {}
   text = {}
   osd_msg = {}
   npc_desc = {}
   bar_desc = {}
   
   title[1] = "Nexus Shipyards needs you"
   text[1] = [[You approach the man and he introduces himself: "Hello, my name is Arnold Smith, I work for the Nexus Shipyards. I'm looking for a talented pilot to make a demonstration to one of our potential customers.
   Actually, this is more than a demonstration: if you accept, you will have to destroy a pirate Ancestor with only a Shark." The man shortly stops talking to look at your reaction. He then goes ahead: 
   "Of course, the pilot of the Ancestor has a bounty on his head, so it won't be a murder. The sum of the bounty will be paid to you and Nexus will add a little amount. Are you in?"]]
   
   refusetitle = "Sorry, not interested"
   refusetext = [["That's your choice," the man says. "Of course, there are other talented pilots in this galaxy, but I was quite sure, you were the perfect one for this job. Don't hesitate if you change your mind."]]
   
   title[2] = "Wonderful"
   text[2] = [[As you says that you accept the task, the man seems to relax. "Great! I knew I could trust you. I meet you on %s in the %s system. I'll be with my boss and our customer, the Baron Sauterfeldt."]]
   
   title[3] = "Ready for action"
   text[3] = [["Nice to see you again," the man says smiling. "I hope you are ready to kick some pirate's ass! Please follow me, I will introduce you to my boss, who is the sales manager of Nexus Shipyards and to the Baron."
   The man guides you out of the bar and to some kind of control room where you see some important-looking people. After being introduced to some of them, Arnold Smith takes you apart for a final briefing:
   "Remember that the Ancestor has much more firepower and sustainability than the Shark. You'll have to outmaneuver the foe and wait until he has launched his last rocket to become offensive."
   He then follows you to the spaceport. After a short waiting, he receives an holocall saying that the pirate just jumped in %s. Looks like it's time to take off.]]
   text[4] = [["Nice to see you again," the man says smiling. "I hope you are ready to kick some pirate's ass! Please follow me, I will introduce you to my boss, who is the sales manager of Nexus Shipyards and to the Baron."
   The man guides you out of the bar and to some kind of control room where you see some important-looking people. After being introduced to some of them, Arnold Smith takes you apart for a final briefing:
   "Remember that the Ancestor has much more firepower and sustainability than the Shark. You'll have to outmaneuver the foe and wait until he has launched his last rocket to become offensive."
   He then follows you to the spaceport. After a short waiting, he receives an holocall saying that the pirate just jumped in %s. Looks like it's time to take off.]]
   
   title[4] = "He ran away"
   text[5] = [[As you get out of your ship, Arnold Smith says you that as the pirate ran away, you didn't kill him. "Once we will have found another Pirate to kill, I will be back with my mission," he says.]]
   
   title[5] = "Congratulations!"
   text[6] = [[As you step on the ground, Arnold Smith comes to you: "Now that the Baron has seen how helpful a Shark can be in the hands of a good pilot, I have no doubts he will buy some to protect the %s-%s trade route".
   He then hands you your pay. "If we are again in need of a pilot, we will try to contact you again."]]
   
   title[6] = "You ran away!"
   text[7] = [[Your mission failed.]]
   
   title[7] = "It's time to take off"
   text[8] = [[The pirate just jumped in from %s!]]
   
   title[8] = "You need a Shark"
   text[9] = [[You receive a comm from %s: "You were supposed to use a Shark in this battle. You can't do this battle with an other ship." Your mission failed.]]
   
   -- Mission details
   misn_title = "A Shark Bites"
   misn_reward = "%s credits"
   misn_desc = "Nexus Shipyards needs you to demonstrate to Baron Sauterfeldt that a Shark is able to defend his system against pirates."
   
   -- NPC
   npc_desc[1] = "A honest-looking man"
   bar_desc[1] = "This man looks like a honest citizen and seems to try to draw your attention."
   
   npc_desc[2] = "Arnold Smith"
   bar_desc[2] = [[The Nexus employee who recruited you for a very special demo of the "Shark" fighter.]]
   
   -- OSD
   osd_title = "A Shark Bites"
   osd_msg[1] = "Buy a Shark, fly to the %s system and land on %s"
   osd_msg[2] = "Go to %s and kill the pirate with your Shark (and not an Empire Shark)"
   osd_msg[3] = "Land on %s and collect your fee"
   piratejump_msg = "The pirate ran away: land on %s"
end

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
      tk.msg(title[5], text[6]:format(battlesys:name(), missys:name()))
      player.pay(reward)
      misn.osdDestroy(osd)
      hook.rm(enterhook)
      hook.rm(landhook)
      hook.rm(jumpouthook)
      misn.finish(true)
   end
   if stage == 2 then   --You were supposed to kill him, not to go away !
      tk.msg(title[6], text[7])
      misn.finish(false)
   end
   if stage == 3 and planet.cur() == mispla then   --You were supposed to kill him, not to let him go !
      tk.msg(title[4], text[5])
      misn.finish(false)
   end
end

--jumping out the system
function jumpout()
   if stage == 2 then   --You were supposed to kill him, not to go away !
      tk.msg(title[6], text[7])
      misn.finish(false)
   end
end

function enter()
   --Jumping in Toaxis for the battle
   if system.cur() == battlesys and stage == 1 then
      
      --Check if the player uses a Shark
      playership = player.pilot():ship()
      playershipname = playership:name()   
      
      if playershipname ~= "Shark" then
         tk.msg(title[8], text[9]:format(mispla:name()))
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
   
   --Checking if the player and the Baron already met
   if player.misnDone("Baron") == true then
      tk.msg(title[3], text[3]:format(battlesys:name()))
      else
      tk.msg(title[3], text[4]:format(battlesys:name()))
   end
   misn.osdActive(2)
   stage = 1
   
   marker1 = misn.markerAdd(battlesys, "low")
   player.takeoff()
end

function pirate_jump()  --he went away
   stage = 3
   misn.markerRm(marker1)
   marker2 = misn.markerAdd(missys, "low")
   misn.osdCreate(misn_title, piratejump_msg:format(mispla:name()))
end

function pirate_dead()  --wou win
   stage = 4
   misn.markerRm(marker1)
   marker2 = misn.markerAdd(missys, "low")
   misn.osdActive(3)
end
