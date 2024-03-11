--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Long Distance Recruitment">
 <unique />
 <priority>4</priority>
 <cond>
   if faction.playerStanding("Empire") &lt; 0 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <chance>75</chance>
 <done>Soromid Long Distance Recruitment</done>
 <location>Bar</location>
 <faction>Empire</faction>
 <notes>
  <campaign>Empire Shipping</campaign>
 </notes>
</mission>
--]]
--[[

   Second diplomatic mission to Dvaered space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--
local fmt = require "format"
local emp = require "common.empire"
local vn = require "vn"
local vntk = require "vntk"
local lmisn = require "lmisn"

-- Mission constants
local targetworld, targetworld_sys = spob.getS("Praxis")


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

   ---Intro Text
   czesc(_([[Lieutenant Czesc waves you over when he notices you enter the bar. "I knew we would run into each other soon enough. Great job delivering that bureaucrat. We should be up and running in Soromid space in no time!" He presses a button on his wrist computer. "We're hoping to expand to Dvaered territory next. Can I count on your help?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done( emp.czesc.transition )

   -- Flavour text and mini-briefing
   vn.label("accept")
   czesc(fmt.f( _([["Great!" says Lieutenant Czesc. "I'll send a message to the bureaucrat to meet you at the hanger. The Dvaered are, of course, allies of the Empire. Still, they offend easily, so try not to talk too much. Your mission is to drop the bureaucrat off on {pnt} in the {sys} system. He will take it from there and report back to me when the shipping contract has been established. Afterwards, keep an eye out for me in Empire space, and we can continue the operation."]]),
      {pnt=targetworld, sys=targetworld_sys} ) )
   vn.func( function () accepted = true end )

   vn.done( emp.czesc.transition )
   vn.run()

   if not accepted then return end

   ---Accept the mission
   misn.accept()

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setTitle(_("Dvaered Long Distance Recruitment"))
   misn.setReward( emp.rewards.ldc2 )
   local misn_desc = fmt.f(_("Deliver a shipping diplomat for the Empire to {pnt} in the {sys} system"), {pnt=targetworld, sys=targetworld_sys})
   misn.setDesc( misn_desc )
   misn.osdCreate(_("Dvaered Long Distance Recruitment"), {misn_desc})

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

   player.pay( emp.rewards.ldc2 )
   faction.modPlayerSingle( "Empire",3 )
   lmisn.sfxVictory()

   -- More flavour text
   vntk.msg(_("Mission Accomplished"), fmt.f( _([[You drop the bureaucrat off on {pnt}, and he hands you a credit chip. You remember Lieutenant Czesc told you to look for him on Empire controlled planets after you finish.]]),
      {pnt=targetworld}).."\n\n"..fmt.reward(emp.rewards.ldc2) )

   emp.addShippingLog( fmt.f( _([[You delivered a shipping bureaucrat to {pnt} for the Empire. Lieutenant Czesc told you to look for him on Empire controlled planets after you finish.]]), {pnt=targetworld} ) )
   misn.finish(true)
end
