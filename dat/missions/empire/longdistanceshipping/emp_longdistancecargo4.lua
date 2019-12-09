--[[

   Fourth diplomatic mission to Frontier space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--

include "dat/scripts/numstring.lua"
include "dat/scripts/jumpdist.lua"

bar_desc = _("Lieutenant Czesc from the Empire Aramda Shipping Division is sitting at the bar.")
misn_title = _("Frontier Long Distance Recruitment")
misn_reward = _("500,000 credits")
misn_desc = _("Deliver a shipping diplomat for the Empire to The Frontier Council in Gilligan's Light system.")
title = {}
title[1] = _("Spaceport Bar")
title[2] = _("Frontier Alliance Long Distance Recruitment")
title[3] = _("Mission Accomplished")
text = {}
text[1] = _([["We have to stop running into each other like this." Lieutenant Czesc laughs at his joke. "Just kidding, you know I owe you for helping set up these contracts. So far, everything has been moving smoothly on our end. We're hoping to extend our relations to the Frontier Alliance. You know the drill by this point. Ready to help?"]])
text[2] = _([["I applaud your commitment," Lieutenant Czesc says, "and I know these aren't the most exciting missions, but they're most useful. The frontier can be a bit dangerous, so make sure you're prepared. You need to drop the bureaucrat off at The Frontier Council in Gilligan's Light system. After this, there should only be one more faction to bring into the fold. I expect to see you again soon."]])
text[3] = _([[You deliver the diplomat to The Frontier Council, and she hands you a credit chip. Thankfully, Lieutenant Czesc mentioned only needing your assistance again for one more mission. This last bureaucrat refused to stay in her quarters, preferring to hang out on the bridge and give you the ins and outs of Empire bureaucracy. Only your loyalty to the Empire stopped you from sending her out into the vacuum of space.]])


function create ()
 -- Note: this mission does not make any system claims.
 
      -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Gilligan's Light")
   targetworld = planet.get("The Frontier Council")


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
