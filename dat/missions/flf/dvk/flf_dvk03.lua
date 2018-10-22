--[[

   Assault on Raelid
   Copyright (C) 2018 Julie Marchant <onpon4@riseup.net>

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

include "numstring.lua"
include "dat/missions/flf/flf_common.lua"

-- Localization stuff
title = {}
text = {}

title[1] = _("The Next Level")
text[1] = _([["Hello there, %s! You're just in time. We were just discussing our next operation against the Dvaered oppressors! Do you want in?"]])

title[2] = _("Not This Time")
text[2] = _([["Okay. Just let me know if you change your mind."]])

title[3] = _("A Decisive Strike")
text[3] = _([[You take a seat among the group. "Fantastic!" says Benito. "Let me get you caught up, then. Do you remember that mission in Raelid you helped with a while back?" You nod. You were wondering what you were actually creating a diversion from. "Yes, well, I never got around to telling you what we actually did there. See, we've been wanting to destroy Raelid Outpost for some time, mostly because it's often used as a front for trying to scout us out. So while you were getting the attention of those idiot Dvaered oppressors, we rigged a special bomb and covertly installed it onto the outpost!"]])

text[4] = _([["Now, the bomb is not perfect. Given how hastily we had to install the thing, we could not make it so that it could be detonated remotely. Instead, it has to be detonated manually, by blasting the station repeatedly. Shooting it down, in other words.
    "So here's the plan. We have hired a large group of pirates to help us out by creating a massive disturbance far away from our target. You are to wait until the coast is clear, then swarm in and attack the outpost with all you've got. You, %s, will lead the charge. You have to determine the optimal time, when the Dvaereds are far enough away for you to initiate the attack, but before the pirates are inevitably overwhelmed. Simply hail one of the others when it's time to attack, then make a beeline for Raelid Outpost and shoot at it with all you've got!"]])

text[5] = _([["You guys are some of our best pilots, so try not to get killed, eh? A moment of triumph is upon us! Down with the oppressors!" The last line earns Benito a cheer from the crowd. Well, time to get your ship ready for the battle.]])

misn_title = _("Assault on Raelid")
misn_desc = _("Join with the other FLF pilots for the assault on Raelid Outpost.")
misn_reward = _("A great victory against the Dvaereds")

npc_name = _("Benito")
npc_desc = _("Benito is seated at a table with several other FLF soldiers. She motions for you to come over.")

osd_title   = _("Assault on Raelid")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system")
osd_desc[2] = _("Wait until the coast is clear, then hail one of your wingmates")
osd_desc[3] = _("Attack Raelid Outpost until it is destroyed")
osd_desc[4] = _("Return to FLF base")
osd_desc["__save"] = true


function create ()
   missys = system.get( "Raelid" )
   if not misn.claim( missys ) then
      misn.finish( false )
   end

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   if tk.yesno( title[1], text[2]:format( player.name() ) ) then
      tk.msg( title[3], text[3] )
      tk.msg( title[3], text[4]:format( player.name() ) )
      tk.msg( title[3], text[5] )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( missys:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( missys:name() ) )
      marker = misn.markerAdd( missys, "plot" )
      misn.setReward( misn_reward )

      credits = 300000
      reputation = 20

      hook.enter( "enter" )
   else
      tk.msg( title[2], text[2] )
   end
end


function enter ()
end

