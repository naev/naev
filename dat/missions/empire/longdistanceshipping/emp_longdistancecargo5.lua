--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Sirius Long Distance Recruitment">
 <unique />
 <priority>4</priority>
 <cond>
   if faction.playerStanding("Empire") &lt; 0 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <chance>75</chance>
 <done>Frontier Long Distance Recruitment</done>
 <location>Bar</location>
 <faction>Empire</faction>
 <notes>
  <campaign>Empire Shipping</campaign>
 </notes>
</mission>
--]]
--[[

   Fifth diplomatic mission to Sirius space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--
local fmt = require "format"
local emp = require "common.empire"
local vn = require "vn"
local vntk = require "vntk"
local lmisn = require "lmisn"

-- Mission constants
local targetworld, targetworld_sys = spob.getS("Madria")

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
   czesc(_([[Lieutenant Czesc approaches as you enter the bar. "If it isn't my favourite Empire Armada employee. We're on track to establish a deal with House Sirius. This should be the last contract to be negotiated. Ready to go?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done( emp.czesc.transition )

   -- Flavour text and mini-briefing
   vn.label("accept")
   czesc(fmt.f( _([["You know how this goes by now." says Lieutenant Czesc, "Drop the bureaucrat off at {pnt} in the {sys} system. Sirius space is quite a distance, so be prepared for anything. Afterwards, come find me one more time, and we'll finalize the paperwork to get you all set up for these missions."]]),
      {pnt=targetworld, sys=targetworld_sys} ) )
   vn.func( function () accepted = true end )

   vn.done( emp.czesc.transition )
   vn.run()

   if not accepted then return end

   -- Accept the mission
   misn.accept()

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setTitle(_("Sirius Long Distance Recruitment"))
   misn.setReward( emp.rewards.ldc5 )
   local misn_desc = fmt.f(_("Deliver a shipping diplomat for the Empire to {pnt} in the {sys} system"), {pnt=targetworld, sys=targetworld_sys})
   misn.setDesc( misn_desc )
   misn.osdCreate(_("Sirius Long Distance Recruitment"), {misn_desc})

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

   player.pay( emp.rewards.ldc5 )
   faction.modPlayerSingle( "Empire",3 )
   lmisn.sfxVictory()

   -- More flavour text
   vntk.msg( _("Mission Accomplished"), fmt.f( _([[You drop the diplomat off on {pnt}, and she hands you a credit chip. Lieutenant Czesc said to look for him in an Empire bar for some paperwork. Bureaucracy at its finest.]]),
      {pnt=targetworld}).."\n\n"..fmt.reward(emp.rewards.ldc5) )

   emp.addShippingLog( fmt.f( _([[You delivered a shipping bureaucrat to {pnt} for the Empire. Lieutenant Czesc said to look for him in an Empire bar for some paperwork. Bureaucracy at its finest.]]), {pnt=targetworld} ) )
   misn.finish(true)
end
