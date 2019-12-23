--[[

   First diplomatic mission to Soromid space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--

include "dat/scripts/numstring.lua"
include "dat/scripts/jumpdist.lua"

bar_desc = _("Lieutenant Czesc from the Empire Aramda Shipping Division is sitting at the bar.")
misn_title = _("Soromid Long Distance Recruitment")
misn_reward = _("500,000 credits")
misn_desc = _("Deliver a shipping diplomat for the Empire to Soromid Customs Central in the Oberon system.")
title = {}
title[1] = _("Spaceport Bar")
title[2] = _("Soromid Long Distance Recruitment")
title[3] = _("Mission Accomplished")
text = {}
text[1] = _([[You approach Lieutenant Czesc. His demeanour brightens as he sees you. "Hello! I've been looking for you. You've done a great job with those Empire Shipping missions and we have a new exicting opportunity. You see, the head office is looking to expand business with other factions, which has enormous untapped business potential. They need someone competent and trustworthy to help them out. That's where you come in. Interested?"]])
text[2] = _([["I knew I could count on you," Lieutenant Czesc exclaims. "These missions will be long distance, meaning that you'll usually have to go at least 3 jumps to make the delivery. In addition, you'll often find yourself in the territory of other factions, where the Empire may not be able to protect you. In return, you will be nicely compensated. He hits a couple buttons on his wrist computer. "First, we need to set up operations with the other factions. We'll need a bureaucrat to handle the red tape and oversee the operations. We will begin with the Soromid. I know those genetically modified beings are kind of creepy, but business is business. Please accompany a bureaucrat to Soromid Customs Central in the Oberon system. He will report back to me when this mission is accomplished. I tend to travel within Empire space handling minor trade disputes, so keep an eye out for me in the bar on Empire controlled planets."]])
text[3] = _([[You drop the bureaucrat off at Soromid Customs Central, and he hands you a credit chip. Lieutenant Czesc told you to keep an eye out for him in Empire space to continue the operation.]])


function create ()
 -- Note: this mission does not make any system claims.
 
      -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Oberon")
   targetworld = planet.get("Soromid Customs Central")


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
