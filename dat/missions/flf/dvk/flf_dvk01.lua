--[[

   Diversion from Raelid.
   Copyright (C) 2014, 2015, 2018 Julie Marchant <onpon4@riseup.net>

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

include "dat/missions/flf/flf_diversion.lua"

-- localization stuff
title = {}
text = {}

title[1] = _("Taking One for the Team")
text[1] = _([[Benito smiles as you approach her. "Hello again, %s!" she says. "I have another mission for you, should you choose to accept it. See, we have... an important covert operation we need to launch in Raelid. I won't bore you with the details of that operation, but I need someone to distract the Dvaered forces while we do this. You'll basically need to travel to the %s system and wreak havoc there so that the Dvaereds go after you and not the soldiers conducting the operation.
    "Of course, this will be a highly dangerous mission, and I can't guarantee any backup for you. You will be paid substantially, however, and this will surely earn you more respect among our ranks. Would you be interested?"]])

text[2] = _([["Great! The team in charge of the operation will be hiding out around Raelid until they get an opening from your efforts. I will message you when they succeed. Good luck, and try not to get yourself killed!" She grins, and you grin back. Now to cause some mayhem...]])

title[3] = _("Maybe Another Time")
text[3] = _([["OK, then. Feel free to come back later if you change your mind."]])

success_text = {}
success_text[1] = _([[You receive a transmission. It's from Benito. "Operation successful!" she says. "You should get back to the base now before you get killed! I'll be waiting for you there."]])

pay_text = {}
pay_text[1] = _([[As you dock the station, Benito approaches you with a smile. "Thank you for your help," she says. "The mission was a rousing success! What we've accomplished will greatly help our efforts against the Dvaereds in the future." She hands you a credit chip. "That's your payment. Until next time!" And with that, she sees herself out as a number of additional FLF soldiers congratulate you. It occurs to you that you never learned what the mission actually was. Perhaps you will find out some other time.]])

misn_title = _("Diversion from Raelid")
misn_desc = _("A covert operation is being conducted in Raelid. You are to create a diversion from this operation by wreaking havoc in the nearby %s system.")
misn_reward = _("Substantial pay and a great amount of respect")

npc_name = _("Benito")
npc_desc = _("Benito seems to want to speak with you.")


function create ()
   missys = system.get( "Tuoladis" )
   if not misn.claim( missys ) then misn.finish( false ) end

   dv_attention_target = 20
   credits = 250000
   reputation = 10

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   if tk.yesno( title[1], text[1]:format( player.name(), missys:name() ) ) then
      tk.msg( title[1], text[2] )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( missys:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( missys:name() ) )
      marker = misn.markerAdd( missys, "plot" )
      misn.setReward( misn_reward )

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
   if planet.cur():faction():name() == "FLF" then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( credits )
      flf_setReputation( 35 )
      faction.get("FLF"):modPlayer( reputation )
      misn.finish( true )
   end
end
