--[[
   
   This is the sixth mission of the Shark's teeth campaign. The player has to take contact with the FLF.
   
   Stages :
   0) Way to Eiger/Surano
   1) Way back to Darkshed
   
--]]

include "numstring.lua"

lang = naev.lang()
if lang == "es" then
   else -- default english
   title = {}
   text = {}
   osd_msg = {}
   npc_desc = {}
   bar_desc = {}
   
   title[1] = "Nice to see you again!"
   text[1] = [[As you sit at his table, Arnold Smith welcomes you with a smile: "Hello, old boy, are you ready to take part to another sales project?" He then becomes serious: "This time, it's serious game: I really need your help to get in touch with somebody who could be very interested in the Shark.
   The problem is that this person is very difficult to spot." You ask him if he wants you to sell imperial fighters to a pirate, but he answers: "No, it isn't precisely that. In fact, we are looking to a contact with the FLF. Do you think, you can do it?"]]
   
   refusetitle = "Sorry, not interested"
   refusetext = [["Oh, I see, so, I guess, I'll have to find somebody else..." Smith says.]]
   
   title[2] = "Good luck"
   text[2] = [["Ok, that's good news for me. I don't know if you know people in there, and I don't want to know it. If you don't, I suggest you to try to hail a lone FLF ship. I think you could encounter some in %s.
   Maybe try to disable a FLF ship, board it and explain to it's crew what we want. If you success, set up a meeting in %s. I think, nobody is going to disturb us there.
   Ah, and don't mention the company for which we are working."]]
   
   title[3] = "Good news"
   text[3] = [[As you land, you see Smith coming at you. When you have finished explaining to him that you managed to take contact with the FLF, he seems happy and pays you your fee.
   "Now, meet me in the bar when you are ready to bring me to %s."]]
   
   title[4] = "The FLF crew"
   text[4] = [[As you board the ship, the FLF crew seems to be ready to die at their post. Instead of using your blasters to cook them all, you explain to them that you were sent by a friend of theirs who wants to sell ships to the FLF. You write on a papersheet the information they need to give to their officers, and let them alone."]]
   
   title[5] = "The FLF executive"
   text[5] = [[After a short hesitation, you approach the man, who seems at first not wanting to talk to you. As you explain that you were sent by a company that wants to sell fighters to the FLF, he looks at you, surprised. You explain everything about the meeting in %s and he answers you: "Well, I will check with my superiors. If we don't come, that means that we are not interested."]]
   
   -- Mission details
   misn_title = "The FLF Contact"
   misn_reward = "%s credits"
   misn_desc = "Nexus Shipyard asks you to take contact with the FLF"
   
   -- NPC
   npc_desc[1] = "Arnold Smith"
   bar_desc[1] = [[It seems, Nexus Shipyards is still looking for Shark customers.]]
   npc_desc[2] = "FLF executive"
   bar_desc[2] = [[This guy looks important]]
   
   -- OSD
   osd_title = "The FLF Contact"
   osd_msg[1] = "Placeholder"
   osd_sindbad = "Go to %s, disable and board a FLF ship or go to %s and speak to a FLF officer."
   osd_nosindbad = "Go to %s, disable and board a FLF ship"
   osd_msg[2] = "Go back to %s in %s"
   
end

function create ()
   
   --Change here to change the planets and the systems
   mispla, missys = planet.get("Eiger")
   paypla, paysys = planet.get("Darkshed")
   nextsys = system.get("Arandon") -- This should be the same as the system used in sh06!
   
   --Does the player have access to Eiger
   if diff.isApplied( "FLF_base") then
      sindbad = planet.get("Eiger")
      osd_msg[1] = osd_sindbad:format(missys:name(), nextsys:name(), mispla:name())
      else
      osd_msg[1] = osd_nosindbad:format(missys:name())
   end
   osd_msg[2] = osd_msg[2]:format(paypla:name(), paysys:name())
   paysys = system.get(paysys:name())
   paypla = planet.get(paypla:name())
   
   if not misn.claim(missys) then
      misn.finish(false)
   end
   
   misn.setNPC(npc_desc[1], "neutral/male1")
   misn.setDesc(bar_desc[1])
end

function accept()
   
   stage = 0 
   reward = 200000
   
   if tk.yesno(title[1], text[1]) then
      misn.accept()
      tk.msg(title[2], text[2]:format(missys:name(), nextsys:name()))
      
      misn.setTitle(misn_title)
      misn.setReward(misn_reward:format(numstring(reward)))
      misn.setDesc(misn_desc)
      osd = misn.osdCreate(osd_title, osd_msg)
      misn.osdActive(1)

      marker = misn.markerAdd(missys, "low")
      
      landhook = hook.land("land")
      enterhook = hook.enter("enter")
      else
      tk.msg(refusetitle, refusetext)
      misn.finish(false)
   end
end

function land()
   --The player is landing on Eiger
   if stage == 0 and planet.cur() == sindbad then
      flfguy = misn.npcAdd("talktoguy", npc_desc[2], "neutral/thief1", bar_desc[2])
   end
   
   --Job is done
   if stage == 1 and planet.cur() == paypla then
      tk.msg(title[3], text[3]:format(nextsys:name()))
      player.pay(reward)
      misn.osdDestroy(osd)
      hook.rm(enterhook)
      hook.rm(landhook)
      misn.finish(true)
   end
end

function talktoguy()
   tk.msg(title[5], text[5]:format(nextsys:name()))
   stage = 1
   misn.osdActive(2)
   misn.markerRm(marker)
   marker2 = misn.markerAdd(paysys, "low")
end

function enter()
   --Entering in Surano in order to find and disable a FLF Lancelot
   if system.cur() == missys then
      --Lets unspawn everybody
      pilot.clear()
      pilot.toggleSpawn(false)
      
      lancelot = pilot.add( "FLF Lancelot", nil, 0 )[1]
      hook.pilot(lancelot, "board", "discuss")
   end
end

function discuss()
   tk.msg(title[4], text[4])
   stage = 1
   misn.osdActive(2)
   misn.markerRm(marker)
   marker2 = misn.markerAdd(paysys, "low")
end
