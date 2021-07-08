--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kex's Freedom 2">
 <flags>
  <unique />
 </flags>
 <avail>
  <location>Bar</location>
  <chance>100</chance>
  <planet>Minerva Station</planet>
  <done>Kex's Freedom 1</done>
 </avail>
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
local minerva = require "campaigns.minerva"
local portrait = require 'portrait'
local love_shaders = require 'love_shaders'
local vn = require 'vn'
require 'numstring'

logidstr = minerva.log.kex.idstr

-- Mission states:
--  nil: mission not accepted yet
--  0: mission accepted go to targetplanet
--  1: run away and return alive to minerva station
--  2: got away
misn_state = nil

targetplanet = "Niflheim"
targetsys = planet.get(targetplanet):system():nameRaw()

misn_reward = _("A step closer to Kex's freedom")
misn_title = _("Freeing Kex")
misn_desc = string.format(_("You have been entrusted with stealing information from Baroness Eve at %s in the %s system."), _(targetplanet), _(targetsys))

money_reward = 200e3

function create ()
   if not misn.claim( system.get(targetsys) ) then
      misn.finish( false )
   end
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
   misn.setDesc( misn_desc )

   misn.setNPC( minerva.kex.name, minerva.kex.portrait, minerva.kex.description )
end

function accept ()
   approach_kex()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      return
   end

   misn.accept()

   shiplog.append( logidstr, _("You have agreed to help Kex steal information from Baroness Eve.") )

   misn.osdCreate( misn_title,
      { string.format(_("Go to %s in the %s system and hack the main database"), _(targetplanet), _(targetsys) ),
      _("Return to Kex at Minerva Station") } )

   misn_marker = misn.markerAdd( system.get(targetsys) )

   hook.land("generate_npc")
   hook.load("generate_npc")
   hook.enter("enter")

   generate_npc()
end


function generate_npc ()
   if planet.cur() == planet.get("Minerva Station") then
      npc_kex = misn.npcAdd( "approach_kex", minerva.kex.name, minerva.kex.portrait, minerva.kex.description )

   elseif planet.cur() == planet.get(targetplanet) and misn_state==0 then

      -- Don't save in case the player gets stuck with a shitty ship and gets massacred over and over
      player.allowSave(false)

      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(string.format(_("You land on %s and proceed to discretely approach one of the terminals and connect the program that Kex gave you."), _(targetplanet)))
      vn.na(_("The terminal lights up dimly and begins to compute whatever was on the program. After a while it seems to begin downloading large amounts of data while you impatiently wait."))
      vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
      vn.na(_("Suddenly, an alarm begins to blare. Before the security screen shuts down in the terminal you are able to grab the program as you make a run for your ship. In the background, you hear people yelling and running around. It seems like you have to get out of here as soon as possible."))
      vn.na(_("This is probably what Kex meant with things going sour. It seems like you have a head start on your pursuers, but you may not be able to count on it for long."))
      vn.run()

      -- Advance mission and get out of there
      misn.markerMove( misn_marker, system.get("Limbo") )
      misn_state = 1
      misn.osdActive(2)
      player.takeoff()
   end
end

function approach_kex ()
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.kex )
   local kex = vn.newCharacter( minerva.vn_kex() )
   vn.transition()

   -- Mission is over
   if misn_state==2 then

      vn.na(_("You return tired from your escapade once again to Kex, who is at his favourite spot at Minerva station."))
      kex(_([["You look like a mess kid, You alright?"]]))
      vn.na(_("You explain how you weren't able to get all the data before you had to scram out of there, and hand him the data you were able to recover."))
      kex(_([["Damn, the security was tighter than anticipated. I'm glad to see you made it in one piece despite the opposition. Aristocrats always seem to have cards up their sleeves."]]))
      kex(_([["Let me take a brief look at the data you were able to collect."
He plugs in the program directly into a port under his wing and his eyes go blank for a bit.
"I see, looks like you were able to get quite a lot of data. However, their database had much more than expected. It will probably take me quite a bit longer to process this all and see what we got.]]))
      kex(_([["In the meantime, it looks like we were also able to take some credits. I'll wire you up with a reward for your efforts."]]))
      vn.sfxMoney()
      vn.func( function () player.pay( money_reward ) end )
      vn.na(string.format(_("You received #g%s#0."), creditstring( money_reward )))
      kex(_([["I'm going to go process this data. It's going to take me a while, but come back here in a period or so and I should hopefully have some more results."]]))
      vn.sfxVictory()
      vn.run()

      shiplog.append( logidstr, _("You managed to find a crate destined to the Minerva CEO through luck, and found that it was sent by Baroness Eve."))
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
      if misn_state == nil then
         table.insert( opts, 1, { _("Ask about Baroness Eve"), "baroness" } )
      end
      if misn_state == 0 then
         table.insert( opts, 1, { _("Ask about the job"), "job" } )
      end
      return opts
   end )

   vn.label("baroness")
   kex(string.format(_([["Glad you asked. It turns out she's apparently a big shot at %s in the %s system. She deals in all sorts of trade between the Empire and Za'lek, although there also seems to be some bad rumours about her floating around."]]), _(targetplanet), _(targetsys)))
   kex(_([["Although it's not very clear what exactly happened, there was a fairly big incident that ended up in a large fight between mercenaries, Empire forces, and pirates in the system. It disrupted the trade routes of the area for quite a few periods."]]))
   kex(_([["I was able to get more information from some other documents in the crate, as seems like they were fairly sloppy with some of the security details. I have managed to put together a small program that hopefully should be able to access their main database, however, I'm going to need you to connect it by hand."]]))
   kex(string.format(_([["This time the job is probably going to be much trickier than the last. Even though the documents had some security issues, it is likely that things will be much more different once you get over to %s. Still, I think you should be able to handle this easily. Would you be up for the challenge?"]]), _(targetplanet) ))
   vn.menu( {
      { _("Accept"), "accept" },
      { _("Decline"), "decline" },
   } )

   vn.label("accept")
   kex(string.format(_([["Great! Let me get you set up with the program. All you have to do is land on %s and plug it in to any terminal that there should be around the docks. That should also make it easier for you to get out of there if things go sour."]]), _(targetplanet)))
   vn.func( function ()
      misn_state = 0
   end )
   vn.jump("menu_msg")

   vn.label("decline")
   kex(_([[He looks dejected.
"I see. If you change your mind, I'll be around."]]))
   vn.done()

   vn.label("job")
   kex(_([["So the idea is to try to hack into the Baroness Eve's database and see if we can get any dirt of the CEO of Minerva Station. Since we already know that they have dealings with each other, it seems like there has to be something there if we can access all the data."]]))
   kex(string.format(_([["All you have to do is land on %s and plug in the program I gave you to any terminal that there should be around the docks. That should also make it easier for you to get out of there if things go sour."]]), _(targetplanet)))
   kex(_([["It's likely that there will be more security around this time, so make sure you take a ship that can deal with trouble. Although, I don't think that will be a problem for you."]]))
   vn.jump("menu_msg")

   vn.label("duck")
   kex(_([["It's as awful as you would expect it to be. I mean, sure I can float and fly short distances, but you can't reach any tall places and have to constantly avoid getting stepped on."]]))
   kex(_([["The worst part is the lack of communication. I mean, I've sort of lost my humanity and can't engage in all the activities I used to like. You know, drinking with friends, going out, basic human empathy, shit like that."]]))
   kex(_([["I would like to think that I'm used to it after all these cycles, but you never get used to it. There's a moment of bliss when you wake up first thing in the morning, and you still haven't remembered you're a duck, but it soon comes crashing down. You just can't get used to it."]]))
   kex(_([["I do take it a bit better these days, but the beginning was especially hard. You just lose all the will to live. There's like no going back, right? All the things I used to have and cherish, they're all gone. My work, my friends, my family…"]]))
   kex(_([["I never chose to be in this state and often think that it would have been better if I had never been saved from the wreckage, but here I am."]]))
   vn.func( function () kex.shader = love_shaders.aura() end )
   kex(_([[His expression darkens.
"The only real thing that keeps me ticking is revenge on the bastard who did this to me. Turning me into this is beyond humiliating and I intend to have him suffer as much as I have."]]))
   vn.func( function () kex.shader = nil end )
   kex(_([["If you can avoid, don't become a duck kid."
He seems to lighten up a bit.]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end

function enter ()
   -- Main stuff starts
   if misn_state==1 and system.cur() == system.get(targetsys) then
      player.allowSave(true)
      player.allowLand(false)

      fbaroness = faction.dynAdd( "Mercenary", "Baroness", _("Baroness Eve") )
      local pos = vec2.new( 9000, 3500 )
      local function addenemy( shipname )
         local p = pilot.add( shipname, fbaroness, pos+vec2.newP( 800*rnd.rnd(), 359*rnd.rnd()) )
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
      spawn_timer = 10e3
      spawn_hook = hook.timer( spawn_timer, "spawn_enemies" )

      hook.timer( 500, "heartbeat" )

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
      hook.timer( 500, "heartbeat" )
   end
end

function blockade_attacked ()
   blockade_end()
end

-- Spawn enemies that will eventually bog down the player
function spawn_enemies ()
   local pp = player.pilot()
   local pnt = planet.get(targetplanet)
   local function addenemy( shipname )
      local p = pilot.add( shipname, fbaroness, pnt )
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

   spawn_timer = 20e3
   spawn_hook = hook.timer( spawn_timer, "spawn_enemies" )
end

function gotaway ()
   spawn_enemies = function () end
   misn_state = 2
end

