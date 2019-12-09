--[[

   Fifth diplomatic mission to Sirius space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--

include "dat/scripts/numstring.lua"
include "dat/scripts/jumpdist.lua"

bar_desc = _("Lieutenant Czesc from the Empire Aramda Shipping Division is sitting at the bar.")
misn_title = _("Sirius Long Distance Recruitment")
misn_reward = _("500,000 credits")
misn_desc = _("Deliver a shipping diplomat for the Empire to Madria in the Esker system.")
title = {}
title[1] = _("Spaceport Bar")
title[2] = _("Sirius Long Distance Recruitment")
title[3] = _("Mission Accomplished")
text = {}
text[1] = _([[Lieutenant Czesc approaches as you enter the bar. "If it isn't my favorite Empire Armada employee. We're on track to establish a deal with House Sirius. This should be the last contract to be negotiated. Ready to go?"]])
text[2] = _([["You know how this goes by now." says Lieutenant Czesc, "Drop the bureaucrat off at Madria in the Esker system. Sirius space is quite a distance, so be prepared for anything. Afterwards, come find me one more time and we'll finalize the paperwork to get you all set up for these missions."]])
text[3] = _([[You drop the diplomat off on Madria, and she hands you a credit chip. Lieutenant Czesc said to look for him in an Empire bar for some paperwork. Bureaucracy at its finest.]])


function create ()
 -- Note: this mission does not make any system claims.
 
      -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Esker")
   targetworld = planet.get("Madria")


   misn.setNPC( _("Lieutenant"), "empire/unique/czesc" )
   misn.setDesc( bar_desc )
end


function accept ()
   -- Set marker to a system, visible in any mission computer and the onboard computer.
   misn.markerAdd( targetworld_sys, "low")
   ---Intro Text
   if not tk.yesno( title[1], text[1] ) then
      misn.finish()
   end
   -- Flavour text and mini-briefing
   tk.msg( title[2], text[2] )
   ---Accept the mission
   misn.accept()
  
   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   reward = 500000
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn.setDesc( string.format( misn_desc, targetworld:name(), targetworld_sys:name() ) )
   misn.osdCreate(title[2], {misn_desc})
   -- Set up the goal
   hook.land("land")
   person = misn.cargoAdd( "Person" , 0 )
end


function land()

   if planet.cur() == targetworld then
         misn.cargoRm( person )
         player.pay( reward )
         -- More flavour text
         tk.msg( title[3], text[3] )
         faction.modPlayerSingle( "Empire",3 );
         misn.finish(true)
   end
end

function abort()
   misn.finish(false)
end
