--[[

   FLF Instability
   Copyright (C) 2014-2017 Julie Marchant <onpon4@riseup.net>

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

title = {}
text = {}

title[1] = _("An Explanation Is Needed")
text[1] = _([[Cheryl has clearly been waiting for you. "Ah, hello, %s. I have another mission for you. See, some rogue FLF pilots have been attacking Dvaered and Empire ships lately. It's not many of them, but I need... oh, no, don't worry! This is not another mission to destroy rogue pilots. I just need you to go to Commander Petridis and explain the situation." You feel a sense of relief at the revelation. "Are you ready for this?"]])

text[2] = _([[Thank you. Report back here when you're done.]])

title[3] = _("The Situation")
text[3] = _([[An unfamiliar look of anger and confusion is in Commander Petridis's eyes. "What is the meaning of these attacks?" he demands. "And why have I been seeing FLF pilots fighting each other?"
    As you explain the situation, you see a look of relief on his face. "I see," he says. "That is quite troublesome, to say the least. But I'm relieved to hear that your organization itself has not turned traitor on us. By God, that would be a nightmare for me. I could get demoted, or even dishonorably discharged."]])

text[4] = _([[Quickly, the look on his face changes to one of concern. "But this is a serious situation. Dvaered pilots tend to be... hot-headed. This situation will make it much more difficult to convince them to play nice with those of you who are still on our side. And who knows how badly this will escalate?
    "But we will just have to perservere. I will let my superiors know what is going on, and we will do our best to sort out this mess from our end. I suspect, from now on, the Empire will take notice when you take out a rogue FLF squadron in our territory." Petridis stands and offers you his hand, which you shake. "Thank you for working with us, %s." With that, he sees himself out. Now, to report back to Cheryl...]])

title[5] = _("Not So Bad")
text[5] = _([[You approach Cheryl. "Ah, %s. Is that all sorted out?" You report that, yes, the message has been delivered, and with no hitches. "Perfect," she says. "I'm glad to hear there wasn't any bloodshed this time. Here is your pay. I have quite a lot of work to do now, so I'll see you when it's time for the next mission."]])

misn_title = _("Empire Meeting")
misn_desc = _("You are to meet with Empire Commander Petridis and discuss the situation with rogue FLF pilots.")
misn_reward = _("Sorting out this mess")

npc_name = _("Cheryl")
npc_desc = _("Cheryl seems to be waiting for you.")

emp_name = _("Petridis")
emp_desc = _("Commander Petridis seems to be expecting you. He seems somewhat anxious.")

osd_title   = _("Empire Meeting")
osd_desc    = {}
osd_desc[1] = _("Go to %s in the %s system and talk to Commander Petridis")
osd_desc[2] = _("Return to FLF base and report back to Cheryl")
osd_desc["__save"] = true


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
   tk.msg( title[3], text[3] )
   tk.msg( title[3], text[4]:format( player.name() ) )

   misn.npcRm( npc )
   job_done = true
   if marker ~= nil then misn.markerRm( marker ) end
   misn.osdActive( 2 )
end
