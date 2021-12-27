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

local hawking -- Non-persistent state
-- luacheck: globals enter hail land (Hook functions passed by name)

-- Mission constants
local paypla, paysys = spob.getS("Darkshed")
local nextpla, nextsys = spob.getS("Curie") -- This should be the same as the planet used in sh04_meeting!

function create ()
   --Change here to change the planets and the systems
   mem.mispla,mem.missys = spob.getLandable(faction.get("Frontier"))  -- mem.mispla will be useful to locate the Hawking

   if not misn.claim(mem.missys) then
      misn.finish(false)
   end

   misn.setNPC(_("Arnold Smith"), "neutral/unique/arnoldsmith.webp", _([[This guy is looking more and more shifty.]]))
end

function accept()
   mem.stage = 0

   if tk.yesno(_("A new job"), fmt.f(_([["Hello there, nice to meet you again! According to the information that you brought us, the negotiations between the Frontier officials and House Sirius are proceeding very quickly. We have to act now. There is a member of the Frontier Council who, for political reasons, could help us.
    "I can't send him a message without being spotted by the Sirii, so I need you to contact him. He's probably piloting his Hawking in the {sys} system. Go there, hail him, and let him know that I have to see him on {next_pnt} in the {next_sys} system. He will understand.
    "Can I count on you to do this for me?"]]), {sys=mem.missys, next_pnt=nextpla, next_sys=nextsys})) then
      misn.accept()
      tk.msg(_("Time to go"), _([["Fantastic! I am known as Donald Ulnish to the Council member. Good luck."]]))

      misn.setTitle(_("Invitation"))
      misn.setReward(fmt.credits(shark.rewards.sh03))
      misn.setDesc(_("Nexus Shipyards asks you to help initiate a secret meeting"))
      misn.osdCreate(_("Invitation"), {
         fmt.f(_("Go to {sys}, find and hail the Air Force One"), {sys=mem.missys}),
         fmt.f(_("Report back to {pnt} in the {sys} system"), {pnt=paypla, sys=paysys}),
      })
      misn.osdActive(1)

      mem.marker = misn.markerAdd(mem.missys, "low")

      mem.landhook = hook.land("land")
      mem.enterhook = hook.enter("enter")
      else
      tk.msg(_("Sorry, not interested"), _([["OK, come back when you are interested."]]))
      misn.finish(false)
   end
end

function land()

   --Job is done
   if mem.stage == 1 and spob.cur() == paypla then
   tk.msg(_("Good job"), fmt.f(_([[Smith seems to relax as you tell him that everything went according to plan. "Fantastic! I have another mission for you; meet me in the bar when you are ready to bring me to {pnt} in the {sys} system."]]), {pnt=nextpla, sys=nextsys}))
      player.pay(shark.rewards.sh03)
      pir.reputationNormalMission(rnd.rnd(2,3))
      misn.osdDestroy()
      hook.rm(mem.enterhook)
      hook.rm(mem.landhook)
      shark.addLog( _([[You helped Nexus Shipyards initiate a secret meeting with a member of the Frontier Council. Arnold Smith said that he has another mission for you and to meet him in the bar on Darkshed when you are ready to transport him to Curie.]]) )
      misn.finish(true)
   end
end

function enter()
   --the system where the player must look for the Hawking
   if system.cur() == mem.missys then
      hawking = pilot.add("Hawking", "Frontier", mem.mispla:pos() + vec2.new(-400,-400), _("Air Force One"), {ai="trader"} )
      hawking:setHilight(true)
      mem.hailhook = hook.pilot(hawking, "hail", "hail")
   end
end

function hail()
   --The player takes contact with the Hawking
   if mem.stage == 0 then
      tk.msg(fmt.f(_("Time to go back to {pnt}"), {pnt=paypla}), fmt.f(_([[The captain of the Hawking answers you. When you say that you have a message from Donald Ulnish, he redirects you to one of his officers who takes the message. Now, back to {pnt}.]]), {pnt=paypla}))
      mem.stage = 1
      misn.osdActive(2)
      misn.markerRm(mem.marker)
      mem.marker2 = misn.markerAdd(paypla, "low")
      player.commClose()
   end
end
