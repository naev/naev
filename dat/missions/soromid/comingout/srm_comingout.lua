--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Coming Out">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <chance>20</chance>
  <location>Bar</location>
  <faction>Soromid</faction>
 </avail>
 <notes>
  <campaign>Coming Out</campaign>
 </notes>
</mission>
--]]
--[[

   Coming Out

--]]
local fmt = require "format"
local srm = require "common.soromid"

-- luacheck: globals do_chatter init_chatter land (Hook functions passed by name)

-- Mission constants
local misplanet, missys = planet.getS( "Durea" )
local credits = 50e3

local chatter = {}
chatter[1] = _([["I just want to say again, thank you so much for helping me," C says. "It's a bit nerve-wracking, coming out to my parents, and I've met so many people who... anyway, it's nice to meet someone who understands and respects my wishes and doesn't react with sarcasm. I hope my parents understand...."
    After confirming that it's about the pronouns you use to refer to them, you simply say that it should be common decency to respect people's wishes regarding how they wish to be referred to. This leads to a conversation about common decency and people who do rude things for no good reason.]])

chatter[2] = _([[C perks up. "Can I tell you something?" You respond affirmatively. "Thank you," they say. "I was going to come out to my parents first, but I'm so nervous and... I feel safe coming out to you, you know? Since you've been so nice to me this entire time.
    "I was assigned male at birth, and society tends to see me as a man. But I've come to the conclusion that... I'm transgender. I feel like I should be... a woman, I think? I'm not sure. Does... does that make sense? I'm not being ridiculous, am I?" You respond that, no, they're not ridiculous; everyone is different and no one can know C better than they can. C seems relieved to hear this.]])

chatter[3] = _([["Hey... is there any chance, um... I know I asked you to use they/them pronouns for me before, but could you use she/her pronouns instead, please? I hope that's not too much trouble...." You assure her that it's no trouble at all and the two of you have a rather long and interesting conversation about the place of pronouns in society. You can tell that she's a lot happier and more comfortable than she was before.]])

chatter[4] = _([["I've given it some thought," C says. "I think... my new name, I like the sound of 'Chelsea'." You respond that it sounds like a nice name, plastering a smile on Chelsea's face.]])

chatter[5] = _([[You and Chelsea have a long conversation about ships and piloting. It turns out that she's quite interested in the subject and has aspirations of being a mercenary some day. You talk about your adventures with passion and share some tips on how to get started with being a freelance pilot.]])

local reminders = {}
reminders[1] = _("You idly talk some more with Chelsea about the joys of being a pilot.")
reminders[2] = _("You hear a small noise and nearly jump in your seat, but you look over your shoulder and see it's just Chelsea reading something.")
reminders[3] = _("Chelsea watches in awe as you work the ship. You can't help but smile a little.")
reminders[4] = _("Chelsea remarks that she likes a song you're listening to, which gets you into a conversation about music you and Chelsea like to listen to.")
reminders[5] = _("Somehow, you and Chelsea get into a discussion about drinks that each of you likes and dislikes, and stories of bars you've been to throughout the galaxy.")
reminders[6] = _("Chelsea dozes off in a chair for a few hectoseconds before waking up with a yawn, prompting you to yawn as well.")
reminders[7] = _("You start to forget for a while that you have a passenger until Chelsea asks you a question about something you're doing.")
reminders[8] = _("You talk to Chelsea a little about some interesting experiences you've had as a pilot.")
reminders[9] = _("You have a brief conversation with Chelsea about interesting sights you both have seen in your travels.")

function create ()
   -- Note: This mission does not make system claims
   if system.cur():jumpDist( missys, true ) < #chatter * 3 / 2 then
      misn.finish( false )
   end

   mem.started = false
   mem.chatter_index = 0

   misn.setNPC( _("Quiet stranger"), "soromid/unique/chelsea.webp", _("A stranger is sitting quietly at a table, alone, glancing around the bar. In need of a suitable pilot, perhaps?") )
end


function accept ()
   local txt = mem.started and _([["Oh, hi again. I'm still having trouble finding someone. Can you help me? It would mean so much if you could."]]) or _([[The stranger seems rather nervous as you approach, but speaks up. The stranger's voice is deeper than you expected from their appearance, but you pay it no mind.
    "H-hi! N-nice to meet you. I'm... um, you know what, you can just call me 'C' for now. I'm not sure about the name yet. They/them pronouns if that's okay." You agree to the request, introduce yourself, and chat with them a bit. They seem like a nice person.
    After a while, C crosses their arms in thought. "So... um, you're a pilot, you say, right? Thing is, I'm in need of some transportation, but I don't really have much money on me so no one's willing to take me. It's a bit far and there's a lot of pirates in the area, so I get why no one wants to do it, but you know... I just... I need to see my parents. Just something I need to tell them. I'm not picky about how long it takes, and I promise I won't cause any trouble.... Could you do this for me?"]])
   mem.started = true

   if tk.yesno( _("Just Some Transportation"), txt ) then
      tk.msg( _("Just Some Transportation"), fmt.f(_([[C seems relieved at your answer. "Thank you so much," they say. "I really appreciate it. I'll make it up to you somehow. My parents live in {pnt} in the {sys} system. Like I said, no rush. Just as long as I get there, that's what matters."]]), {pnt=misplanet, sys=missys} ) )

      misn.accept()

      misn.setTitle( _("Coming Out") )
      misn.setDesc( fmt.f(_("Your new friend needs you to take them to their parents in {sys}."), {sys=missys} ) )
      misn.setReward( _("The satisfaction of helping out a new friend") )
      mem.marker = misn.markerAdd( missys, "low" )

      misn.osdCreate( _("Coming Out"), {
         fmt.f(_("Go to the {sys} system and land on the planet {pnt}."), {sys=missys, pnt=misplanet} ),
      } )

      hook.land( "land" )

      mem.chatter_freq = time.create( 0, 10, 0 )
      mem.chatter_freq_mod = 8000
      mem.reminder_freq = time.create( 0, 20, 0 )
      mem.date_hook = hook.date( mem.chatter_freq, "init_chatter" )
   else
      tk.msg( _("Just Some Transportation"), _([["Okay. I understand. Thanks anyway."]]) )
      misn.finish()
   end
end


-- Using a timer to make sure the messages don't show up immediately
-- as you jump in (at least, not most of the time; it's technically
-- still possible and that's fine).
function init_chatter ()
   hook.rm( mem.timer_hook )
   mem.timer_hook = hook.timer( 10.0, "do_chatter" )
end


function do_chatter ()
   local freq = mem.reminder_freq

   if mem.chatter_index < #chatter then
      mem.chatter_index = mem.chatter_index + 1
      tk.msg( "", chatter[ mem.chatter_index ] )
      if mem.chatter_index < #chatter then freq = mem.chatter_freq end
   else
      local i = rnd.rnd( 1, #reminders )
      tk.msg( "", reminders[ i ] )
   end

   local this_mod = rnd.rnd(-mem.chatter_freq_mod, mem.chatter_freq_mod)
   hook.rm( mem.date_hook )
   mem.date_hook = hook.date( freq + time.create( 0, 0, this_mod ), "init_chatter" )
end


function land ()
   if planet.cur() == misplanet then
      -- Use any remaining chatter boxes (make sure the player gets the
      -- whole story)
      while mem.chatter_index < #chatter do
         mem.chatter_index = mem.chatter_index + 1
         tk.msg( "", chatter[ mem.chatter_index ] )
      end

      tk.msg( "", _([[As you step off the ship with Chelsea in tow, you can tell that she's nervous about the whole thing. She asks you one more favor. "Can you come with me, as a friend?" You smile and say that you can.
    As it turns out, Chelsea has arranged to see her parents at the bar. When you arrive, Chelsea's parents immediately recognize and greet her, calling her by a different name. A look of sadness appears on Chelsea's face as she hears this, but she quickly hides it and her parents don't seem to notice. Chelsea greets her parents, introduces you as her friend, then sits down. You sit down next to her.]]) )
      tk.msg( "", _([["It's been so long!" Chelsea's mother says. "Your hair's getting long! I'm so glad you were able to make it over."
    Chelsea laughs nervously. "There's something I have to tell you," she says. "I, um... I'm transgender. I would like for you to call me Chelsea and refer to me with she/her pronouns from now on... if that's okay."
    Chelsea's father responds immediately and briefly. "Sure," he says. "I don't have any problem with that. However you dress or whatever name you use, that's fine by me."
    Her mother is silent for a moment, then gets up out of her chair. Chelsea gets up as well, a look of fear in her eyes, then a look of surprise as her mother locks her in an embrace. She hugs her mother back and starts to sob. "I'm proud of you, Chelsea," her mother says. Out of Chelsea's view, her father shrugs and finishes his drink.]]) )
      tk.msg( "", fmt.f(_([[After Chelsea's mother releases her, she wipes a few tears from her eyes and gives you a friendly hug. "Thank you," she says. "You've been a great friend and joining me here has been a great help." She lets go of you. "I'm sure you must be busy! But hey, do come back once in a while, OK? I'm going to stick around on this planet for the time being. It'll be nice to see you again!"
    Taking your cue, you say goodbye for now, excuse yourself from the table, and make your way back to your ship. As you enter your cockpit you find a credit chip worth {credits}! It's not much, but it's something.]]), {credits=fmt.credits(credits)} ) )
      player.pay(credits)

      local t = time.get():tonumber()
      var.push( "comingout_time", t )

      srm.addComingOutLog( _([[You have made a new friend, Chelsea. You helped escort her to her parents and helped her feel secure coming out as transgender to her parents. Chelsea has asked you to return to Durea to visit once in a while.]]) )

      misn.finish(true)
   end
end
