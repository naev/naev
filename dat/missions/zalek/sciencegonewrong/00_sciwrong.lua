--[[
   Mission: The one with the Shopping

   Description: A Za'lek scientist asks the player to fetch some raw material that he needs to build his prototype.
   Multiple systems have to be visited and materials acquired from contact persons in the bar. 

   Difficulty: Easy to Medium? Depending on whether the player lingers around too much and gets caught.

   Author: fart but based on Mission Ideas in wiki: wiki.naev.org/wiki/Mission_Ideas
--]]

require "numstring.lua"
require "proximity.lua"
require "fleethelper.lua"
require "dat/missions/zalek/common.lua"


-- set mission variables
t_sys = {}
t_pla = {}
t_sys[1] = "Daravon"
t_pla[1] = "Vilati Vilata"
t_sys[2] = "Waterhole"
t_pla[2] = "Waterhole's Moon"
-- t_x[3] is empty bc it depends on where the mission will start finally. (To be set in mission.xml and then adjusted in the following campaign missions)
t_sys[3] = "Seiben"
t_pla[3] = "Gastan"
pho_mny = 50000
reward = 1000000 
title = {}
text = {}
osd_msg = {}
-- set text variables
-- Mission details
misn_title = _("The one with the Shopping")
misn_reward = _("The gratitude of science and a bit of compensation")
misn_desc = _("You've been hired by Dr. Geller to collect some materials he urgently needs for his research.")
bar_desc = _("You see a scientist talking to various pilots. Perhaps you should see what he's looking for.")
trd1_desc = _("A scientist conspicuously sits in the corner. Perhaps he might be ther person you're supposed to get this stuff for ")
trd2_desc = _("You see a shifty-looking dealer of some kind. Maybe he has what you're looking for.")

title[1] = _([[In the bar]])
text[1]  = _([["Oh, hello! You look like you're a pilot; is that right? I've got a job for you. Allow me to introduce myself; my name is Dr. Geller, and I am on the brink of revolutionizing science! I've basically already done it; there's just some minor fiddling to do. Would you like to help me out? I just need you to find some samples that I can study."]])
text[2] = _([["Excellent! Here is the list." He hands you a memory chip and turns away even before you can say anything and without giving you any cash to actually do his shopping. Once you check the list you find that it contains not only a list of materials he needs, but also information where to retrieve these and a list of contact traders.]])
text[3] = _([["With what can I help you, my friend?" says the shifty figure. You hand him the memory chip the scientist handed you.]])
-- dialogue with first trader
text[4] = _([["Of course I have what you need. I'll trade it for some 3t phosphine. You can find it on %s in the %s system."]])
text[6] = _([["Ah, yes indeed," he says as he inspects a sample in front of him. "That will do. And here, as promised: a piece of a ghost ship from the nebula. 100% authentic! At least, according to my supplier."]])
text[7] = _([["What are you still doing here? No phosphine, no trade."]])
-- dialogue with 2nd trader
-- %s for pho_mny
text[8] = _([["You approach the dealer and explain what you are looking for. He raises his eyebrow. "It will be %s credits. But if you get caught by the authorities, you're on your own. Far as I'm concerned I never saw you. Deal?"]])
mnytitle = _([[In the bar]])
mnytext = _([["You don't have enough money. Stop wasting my time."]])
text[9] = _([["Pleasure to do business with you."]])
text[10] = _([["Then we have nothing to to discuss."]])
-- call of the police
title[2] = _([[On the intercom]])
text[11] = _([["We have reason to believe you are carrying controlled substances without a proper license. Please stop your ship and prepare to be boarded."]])
text[12] = _([["Stand down for inspection."]])
title[3] = _([[On your ship]])
text[13] = _([["You are accused of violating regulation on the transport of toxic materials. Your ship will be searched now. If there are no contraband substances, we will be out of your hair in just a moment."]])
text[14] = _([[The inspectors search through your ship and cargo hold. It doesn't take long for them to find the phosphine; they confiscate it and fine you %s credits.]])
text[15] = _([[Dr. Geller looks up at you as you approach. "Do you have what I was looking for?" You present the ghost ship piece and his face begins to glow. "Yes, that's it! Now I can continue my research. I've been looking everywhere for a sample!" You ask him about the so-called ghost ships. He seems amused by the question. "Some people believe in ridiculous nonsense related to this. There is no scientific explanation for the origin of these so-called ghost ships yet, but I think it has to do with some technology involved in the Incident. Hard to say exactly what, but hey, that's why we do research!"]])
text[16] = _([[As he turns away, you audibly clear your throat, prompting him to turn back to you. "Oh, yes, of course you want some payment for your service. My apologies for forgetting." He hands you a credit chip with your payment. "I might need your services again in the future, so do stay in touch!"]])
trd_disc = _([[This guy seems to be the trader, surrounded by bodyguards he looks a bit shifty.]])
-- osd_msg
osd_msg[1] = _("Go to the %s system and talk to the trader on %s")
osd_msg[2] = _("Go to the %s system and talk to the contact person on %s")
osd_msg[3] = _("Return to the %s system to the trader on %s")
osd_msg[4] = _("Return to the %s system and deliver to Dr. Geller on %s")
-- refusetext 
refusetitle = _("No Science Today")
refusetext = _("I guess you don't care for science...")

log_text = _([[You helped Dr. Geller obtain a "ghost ship piece" for his research. When you asked about these so-called ghost ships, he seemed amused. "Some people believe in ridiculous nonsense related to this. There is no scientific explanation for the origin of these so-called ghost ships yet, but I think it has to do with some technology involved in the Incident. Hard to say exactly what, but hey, that's why we do research!"]])


function create ()
   -- Spaceport bar stuff
   misn.setNPC( _("A scientist"), "zalek/unique/geller" )
   misn.setDesc( bar_desc )
end
function accept()
   -- Mission details:
   if not tk.yesno( title[1], text[1] ) then
      tk.msg(refusetitle, refusetext)
      misn.finish()
   end
   tk.msg( title[1], text[2] )
   misn.accept()
   misn.osdCreate(misn_title, {osd_msg[1]:format(t_sys[1], t_pla[1])})
   misn.setDesc(misn_desc)
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn_mark = misn.markerAdd( system.get(t_sys[1]), "high" )
   talked = false
   lhook1 =  hook.land("land1", "land")
end

function land1()
   if planet.cur() == planet.get(t_pla[1]) and not talked and not traded then
      bar1pir1 = misn.npcAdd("first_trd", _("Trader"), "neutral/scientist3", trd1_desc)
   elseif planet.cur() == planet.get(t_pla[1]) and talked and traded1 then
      bar1pir1 = misn.npcAdd("third_trd", _("Trader"), "neutral/scientist3", trd1_desc)
   end
end

function land2()
   if planet.cur() == planet.get(t_pla[2]) and talked and not traded1 then
      bar2pir1 = misn.npcAdd("second_trd", _("Contact Person"), "neutral/unique/dealer", trd2_desc)
   end
end

-- first trade: send player 2 2nd system, if he goes back here, tell them to get going...
function first_trd()
  if talked then
     tk.msg(title[1], text[7])
     return
  else
     tk.msg(title[1], text[3])
     tk.msg(title[1], text[4]:format(t_pla[2]:name(), t_sys[2]:name()))
     talked = true
  end
  
  
  osd_msg[2] = osd_msg[2]:format(t_sys[2], t_pla[2])
  misn.osdCreate(misn_title, {osd_msg[2]})
  
  misn.markerMove(misn_mark, system.get(t_sys[2]))
  
  lhook2 = hook.land("land2", "land")
  
end
-- 2nd trade: Get player the stuff and make them pay, let them be hunted by the police squad
function second_trd()
  misn.npcRm(bar2pir1)
  if not tk.yesno( title[1], text[8]:format(numstring(pho_mny)) ) then
     tk.msg(title[1], text[10])
     return
  end
  -- take money from player, if player does not have the money, refuse
  if player.credits() < 50000 then
     tk.msg(mnytitle, mnytext)
     return
  end
  player.pay(-pho_mny)
  carg_id = misn.cargoAdd("Phosphine", 3)
  tk.msg(title[1], text[9])
  osd_msg[3] = osd_msg[3]:format(t_sys[1],t_pla[1])
  misn.osdCreate(misn_title, {osd_msg[3]})
 -- create hook that player will be hailed by authorities bc of toxic materials 
  misn.markerMove(misn_mark, system.get(t_sys[1]))
  hook.rm(lhook2)
  hook.enter("sys_enter")
  traded1 = true
end

-- 3rd trade: Get the stuff the scientist wants
function third_trd()
  tk.msg(title[1], text[6])
  
  misn.npcRm(bar1pir1)
  misn.cargoRm(carg_id)
  player.msg(t_sys[3])
  osd_msg[4] = osd_msg[4]:format(t_sys[3],t_pla[3])
  misn.osdCreate(misn_title, {osd_msg[4]})
  
  misn.markerMove(misn_mark, system.get(t_sys[3]))
  
  hook.rm(lhook1)
  
  traded2 = true
  lhook1 = hook.land("fnl_ld", "land")
end

-- final land: let the player land and collect the reward
function fnl_ld ()
   if planet.cur() == planet.get(t_pla[3]) and traded2 then
      tk.msg(title[1],text[15])
      tk.msg(title[1],text[16])
      player.pay(reward)
      zlk_addSciWrongLog( log_text )
      misn.finish(true)
   end
end
-- when the player takes off the authorities will want them
function sys_enter ()
   if system.cur() == system.get(t_sys[2]) then
      hook.timer(7000, "call_the_police")
   end
end
function call_the_police ()
   spwn_police()
   tk.msg(title[2],text[11])
   tk.msg(title[2],text[12])
   boardh = hook.pilot(player.pilot(), "disable", "go_board")
end

function spwn_police ()
      lance1 = pilot.add("Empire Lancelot", "empire", system.get("Provectus Nova"), "Empire")
      lance1 = lance1[1]
      lance2 = pilot.add("Empire Lancelot", "empire", system.get("Provectus Nova"), "Empire")
      lance2 = lance2[1]
      adm1 = pilot.add("Empire Admonisher", "empire", system.get("Provectus Nova"), "Empire")
      adm1 = adm1[1]
      -- Re-outfit the ships to use disable weapons. Make a proper function for that.
      lance1:rmOutfit("all")
      lance1:addOutfit("Heavy Ion Cannon", 1)
      lance1:addOutfit("Ion Cannon", 1)
      lance1:addOutfit("TeraCom Medusa Launcher", 2)
      lance2:rmOutfit("all")
      lance2:addOutfit("Heavy Ion Cannon", 1)
      lance2:addOutfit("Ion Cannon", 1)
      lance2:addOutfit("TeraCom Medusa Launcher", 2)

      adm1:rmOutfit("all")
      adm1:addOutfit("Heavy Ion Turret", 2)
      adm1:addOutfit("EMP Grenade Launcher", 1)
      adm1:addOutfit("Ion Cannon", 2)
      lance1:setHostile(true)
      lance2:setHostile(true)
      adm1:setHostile(true)
end
-- move to the player ship 

function go_board ()
   if adm1 then
      adm1:control()
      adm1:setHostile(false)
      adm1:goto(player.pos())
      hook.pilot(adm1, "idle", "fine_vanish")
   end
   if lance1 then
      lance1:control()
      lance1:setHostile(false)
      lance1:goto(player.pos())
      hook.pilot(lance1, "idle", "fine_vanish")
   end
   if lance2 then
      lance2:control()
      lance2:setHostile(false)
      lance2:goto(player.pos())
      hook.pilot(lance2, "idle", "fine_vanish")
   end
end
-- display msgs and have the ships disappear and fail the mission...
function fine_vanish ()
   fine = 100000
   tk.msg(title[3],text[13])
   tk.msg(title[3],text[14]:format(numstring(fine)))
   if player.credits() > fine then
      player.pay(-fine)
   else
      player.pay(-player.credits())
   end
   misn.cargoRm(carg_id)
   if adm1 then
      adm1:hyperspace()
   end
   if lance1 then
      lance1:hyperspace()
   end
   if lance2 then
      lance2:hyperspace()
   end
   player.msgClear()
   player.msg(_("Mission failed!"))
   hook.rm(l1ho)
   hook.rm(l2ho)
   hook.rm(admho)
   misn.finish(false)
end
