--[[
   Mission: The one with the Shopping

   Description: A Za'lek scientist asks the player to fetch some raw material that he needs to build his prototype.
   Multiple systems have to be visited and materials acquired from contact persons in the bar. 

   Difficulty: Easy to Medium? Depending on whether the player lingers around too much and gets caught.

   Author: fart but based on Mission Ideas in wiki: wiki.naev.org/wiki/Mission_Ideas
--]]

include "proximity.lua"
include "fleethelper.lua"
include "dat/missions/empire/common.lua"

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
-- player makes maximum 490k+, minimum 300k
pho_mny = 50000
reward = pho_mny+300000+rnd.rnd(3,4)*100000+rnd.rnd(10)*1000 
title = {}
text = {}
osd_msg = {}
-- set text variables
-- Mission details
misn_title = _("The one with the Shopping")
misn_reward = _("The gratitude of science and a bit of compensation")
misn_desc = _("You've been hired by Dr. Geller to collect some materials he urgently needs to build his prototype.")
bar_desc = _("You see this scientist talking to various pilots. He looks at you with a measuring look.")
trd1_desc = _("This seems to be a regular trader with some bodyguards. Business is dangerous nowadays...")
trd2_desc = _("This trader appears to be a bit shifty... no doubt it is the person you are looking for")

title[1] = _([[In the bar]])
text[1]  = _([["Hey you! You are a pilot right? I've got a job for you. You see I am on the brink of revolutionizing science! I basically already did it, there is just some minor fiddling to do. According to the Za'lek council our theories have to be tested. No faith in my abilities... but I will show them! They will learn not to question Dr. Geller. Hah! Anyways. I have some minor errands that I need you to run for me, just some shopping, are you in?]])
text[2] = _([["Excellent, here is the list" he hands you a memory chip and turns away even before you can say anything and without giving you any cash to actually do his shopping... Once you check the list you find that it contains not only a list of materials he needs, but also information where to retrieve these and a list of contact traders.]])
text[3] = _([["With what can I help you, my friend?" says the shifty figure, and you hand him the chip the scientist handed you.]])
-- dialogue with first trader
text[4] = _([["Of course I have what you need, but do you have what I need?"]])
text[5] = _([["No? Well then come back if you can offer me 3t Phosphine you can find it on %s" Phosphine? You ponder... you once knew the reason why that was hard to come by.]])
text[6] = _([["Ah yes, indeed" he says inspecting a sample in front of him. That will do. And here you get, what was promised: A piece of a ghost ship from the nebula. 100% authentic my supplier claims!" he says, handing you one single chip.]])
text[7] = _([["What are you still doing here? No Phosphine, no trade...."]])
-- dialogue with 2nd trader
-- %s for pho_mny
text[8] = _([["It seems like I have found myself a customer... 
well, let me see what you need and I will tell you how much it costs you." he says. "Oh." he says while lowering his voice. "Yes we do have that, but for a corresponding price, as you might understand. It will be %s credits. If anything happens: I have never seen you." You are a bit flabbergasted. Are you willing to pay?]])
mnytitle = _([[In the bar]])
mnytext = _([["You don't have enough money... I don't mean to be rude, but until you show up with some good old bling bling I have nothing to talk about with you..."]])
text[9] = _([["Very well, always a pleasure to make a business."]])
text[10] = _([["Then we have nothing to talk about. Come back if you made up your mind and have enough credits with you"]])
-- call of the police
title[2] = _([[On the intercom]])
text[11] = _([["You are suspected to be transporting highly toxic materials without a license, please stop your ship and prepare to be boarded."]])
text[12] = _([["Stand down for inspection."]])
title[3] = _([[On your ship]])
text[13] = _([["Freeze! You are in accused of violating the toxic materials regulation. Your ship will be searched and any suspicious elements confiscated!"]])
text[14] = _([[The inspectors search through your ship and cargo department. Once they find the containers they confiscate the Phosphine and charge you with a fine of 100 000 credits for violating the transportation regulation on toxic materials.]])
text[15] = _([[Dr Geller looks at you. "Have you got it?" You present the chip and he begins to glow. "Yes that is it!! Now I can continue my research." You ask him about the ghost ship remark by the trader. "Some people believe this nonsense. There is no scientific explanation of the ghost ships. I think it is some technology related to sol that got blasted off during the incident. They must have done some advanced research on neuronal computing."]])
text[16] = _([[He turns away and after you clear your throat with a pretty audible "Uh uhm" turns back to you. "Oh, yes, of course you want some payment for your service, right? Well, here you go. I might need you again in the future." He hands you a payment slip and walks away.]])
trd_disc = _([[This guy seems to be the trader, surrounded by bodyguards he looks a bit shifty.]])
-- osd_msg
osd_msg[1] = _("Go to the %s system and talk to the trader on %s.")
osd_msg[2] = _("Go to the %s system and talk to the contact person on %s.")
osd_msg[3] = _("Return to the %s system to the trader on %s.")
osd_msg[4] = _("Return to the %s system and deliver to Dr.Geller on %s.")
-- refusetext 
refusetitle = _("No Science Today")
refusetext = _("I guess you don't care that much about science...")

function create ()
   -- Spaceport bar stuff
   misn.setNPC( _("A scientist"),  "zalek_scientist_placeholder")
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
    bar1pir1 = misn.npcAdd("first_trd", "Trader", "neutral/thief1", trd1_desc)
  elseif planet.cur() == planet.get(t_pla[1]) and talked and traded1 then
    bar1pir1 = misn.npcAdd("third_trd", "Trader", "neutral/thief1", trd1_desc)
  end
end

function land2()
  if planet.cur() == planet.get(t_pla[2]) and talked and not traded1 then
    bar2pir1 = misn.npcAdd("second_trd", "Contact Person", "neutral/thief3", trd2_desc)
  end
end

-- first trade: send player 2 2nd system, if he goes back here, tell them to get going...
function first_trd()
  if talked then
     tk.msg(title[1], text[7])
     return
  else
     tk.msg(title[1], text[3])
     tk.msg(title[1], text[4])
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
  if not tk.yesno( title[1], text[8]:format(pho_mny) ) then
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
      l1ho = hook.pilot(lance1, "death", "lanc1_dead")
      l2ho = hook.pilot(lance2, "death", "lanc2_dead")
      admho = hook.pilot(adm1, "death", "adm_dead")
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
   tk.msg(title[3],text[13])
   tk.msg(title[3],text[14])
   if player.credits() > 100000 then
      player.pay(-100000)
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
-- if the player just kills the empire ships he looses emp reputation...
function lanc1_dead () 
   faction.modPlayerSingle( "Empire", -2 )
   lance1 = false
end
function lanc2_dead () 
   faction.modPlayerSingle( "Empire", -2 )
   lance2 = false
end
function adm_dead () 
   faction.modPlayerSingle( "Empire", -2 )
   adm1 = false
end
