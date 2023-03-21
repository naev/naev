--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Soromid Long Distance Recruitment">
 <unique />
 <priority>4</priority>
 <cond>faction.playerStanding("Empire") &gt;= 0 and var.peek("es_cargo") == true and var.peek("es_misn") ~= nil and var.peek("es_misn") &gt;= 2</cond>
 <chance>30</chance>
 <location>Bar</location>
 <faction>Empire</faction>
 <notes>
  <done_misn name="Empire Shipping">2 times or more</done_misn>
  <campaign>Empire Shipping</campaign>
 </notes>
</mission>
--]]
--[[

   First diplomatic mission to Soromid space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--
local fmt = require "format"
local emp = require "common.empire"
local vn = require "vn"
local vntk = require "vntk"
local lmisn = require "lmisn"

-- Mission constants
local targetworld, targetworld_sys = spob.getS("Soromid Customs Central")

function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( emp.czesc.name, emp.czesc.portrait, emp.czesc.description )
end


function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local czesc = vn.newCharacter( emp.vn_czesc() )
   vn.transition( emp.czesc.transition )

   -- Intro Text
   czesc(_([[You approach Lieutenant Czesc. His demeanor brightens as he sees you. "Hello! I've been looking for you. You've done a great job with those Empire Shipping missions and we have a new exciting opportunity. You see, the head office is looking to expand business with other factions, which has enormous untapped potential. They need someone competent and trustworthy to help them out. That's where you come in. Interested?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done( emp.czesc.transition )

   -- Flavour text and mini-briefing
   vn.label("accept")
   czesc(_([["I knew I could count on you," Lieutenant Czesc exclaims. "These missions will be long distance, meaning that you'll usually have to go at least 3 jumps to make the delivery. In addition, you'll often find yourself in the territory of other factions, where the Empire may not be able to protect you. In return, you will be nicely compensated."]]))
   czesc(fmt.f(_([[He hits a couple buttons on his wrist computer. "First, we need to set up operations with the other factions. We'll need a bureaucrat to handle the red tape and oversee the operations. We will begin with the Soromid. I know those genetically modified beings are kind of creepy, but business is business. Please transport a bureaucrat to {pnt} in the {sys} system. He will report back to me when this mission is accomplished. I tend to travel within Empire space handling minor trade disputes, so keep an eye out for me in the bar on Empire controlled planets."]]),
      {pnt=targetworld, sys=targetworld_sys}))
   vn.func( function () accepted = true end )

   vn.done( emp.czesc.transition )
   vn.run()

   if not accepted then return end

   -- Accept the mission
   misn.accept()

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setTitle(_("Soromid Long Distance Recruitment"))
   misn.setReward( emp.rewards.ldc1 )
   local misn_desc = fmt.f(_("Deliver a shipping diplomat for the Empire to {pnt} in the {sys} system"),
      {pnt=targetworld, sys=targetworld_sys})
   misn.setDesc( misn_desc )
   misn.osdCreate(_("Soromid Long Distance Recruitment"), {misn_desc})

   -- Set up the goal
   misn.markerAdd( targetworld, "low" )
   hook.land("land")
   local c = commodity.new( N_("Diplomat"), N_("An Imperial trade representative.") )
   mem.person = misn.cargoAdd( c, 0 )
end


function land()
   if spob.cur() ~= targetworld then
      return
   end

   player.pay( emp.rewards.ldc1 )
   faction.modPlayerSingle( "Empire",3 )
   lmisn.sfxVictory()

   -- More flavour text
   vntk.msg( _("Mission Accomplished"), fmt.f( _([[You drop the bureaucrat off at {pnt}, and he hands you a credit chip. Lieutenant Czesc told you to keep an eye out for him in Empire space to continue the operation.]]),
      {pnt=targetworld}).."\n\n"..fmt.reward(emp.rewards.ldc1) )

   emp.addShippingLog( fmt.f( _([[You delivered a shipping bureaucrat to {pnt} for the Empire. Lieutenant Czesc told you to keep an eye out out for him in Empire space to continue the operation.]]), {pnt=targetworld} ) )
   misn.finish(true)
end
