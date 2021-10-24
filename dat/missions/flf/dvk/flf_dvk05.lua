--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Alliance of Inconvenience">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <chance>30</chance>
  <done>Diversion from Haleb</done>
  <location>Bar</location>
  <faction>FLF</faction>
 </avail>
 <notes>
  <campaign>Save the Frontier</campaign>
 </notes>
</mission>
--]]
--[[

   Alliance of Inconvenience

--]]
local fmt = require "format"
local flf = require "missions.flf.flf_common"
require "missions.flf.flf_pirates"


pay_text = {}
pay_text[1] = _([[Benito seems pleased upon your return to hear that the mission was successful. "Excellent," she says. "It's kind of an annoying detour, I know, but I appreciate that your help very much. I'll try to have a better mission for you next time, eh?" You both grin and exchange some pleasantries before parting ways.]])

function create ()
   missys = system.get( "Zylex" )
   if not misn.claim( missys ) then misn.finish( false ) end

   level = 3
   ships = 5
   has_boss = false
   has_phalanx = false
   has_kestrel = false
   flfships = 0
   reputation = 1

   credits = 1e6

   late_arrival = false
   late_arrival_delay = rnd.uniform( 10.0, 120.0 )

   misn.setNPC( _("Benito"), "flf/unique/benito.webp", _("Benito is shuffling around papers and overall appearing a bit stressed. Perhaps you should see what is the matter.") )
end


function accept ()
   if tk.yesno( _("Inconvenient happenings"), _([[Benito looks up and immediately seems relieved to see you. It seems she has a problem she's dealing with. "Well met, %s," she says. "I've just been thinking of how to deal with a problem for a while now, and seeing you, that makes the solution so much simpler! Can you help me? It's nothing too huge, just destroying some more ships. We're prepared to give you %s in exchange."]]):format( player.name(), fmt.credits( credits ) ) ) then
      tk.msg( _("Inconvenient happenings"), _([["Thanks! As always, you're a life saver. Well, for us, that is." Benito smirks before continuing. "See, a few pirates have decided to be opportunistic, going against the clans we are allied with, and attack our ships. Now, we expected this to happen at some point, but this puts us in an annoying pickle: we need to dispatch the aggressors, which the pirate clans have already agreed is perfectly acceptable, but we can't go diverting our forces from more important tasks to fight rogue pirates. So that's where you come in: I need you to go to the %s system and dispatch this group of pirates that's causing us trouble. Like I said, the clans are okay with it, so you won't get into trouble with the pirates as a whole."
    Fighting pirates, huh? This sounds like it'll be almost like old times, before you joined the FLF. Interesting. Well, it's about time to get going, then.]]):format( missys:name() ) )

      misn.accept()
      osd_desc[1] = osd_desc[1]:format( missys:name() )
      misn.osdCreate( _("Pirate Disturbance"), osd_desc )

      local desc = setDescription()
      misn.setDesc( desc )

      misn.setTitle( misn_title[level]:format( missys:name() ) )
      marker = misn.markerAdd( missys, "high" )
      misn.setReward( fmt.credits( credits ) )

      pirate_ships_left = 0
      job_done = false

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )
   else
   end
end


function land_flf ()
   leave()
   last_system = planet.cur()
   if planet.cur():faction() == faction.get("FLF") then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( credits )
      flf.setReputation( 80 )
      faction.get("FLF"):modPlayerSingle( reputation )
      misn.finish( true )
   end
end
