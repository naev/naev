--[[
   
   This is the second mission of the Shark's teeth campaign. The player has to take part to a fake battle.
   
   Stages :
   0) Way to Toaxis
   1) Battle
   2) Going to Darkshed
   
--]]

include "numstring.lua"

title = {}
text = {}
osd_msg = {}
npc_desc = {}
bar_desc = {}

title[1] = _("Nexus Shipyards needs you")
text[1] = _([[As you approach Smith, he recognizes you. "I have an other job for you. Let me explain it: the baron isn't convinced yet that the Shark is the fighter he needs. He saw your fight against the Ancestor, but he would like to see a Shark beating a destroyer class ship. Of course, Nexus Shipyards don't want to take the risk to make you face a destroyer with a Shark.
   So, we have an other plan: we need somebody to pretend to be an outlaw destroyer pilot and to let one of our men disable his ship. If you are ready for it, all you have to do is to jump in %s with a destroyer class ship and to let the Shark disable you. As there isn't any Empire bounty this time, you will only be paid %s credits.
   What do you say, are you in?" ]])
   
refusetitle = _("Sorry, not interested")
refusetext = _([["Ok, so never mind." Smith says.]])

title[2] = _("Wonderful")
text[2] = _([["I had no doubts you would accept," Smith says. "Go and meet our pilot in %s. After the job is done, meet me on %s in %s"]])

title[3] = _("Reward")
text[3] = _([[As you land, you see Arnold Smith who was waiting for you. He explains you that the baron was so impressed by the battle that he signed the contract. You made it possible for Nexus Shipyard to sell 20 Shark fighters. Of course, that's not a very big thing for a multi-stellar company like Nexus, but as a reward, they give you twice the sum of credits they promised to you.]])

title[4] = _("You ran away!")
title[5] = _("The shark is destroyed!")
title[6] = _("The shark ran away")
text[4] = _([[Your mission failed.]])

-- Mission details
misn_title = _("Sharkman is back")
misn_reward = _("%s credits")
misn_desc = _("Nexus Shipyards needs you to demonstrate again to Baron Sauterfeldt that a Shark is able to defend his system against pirates.")

-- NPC
npc_desc[1] = _("Arnold Smith")
bar_desc[1] = _([[The Nexus employee seems to be looking for pilots. Maybe he has an other task for you.]])

-- OSD
osd_title = _("Sharkman Is Back")
osd_msg[1] = _("Jump in %s with a destroyer class ship and let the shark disable you")
osd_msg[2] = _("Go to %s in %s to collect your pay")

function create ()
   
   --Change here to change the planet and the system
   bsyname = "Toaxis"
   psyname = "Alteris"
   pplname = "Darkshed"
   --System neighbouring Toaxis with zero pirate presence due to a "Virtual Pirate Unpresence" asset
   esyname = "Ingot"
   battlesys = system.get(bsyname)
   paysys = system.get(psyname)
   paypla = planet.get(pplname)
   escapesys = system.get(esyname)
   
   if not misn.claim(battlesys) then
      misn.finish(false)
   end
   
   misn.setNPC(npc_desc[1], "neutral/male1")
   misn.setDesc(bar_desc[1])
end

function accept()
   
   stage = 0 
   reward = 100000
   
   if tk.yesno(title[1], text[1]:format(battlesys:name(), numstring(reward/2))) then
      misn.accept()
      tk.msg(title[2], text[2]:format(battlesys:name(), paypla:name(), paysys:name()))
      
      osd_msg[1] = osd_msg[1]:format(battlesys:name())
      osd_msg[2] = osd_msg[2]:format(paypla:name(), paysys:name())
      
      misn.setTitle(misn_title)
      misn.setReward(misn_reward:format(numstring(reward/2)))
      misn.setDesc(misn_desc)
      osd = misn.osdCreate(osd_title, osd_msg)
      misn.osdActive(1)

      marker = misn.markerAdd(battlesys, "low")
      
      jumpouthook = hook.jumpout("jumpout")
      landhook = hook.land("land")
      enterhook = hook.enter("enter")
   else
      tk.msg(refusetitle, refusetext)
      misn.finish(false)
   end
end

function jumpout()
   if stage == 1 then --player trying to escape
      tk.msg(title[4], text[4])
      misn.finish(false)
   end
end

function land()
   if stage == 1 then --player trying to escape
      tk.msg(title[4], text[4])
      misn.finish(false)
   end
   if stage == 2 and planet.cur() == paypla then
      tk.msg(title[3], text[3])
      player.pay(reward)
      misn.osdDestroy(osd)
      hook.rm(enterhook)
      hook.rm(landhook)
      hook.rm(jumpouthook)
      misn.finish(true)
   end
end

function enter()
   
   playerclass = player.pilot():ship():class()
   --Jumping in Toaxis for the battle with a destroyer class ship
   if system.cur() == battlesys and stage == 0 and playerclass == "Destroyer" then
      
      -- spawns the Shark 
      sharkboy = pilot.addRaw( "Shark","baddie", nil, "Civilian" )
      sharkboy:setHostile()
      sharkboy:setHilight()
      
      --The shark becomes nice outfits
      sharkboy:rmOutfit("all")
      sharkboy:rmOutfit("cores")
      
      sharkboy:addOutfit("S&K Ultralight Combat Plating")
      sharkboy:addOutfit("Milspec Prometheus 2203 Core System")
      sharkboy:addOutfit("Tricon Zephyr Engine")
      
      sharkboy:addOutfit("Reactor Class I",2)
      sharkboy:addOutfit("Battery",2)
      
      sharkboy:addOutfit("Ion Cannon",3)-- The goal is to disable the player
      
      sharkboy:setHealth(100,100)
      sharkboy:setEnergy(100)
      sharkboy:setFuel(true)
      stage = 1
      
      hook.pilot( sharkboy, "death", "shark_dead" )
      hook.pilot( sharkboy, "jump", "shark_jump" )
      hook.pilot( player.pilot(), "disable", "disabled" )
   end
end

function shark_dead()  --you killed the shark
   tk.msg(title[5], text[4])
   misn.finish(false)
end

function shark_jump()  --the shark jumped away before having disabled the player
   if stage == 1 then
      tk.msg(title[6], text[4])
      misn.finish(false)
   end
end

function disabled(pilot, attacker)
   if attacker == sharkboy then
      stage = 2
      misn.osdActive(2)
      misn.markerRm(marker)
      marker2 = misn.markerAdd(paysys, "low")
   end
   sharkboy:control()
   --making sure the shark doesn't continue attacking the player and minimizing the chance of it being accidentally destroyed by pirates
   sharkboy:hyperspace(escapesys)
end
