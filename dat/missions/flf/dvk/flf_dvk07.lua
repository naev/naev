--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The FLF Split">
 <unique />
 <priority>2</priority>
 <chance>30</chance>
 <done>Assault on Haleb</done>
 <location>Bar</location>
 <faction>FLF</faction>
 <cond>faction.playerStanding("FLF") &gt;= 90</cond>
 <notes>
   <campaign>Save the Frontier</campaign>
 </notes>
</mission>
 --]]
--[[

   The FLF Split

--]]
local fmt = require "format"
local flf = require "missions.flf.flf_common"
require "missions.flf.flf_rogue"

-- luacheck: globals land_flf (inherited from mission above, TODO get rid of)

function create ()
   mem.missys = system.get( "Sigur" )
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   mem.level = 3
   mem.ships = 4
   mem.flfships = 2

   mem.credits = 100e3

   mem.late_arrival = true
   mem.late_arrival_delay = rnd.uniform( 10.0, 120.0 )

   misn.setNPC( _("Benito"), "flf/unique/benito.webp", _("Benito seems to be frantically searching for a pilot.") )
end


function accept ()
   if tk.yesno( _("The Split"), fmt.f( _([[As you approach, you notice that Benito has an unusually annoyed expression. But when she sees you, she calms down somewhat. "Ah, {player}." She sighs. "No one is willing to take up this mission, and while I can understand it's a tough one, it really has to be taken care of.
    "See, for some reason, a group of FLF pilots has decided to turn traitor on us. They're hanging around outside of Sindbad and shooting us down. They need to be stopped, but no one wants to get their hands dirty killing fellow FLF pilots. But they're not FLF pilots anymore! They betrayed us! Can't anyone see that?" She takes a deep breath. "Will you do it, please? You'll be paid for the service, of course."]]), {player=player.name()} ) ) then
      tk.msg( _("The Split"), _([["Yes, finally!" It's as if a massive weight has been lifted off of Benito's shoulders. "Everyone trusts you a lot, so I'm sure this will convince them that, yes, killing traitors is the right thing to do. They're no better than Dvaereds, or those Empire scum who started shooting at us recently! Thank you for accepting the mission. Now I should at least be able to get a couple more pilots to join in and help you defend our interests against the traitors. Good luck!"]]) )

      misn.accept()

      misn.setTitle( _("The Split") )
      misn.setDesc( _("A fleet of FLF soldiers has betrayed the FLF. Destroy this fleet.") )
      misn.setReward( _("Getting rid of treacherous scum") )
      mem.marker = misn.markerAdd( mem.missys, "high" )

      mem.osd_desc[1] = fmt.f( _("Fly to the {sys} system"), {sys=mem.missys} )
      misn.osdCreate( _("Rogue FLF"), mem.osd_desc )

      mem.rogue_ships_left = 0
      mem.job_done = false
      mem.last_system = spob.cur()

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )
   else
      tk.msg( _("The Split"), _([["Ugh, this is so annoying... I understand, though. Just let me know if you change your mind, okay?"]]) )
   end
end


function land_flf ()
   leave()
   mem.last_system = nil
   if spob.cur():faction() == faction.get("FLF") then
      tk.msg( "", _([[Upon your return to the station, you are greeted by Benito. "Thanks once again for a job well done. I really do appreciate it. Not only have those traitors been taken care of, the others have become much more open to the idea that, hey, traitors are traitors and must be eliminated." She hands you a credit chip. "Here is your pay. Thank you again."]]) )
      player.pay( mem.credits )
      flf.setReputation( 98 )
      flf.addLog( _([[Regrettably, some rogue FLF pilots have turned traitor, forcing you to destroy them. Your action helped to assure fellow FLF pilots that treacherous FLF pilots who turn on their comrades are enemies just like any other.]]) )
      misn.finish( true )
   end
end
