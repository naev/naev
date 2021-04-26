--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kex's Freedom 1">
 <flags>
  <unique />
 </flags>
 <avail>
  <location>Bar</location>
  <chance>100</chance>
  <planet>Minerva Station</planet>
 </avail>
 <notes>
  <campaign>Minerva</campaign>
  <evt_done name="Chicken Rendezvous" />
 </notes>
</mission>
--]]

--[[
--]]
local minerva = require "minerva"
local portrait = require 'portrait'
local vn = require 'vn'
require 'numstring'

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted go to targetsys
--  1: have to destroy all the enemy ships
--  2: return to kex
misn_state = nil

targetsys = "Provectus Nova"

misn_reward = "A step closer to Kex's freedom"
misn_title = "Freeing Kex"

function create ()
   if not misn.claim( system.get(targetsys) ) then
      misn.finish( false )
   end
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )

   generate_npc ()
end


function generate_npc ()
   if planet.cur() == planet.get("Minerva Station") then
      npc_kex = misn.npcAdd( "approach_kex", minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
      if var.peek("kex_talk_ceo") then
         npc_ceo = misn.npcAdd( "approach_ceo", minerva.ceo.name, minerva.ceo.portrait, minerva.ceo.description )
      end
   end
end


function approach_kex ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition()
   vn.na(_("You find Kex taking a break at his favourite spot at Minerva station."))

   --[[
   if done_cond then
      -- Remove unnecessary variables to keep it clean
      var.pop( "kex_talk_station" )
      var.pop( "kex_talk_ceo" )
      var.pop( "ceo_talk_intro" )
      misn.finish( true )
      return
   end
   --]]

   vn.label("menu_msg")
   kex(_([["What's up kid?"]]))
   vn.menu( function ()
      local opts = {
         { _("Ask about the station"), "station" },
         { _("Leave"), "leave" },
      }
      if var.peek("kex_talk_station") and misn_state == nil then
         table.insert( opts, 1, { _("Ask about the CEO"), "ceo" } )
      end
      if misn_state == 0 then
         table.insert( opts, 1, { _("Ask about the job"), "job" } )
      end
      return opts
   end )

   vn.label("station")
   kex(_([["Nothing really great about this place. Don't know why it brings so many people from all over the universe, most of whom end up returning in rags broke. This station is a graveyard of hopes and dreams."]]))
   kex(_([["This cycle a kid who was engaged to his childhood sweetheart came to celebrate his engagement. They got absorbed into the game and decided to bet their savings for the wedding."]]))
   kex(_([["He wouldn't listen to reason, and eventually lost all of his savings. He ended up getting into the fight with the security at the station, and ended up taken away in a stretcher with a broken back. Not very pretty."]]))
   kex(_([["Despite a life destroyed, management was fairly happy, seems like he had quite a lot saved up until he came here. Just another day at the station."]]))
   kex(_([["When you spend here so long as I have, you tend to see rhythm of the station, and it isn't very good. Brings out the worst in people."]]))
   kex(_([["Most of the problem lies in the management, they're all a bunch of assholes led by the CEO, which is even a bigger asshole."]]))
   vn.func( function ()
      var.push( "kex_talk_station", true )
   end )
   vn.jump("menu_msg")

   vn.label("ceo")
   vn.func( function ()
      if var.peek( "ceo_talk_intro" ) then
         vn.jump( "talked_ceo" )
      end
   end )
   kex(_([["They've been running this joint since before I got here. Lots of rumours of them doing shady stuff that some are bound to be true."]]))
   kex(_([["The station is lively enough that it is fairly hard to keep track of what is going on, but there are some patterns like mysterious packages and unaccounted amounts of credits disappearing."]]))
   kex(_([["The only thing keeping me from my freedom is the fact that the CEO legally owns me. They don't believe I am fully sentient, but I'm sure if they knew it would just make everything harder for me so I have to lay low."]]))
   vn.na(_("You ask about where you can find the CEO."))
   kex(_([["He shouldn't be too hard to find most of the time given his drinking habits. You can usually find him around the bar lounge on the 7th floor. You aren't going to go talk to him? There's no way that anything like that could work.
He lets out a sigh."]]))
   vn.func( function ()
      var.push( "kex_talk_ceo", true )
      if not npc_ceo then
         npc_ceo = misn.npcAdd( "approach_ceo", minerva.ceo.name, minerva.ceo.portrait, minerva.ceo.description )
      end
   end )
   vn.jump("menu_msg")

   vn.label("talked_ceo")
   kex(_([["Oh, you already talked to them? How did it go?"
He looks at you expectantly.]]))
   vn.na(_("You mention that you got nowhere."))
   kex(_([[He looks a bit glum.
"Yeah, I don't think there is talking sense into that one… Maybe if we…"
He stops to think a bit.]]))
   kex(_([["This may sound crazy, but I think it might work. You're a good pilot from what I hear right?"
You looks at you with determination.
"Given that the issue is the CEO, if we can somehow get rid of the CEO, there should be no issue, right?"]]))
   kex(_([["Since I know for a certainly that he is involved in, let's call it, unsavory business, all we have to do is get him bust and in the confusion it should be more than easy for me to go free."]]))
   kex(_([[He looks down at the floor.
"I know we've known each other for a relatively short time, but wouldn't you help a duck out? I might be able to make it worth your time afterwards."]]))
   vn.menu({
      { _("Help a duck out."), "help" },
      { _("Maybe later."), "nohelp" },
   })

   vn.label("help")
   kex(string.format(_([["Great! I managed to look at the station delivery logs and it seems like there is a shady delivery heading here. If you could could go to the %s system. All you have to do is intercept it and get the incriminating evidence and it should be easy as pie! I'll send you the precise information later."]]), _(targetsys)))
   kex(_([["Hope"]]))
   vn.func( function ()
      if misn_state==nil then
         misn_state = 0
         misn.accept()
         hook.land("generate_npc")
         hook.load("generate_npc")
         hook.enter("enter")
      end
   end )
   vn.jump("menu_msg")

   vn.label("nohelp")
   kex(_([[He looks dejected.
"I see. If you change your mind, I'll be around."]]))
   vn.jump("menu_msg")

   vn.label("job")
   kex(_([["We have to find the dirt on the CEO and get him removed. It is the only change I have for freedom."]]))
   kex(string.format(_([[They should be receiving a delivery. You should go intercept it at the %s system before it gets here. I have sent you all the precise information. It should be a breeze with with your piloting skills.]]), _(targetsys)))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.done()
   vn.run()
end


function approach_ceo ()
   vn.clear()
   vn.scene()
   local ceo = vn.newCharacter( minerva.vn_ceo() )
   vn.transition()
   vn.na(_("You find the Minerva CEO drinking alone. He looks somewhat drunk. You approach him and start a conversation."))

   ceo(_([["Is that you Fred?"
He looks at you and squints.
"You're not Fred."
He takes a large swig of his drink.]]))
   ceo(_([["Well, what do you want?"]]))
   vn.na(_("You ask about the cyborg chicken."))
   ceo(_([["The crown jewel of Minerva station? The goose that lays golden eggs? The, uh… pineapple on the pizza?"
He furrows his brows a bit.]]))
   ceo(_([["Ever since I got that absurd chicken, business has been booming. I wouldn't sell the beast for all the credits in the world!"]]))
   vn.na(_("You mention that everything must have its price."))
   ceo(_([[He squints again at you.
"I like your guts, but your guts aren't getting your hands on my chicken. Damn, that sounded better in my head."
He takes another swig from his drinks.]]))
   ceo(_([["Now unless you are getting me another drink you can scoot off."]]))
   vn.na(_("It doesn't look like you are getting anywhere. You take your leave."))
   vn.func( function ()
      var.push( "ceo_talk_intro", true )
   end )

   vn.done()
   vn.run()
end

function enter ()
   if system.cur() == system.get(targetsys) then
   end
end
