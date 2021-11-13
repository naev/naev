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

function create ()
 -- Note: this mission does not make any system claims.

   misn.setNPC( _("Lieutenant"), "empire/unique/czesc.webp", _("Lieutenant Czesc from the Empire Armada Shipping Division is sitting at the bar.") )
end


function accept ()
   -- Set marker to a system, visible in any mission computer and the onboard computer.
   misn.markerAdd( targetworld_sys, "low")
   ---Intro Text
   if not tk.yesno( _("Spaceport Bar"), _([[Lieutenant Czesc approaches as you enter the bar. "If it isn't my favorite Empire Armada employee. We're on track to establish a deal with House Sirius. This should be the last contract to be negotiated. Ready to go?"]]) ) then
      misn.finish()
   end
   -- Flavour text and mini-briefing
   tk.msg( _("Sirius Long Distance Recruitment"), _([["You know how this goes by now." says Lieutenant Czesc, "Drop the bureaucrat off at Madria in the Esker system. Sirius space is quite a distance, so be prepared for anything. Afterwards, come find me one more time and we'll finalize the paperwork to get you all set up for these missions."]]) )
   ---Accept the mission
   misn.accept()

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setTitle(_("Sirius Long Distance Recruitment"))
   misn.setReward( fmt.credits( emp.rewards.ldc5 ) )
   misn.setDesc( _("Deliver a shipping diplomat for the Empire to Madria in the Esker system") )
   misn.osdCreate(_("Sirius Long Distance Recruitment"), {_("Deliver a shipping diplomat for the Empire to Madria in the Esker system")})
   -- Set up the goal
   hook.land("land")
   local c = misn.cargoNew( N_("Diplomat"), N_("An Imperial trade representative.") )
   mem.person = misn.cargoAdd( c, 0 )
end


function land()

   if planet.cur() == targetworld then
         misn.cargoRm( mem.person )
         player.pay( emp.rewards.ldc5 )
         -- More flavour text
         tk.msg( _("Mission Accomplished"), _([[You drop the diplomat off on Madria, and she hands you a credit chip. Lieutenant Czesc said to look for him in an Empire bar for some paperwork. Bureaucracy at its finest.]]) )
         faction.modPlayerSingle( "Empire",3 )
         emp.addShippingLog( _([[You delivered a shipping bureaucrat to Madria for the Empire. Lieutenant Czesc said to look for him in an Empire bar for some paperwork. Bureaucracy at its finest.]]) )
         misn.finish(true)
   end
end

function abort()
   misn.finish(false)
end
