--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Diversion from Raelid">
 <unique />
 <priority>2</priority>
 <chance>60</chance>
 <done>Disrupt a Dvaered Patrol</done>
 <location>Bar</location>
 <faction>FLF</faction>
 <cond>faction.playerStanding("FLF") &gt;= 10</cond>
 <notes>
  <campaign>Save the Frontier</campaign>
  <tier>4</tier>
 </notes>
</mission>
--]]
--[[

   Diversion from Raelid.

--]]
local fmt = require "format"
local flf = require "missions.flf.flf_common"
require "missions.flf.flf_diversion"

-- luacheck: globals success_text pay_text land (inherited from mission above, TODO remove horrible hack and make unique)

success_text = {
   _([[You receive a transmission. It's from Benito. "Operation successful!" she says. "You should get back to the base now before you get killed! I'll be waiting for you there."]]),
}

pay_text = {
   _([[As you dock the station, Benito approaches you with a smile. "Thank you for your help," she says. "The mission was a resounding success! What we've accomplished will greatly help our efforts against the Dvaereds in the future." She hands you a credit chip. "That's your payment. Until next time!" Benito sees herself out as a number of additional FLF soldiers congratulate you. It occurs to you that you never learned what the mission actually was. Perhaps you will find out some other time.]]),
}

function create ()
   mem.missys = system.get( "Tuoladis" )
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   mem.dv_attention_target = 20
   mem.credits = 250e3
   mem.reputation = 2

   misn.setNPC( _("Benito"), "flf/unique/benito.webp", _("Benito seems to want to speak with you.") )
end


function accept ()
   if tk.yesno( _("Taking One for the Team"), fmt.f(_([[Benito smiles as you approach her. "Hello again, {player}!" she says. "I have another mission for you, should you choose to accept it. See, we have... an important covert operation we need to launch in Raelid. I won't bore you with the details of that operation, but I need someone to distract the Dvaered forces while we do this. You'll basically need to travel to the {sys} system and wreak havoc there so that the Dvaereds go after you and not the soldiers conducting the operation.
    "Of course, this will be a highly dangerous mission, and I can't guarantee any backup for you. You will be paid substantially, however, and this will surely earn you more respect among our ranks. Would you be interested?"]]), {player=player.name(), sys=mem.missys} ) ) then
      tk.msg( _("Taking One for the Team"), _([["Great! The team in charge of the operation will be hiding out around Raelid until they get an opening from your efforts. I will message you when they succeed. Good luck, and try not to get yourself killed!" She grins, and you grin back. Now to cause some mayhem...]]) )

      misn.accept()

      mem.osd_desc[1] = fmt.f( mem.osd_desc[1], {sys=mem.missys} )
      misn.osdCreate( _("FLF Diversion"), mem.osd_desc )
      misn.setTitle( _("Diversion from Raelid") )
      misn.setDesc( fmt.f( _("A covert operation is being conducted in Raelid. You are to create a diversion from this operation by wreaking havoc in the nearby {sys} system."), {sys=mem.missys} ) )
      mem.marker = misn.markerAdd( mem.missys, "plot" )
      misn.setReward( _("Substantial pay and a great amount of respect") )

      mem.dv_attention = 0
      mem.job_done = false

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )
   else
      tk.msg( _("Maybe Another Time"), _([["OK, then. Feel free to come back later if you change your mind."]]) )
   end
end


function land ()
   if spob.cur():faction() == faction.get("FLF") then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( mem.credits )
      flf.setReputation( 30 )
      faction.get("FLF"):modPlayer( mem.reputation )
      flf.addLog( _([[You helped the FLF conduct some kind of operation in Raelid by distracting the Dvaereds in another system.]]) )
      misn.finish( true )
   end
end
