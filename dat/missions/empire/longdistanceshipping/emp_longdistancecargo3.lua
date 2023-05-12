--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Long Distance Recruitment">
 <unique />
 <priority>4</priority>
 <cond>faction.playerStanding("Empire") &gt;= 0</cond>
 <chance>75</chance>
 <done>Dvaered Long Distance Recruitment</done>
 <location>Bar</location>
 <faction>Empire</faction>
 <notes>
  <campaign>Empire Shipping</campaign>
 </notes>
</mission>
--]]
--[[

   Third diplomatic mission to Za'lek space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--
local fmt = require "format"
local emp = require "common.empire"
local vn = require "vn"
local vntk = require "vntk"
local lmisn = require "lmisn"

-- Mission constants
local targetworld, targetworld_sys = spob.getS("Gerhart Central")

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
   czesc(_([[Lieutenant Czesc sits at the bar. He really does seem to handle business all across the Empire. You take the seat next to him. "Thanks to your help, the Empire Armada Shipping Division will soon operate across the galaxy. Our next mission is to get House Za'lek on board. Interested in helping out again?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done( emp.czesc.transition )

   -- Flavour text and mini-briefing
   vn.label("accept")

   -- Flavour text and mini-briefing
   czesc(fmt.f( _([["I had a feeling you would!" says Lieutenant Czesc. "I've got another bureaucrat ready to establish trade ties. The Za'lek are rather mysterious, so keep your wits about you. The diplomat only needs to go to the {pnt} in the {sys} system. He will let me know when trade relations have been established. There is still more work to be done, so I expect to see you again soon."]]),
      {pnt=targetworld, sys=targetworld_sys} ) )
   vn.func( function () accepted = true end )

   vn.done( emp.czesc.transition )
   vn.run()

   if not accepted then return end

   ---Accept the mission
   misn.accept()

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setTitle(_("Za'lek Long Distance Recruitment"))
   misn.setReward( emp.rewards.ldc3 )
   local misn_desc = fmt.f(_("Deliver a shipping diplomat for the Empire to {pnt} in the {sys} system"), {pnt=targetworld, sys=targetworld_sys})
   misn.setDesc( misn_desc )
   misn.osdCreate(_("Za'lek Long Distance Recruitment"), {misn_desc})

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

   player.pay( emp.rewards.ldc3 )
   faction.modPlayerSingle( "Empire",3 )
   lmisn.sfxVictory()

   -- More flavour text
   vntk.msg( _("Mission Accomplished"), fmt.f( _([[You drop the diplomat off on {pnt}, and she hands you a credit chip. Lieutenant Czesc mentioned more work, so you figure you'll run into him at a bar again soon.]]),
      {pnt=targetworld}).."\n\n"..fmt.reward(emp.rewards.ldc3) )

   emp.addShippingLog( fmt.f( _([[You delivered a shipping bureaucrat to {pnt} for the Empire. Lieutenant Czesc mentioned more work, so you figure you'll run into him at a bar again soon.]]), {pnt=targetworld} ) )
   misn.finish(true)
end
