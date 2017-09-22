--[[

   The Ceasefire
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

title[1] = _("We Can End This Destructive Conflict")
text[1] = _([[Cheryl notices you and motions you over. "Great news, %s! I have just received a report that the Dvaereds are finally open to a negotiation for cease-fire! This will probably be the most important mission you will ever go on. Are you ready?"]])

text[2] = _([["Perfect! Go meet with Commander Petridis. Someone from House Dvaered should be with him. You are to establish a cease-fire with House Dvaered, and if possible, a full-scale alliance with the Empire. Good luck!"]])

title[3] = "The Ceasefire"
text[3] = _([[As you approach the table, you can see that the Dvaered officer sitting with Petridis is skeptical. He cautiously, silently greets you as you sit down. The meeting mainly consists of you and Petridis discussing the terms of the alliance, and the Dvaered officer frowning. It seems that hatred runs both ways between the FLF and Dvaereds.]])

text[4] = _([[After some of the most intense negotiations you have ever been involved with in your life, the terms of both the ceasefire and the alliance are agreed on. You shake hands both with Petridis and with the Dvaered officer. Time to report back to base!]])

title[5] = _("The New FLF")
text[5] = _([[As you begin to approach Sindbad, you are hailed by what appears to be an FLF pilot. When you answer, you see a familiar face: it's Corporal Benito! But something seems off...
    "Hello, %s. I see you have finally done the unthinkable: made peace with those Dvaered scum. I thought you were one of us. But now, I can see clearly that I was mistaken. We, the true believers in the cause of the FLF, have banded together. We will become the new FLF. We will purge you traitorous scum, restore the organization to its former glory, and put an end to Dvaered oppression once and for all!" Before you have time to react, Benito terminates the conversation. Suddenly, you get readings. A group of rogue FLF ships is warping in!]])

title[6] = _("The Last Stand")
text[6] = _([[As rogue FLF ships continue to pour in, you begin to see the dire nature of the situation. The rogue FLF ships are endless. Before long, they will destroy you, and then they will take Sindbad. What will happen to your comrades, your friends, still in Sindbad? The thought sends chills down your spine as you remember Benito's last words to you.
    Suddenly, you notice an especially loud signal coming from the direction of Sindbad. It's a distress call! The signal will surely reach Empire and Dveared space, and it includes the coordinates of the secret jump point. You realize how desperate the situation must be for so much to be risked. All you can do now is fight, and hope that the Empire comes to your rescue.]])

title[7] = _("An End To Terrorism Approaches")
text[7] = _([[Having successfully negotiated a peace treaty with the Empire and a cease-fire with House Dvaered, and having successfully defended Sindbad from the assault by the New FLF, you are greeted with warmth for the first time in what feels like an eternity. The FLF may have fractured, but what remains appears to be stronger than ever. You do not see resentment in people's faces. On the contrary, everyone seems to be happy about the alliance and ceasefire.
    One of your comrades buys you a drink, and many others personally thank you for bringing about peace and protecting Sindbad. Finally, things are really looking up.]])

misn_title = _("Empire Meeting")
misn_desc = _("You are to meet with Empire Commander Petridis sort out the terms of a ceasefire with the Dvaereds.")
misn_reward = _("Peace")

npc_name = _("Cheryl")
npc_desc = _("She seems to be in a very cheerful mood.")

emp_name = _("Petridis")
emp_desc = _("Commander Petridis is sitting with an unfamiliar Dvaered officer.")

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
      tk.msg( title[7], text[7]:format( player.name() ) )
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
