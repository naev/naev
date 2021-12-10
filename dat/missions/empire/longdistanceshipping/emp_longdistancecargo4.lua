--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Frontier Long Distance Recruitment">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <cond>faction.playerStanding("Empire") &gt;= 0</cond>
  <chance>75</chance>
  <done>Za'lek Long Distance Recruitment</done>
  <location>Bar</location>
  <faction>Empire</faction>
 </avail>
 <notes>
  <campaign>Empire Shipping</campaign>
 </notes>
</mission>
--]]
--[[

   Fourth diplomatic mission to Frontier space that opens up the Empire long-distance cargo missions.

   Author: micahmumper

]]--
local fmt = require "format"
local emp = require "common.empire"

-- Mission constants
local targetworld, targetworld_sys = planet.getS("The Frontier Council")

-- luacheck: globals land (Hook functions passed by name)

function create ()
   -- Note: this mission does not make any system claims.

   misn.setNPC( _("Lieutenant"), "empire/unique/czesc.webp", _("Lieutenant Czesc from the Empire Armada Shipping Division is sitting at the bar.") )
end


function accept ()
   -- Set marker to a system, visible in any mission computer and the onboard computer.
   misn.markerAdd( targetworld_sys, "low")
   ---Intro Text
   if not tk.yesno( _("Spaceport Bar"), _([["We have to stop running into each other like this." Lieutenant Czesc laughs at his joke. "Just kidding, you know I owe you for helping set up these contracts. So far, everything has been moving smoothly on our end. We're hoping to extend our relations to the Frontier Alliance. You know the drill by this point. Ready to help?"]]) ) then
      misn.finish()
   end
   -- Flavour text and mini-briefing
   tk.msg( _("Frontier Alliance Long Distance Recruitment"), fmt.f( _([["I applaud your commitment," Lieutenant Czesc says, "and I know these aren't the most exciting missions, but they're most useful. The frontier can be a bit dangerous, so make sure you're prepared. You need to drop the bureaucrat off at {pnt} in the {sys} system. After this, there should only be one more faction to bring into the fold. I expect to see you again soon."]]), {pnt=targetworld, sys=targetworld_sys} ) )
   ---Accept the mission
   misn.accept()

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setTitle(_("Frontier Long Distance Recruitment"))
   misn.setReward( fmt.credits( emp.rewards.ldc4 ) )
   local misn_desc = fmt.f(_("Deliver a shipping diplomat for the Empire to {pnt} in the {sys} system"), {pnt=targetworld, sys=targetworld_sys})
   misn.setDesc( misn_desc )
   misn.osdCreate(_("Frontier Alliance Long Distance Recruitment"), {misn_desc})
   -- Set up the goal
   hook.land("land")
   local c = commodity.new( N_("Diplomat"), N_("An Imperial trade representative.") )
   mem.person = misn.cargoAdd( c, 0 )
end


function land()

   if planet.cur() == targetworld then
         misn.cargoRm( mem.person )
         player.pay( emp.rewards.ldc4 )
         -- More flavour text
         tk.msg( _("Mission Accomplished"), fmt.f( _([[You deliver the diplomat to {pnt}, and she hands you a credit chip. Thankfully, Lieutenant Czesc mentioned only needing your assistance again for one more mission. This last bureaucrat refused to stay in her quarters, preferring to hang out on the bridge and give you the ins and outs of Empire bureaucracy. Only your loyalty to the Empire stopped you from sending her out into the vacuum of space.]]), {pnt=targetworld} ) )
         faction.modPlayerSingle( "Empire",3 )
         emp.addShippingLog( fmt.f( _([[You delivered a shipping bureaucrat to {pnt} for the Empire. Thankfully, Lieutenant Czesc mentioned only needing your assistance again for one more mission. This last bureaucrat refused to stay in her quarters, preferring to hang out on the bridge and give you the ins and outs of Empire bureaucracy. Only your loyalty to the Empire stopped you from sending her out into the vacuum of space.]]), {pnt=targetworld} ) )
         misn.finish(true)
   end
end

function abort()
   misn.finish(false)
end
