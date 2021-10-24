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
  <campaign>Empire Shipping</campaign>
 </notes>
</mission>
--]]
--[[

   Sixth (and final) mission that explains the Empire long-distance cargo missions.

   Author: micahmumper

]]--
local fmt = require "format"
local emp = require "common.empire"

text = {}
text[1] = _([[Lieutenant Czesc slaps you on the back as you take a seat next to him at the bar. "We've done it! We have set up Empire Armada Shipping outposts across quite a bit of the galaxy. I just have one more favor to ask. I need transport back to Halir in the Gamma Polaris system. Once there I can authorize you to help out with the long-distance shipping missions. Can I count on you?"]])
text[2] = _([[Internally you groan from the idea of having to do another haul across the galaxy for more paperwork, but at least you'll have access to new missions. Lieutenant Czesc excitedly gets up from the bar. "Let's get going as soon as possible. There's no place like home!"]])
text[3] = _([[Lieutenant Czesc exits your ship and takes a deep breath of air. "I love the smell of bureaucracy in the morning." He shakes your hand. "Thanks for all your help, Captain! Follow me to headquarters and we can do some paperwork to get you all set up. After that you should start to receive long-distance shipping missions. They pay better than our regular shipping missions, but often require traveling longer distances and into territory controlled by other factions. You'll probably be more likely to see them on the edges of Empire space where cargo is ready to head out to other factions. Again, I can't thank you enough! The Empire does not quickly forget such dedication."]])

function create ()
 -- Note: this mission does not make any system claims.

      -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Gamma Polaris")
   targetworld = planet.get("Halir")

   misn.setNPC( _("Lieutenant"), "empire/unique/czesc.webp", _("Lieutenant Czesc from the Empire Armada Shipping Division is sitting at the bar.") )
   if targetworld == startworld then --makes sure pilot is not currently on Gamma Polaris
       misn.finish(false)
    end
end


function accept ()
   -- Set marker to a system, visible in any mission computer and the onboard computer.
   misn.markerAdd( targetworld_sys, "low")
   ---Intro Text
   if not tk.yesno( _("Spaceport Bar"), text[1] ) then
      misn.finish()
   end
   -- Flavour text and mini-briefing
   tk.msg( _("Empire Long Distance Recruitment"), text[2] )
   ---Accept the mission
   misn.accept()

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   reward = 500e3
   misn.setTitle(_("Empire Long Distance Recruitment"))
   misn.setReward(fmt.credits(reward))
   misn.setDesc( string.format( _("Deliver Lieutenant Czesc to Halir in the Gamma Polaris system"), targetworld:name(), targetworld_sys:name() ) )
   misn.osdCreate(_("Empire Long Distance Recruitment"), {_("Deliver Lieutenant Czesc to Halir in the Gamma Polaris system")})
   -- Set up the goal
   hook.land("land")
   local c = misn.cargoNew( N_("Diplomat"), N_("An Imperial trade representative.") )
   person = misn.cargoAdd( c, 0 )
end


function land()

   if planet.cur() == targetworld then
         misn.cargoRm( person )
         player.pay( reward )
         -- More flavour text
         tk.msg( _("Mission Accomplished"), text[3] )
         faction.modPlayerSingle( "Empire",3 )
         emp.addShippingLog( _([[You transported Lieutenant Czesc to Halir for some paperwork. You can now do long-distance cargo missions for the Empire. They pay better than regular Empire shipping missions, but often require traveling longer distances and into territory controlled by other factions. You'll probably be more likely to see them on the edges of Empire space where cargo is ready to head out to other factions.]]) )
         misn.finish(true)
   end
end

function abort()
   misn.finish(false)
end
