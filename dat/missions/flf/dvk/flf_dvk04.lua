--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Diversion from Haleb">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <chance>40</chance>
   <done>Assault on Raelid</done>
   <location>Bar</location>
   <faction>FLF</faction>
   <cond>faction.playerStanding("FLF") &gt;= 70</cond>
  </avail>
 </mission>
 --]]
--[[

   Diversion from Haleb

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--]]

require "dat/missions/flf/flf_diversion.lua"
require "dat/missions/flf/flf_common.lua"

-- localization stuff
title = {}
text = {}

title[1] = _("This looks familiar...")
text[1] = _([[Benito greets you as always. After a few pleasentries, she gets down to business. "I've been looking for you, %s!" she says. "I have another special diversion operation for you. This time, it's a diversion in the %s system, so we can get some important work done in the Haleb system. It's the same deal as the diversion from Raelid you did some time ago." Aha, preparation for destruction of another Dvaered base! "You'll be paid %s credits if you accept. Would you like to help with this one?"]])

text[2] = _([[Benito grins. "I knew you would want to do it. As always, the team will be waiting for a chance to do their work and hail you when they finish. Good luck, not like a pilot as great as you needs it!" You grin, and Benito excuses herself. Time to cause some mayhem again!]])

title[3] = _("Maybe Another Time")
text[3] = _([["OK, then. Feel free to come back later if you change your mind."]])

success_text = {}
success_text[1] = _([[You receive a transmission from Benito. "Operation successful!" she says. "I've got your pay waiting for you back at home, so don't get yourself blown up on the way back!"]])

pay_text = {}
pay_text[1] = _([[When you return, Benito hands you the agreed-upon payment, after which you exchange some pleasentries before parting ways once again.]])

misn_title = _("Diversion from Haleb")
misn_desc = _("A covert operation is being conducted in Haleb. You are to create a diversion from this operation by wreaking havoc in the nearby %s system.")
misn_reward = _("%s credits")

npc_name = _("Benito")
npc_desc = _("Benito looks in your direction and waves you over. It seems your services are needed again.")

log_text = _([[You diverted Dvaered forces away from Haleb so that other FLF agents could complete an important operation there, most likely planting a bomb on another Dvaered base.]])


function create ()
   missys = system.get( "Theras" )
   if not misn.claim( missys ) then misn.finish( false ) end

   dv_attention_target = 40
   credits = 400000
   reputation = 3

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   if tk.yesno( title[1], text[1]:format(
         player.name(), missys:name(), numstring( credits ) ) ) then
      tk.msg( title[1], text[2] )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( missys:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( missys:name() ) )
      marker = misn.markerAdd( missys, "plot" )
      misn.setReward( misn_reward:format( numstring( credits ) ) )

      dv_attention = 0
      job_done = false

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )
   else
      tk.msg( title[3], text[3] )
   end
end


function land ()
   if planet.cur():faction() == faction.get("FLF") then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( credits )
      flf_setReputation( 75 )
      faction.get("FLF"):modPlayer( reputation )
      flf_addLog( log_text )
      misn.finish( true )
   end
end
