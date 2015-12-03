--[[

   FLF-Empire Negotiations
   Copyright (C) 2014, 2015 Julian Marchant <onpon4@riseup.net>

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

include "dat/missions/flf/flf_common.lua"

-- localization stuff
lang = naev.lang()
if lang == "notreal" then
else -- default English
   title = {}
   text = {}

   title[1] = "A Better Way"
   text[1] = [[The woman looks up to you and smiles. She gestures for you to take a seat, which you do.
    "Greetings, %s", she says. "You're probably wondering who I am. My name is Cheryl, and I'm one of the higher-ranking officials of the FLF." You try to hide your disbelief in what she says, but judging from Cheryl's chuckle, she seems to see right through you. "Yes, I know. You probably imagined that I would be middle-aged and wearing a fancy uniform, didn't you?" You begin to respond to this statement, but she stops you, and her expression suddenly becomes serious.]]

   text[2] = [["Our records show that you have been of serious help to the Empire. It seems you were one of the people responsible for the destruction of the Collective, in particular. Is this correct?" You pause for a second, wondering why Cheryl would be asking such a question.
    Finally, you answer by agreeing that what she says is true, but assuring her that they didn't send you here. She laughs. "Don't be silly. I don't think you're a spy. We are not exactly friends with the Empire; the Empire and House Dvaered are allies, after all, at least on paper. But we are not enemies with them, either."]]

   text[3] = [["In fact, the Empire has become rather annoyed with the Dvaered lately. I think you may have witnessed some of this when you conducted a prisoner exchange for the Empire some time ago. If you will recall, the prisoner exchange was going perfectly smoothly until a squadron of Dvaered ships decided to attack us. In fact, many of our Empire prisoners died in the chaos, and I have been told that the Empire higher-ups were furious.
    "And that's actually what I wanted to talk to you about. Our records show, as well, that you have somehow managed to maintain a very good reputation with the Empire even while opposing the Dvaered with us. That takes some serious dedication, and I commend you for this! It also means you are the perfect person to bring the FLF and the Empire together. Would you be interested?"]]

   text[4] = [[Cheryl smiles. "Wonderful! Your participation in bringing the FLF and the Empire together will be the most substantial contribution to our effort you could ever provide!
    "Of course, this will not be easy, and it will take some time. The first thing we need to do is establish communications with the Empire. This is where you come in. I am going to talk to the other higher-ups in the FLF about appointing you as our official Empire ambassador. In the meantime, I would like you to perform your first duty: go to %s in the %s system, and talk to Commander Petridis. He is the one we usually communicate with when organizing things like prison exchanges. Tell him about our intentions and ask him to let his higher-ups know."
    You tell her that you will do your best, and she wishes you good luck before departing.]]
   
   title[5] = "First Contact"
   text[5] = [[As you approach the officer, a grin shows up on his face. Before you can ask him about Commander Petridis, he starts talking.
    "%s, right? You really are an interesting character, I gotta say. Never in a million years did I expect someone sympathetic to the FLF to help out the Empire as much as you did. If anything, I would have thought that the Collective and the FLF would make perfect allies. And yet here you are."]]

   text[6] = [[You finally get a chance to tell the officer that you're looking for Commander Petridis. At this, he laughs. "Well, your search is over. That's me. I guess that means you've got a message to the Empire from the FLF, eh?"
    Taking his cue, you explain to Petridis that the FLF wishes to establish friendly relations with the Empire.
    "Friendly relations? That would really be something! You know, to be completely honest, a lot of us in the Empire are sick to death of the way House Dvaered handles things. It's always, 'Shoot first, ask questions later.' Why, just last month, those clowns blasted a thief's ship to smitherines because they stole a priceless work of art, and destroyed the art in the process!"]]

   text[7] = [["That being said, House Dvaered is a part of the Empire, and while we don't really care so much about FLF ships engaging with the Dvaered military, you must understand that the Empire cannot join in on this. That is, not unless House Dvaered committed an act of treason against the Empire, which they haven't.
    "Still, I will pass your message along. Even if it doesn't get us anywhere, with you involved in this, I'm sure the higher-ups would be more than willing to start a more friendly dialog. Perhaps we can eventually act as an intermediate between you and House Dvaered, and end this destructive conflict between you two."]]

   text[8] = [[He stands and holds out his hand, which you shake. "It's been a pleasure to talk to you, %s. I hope to see you again in the future." He then leaves the bar. Time to report back to the FLF base!]]

   title[9] = "A New Beginning"
   text[9] = [[Cheryl waits for you as you land. "I trust your mission went well?" You confirm that, yes, you have spoken with Commander Petridis about the FLF's intentions. "Excellent," she remarks. She then gives you a firm handshake. "Congratulations, %s. You are now the official Empire ambassador of the FLF. Your payment for this first mission will be in your account shortly. Please check back later for your next mission." Cheryl then departs.]]

   misn_title = "Empire Negotiations"
   misn_desc = "You are to meet with Empire Commander Petridis and deliver a message that the FLF wishes to establish friendly relations with the Empire."
   misn_reward = "A chance for a new beginning"

   npc_name = "A young woman"
   npc_desc = "This woman appears to be by herself, which is somewhat unusual for an FLF member. She regularly glances in your direction."

   emp_name = "An Empire official"
   emp_desc = "There is an Empire official sitting by himself, enjoying a drink. He seems to take special interest in you."

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
   reputation = 20
   emp_reputation = 5

   misn.setNPC( npc_name, "neutral/miner2" )
   misn.setDesc( npc_desc )
end


function accept ()
   tk.msg( title[1], text[1]:format( player.name() ) )
   tk.msg( title[1], text[2] )
   if tk.yesno( title[1], text[3] ) then
      tk.msg( title[1], text[4]:format( misplanet:name(), missys:name() ) )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( misplanet:name(), missys:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc )
      marker = misn.markerAdd( missys, "plot" )
      misn.setReward( misn_reward )

      job_done = false

      hook.land( "land" )
   else
   end
end


function land ()
   if job_done and planet.cur():faction():name() == "FLF" then
      tk.msg( title[9], text[9]:format( player.name() ) )
      player.pay( credits )
      flf_setReputation( 60 )
      faction.get("FLF"):modPlayer( reputation )
      faction.get("Empire"):modPlayer( emp_reputation )
      misn.finish( true )
   elseif planet.cur() == misplanet then
      npc = misn.npcAdd( "approach", emp_name, "empire/empire1", emp_desc )
   end
end


function approach ()
   tk.msg( title[5], text[5]:format( player.name() ) )
   tk.msg( title[5], text[6] )
   tk.msg( title[5], text[7] )
   tk.msg( title[5], text[8]:format( player.name() ) )

   misn.npcRm( npc )
   job_done = true
   misn.osdActive( 2 )
end
