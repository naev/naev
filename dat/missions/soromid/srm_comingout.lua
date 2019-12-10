--[[

   Coming Out

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

title = {}
text = {}

title[1] = _("Just Some Transportation")
text[1] = _([[As you approach, the stranger still seems rather nervous, but speaks up. The stranger's voice is deeper than you expected, but you pay it no mind.
    "H-hi! N-nice to meet you. I'm... um, you know what, you can just call me 'C' for now. I'm not sure about the name yet. They/them pronouns if that's okay." You agree to the request, introduce yourself, and chat with them a bit. Despite the murmurs throughout the bar they seem like a perfectly nice person. You can't help but wonder why these people would speak so lowly of them.
    After a while, C crosses their arms in thought. "So... um, you're a pilot, you say, right? Thing is, I'm in need of some transportation, but I don't really have much money on me so no one's willing to take me. It's a bit far and there's a lot of pirates in the area, so I get why no one wants to do it, but you know... I just... I need to see my parents. Just something I need to tell them. I'm not picky about how long it takes, and I promise I won't cause any trouble.... Could you do this for me?"]])

text[2] = _([[C seems relieved at your answer. "Thank you so much," they say. "I really appreciate it. I'll make it up to you somehow. My parents live in %s in the %s system. Like I said, no rush. Just as long as I get there, that's what matters." As they walk with you, you notice other people in the bar staring at and eying the two of you. You can't help but wonder: does C have to deal with this every day?]])

text[3] = _([["Okay. I understand. Thanks anyway."]])

text[4] = _([["Oh, hi again. I'm still having trouble finding someone. Can you help me? It would mean so much if you could."]])

chatter = {}

chatter[1] = _([["I just want to say again, thank you so much for helping me," C says. "It's a bit nerve-wracking, coming out to my parents, and those people in the bar and in so many places... anyway, it's nice to meet someone who accepts me for who I am rather than treating me like some kind of freak. I hope my parents understand...."]])

chatter[2] = _([[C perks up. "Can I talk to you about something?" You respond affirmatively. "Thank you," they say. "I've been needing to talk to someone about this for months, but no one will listen. I feel like you will.
    "I was assigned male at birth, and society tends to see me as a man. But I've come to the conclusion that... I'm transgender. I feel like I should be... a woman, I think? I'm not sure. Does... does that make sense? I'm not stupid, am I?" You respond that, no, they're not stupid; everyone is different and no one can know C better than they can. C seems relieved to hear this.]])

chatter[3] = _([["Hey... is there any chance, um... I know I asked you to use they/them pronouns for me before, but could you use she/her pronouns instead, please? I hope that's not too much trouble...." You assure her that it's no trouble at all and the two of you have a rather long and interesting conversation about the place of pronouns in society. She seems relieved.]])

chatter[4] = _([["I've given it some thought. I think... my new name, I like the sound of 'Chelsea'." You respond that it sounds like a nice name and Chelsea smiles.]])

chatter[5] = _([[You and Chelsea have a long conversation about ships and piloting. It turns out that she's quite interested in the subject and has aspirations of being a mercenary some day. You talk about your adventures with passion and share some tips on how to get started with being a freelance pilot.]])

landtext = {}

landtext[1] = _([[As you step off the ship with Chelsea in tow, you can tell that she's nervous about the whole thing. She asks you one more favor. "Can you come with me, as a friend?" You smile and say that you would be delighted.
    As it turns out, Chelsea has arranged to see her parents at the bar. When you arrive, Chelsea's parents immediately recognize and greet her, calling her by a different name. A look of sadness appears on Chelsea's face as she hears this, but she quickly hides it and her parents don't seem to notice. Chelsea greets her parents, introduces you as her friend, then sits down. You sit down next to her.]])

landtext[2] = _([["It's been so long!" Chelsea's mother says. "Your hair's getting long! I'm so glad you were able to make it over."
    Chelsea laughs nervously. "There's something I have to tell you," she says. "I, um... I'm transgender. I would like for you to call me Chelsea and refer to me with she/her pronouns from now on... if that's okay."
    Chelsea's father responds immediately and briefly. "Sure," he says. "I don't have any problem with that. However you dress or whatever name you use, that's fine by me."
    Her mother is silent for a moment, then gets up out of her chair. Chelsea gets up as well, a look of fear in her eyes, then a look of surprise as her mother locks her in an embrace. She hugs her mother back and starts to sob. "I'm proud of you, Chelsea," her mother says. Out of Chelsea's view, her father shrugs and finishes his drink.]])

landtext[3] = _([[After Chelsea's mother releases her, she wipes a few tears from her eyes and gives you a friendly hug. "Thank you," she says. "You've been a great friend and joining me here has been a great help." She lets go of you. "I'm sure you must be busy! But hey, do come back once in a while, OK? I'm going to stick around on this planet for the time being. It'll be nice to see you again!"
    Taking your cue, you say goodbye for now, excuse yourself from the table, and make your way back to your ship. As you enter your cockpit you find a credit chip worth %s credits! It's not much, but it's something.]])

misn_title = _("Coming Out")
misn_desc = _("Your new friend needs you to take them to their parents in %s.")
misn_reward = _("The satisfaction of helping out a new friend")

npc_name = _("A different-looking stranger")
npc_desc = _("Many people are murmuring about this person, who it seems they don't like much if at all. You can't help but wonder why. Meanwhile, the stranger is sitting quietly at a table, alone.")

osd_desc    = {}
osd_desc[1] = _("Go to the %s system and land on the planet %s.")

function create ()
   misplanet, missys = planet.get( "Durea" )
   -- Note: This mission does not make system claims
   credits = 10000
   started = false
   chatter_index = 0

   -- FIXME: Make a real portrait for C
   misn.setNPC( npc_name, "none" )
   misn.setDesc( npc_desc )
end


function accept ()
   local txt = started and text[1] or text[4]
   started = true

   if tk.yesno( title[1], txt ) then
      tk.msg( title[1], text[2]:format( misplanet:name(), missys:name() ) )

      misn.accept()

      misn.setTitle( misn_title )
      misn.setDesc( misn_desc )
      misn.setReward( misn_reward )
      marker = misn.markerAdd( missys )

      osd_desc[1] = osd_desc[1]:format( missys, misplanet )
      misn.osdCreate( misn_title, osd_desc )

      hook.land( "land" )

      chatter_freq = time.create( 0, 0, 5000 )
      hook.date( chatter_freq, "chatter" )
   else
      tk.msg( title[1], text[3] )
   end
end


function chatter ()
   if chatter_index < #chatter then
      chatter_index = chatter_index + 1
      tk.msg( "", chatter[ chatter_index ] )
      hook.date( chatter_freq, "chatter" )
   end
end


function land ()
   if planet.cur() == misplanet then
      -- Use any remaining chatter boxes (make sure the player gets the
      -- whole story)
      while chatter_index < #chatter do
         chatter_index = chatter_index + 1
         tk.msg( "", chatter[ chatter_index ] )
      end

      tk.msg( "", landtext[1] )
      tk.msg( "", landtext[2] )
      tk.msg( "", landtext[3]:format( numstring( credits ) ) )
      player.pay(credits)
      misn.finish(true)
   end
end

