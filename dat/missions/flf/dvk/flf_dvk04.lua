--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Diversion from Haleb">
 <unique />
 <priority>2</priority>
 <chance>40</chance>
 <done>Assault on Raelid</done>
 <location>Bar</location>
 <faction>FLF</faction>
 <cond>faction.playerStanding("FLF") &gt;= 70</cond>
 <notes>
  <campaign>Save the Frontier</campaign>
 </notes>
</mission>
--]]
--[[

   Diversion from Haleb

--]]
local fmt = require "format"
local flf = require "missions.flf.flf_common"
require "missions.flf.flf_diversion"

-- luacheck: globals success_text pay_text land (overwriting main mission, TODO get rid of this hack)

success_text = {
   _([[You receive a transmission from Benito. "Operation successful!" she says. "I've got your pay waiting for you back at home, so don't get yourself blown up on the way back!"]]),
}

pay_text = {
   _([[When you return, Benito hands you the agreed-upon payment, after which you exchange some pleasantries before parting ways once again.]]),
}

function create ()
   mem.missys = system.get( "Theras" )
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   mem.dv_attention_target = 40
   mem.credits = 400e3
   mem.reputation = 3

   misn.setNPC( _("Benito"), "flf/unique/benito.webp", _("Benito looks in your direction and waves you over. It seems your services are needed again.") )
end


function accept ()
   if tk.yesno( _("This looks familiar..."), fmt.f( _([[Benito greets you as always. After a few pleasantries, she gets down to business. "I've been looking for you, {player}!" she says. "I have another special diversion operation for you. This time, it's a diversion in the {sys} system, so we can get some important work done in the Haleb system. It's the same deal as the diversion from Raelid you did some time ago." Aha, preparation for destruction of another Dvaered base! "You'll be paid {credits} if you accept. Would you like to help with this one?"]]),
         {player=player.name(), sys=mem.missys, credits=fmt.credits(mem.credits)} ) ) then
      tk.msg( _("This looks familiar..."), _([[Benito grins. "I knew you would want to do it. As always, the team will be waiting for a chance to do their work and hail you when they finish. Good luck, not like a pilot as great as you needs it!" You grin, and Benito excuses herself. Time to cause some mayhem again!]]) )

      misn.accept()

      mem.osd_desc[1] = fmt.f( mem.osd_desc[1], {sys=mem.missys} )
      misn.osdCreate( _("FLF Diversion"), mem.osd_desc )
      misn.setTitle( _("Diversion from Haleb") )
      misn.setDesc( fmt.f( _("A covert operation is being conducted in Haleb. You are to create a diversion from this operation by wreaking havoc in the nearby {sys} system."), {sys=mem.missys} ) )
      mem.marker = misn.markerAdd( mem.missys, "plot" )
      misn.setReward( mem.credits )

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
      flf.setReputation( 75 )
      faction.get("FLF"):modPlayer( mem.reputation )
      flf.addLog( _([[You diverted Dvaered forces away from Haleb so that other FLF agents could complete an important operation there, most likely planting a bomb on another Dvaered base.]]) )
      misn.finish( true )
   end
end
