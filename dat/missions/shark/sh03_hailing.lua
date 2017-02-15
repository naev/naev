--[[
   
   This is the fourth mission of the Shark's teeth campaign. The player has to hail a frontier ship.
   There should not be any ambush in this mission but the player must fear it from the beginning to the end 
   
   Stages :
   0) Way to Frontier system
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
   
   title[1] = "A new job"
   text[1] = [["Hello there, nice to meet you again!" Smith says. "According to the information that you brought us, the negotiations between the Frontier officials and House Sirius are going very fast. We have to act now: I have noticed a member of the Frontier Council who, for political reasons, could help us.
   I can't send him a message without being spotted by the Sirii, so I need you to take contact with him: he must be flying in his Hawking in %s now. Go there, hail him, say him that I have to see him on %s in %s and he will understand. Oh, by the way, my new name is Donald Ulnish.
   And there is something else: I heard that the Sirii hired some henchmen to go after you, you maybe already have met them. The good news is that they don't know that Nexus is involved, but still, be careful.
   Are you in?"]]
   
   refusetitle = "Sorry, not interested"
   refusetext = [["Ok, so come back when you are interested," Smith says.]]
   
   title[2] = "Time to go"
   text[2] = [["Good luck"]]
   
   title[3] = "Good job"
   text[3] = [[Smith seems to relax as you says him that everything went according to plan. "Ok, so now, meet me in the bar when you are ready to bring me to %s in %s.]]
   
   title[4] = "Time to go back to %s"
   text[4] = [[The captain of the Hawking answers you. When you say that you have a message from Donald Ulnish, he redirects you to one of his officers. After having delivered your message, you quit, hoping that the journey back will be as quiet as the trip from %s.]]
   
   
   -- Mission details
   misn_title = "Invitation"
   misn_reward = "%s credits"
   misn_desc = "Nexus Shipyard asks you to help in a secret meeting"
   
   -- NPC
   npc_desc[1] = "Arnold Smith"
   bar_desc[1] = [[Arnold Smith (aka James Neptune): this guy seems more and more shifty]]
   
   -- OSD
   osd_title = "Invitation"
   osd_msg[1] = "Go to %s, find and hail the Air Force One"
   osd_msg[2] = "Report back to %s in %s"
   
end

function create ()
   
   --Change here to change the planets and the systems
   mispla,missys = planet.getLandable(faction.get("Frontier"))  -- mispla will be usefull to locate the Hawking
   pplname = "Darkshed"
   psyname = "Alteris"
   paysys = system.get(psyname)
   paypla = planet.get(pplname)
   nextpla, nextsys = planet.get("Curie") -- This should be the same as the planet used in sh04_meeting.lua!
   
   if not misn.claim(missys) then
      misn.finish(false)
   end
   
   misn.setNPC(npc_desc[1], "neutral/male1")
   misn.setDesc(bar_desc[1])
end

function accept()
   
   stage = 0 
   reward = 50000
   
   if tk.yesno(title[1], text[1]:format(missys:name(), nextpla:name(), nextsys:name())) then
      misn.accept()
      tk.msg(title[2], text[2])
      
      osd_msg[1] = osd_msg[1]:format(missys:name())
      osd_msg[2] = osd_msg[2]:format(pplname, psyname)

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
   
   --Job is done
   if stage == 1 and planet.cur() == paypla then
   tk.msg(title[3], text[3]:format(nextpla:name(), nextsys:name()))
      player.pay(reward)
      misn.osdDestroy(osd)
      hook.rm(enterhook)
      hook.rm(landhook)
      misn.finish(true)
   end
end

function enter()
   --the system where the player must look for the Hawking
   if system.cur() == missys then
      hawking = pilot.addRaw("Hawking", "trader", mispla:pos() + vec2.new(-400,-400), "Frontier" )
      hawking:rename("Air Force One")
      hawking:setHilight(true)
      hailhook = hook.pilot(hawking, "hail", "hail")
   end
end

function hail()
   --The player takes contact with the Hawking
   if stage == 0 then
      tk.msg(title[4]:format(paypla:name()), text[4]:format(paypla:name()))
      stage = 1
      misn.osdActive(2)
      misn.markerRm(marker)
      marker2 = misn.markerAdd(paysys, "low")
   end
end
