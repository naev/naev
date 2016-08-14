--[[

   FLF-Empire Negotiations 2
   Copyright (C) 2014-2016 Julie Marchant <onpon4@riseup.net>

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

-- localization stuff
lang = naev.lang()
if lang == "notreal" then
else -- default English
   title = {}
   text = {}

   title[1] = "More Negotiations"
   text[1] = [[Cheryl notices you and starts to speak. "Ah, %s! Just the person I was looking for. Are you ready for your next mission?"]]

   text[2] = [[Splendid! We just need you to meet with Commander Petridis again and work out the details of our cooperation. Report back here when you're done.]]

   title[3] = "The Condition"
   text[3] = [[As you approach Commander Petridis, he looks up at you and smiles. "Ah, %s. I was just thinking about you. I have good news! The Emperor himself has agreed to your proposal." He sits up straight and looks you dead in the eye. "However, there is one condition." You pause for a moment, then ask what that condition is.
    "Pretty simple, at least in the abstract," he answers. "Here's the deal: we will do what we can to peacefully put an end to Dvaered influence on the Frontier systems. We will also work to establish the FLF as a legitimate police organization to keep the Frontier safe from future threats of invasion.
    "However, there's going to need to be some serious effort on the part of the FLF. You are going to need to prove that you are after what you say you're after. That means putting an end to all terrorist activities and re-establishing yourselves as a purely defensive militia."]]

   text[4] = [[Petridis searches his coat for something for a short while, then finally pulls out a data chip. "Here, take this to your superiors at the FLF. The full details of the agreement are here." You take the chip, and Petridis stands up. "Alright, I have some work I have to do. I'll see you later, %s."]]

   title[5] = "The Review"
   text[5] = [[You are approached by Cheryl. "Welcome back, %s. I trust the mission went well?" You explain your results and hand her the chip. "Great to hear! I and the other higher-ups will review this agreement. I'll see you again when we reach our decision on what to do next. In any case, here is your payment for the mission. Thank you for your service." She hands you a credit chip and walks off.]]

   misn_title = "Empire Negotiations"
   misn_desc = "You are to meet with Empire Commander Petridis and discuss the cooperation that is to ensue between the FLF and the Empire."
   misn_reward = "The beginning of a beautiful friendship"

   npc_name = "Cheryl"
   npc_desc = "Cheryl sits by herself, just as she did last time. She must be waiting for you."

   emp_name = "Petridis"
   emp_desc = "Commander Petridis sits alone, idly reading the news terminal."

   osd_title   = "Empire Negotiations"
   osd_desc    = {}
   osd_desc[1] = "Go to %s in the %s system and talk to Commander Petridis"
   osd_desc[2] = "Return to FLF base and report back to Cheryl"
   osd_desc["__save"] = true
end


function create ()
   -- Note: this mission does not make any system claims.
   missys = system.get( "Raelid" )
   misplanet = planet.get( "Marius Station" )

   credits = 100000
   reputation = 5
   emp_reputation = 5

   misn.setNPC( npc_name, "neutral/miner2" )
   misn.setDesc( npc_desc )
end


function accept ()
   if tk.yesno( title[1], text[1]:format( player.name() ) ) then
      tk.msg( title[1], text[2] )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( misplanet:name(), missys:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc )
      marker = misn.markerAdd( missys, "low" )
      misn.setReward( misn_reward )

      job_done = false

      hook.land( "land" )
   else
   end
end


function land ()
   if job_done and planet.cur():faction():name() == "FLF" then
      tk.msg( title[5], text[5]:format( player.name() ) )
      player.pay( credits )
      faction.get("FLF"):modPlayerSingle( reputation )
      faction.get("Empire"):modPlayerSingle( emp_reputation )
      misn.finish( true )
   elseif planet.cur() == misplanet then
      npc = misn.npcAdd( "approach", emp_name, "empire/empire1", emp_desc )
   end
end


function approach ()
   tk.msg( title[3], text[3]:format( player.name() ) )
   tk.msg( title[3], text[4]:format( player.name() ) )

   misn.npcRm( npc )
   job_done = true
   if marker ~= nil then misn.markerRm( marker ) end
   misn.osdActive( 2 )
end
