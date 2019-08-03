--[[

   Third diplomatic mission to Za'lek space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--

include "dat/scripts/numstring.lua"
include "dat/scripts/jumpdist.lua"

bar_desc = _("Lieutenant Czesc from the Empire Aramda Shipping Division is sitting at the bar.")
misn_title = _("Za'lek Long Distance Recruitment")
misn_reward = _("500,000 credits")
misn_desc = _("Deliver a shipping diplomat for the Empire to Gerhart Station in the Ganth system.")
title = {}
title[1] = _("Spaceport Bar")
title[2] = _("Za'lek Long Distance Recruitment")
title[3] = _("Mission Accomplished")
text = {}
text[1] = _([[Lieutenant Czesc sits at the bar. He really does seem to handle business all across the Empire. You take the seat next to him. "Thanks to your help, the Empire Aramda Shipping Division will soon operate across the galaxy. Our next mission is to get House Za'lek on board. Interested in helping out again?"]])
text[2] = _([["I had a feeling you would!" says Lieutenant Czesc. "I've got another bureaucrat ready to establish trade ties. The Za'lek are rather mysterious, so keep your wits about you. The diplomat only needs to go to the Gerhart Station in the Ganth system. He will let me know when trade relations have been established. There is still more work to be done, so I expect to see you again soon."]])
text[3] = _([[You drop the diplomat off on Outbound Station, and she hands you a credit chip. Lieutenant Czesc mentioned more work, so you figure you'll run into him at a bar again soon.]])


function create ()
 -- Note: this mission does not make any system claims.
 
      -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Ganth")
   targetworld = planet.get("Gerhart Station")


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
