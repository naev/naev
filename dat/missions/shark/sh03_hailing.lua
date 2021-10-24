--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Invitation">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>3</priority>
  <done>Unfair Competition</done>
  <chance>50</chance>
  <location>Bar</location>
  <planet>Darkshed</planet>
 </avail>
 <notes>
  <campaign>Nexus show their teeth</campaign>
 </notes>
</mission>
--]]
--[[
   This is the fourth mission of the Shark's teeth campaign. The player has to hail a frontier ship.
   There should not be any ambush in this mission but the player must fear it from the beginning to the end

   Stages :
   0) Way to Frontier system
   1) Way back to Darkshed
--]]
local pir = require "common.pirate"
local fmt = require "format"
local shark = require "common.shark"

osd_msg = {}

-- Mission details

-- NPC

-- OSD
osd_msg[1] = _("Go to %s, find and hail the Air Force One")
osd_msg[2] = _("Report back to %s in the %s system")

function create ()

   --Change here to change the planets and the systems
   mispla,missys = planet.getLandable(faction.get("Frontier"))  -- mispla will be useful to locate the Hawking
   pplname = "Darkshed"
   psyname = "Alteris"
   paysys = system.get(psyname)
   paypla = planet.get(pplname)
   nextpla, nextsys = planet.get("Curie") -- This should be the same as the planet used in sh04_meeting!

   if not misn.claim(missys) then
      misn.finish(false)
   end

   misn.setNPC(_("Arnold Smith"), "neutral/unique/arnoldsmith.webp", _([[This guy is looking more and more shifty.]]))
end

function accept()

   stage = 0
   reward = 750e3

   if tk.yesno(_("A new job"), _([["Hello there, nice to meet you again! According to the information that you brought us, the negotiations between the Frontier officials and House Sirius are proceeding very quickly. We have to act now. There is a member of the Frontier Council who, for political reasons, could help us.
    "I can't send him a message without being spotted by the Sirii, so I need you to contact him. He's probably piloting his Hawking in the %s system. Go there, hail him, and let him know that I have to see him on %s in the %s system. He will understand.
    "Can I count on you to do this for me?"]]):format(missys:name(), nextpla:name(), nextsys:name())) then
      misn.accept()
      tk.msg(_("Time to go"), _([["Fantastic! I am known as Donald Ulnish to the Council member. Good luck."]]))

      osd_msg[1] = osd_msg[1]:format(missys:name())
      osd_msg[2] = osd_msg[2]:format(pplname, psyname)

      misn.setTitle(_("Invitation"))
      misn.setReward(fmt.credits(reward))
      misn.setDesc(_("Nexus Shipyards asks you to help initiate a secret meeting"))
      osd = misn.osdCreate(_("Invitation"), osd_msg)
      misn.osdActive(1)

      marker = misn.markerAdd(missys, "low")

      landhook = hook.land("land")
      enterhook = hook.enter("enter")
      else
      tk.msg(_("Sorry, not interested"), _([["OK, come back when you are interested."]]))
      misn.finish(false)
   end
end

function land()

   --Job is done
   if stage == 1 and planet.cur() == paypla then
   tk.msg(_("Good job"), _([[Smith seems to relax as you tell him that everything went according to plan. "Fantastic! I have another mission for you; meet me in the bar when you are ready to bring me to %s in the %s system."]]):format(nextpla:name(), nextsys:name()))
      player.pay(reward)
      pir.reputationNormalMission(rnd.rnd(2,3))
      misn.osdDestroy(osd)
      hook.rm(enterhook)
      hook.rm(landhook)
      shark.addLog( _([[You helped Nexus Shipyards initiate a secret meeting with a member of the Frontier Council. Arnold Smith said that he has another mission for you and to meet him in the bar on Darkshed when you are ready to transport him to Curie.]]) )
      misn.finish(true)
   end
end

function enter()
   --the system where the player must look for the Hawking
   if system.cur() == missys then
      hawking = pilot.add("Hawking", "Frontier", mispla:pos() + vec2.new(-400,-400), _("Air Force One"), {ai="trader"} )
      hawking:setHilight(true)
      hailhook = hook.pilot(hawking, "hail", "hail")
   end
end

function hail()
   --The player takes contact with the Hawking
   if stage == 0 then
      tk.msg(_("Time to go back to %s"):format(paypla:name()), _([[The captain of the Hawking answers you. When you say that you have a message from Donald Ulnish, he redirects you to one of his officers who takes the message. Now, back to %s.]]):format(paypla:name()))
      stage = 1
      misn.osdActive(2)
      misn.markerRm(marker)
      marker2 = misn.markerAdd(paysys, "low")
      player.commClose()
   end
end
