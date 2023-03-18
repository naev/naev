--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Alliance of Inconvenience">
 <unique />
 <priority>2</priority>
 <chance>30</chance>
 <done>Diversion from Haleb</done>
 <location>Bar</location>
 <faction>FLF</faction>
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

-- luacheck: globals pay_text misn_title setDescription land_flf (inherited from mission above, TODO get rid of this hack)

pay_text = {
   _([[Upon your return, Benito seems pleased to hear that the mission was successful. "Excellent," she says. "It's kind of an annoying detour, I know, but I appreciate your help very much. I'll try to have a better mission for you next time, eh?" You both grin and exchange some pleasantries before parting ways.]]),
}

function create ()
   mem.missys = system.get( "Zylex" )
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   mem.level = 3
   mem.ships = 5
   mem.has_boss = false
   mem.has_phalanx = false
   mem.has_kestrel = false
   mem.flfships = 0
   mem.reputation = 1

   mem.credits = 1e6

   mem.late_arrival = false
   mem.late_arrival_delay = rnd.uniform( 10.0, 120.0 )

   misn.setNPC( _("Benito"), "flf/unique/benito.webp", _("Benito is shuffling around papers and overall appearing a bit stressed. Perhaps you should see what is the matter.") )
end


function accept ()
   if tk.yesno( _("Inconvenient happenings"), fmt.f( _([[Benito glances up and immediately looks relieved to see you. It seems she has a problem she's dealing with. "Well met, {player}," she says. "I've just been thinking of how to deal with a problem for a while now, and, seeing you, that makes the solution so much simpler! Can you help me? It's nothing too huge, just destroying some more ships. We're prepared to give you {credits} in exchange."]]), {player=player.name(), credits=fmt.credits(mem.credits)} ) ) then
      tk.msg( _("Inconvenient happenings"), fmt.f( _([["Thanks! As always, you're a life saver. Well, for us, that is." Benito smirks before continuing. "See, a few pirates have decided to be opportunistic, going against the clans we are allied with, and attacking our ships. Now, we expected this to happen at some point, but this puts us in an annoying pickle: we need to dispatch the aggressors, which the pirate clans have already agreed is perfectly acceptable, but we can't go diverting our forces from more important tasks to fight rogue pirates. So that's where you come in: I need you to go to the {sys} system and dispatch this group of pirates that's causing us trouble. Like I said, the clans are okay with it, so you won't get into trouble with the pirates as a whole."
    Fighting pirates, huh? This sounds like it'll be almost like old times, before you joined the FLF. Interesting. Well, it's about time to get going, then.]]), {sys=mem.missys} ) )

      misn.accept()
      mem.osd_desc[1] = fmt.f( mem.osd_desc[1], {sys=mem.missys} )
      misn.osdCreate( _("Pirate Disturbance"), mem.osd_desc )

      local desc = setDescription()
      misn.setDesc( desc )

      misn.setTitle( fmt.f( misn_title[mem.level], {sys=mem.missys} ) )
      mem.marker = misn.markerAdd( mem.missys, "high" )
      misn.setReward( mem.credits )

      mem.pirate_ships_left = 0
      mem.job_done = false

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )
   end
end


function land_flf ()
   leave()
   mem.last_system = spob.cur()
   if spob.cur():faction() == faction.get("FLF") then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( mem.credits )
      flf.setReputation( 80 )
      faction.get("FLF"):modPlayerSingle( mem.reputation )
      misn.finish( true )
   end
end
