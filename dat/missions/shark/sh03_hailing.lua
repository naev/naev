--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Invitation">
 <unique />
 <priority>3</priority>
 <done>Unfair Competition</done>
 <chance>50</chance>
 <location>Bar</location>
 <spob>Darkshed</spob>
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
local vn = require "vn"
local ccomm = require "common.comm"

local hawking -- Non-persistent state

-- Mission constants
local paypla, paysys = spob.getS("Darkshed")
local nextpla, nextsys = spob.getS("Curie") -- This should be the same as the planet used in sh04_meeting!

function create ()
   --Change here to change the planets and the systems
   mem.mispla,mem.missys = spob.getLandable(faction.get("Frontier"))  -- mem.mispla will be useful to locate the Hawking

   if not misn.claim(mem.missys) then
      misn.finish(false)
   end

   misn.setNPC( shark.arnold.name, shark.arnold.portrait, _([[This guy is looking more and more shifty.]]))
end

function accept()
   mem.stage = 0
   local accepted = false

   vn.clear()
   vn.scene()
   local arnold = vn.newCharacter( shark.vn_arnold() )
   vn.transition( shark.arnold.transition )

   arnold(_([["Hello there, nice to meet you again! According to the information that you brought us, the negotiations between the Frontier officials and House Sirius are proceeding very quickly. We have to act now. There is a member of the Frontier Council who, for political reasons, could help us."]]))
   arnold(fmt.f(_([["I can't send him a message without being spotted by the Sirii, so I need you to contact him. He's probably piloting his Hawking in the {sys} system. Go there, hail him, and let him know that I have to see him on {next_pnt} in the {next_sys} system. He will understand."
"Can I count on you to do this for me?"]]),
      {sys=mem.missys, next_pnt=nextpla, next_sys=nextsys}))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   arnold(_([["OK, come back when you are interested."]]))
   vn.done( shark.arnold.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   arnold(_([["Fantastic! I am known as Donald Ulnish to the Council member. Good luck."]]))

   vn.done( shark.arnold.transition )
   vn.run()

   if not accepted then return end

   misn.accept()

   misn.setTitle(_("Invitation"))
   misn.setReward(shark.rewards.sh03)
   misn.setDesc(_("Nexus Shipyards asks you to help initiate a secret meeting"))
   misn.osdCreate(_("Invitation"), {
      fmt.f(_("Go to {sys}, find and hail the Air Force One"), {sys=mem.missys}),
      fmt.f(_("Report back to {pnt} in the {sys} system"), {pnt=paypla, sys=paysys}),
   })
   misn.osdActive(1)

   mem.marker = misn.markerAdd(mem.missys, "low")

   hook.land("land")
   hook.enter("enter")
end

function land()
   --Job is done
   if mem.stage == 1 and spob.cur() == paypla then
      vn.clear()
      vn.scene()
      local arnold = vn.newCharacter( shark.vn_arnold() )
      vn.transition( shark.arnold.transition )
      arnold(fmt.f(_([[Smith seems to relax as you tell him that everything went according to plan. "Fantastic! I have another mission for you; meet me in the bar when you are ready to bring me to {pnt} in the {sys} system."]]),
         {pnt=nextpla, sys=nextsys}))
      vn.func( function ()
         player.pay(shark.rewards.sh03)
         pir.reputationNormalMission(rnd.rnd(2,3))
      end )
      vn.sfxVictory()
      vn.na(fmt.reward(shark.rewards.sh03))
      vn.run()
      shark.addLog( _([[You helped Nexus Shipyards initiate a secret meeting with a member of the Frontier Council. Arnold Smith said that he has another mission for you and to meet him in the bar on Darkshed when you are ready to transport him to Curie.]]) )
      misn.finish(true)
   end
end

function enter()
   --the system where the player must look for the Hawking
   if system.cur() == mem.missys then
      hawking = pilot.add("Hawking", "Frontier", mem.mispla:pos() + vec2.new(-400,-400), _("Air Force One"), {ai="trader"} )
      hawking:memory().land_planet = false
      hawking:setHilight(true)
      mem.hailhook = hook.pilot(hawking, "hail", "hail")
   end
end

function hail()
   --The player takes contact with the Hawking
   if mem.stage ~= 0 then
      return
   end

   vn.clear()
   vn.scene()
   local p = ccomm.newCharacter( vn, hawking )
   vn.transition()
   p(fmt.f(_([[The captain of the Hawking answers you. When you say that you have a message from Donald Ulnish, he redirects you to one of his officers who takes the message. Now, back to {pnt}.]]),
      {pnt=paypla}))
   vn.run()

   mem.stage = 1
   misn.osdActive(2)
   misn.markerRm(mem.marker)
   mem.marker2 = misn.markerAdd(paypla, "low")
   player.commClose()
end
