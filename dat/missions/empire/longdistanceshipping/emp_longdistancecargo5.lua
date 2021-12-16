--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Sirius Long Distance Recruitment">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <cond>faction.playerStanding("Empire") &gt;= 0</cond>
  <chance>75</chance>
  <done>Frontier Long Distance Recruitment</done>
  <location>Bar</location>
  <faction>Empire</faction>
 </avail>
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

-- Mission constants
local targetworld, targetworld_sys = planet.getS("Madria")

-- luacheck: globals land (Hook functions passed by name)

function create ()
 -- Note: this mission does not make any system claims.

   misn.setNPC( _("Lieutenant"), "empire/unique/czesc.webp", _("Lieutenant Czesc, from the Empire Armada Shipping Division, is sitting at the bar.") )
end


function accept ()
   misn.markerAdd( targetworld, "low" )
   ---Intro Text
   if not tk.yesno( _("Spaceport Bar"), _([[Lieutenant Czesc approaches as you enter the bar. "If it isn't my favorite Empire Armada employee. We're on track to establish a deal with House Sirius. This should be the last contract to be negotiated. Ready to go?"]]) ) then
      misn.finish()
   end
   -- Flavour text and mini-briefing
   tk.msg( _("Sirius Long Distance Recruitment"), fmt.f( _([["You know how this goes by now." says Lieutenant Czesc, "Drop the bureaucrat off at {pnt} in the {sys} system. Sirius space is quite a distance, so be prepared for anything. Afterwards, come find me one more time and we'll finalize the paperwork to get you all set up for these missions."]]), {pnt=targetworld, sys=targetworld_sys} ) )
   ---Accept the mission
   misn.accept()

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setTitle(_("Sirius Long Distance Recruitment"))
   misn.setReward( fmt.credits( emp.rewards.ldc5 ) )
   local misn_desc = fmt.f(_("Deliver a shipping diplomat for the Empire to {pnt} in the {sys} system"), {pnt=targetworld, sys=targetworld_sys})
   misn.setDesc( misn_desc )
   misn.osdCreate(_("Sirius Long Distance Recruitment"), {misn_desc})
   -- Set up the goal
   hook.land("land")
   local c = commodity.new( N_("Diplomat"), N_("An Imperial trade representative.") )
   mem.person = misn.cargoAdd( c, 0 )
end


function land()

   if planet.cur() == targetworld then
         misn.cargoRm( mem.person )
         player.pay( emp.rewards.ldc5 )
         -- More flavour text
         tk.msg( _("Mission Accomplished"), fmt.f( _([[You drop the diplomat off on {pnt}, and she hands you a credit chip. Lieutenant Czesc said to look for him in an Empire bar for some paperwork. Bureaucracy at its finest.]]), {pnt=targetworld} ) )
         faction.modPlayerSingle( "Empire",3 )
         emp.addShippingLog( fmt.f( _([[You delivered a shipping bureaucrat to {pnt} for the Empire. Lieutenant Czesc said to look for him in an Empire bar for some paperwork. Bureaucracy at its finest.]]), {pnt=targetworld} ) )
         misn.finish(true)
   end
end

function abort()
   misn.finish(false)
end
