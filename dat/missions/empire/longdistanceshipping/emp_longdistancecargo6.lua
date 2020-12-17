--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Long Distance Recruitment">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <cond>faction.playerStanding("Empire") &gt;= 0</cond>
   <chance>100</chance>
   <done>Sirius Long Distance Recruitment</done>   
   <location>Bar</location>
   <faction>Empire</faction>
  </avail>
  <notes>
   <campaign>Long Distance Shipping</campaign>
  </notes>
 </mission>
 --]]
--[[

   Sixth (and final) mission that explains the Empire long-distance cargo missions.

   Author: micahmumper

]]--

require "numstring"
require "jumpdist"
require "missions/empire/common"

bar_desc = _("Lieutenant Czesc from the Empire Armada Shipping Division is sitting at the bar.")
misn_title = _("Empire Long Distance Recruitment")
misn_desc = _("Deliver Lieutenant Czesc to Halir in the Gamma Polaris system")
title = {}
title[1] = _("Spaceport Bar")
title[2] = _("Empire Long Distance Recruitment")
title[3] = _("Mission Accomplished")
text = {}
text[1] = _([[Lieutenant Czesc slaps you on the back as you take a seat next to him at the bar. "We've done it! We have set up Empire Armada Shipping outposts across quite a bit of the galaxy. I just have one more favor to ask. I need transport back to Halir in the Gamma Polaris system. Once there I can authorize you to help out with the long-distance shipping missions. Can I count on you?"]])
text[2] = _([[Internally you groan from the idea of having to do another haul across the galaxy for more paperwork, but at least you'll have access to new missions. Lieutenant Czesc excitedly gets up from the bar. "Let's get going as soon as possible. There's no place like home!"]])
text[3] = _([[Lieutenant Czesc exits your ship and takes a deep breath of air. "I love the smell of bureaucracy in the morning." He shakes your hand. "Thanks for all your help, Captain! Follow me to headquarters and we can do some paperwork to get you all set up. After that you should start to receive long-distance shipping missions. They pay better than our regular shipping missions, but often require traveling longer distances and into territory controlled by other factions. You'll probably be more likely to see them on the edges of Empire space where cargo is ready to head out to other factions. Again, I can't thank you enough! The Empire does not quickly forget such dedication."]])

log_text = _([[You transported Lieutenant Czesc to Halir for some paperwork. You can now do long-distance cargo missions for the Empire. They pay better than regular Empire shipping missions, but often require traveling longer distances and into territory controlled by other factions. You'll probably be more likely to see them on the edges of Empire space where cargo is ready to head out to other factions.]])


function create ()
 -- Note: this mission does not make any system claims.
 
      -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Gamma Polaris")
   targetworld = planet.get("Halir")


   misn.setNPC( _("Lieutenant"), "empire/unique/czesc" )
   misn.setDesc( bar_desc )
   if targetworld == startworld then --makes sure pilot is not currently on Gamma Polaris
       misn.finish(false)
    end
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
   reward = 50000 -- 500K
   misn.setTitle(misn_title)
   misn.setReward(creditstring(reward))
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
         faction.modPlayerSingle( "Empire",3 )
         emp_addShippingLog( log_text )
         misn.finish(true)
   end
end

function abort()
   misn.finish(false)
end
