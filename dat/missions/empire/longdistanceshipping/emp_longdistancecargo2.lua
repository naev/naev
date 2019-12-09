--[[

   Second diplomatic mission to Dvaered space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--

include "dat/scripts/numstring.lua"
include "dat/scripts/jumpdist.lua"

bar_desc = _("Lieutenant Czesc from the Empire Armada Shipping Division is sitting at the bar.")
misn_title = _("Dvaered Long Distance Recruitment")
misn_reward = _("500,000 credits")
misn_desc = _("Deliver a shipping diplomat for the Empire to Praxis in the Ogat system.")
title = {}
title[1] = _("Spaceport Bar")
title[2] = _("Dvaered Long Distance Recruitment")
title[3] = _("Mission Accomplished")
text = {}
text[1] = _([[Lieutenant Czesc waves you over when he notices you enter the bar. "I knew we would run into each other soon enough. Great job delivering that bureaucrat. We should be up and running in Soromid space in no time!" He presses a button on his wrist computer. "We're hoping to expand to Dvaered territory next. Can I count on your help?"]])
text[2] = _([["Great!" says Lieutenant Czesc. "I'll send a message to the bureaucrat to meet you at the hanger. The Dvaered are, of course, allies of the Empire. Still, they offend easily, so try not to talk too much. Your mission is to drop the bureaucrat off on Praxis in the Ogat system. He will take it from there and report back to me when the shipping contract has been confirmed. Afterwards, keep an eye out for me in Empire space and we can continue the operation."]])
text[3] = _([[You drop the bureaucrat off on Praxis, and he hands you a credit chip. You remember Lieutenant Czesc told you to look for him on Empire controlled planets after you finish.]])


function create ()
 -- Note: this mission does not make any system claims.
 
      -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Ogat")
   targetworld = planet.get("Praxis")


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
