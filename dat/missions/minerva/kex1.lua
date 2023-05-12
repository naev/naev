--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kex's Freedom 1">
 <unique />
 <location>Bar</location>
 <chance>100</chance>
 <spob>Minerva Station</spob>
 <cond>player.evtDone("Chicken Rendezvous")</cond>
 <notes>
  <campaign>Minerva</campaign>
  <done_evt name="Chicken Rendezvous" />
 </notes>
</mission>
--]]

--[[
   Freeing Kex 1

   Simple mission where the player has to raid a small transport to start to get clues.
--]]
local minerva = require "common.minerva"
local love_shaders = require 'love_shaders'
local vn = require 'vn'
local fmt = require "format"
local lmisn = require "lmisn"

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted go to targetsys
--  1: have to destroy all the enemy ships
--  2: return to kex
mem.misn_state = nil
local escorts, mainguy -- Non-persistent state

local targetsys = system.get("Provectus Nova")
local jumpinsys = system.get("Waterhole")
local jumpoutsys = system.get("Limbo")

local money_reward = minerva.rewards.kex1

function create ()
   if not misn.claim( targetsys ) then
      misn.finish( false )
   end
   misn.setReward( _("A step closer to Kex's freedom") )
   misn.setTitle( _("Freeing Kex") )
   misn.setDesc( _("Kex wants you to help him find dirt on the Minerva CEO by raiding a transport headed to Minerva Station.") )

   -- We avoid creating a giver NPC and directly use "normal NPC"
   -- Have to make sure to check if misn.accept() works
   generate_npc ()
end


function generate_npc ()
   if spob.cur() == spob.get("Minerva Station") then
      misn.npcAdd( "approach_kex", minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
      if var.peek("kex_talk_ceo") then
         mem.npc_ceo = misn.npcAdd( "approach_ceo", minerva.ceo.name, minerva.ceo.portrait, minerva.ceo.description )
      end
   end
end


function approach_kex ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition("hexagon")
   vn.na(_("You find Kex taking a break at his favourite spot at Minerva station."))

   if mem.misn_state==2 then
      local maikki = minerva.vn_maikkiP{
            shader = love_shaders.color{ color={0,0,0,1} },
            pos = "right" }

      vn.na(_("You tell Kex about your encounter with the transports and how you weren't able to find the supposed cargo."))
      kex(_([["Damn it. That must have been a decoy. These delivery logs are always a mess, and they always seem to be full of nearly identical entries. I picked the most likely, but there was also another at the same time, and it could have been that one."]]))

      vn.appear( maikki, "slideleft", 1, "linear" )
      vn.sfxEerie()
      vn.na(_("You suddenly hear a large thump. You are only able to catch a glimpse of a shadow before it runs away."))
      vn.disappear( maikki, "slideleft", 1, "linear" )

      vn.na(_("You go investigate what happened and find a secured crate. Kex quietly follows and looks surprised when he sees the crate."))
      kex(_([["This is one of the secured crates that I was talking about! How the hell did it get here?"]]))
      vn.na(_("You look around but there seems to be nobody else other than the two of you."))
      kex(_([[Kex looks at you in the eyes.
"In cases like this it's probably best not to ask questions, and just take it as some kind of blessing. I've had things like this happen all the time in the nebula and nothing good ever came from investigating. Just remember to keep an eye open."]]))
      kex(_([[He now looks at the crate.
"Let me see if I can get this open. Mmmm… fairly shoddy Nexus lock. I don't think they make these anymore. Shouldn't be a problem for my implant system."]]))
      kex(_([[He puts his wing on the lock and you hear some sort of click. His eyes close and he begins to hum an old tune.  It almost sounds like a nursery rhyme. This goes on for a while before he suddenly jolts back with his eyes open.]]))
      vn.sfxBingo()
      kex(_([["OK, let us see what we have here."
The crate opens unceremoniously and Kex peers.
"Damn, looks like we have no incriminating evidence, however, it does seem like we can use this as a starting point."]]))
      vn.na(_([[He passes you the document which reads:
"The next shipment will be larger than expected, but I presume you will be able to deal with it as usual. Please take this commission and do whatever you like with it."
It is signed "Baroness Eve".]]))
      kex(_([["I'm not too sure who this 'Baroness Eve' is, but let me see if I can get some information on them and we can see what we can do from there on."]]))
      kex(_([["Oh, I almost forgot. There's quite a few credits in the crate too, I think it's only fair to give you most of them as a reward for your help."]]))
      vn.sfxMoney()
      vn.func( function () player.pay( money_reward ) end )
      vn.na(fmt.reward( money_reward ))
      kex(_([["Meet me up here again in a bit, I'm going to go get some information."
Kex runs off and disappears into the station.]]))
      vn.sfxVictory()
      vn.run()

      minerva.log.kex(_("You managed to find a crate destined to the Minerva CEO through luck, and found that it was sent by Baroness Eve."))

      -- Remove unnecessary variables to keep it clean
      var.pop( "kex_talk_station" )
      var.pop( "kex_talk_ceo" )
      var.pop( "ceo_talk_intro" )
      misn.finish( true )
      return
   end

   vn.label("menu_msg")
   kex(_([["What's up kid?"]]))
   vn.menu( function ()
      local opts = {
         { _("Ask about the station"), "station" },
         { _("Leave"), "leave" },
      }
      if var.peek("kex_talk_station") and mem.misn_state == nil then
         table.insert( opts, 1, { _("Ask about the CEO"), "ceo" } )
      end
      if mem.misn_state == 0 then
         table.insert( opts, 1, { _("Ask about the job"), "job" } )
      end
      return opts
   end )

   vn.label("station")
   kex(_([["Nothing really great about this place. Don't know why it brings so many people from all over the universe, most of whom end up returning broke and in rags. This station is a graveyard of hopes and dreams."]]))
   kex(_([["This cycle, a kid, who was engaged to his childhood sweetheart, came to celebrate his engagement. They got absorbed into the game and decided to bet the money they'd saved for their wedding."]]))
   kex(_([["He wouldn't listen to reason, and eventually lost all of his savings. He ended up getting into a fight with station security and they took him away in a stretcher with a broken back. Not very pretty."]]))
   kex(_([["Despite a life destroyed, management was fairly happy. It seems like he had quite a lot saved up until he came here. Just another day at the station."]]))
   kex(_([["When you've been here as long as I have, you tend to see the rhythm of the station, and it isn't very good. Brings out the worst in people."]]))
   kex(_([["Most of the problem lies in the management, they're all a bunch of assholes led by the CEO, who is an even bigger asshole."]]))
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
   kex(_([["They've been running this joint since before I got here. There are enough rumours of them doing shady stuff that some are bound to be true."]]))
   kex(_([["The station is lively enough that it is fairly hard to keep track of what is going on, but there are some patterns like mysterious packages and unaccounted amounts of credits disappearing."]]))
   kex(_([["The only thing keeping me from my freedom is the fact that the CEO legally owns me. They don't believe I am fully sentient, but I'm sure if they knew it would just make everything harder for me, so I have to lay low."]]))
   vn.na(_("You ask about where you can find the CEO."))
   kex(_([["He shouldn't be too hard to find. Given his drinking habits, you can usually find him around the bar lounge on the 7th floor. You aren't going to go talk to him, are you? There's no way that anything like that could work."
He lets out a sigh.]]))
   vn.func( function ()
      var.push( "kex_talk_ceo", true )
      if not mem.npc_ceo then
         mem.npc_ceo = misn.npcAdd( "approach_ceo", minerva.ceo.name, minerva.ceo.portrait, minerva.ceo.description )
      end
   end )
   vn.jump("menu_msg")

   vn.label("talked_ceo")
   kex(_([["Oh, you already talked to him? How did it go?"
He looks at you expectantly.]]))
   vn.na(_("You mention that you got nowhere."))
   kex(_([[He looks a bit glum.
"Yeah, I don't think there's talking any sense into that one… Maybe if we…"
He stops to think a bit.]]))
   kex(_([["This may sound crazy, but I think it might work. You're a good pilot from what I hear, right?"
He looks at you with determination.
"Given that the issue is the CEO, if we can somehow get rid of the CEO, there should be no issue, right?"]]))
   kex(_([["Since I know for certain that he is involved in, let's call it, "unsavoury business", all we have to do is get him busted and in the confusion it should be more than easy for me to go free."]]))
   kex(_([[He looks down at the floor.
"I know we've known each other for a relatively short time, but would you help a duck out? I might be able to make it worth your while afterwards."]]))
   vn.menu({
      { _("Help a duck out."), "help" },
      { _("Maybe later."), "nohelp" },
   })

   vn.label("help")
   kex(fmt.f(_([["Great! I managed to look at the station delivery logs and it seems like there is a shady delivery heading here. If you could could go to the {sys} system. All you have to do is intercept it and get the incriminating evidence and it should be easy as pie! I'll send you the precise information later."]]), {sys=targetsys}))
   kex(_([["It would be ideal if you can disable the ship and find the evidence itself, however, given that it is always delivered in secured vaults, you should be able to recover the vault from the debris if you roll that way."]]))
   vn.func( function ()
      if mem.misn_state==nil then
         misn.accept()

         minerva.log.kex(_("You agreed to help Kex to find dirt on the Minerva Station CEO to try to get him free."))
         mem.misn_marker = misn.markerAdd( targetsys )
         misn.osdCreate( _("Freeing Kex"),
            { fmt.f(_("Intercept the transport at {sys}"), {sys=targetsys}),
            _("Return to Kex at Minerva Station") } )
         mem.misn_state = 0
         hook.land("generate_npc")
         hook.load("generate_npc")
         hook.enter("enter")
      end
   end )
   vn.jump("menu_msg")

   vn.label("nohelp")
   kex(_([[He looks dejected.
"I see. If you change your mind, I'll be around."]]))
   vn.done()

   vn.label("job")
   kex(_([["We have to find dirt on the CEO and get him removed. It is the only chance I have for freedom."]]))
   kex(fmt.f(_([[They should be receiving a delivery. You should go intercept it at the {sys} system before it gets here. I have sent you all the precise information. It should be a breeze what with your piloting skills.]]), {sys=targetsys}))
   kex(_([["It would be ideal if you can disable the ship and find the evidence itself, however, given that it is always delivered in secured vaults, you should be able to recover the vault from the debris if you roll that way."]]))
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
   if mem.misn_state==1 then
      lmisn.fail(_("You were supposed to raid the transport!"))
   end
   if system.cur() == targetsys then
      if mem.misn_state == 0 then
         local fthugs = faction.dynAdd( "Mercenary", "Convoy", _("Convoy") )

         mainguy = pilot.add( "Rhino", fthugs, jumpinsys, _("Transport") )
         mainguy:setVisplayer(true)
         mainguy:setHilight(true)
         mainguy:control()
         mainguy:hyperspace( jumpoutsys )
         hook.pilot( mainguy, "death", "mainguy_dead" )
         hook.pilot( mainguy, "board", "mainguy_board" )
         hook.pilot( mainguy, "jump", "mainguy_left" )
         hook.pilot( mainguy, "attacked", "mainguy_attacked" )

         local function addescort( shipname )
            local p = pilot.add( shipname, fthugs, jumpinsys, _("Escort") )
            p:setLeader( mainguy )
            hook.pilot( p, "attacked", "mainguy_attacked" )
            return p
         end

         escorts = { addescort("Admonisher"),
                     addescort("Shark"),
                     addescort("Shark") }

         mem.misn_state = 1
      end
   end
end

function mainguy_left ()
   lmisn.fail(_("The transport got away!"))
end

function mainguy_attacked ()
   if not mainguy:exists() then return end
   if mem.mainguy_attacked_msg then return end

   if mainguy:exists() then
      mainguy:broadcast(_("Transport under attack! Help requested immediately!"))
      mem.mainguy_attacked_msg = true
      mainguy:setHostile(true)
   end

   for k,e in ipairs(escorts) do
      if e:exists() then
         e:setHostile(true)
      end
   end
end

function mainguy_board ()
   local reward = 50e3 + rnd.rnd()*50e3

   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_("You storm the transport and head towards the cargo bay, however, once you get there you find it is empty. You proceed to explore the rest of the ship to see if there is anything of interest."))
   vn.na(fmt.f(_("You are not able to find what you were looking for, but you were able to find {credits} that likely won't be necessary to the crew anymore."), {credits=fmt.credits(reward)}))
   vn.na(_("It might be best to report back to Kex to see if his information was correct."))
   vn.run()

   -- Permanently disable mainguy
   mainguy:disable()

   -- Message update
   minerva.log.kex(_("You boarded a transport destined for the Minerva CEO, but didn't find anything."))
   misn.markerMove( mem.misn_marker, spob.get("Minerva Station") )
   mem.misn_state = 2
   misn.osdActive(2)
   player.unboard()
end

function mainguy_dead ()
   hook.timer( 3.0, "mainguy_dead_scanned" )
end

function mainguy_dead_scanned ()
   player.msg(_("You scan the debris of the transport for any potential cargo, but can't find anything."))
   minerva.log.kex(_("You destroyed a transport destined for the Minerva CEO, but didn't find anything in the debris."))
   misn.markerMove( mem.misn_marker, spob.get("Minerva Station") )
   mem.misn_state = 2
   misn.osdActive(2)
end
