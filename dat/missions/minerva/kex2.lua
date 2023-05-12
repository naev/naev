--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kex's Freedom 2">
 <unique />
 <location>Bar</location>
 <chance>100</chance>
 <spob>Minerva Station</spob>
 <done>Kex's Freedom 1</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
   Freeing Kex 2

   Fairly straight forward mission where the player has to go to the objective
   to steal some stuff before running away from tons of enemy ships.
--]]
local minerva = require "common.minerva"
local love_shaders = require 'love_shaders'
local vn = require 'vn'
local fmt = require "format"

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted go to targetplanet
--  1: run away and return alive to minerva station
--  2: got away
mem.misn_state = nil
local blockade -- Non-persistent state

local targetplanet, targetsys = spob.getS("Niflheim Enclave")

local money_reward = minerva.rewards.kex2

function create ()
   if not misn.claim( targetsys ) then
      misn.finish( false )
   end
   misn.setReward( _("A step closer to Kex's freedom") )
   misn.setTitle( _("Freeing Kex") )
   misn.setDesc( fmt.f(_("You have been entrusted with stealing information from Baroness Eve at {pnt} in the {sys} system."), {pnt=targetplanet, sys=targetsys}) )

   misn.setNPC( minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
end

function accept ()
   approach_kex()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end

   misn.accept()

   minerva.log.kex(_("You have agreed to help Kex steal information from Baroness Eve.") )

   misn.osdCreate( _("Freeing Kex"),
      { fmt.f(_("Go to {pnt} in the {sys} system and hack the main database"), {pnt=targetplanet, sys=targetsys} ),
      _("Return to Kex at Minerva Station") } )

   mem.misn_marker = misn.markerAdd( targetplanet )

   hook.land("generate_npc")
   hook.load("loadfunc")
   hook.enter("enter")

   generate_npc()
end


function loadfunc ()
   -- Reset state on load if player got killed when trying to flee
   if mem.misn_state == 1 then
      mem.misn_state = 0
   end
   generate_npc()
end


function generate_npc ()
   if spob.cur() == spob.get("Minerva Station") then
      misn.npcAdd( "approach_kex", minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
   elseif spob.cur() == targetplanet and mem.misn_state==0 then
      misn.npcAdd( "approach_terminal", _("Terminal"), minerva.terminal.portrait, _("A discrete terminal in the corner of the landing bay, you should be able to use it to load the program Kex gave you.") )
   end
end


function approach_terminal ()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(fmt.f(_("You land on {pnt} and discretely approach one of the terminals.  You upload the program that Kex gave you."), {pnt=targetplanet}))
   vn.na(_("The terminal dimly lights up and begins to compute whatever was in the program. After a while it seems to begin downloading large amounts of data while you nervously wait."))
   vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
   vn.na(_("Suddenly, an alarm begins to blare. Before the security screen appears on the terminal, you are able to grab the program as you make a run for your ship. In the background, you hear people yelling and running around. It seems like you have to get out of here as soon as possible."))
   vn.na(_("This is probably what Kex meant about things going sour. It seems like you have a head start on your pursuers, but you may not be able to count on it for long."))
   vn.run()

   -- Advance mission and get out of there
   misn.markerMove( mem.misn_marker, spob.get("Minerva Station") )
   mem.misn_state = 1
   misn.osdActive(2)
   player.takeoff()
end

function approach_kex ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition("hexagon")

   -- Mission is over
   if mem.misn_state==2 then

      vn.na(_("Still tired from your escapade, you look for Kex at his favourite spot at Minerva station."))
      kex(_([["You look like a mess kid. You alright?"]]))
      vn.na(_("You explain how you weren't able to get all the data before you had to bug out of there, and hand him what data you were able to recover."))
      kex(_([["Damn, security was tighter than anticipated. I'm glad to see you made it in one piece despite the opposition. Aristocrats always seem to have cards up their sleeves."]]))
      kex(_([["Let me take a brief look at the data you were able to collect."
He plugs in the program directly into a port under his wing and his eyes go blank for a bit.
"I see, looks like you were able to get quite a lot of data. However, their database had much more than expected. It will probably take me quite a bit longer to process this all and see what we got."]]))
      kex(_([["In the meantime, it looks like we were also able to grab some credits. I'll wire you a reward for your efforts."]]))
      vn.sfxMoney()
      vn.func( function () player.pay( money_reward ) end )
      vn.na(fmt.reward( money_reward ))
      kex(_([["I'm going to go process this data. It's going to take me a while, but come back here in a period or so and I should hopefully have some more results."]]))
      vn.sfxVictory()
      vn.run()

      minerva.log.kex(_("You stole information from Baroness Eve for Kex."))
      misn.finish( true )
      return
   end

   vn.na(_("You find Kex taking a break at his favourite spot at Minerva station."))

   vn.label("menu_msg")
   kex(_([["What's up kid?"]]))
   vn.menu( function ()
      local opts = {
         { _("Ask about being a duck"), "duck" },
         { _("Leave"), "leave" },
      }
      if mem.misn_state == nil then
         table.insert( opts, 1, { _("Ask about Baroness Eve"), "baroness" } )
      end
      if mem.misn_state == 0 then
         table.insert( opts, 1, { _("Ask about the job"), "job" } )
      end
      return opts
   end )

   vn.label("baroness")
   kex(fmt.f(_([["Glad you asked. It turns out she's apparently a big shot at {pnt} in the {sys} system. She deals in all sorts of trade between the Empire and the Za'lek, although there also seems to be some nasty rumours about her floating around."]]), {pnt=targetplanet, sys=targetsys}))
   kex(_([["Although it's not very clear what exactly happened, there was a fairly big incident that ended up in a large fight between mercenaries, Empire forces, and pirates in the system. It disrupted the trade routes of the area for quite a few periods."]]))
   kex(_([["I was able to get more information from some other documents in the crate, as seems like they were fairly sloppy with some of the security details. I have managed to put together a small program that hopefully should be able to access their main database, however, I'm going to need you to upload it by hand."]]))
   kex(fmt.f(_([["This time the job is probably going to be much trickier than the last. Even though the documents had some security issues, it is likely that things will be different once you get over to {pnt}. Still, I think you should be able to handle this easily. Would you be up for the challenge?"]]), {pnt=targetplanet} ))
   vn.menu( {
      { _("Accept"), "accept" },
      { _("Decline"), "decline" },
   } )

   vn.label("accept")
   kex(fmt.f(_([["Great! Let me get you set up with the program. All you have to do is land on {pnt} and plug it in to any terminal you find around the docks. That should also make it easier for you to get out of there if things go sour."]]), {pnt=targetplanet}))
   vn.func( function ()
      mem.misn_state = 0
   end )
   vn.jump("menu_msg")

   vn.label("decline")
   kex(_([[He looks dejected.
"I see. If you change your mind, I'll be around."]]))
   vn.done()

   vn.label("job")
   kex(_([["So the idea is to try to hack into the Baroness Eve's database and see if we can get any dirt of the CEO of Minerva Station. Since we already know that they have dealings with each other, it seems like there has to be something there if we can access all the data."]]))
   kex(fmt.f(_([["All you have to do is land on {pnt} and upload the program I gave you to any terminal that there should be around the docks. That should also make it easier for you to get out of there if things go sour."]]), {pnt=targetplanet}))
   kex(_([["It's likely that there will be more security around this time, so make sure you take a ship that can deal with trouble. Although, I don't think that will be a problem for you."]]))
   vn.jump("menu_msg")

   vn.label("duck")
   kex(_([["It's as awful as you would expect it to be. I mean, sure I can float and fly short distances, but I can't reach any tall places and have to constantly avoid getting stepped on."]]))
   kex(_([["The worst part is the lack of communication. I mean, I've sort of lost my humanity and can't engage in all the activities I used to like. You know, drinking with friends, going out, basic human empathy, shit like that."]]))
   kex(_([["I would like to think that I'm used to it after all these cycles, but you never get used to it. There's a moment of bliss when I wake up first thing in the morning, and I still haven't remembered I'm a duck, but it soon comes crashing down. I just can't get used to it."]]))
   kex(_([["I do take it a bit better these days, but the beginning was especially hard. You just lose all the will to live. There's like no going back, right? All the things I used to have and cherish, they're all gone. My work, my friends, my family…"]]))
   kex(_([["I never chose to be in this state and often think that it would have been better if I had never been saved from the wreckage, but here I am."]]))
   vn.func( function () kex.shader = love_shaders.aura() end )
   kex(_([[His expression darkens.
"The only real thing that keeps me ticking is revenge on the bastard who did this to me. Turning me into this is beyond humiliating and I intend to have him suffer as much as I have."]]))
   vn.func( function () kex.shader = nil end )
   kex(_([["If you can avoid it, don't become a duck, kid."
He seems to lighten up a bit.]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end

function enter ()
   -- Main stuff starts
   if mem.misn_state==1 and system.cur() == targetsys then
      player.allowSave(true)
      player.allowLand(false)

      local fbaroness = faction.dynAdd( "Mercenary", "Baroness", _("Baroness Eve") )
      local pos = vec2.new( 9000, 3500 )
      local function addenemy( shipname )
         local p = pilot.add( shipname, fbaroness, pos+vec2.newP( 800*rnd.rnd(), rnd.angle()) )
         p:control()
         p:brake()
         p:setHostile(true)
         hook.pilot( p, "attacked", "blockade_attacked" )
         return p
      end

      blockade = {
         addenemy("Pacifier"),
         addenemy("Vendetta"),
         addenemy("Vendetta"),
      }

      -- Spawn chasing guys
      mem.spawn_hook = hook.timer( 10.0, "spawn_enemies", fbaroness )

      hook.timer( 0.5, "heartbeat" )

      hook.jumpout( "gotaway" )
   end
end

local function blockade_end ()
   for k,v in ipairs(blockade) do
      if v:exists() then
         v:control(false)
      end
   end
   -- Clear function so that it doesn't do anything anymore
   blockade_attacked = function () end
   heartbeat = function () end
end

function heartbeat ()
   -- Check to see if the blockade can detect the player
   local pp = player.pilot()
   local inrange = false
   for k,v in ipairs(blockade) do
      if v:exists() then
         local detect, fuzzy =  v:inrange( pp )
         if detect and not fuzzy then
            inrange = true
         end
      end
   end
   if inrange then
      blockade_end()
   else
      hook.timer( 0.5, "heartbeat" )
   end
end

function blockade_attacked ()
   blockade_end()
end

-- Spawn enemies that will eventually bog down the player
function spawn_enemies( fbaroness )
   local function addenemy( shipname )
      local p = pilot.add( shipname, fbaroness, targetplanet )
      p:setHostile(true)
      return p
   end

   local weakenemies = {
      "Shark",
      "Vendetta",
      "Ancestor",
      "Admonisher",
   }
   local strongenemies = {
      "Pacifier",
      "Hawking",
   }
   for i = 1,rnd.rnd(3,5) do
      addenemy( weakenemies[ rnd.rnd(1,#weakenemies) ] )
   end
   addenemy( strongenemies[ rnd.rnd(1,#strongenemies) ] )

   mem.spawn_hook = hook.timer( 20.0, "spawn_enemies", fbaroness )
end

function gotaway ()
   spawn_enemies = function () end
   mem.misn_state = 2
end
