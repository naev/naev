--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Point of Interest - Intro">
 <location>none</location>
 <chance>0</chance>
</mission>
--]]
--[[

   Introduction to Point of Interest (Sensor Anomaly) missions.

   We assume it is started from a derelict event.

   1. Follow NPC to sensor anomaly.
   2. NPC activates scanning outfit and is attacked by a bandit hyena.
   3. Destroy hyena and follow trails.
   4. Loot goal and get Pulse Scanner!

--]]
local fmt = require "format"
local luaspfx = require "luaspfx"
local der = require 'common.derelict'
local poi = require "common.poi"
local tut = require "common.tutorial"
local tutnel= require "common.tut_nelly"
local pir = require "common.pirate"
local vn = require "vn"
local lmisn = require "lmisn"
local pilotai = require "pilotai"
local love_shaders = require "love_shaders"

--[[
States
   0: Mission start, POI is marked
   1: Land and Nelly appears
   2: Fought off pirate
--]]

function create ()
   local syscand = lmisn.getSysAtDistance( nil, 1, 3, function( sys )
      -- Must be claimable
      if not naev.claimTest( {sys} ) then -- Full claim here
         return
      end

      -- Want no inhabited spobs
      for k,p in ipairs(sys:spobs()) do
         local s = p:services()
         if s.land and s.inhabited then
            return false
         end
      end
      return true
   end )

   -- Failed to start
   if #syscand <= 0 then misn.finish(false) end

   mem.sys = syscand[ rnd.rnd(1,#syscand) ]
   mem.risk = 0
   mem.rewardrisk = mem.risk

   -- We do a full claim on the final system
   if not misn.claim( {mem.sys, "nelly"} ) then
      return
   end

   local accept = false
   vn.clear()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.music( der.sfx.ambient )
   vn.transition()

   vn.na(_([[You carefully explore the heavily damaged derelict. It looks like most of it has been picked clean. You are about to give up when the system computer suddenly flickers and seems to boot up.]]))

   local sai = tut.vn_shipai()
   vn.appear( sai, tut.shipai.transition )

   vn.na(fmt.f(_([[As the ship's operating system is starting, {shipai} materializes in front of you.]]),{shipai=tut.ainame()}))
   if not var.peek( "poi_sai_intro" ) then
      -- Worded to be a bit like the explosion messages in EVC
      sai(_([["While you were exploring I managed to bootstrap the ship's systems. No, nothing bad could have happened. I estimated under 10% chance of triggering the ship's security self-destruct mechanism and blowing up the ship. Oh…"]]))
      sai(_([["Anyway, it seems like my work paid off, there seems to be some data marking a sensor anomaly. Should I download the data so that we can explore it?"]]))
      vn.func( function ()
         var.push( "poi_sai_intro", true )
      end )
   else
      sai(_([["Hey, it looks like there is data marking a sensor anomaly stored on the ship computer. Should I download the data so that we can explore it?"]]))
   end

   vn.menu{
      {_("Download the data"), "accept"},
      {_("Leave."), "leave"},
   }

   vn.label("accept")
   vn.func( function () accept = true end )
   if mem.sys:known() then
      sai(fmt.f(_([[{shipai} flickers slightly as they download the data.
"It looks like the data points to a location in the nearby system of {sys}. I have marked the location on your map. However, I'm not certain the ship's current systems would allow it to follow such a faint hint. Maybe it would be wise to land and see if we can improve our systems somehow."]]),{shipai=tut.ainame(),sys=mem.sys}))
   else
      sai(fmt.f(_([[{shipai} flickers slightly as they download the data.
"It looks like the data points to a location in a nearby system. I have marked the location on your map. However, I'm not certain the ship's current systems would allow it to follow such a faint hint. Maybe it would be wise to land and see if we can improve our systems somehow."]]),{shipai=tut.ainame()}))
   end
   vn.na(_([[With nothing else to do on the derelict, you leave it behind, and return to your ship.]]))
   vn.jump("done")

   vn.label("leave")
   vn.na(_([[You decide to leave the information alone and leave the derelict.]]))

   vn.label("done")
   vn.sfx( der.sfx.unboard )
   vn.run()
   player.unboard()

   if accept then
      der.addMiscLog(fmt.f(_([[You found information on a sensor anomaly aboard a derelict in the {sys} system.]]),{sys=system.cur()}))
   else
      der.addMiscLog(_([[You found information about a sensor anomaly aboard a derelict, but decided not to download it.]]))

      -- We want to delay aborting the mission a frame so that the derelict
      -- event thinks it's running fine
      hook.safe( "delay_abort" )
      return
   end

   mem.state = 0
   mem.locked = true

   -- Mission gets accepted in misnSetup
   poi.misnSetup{ sys=mem.sys, found="found", risk=mem.risk }

   misn.osdCreate( _("Sensor Anomaly"), {
      _("Improve your ship's systems"),
      _("Head to the marked location"),
   } )

   hook.enter( "enter" )
   hook.land( "land" )
   hook.load( "land" )
end

function delay_abort ()
   misn.finish(false)
end

local npc_nel
function land ()
   local spb = spob.cur()
   local f = spb:faction()
   if spb:tags().restricted or not f or pir.factionIsPirate(f) then
      return
   end

   if mem.state > 0 then return end

   local desc
   if var.peek("nelly_met") then
      desc = _("Nelly looks bored out of her mind. Wait, is she trying to bend a spoon with her mind?")
   else
      desc = _("You see a bored individual doing something weird with a spoon.")
   end
   npc_nel = misn.npcAdd( "approach_nelly", tutnel.nelly.name, tutnel.nelly.portrait, desc )
end

function approach_nelly ()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )

   if var.peek("nelly_met") then
      vn.na(_([[You find Nelly gripping and staring intently at a spoon. It's almost as if she is trying to bend it with her mind.]]))
      vn.menu{
         {_([["Hello"]]), "cont01" },
         {_([["What are you doing?"]]), "cont01" },
         {_([["Nice spoon"]]), "cont01" },
      }

      vn.label("cont01")
      nel(_([[She doesn't seem to react, too concentrated on her task. You lightly tap her on the shoulder and she bounces out of her chair, dropping her spoon.
"Spoonelius!"
She grabs the fallen spoon and then lifts it up in triumph.
"I did it! I bent the spoon with my mind!"]]))
   else
      vn.na(_([[You get close to the individual and stare at them. They seem so intent on their task that they don't realize you are there. Eventually, they seem to nod off and fall off their chair dropping their spoon. This jolts her out of her slumber.
"Spoonelius!"
She grabs the fallen spoon and then lifts it up in triumph.
"I did it! I bent the spoon with my mind!"]]))
   end

   vn.menu{
      {_([["I'm pretty sure it bent when you dropped it"]]), "bent"},
      {_([[…]]), "silence"},
   }

   vn.label("bent")
   nel(_([[She poses triumphantly with her spoon.
"What? That's impossible. This has to be the result of all my hard training. I've spent 30 minutes on this! Great House Sirius has nothing on me!"]]))
   vn.jump("cont02")

   vn.label("silence")
   nel(_([[She poses triumphantly with her spoon.
"Impressive right! I heard that many people in Great House Sirius can do this and thought, it can't be too hard can it?"]]))
   vn.jump("cont02")

   vn.label("cont02")
   if var.peek("nelly_met") then
      nel(_([["How's it like being humbled by my mental powers!"
She cackles maniacally.]]))
   else
      nel(_([["How's it like being humbled by my mental powers!"
She cackles maniacally.
"Nice to meet you! My name is Nelly!"]]))
      vn.func( function ()
         var.push( "nelly_met", true )
      end )
   end

   nel(_([["I've been bored out of my mind since my delivery contract was terminated. They said my last delivery of luxury goods stank of fish and was useless.  I can't help that drying fish in the cargo hold is one of my family traditions since last cycle!"]]))
   nel(_([["You wouldn't have anything interesting I could help with? I would love an adventure!"]]))

   vn.menu{
      {_([[Tell her about the sensor anomaly]]), "poi"},
      {_([[Leave]]), "leave"},
   }

   vn.label("poi")
   nel(fmt.f(_([[Her eyes light up and twinkle like the endless stars in space.
"That sounds… AWESOME. I'm totally in! I actually have been saving my {outfit} for a chance like this!"]]),
      {outfit="#b".._("Pulse Scanner").."#0"}))
   nel(fmt.f(_([["The location sounds like the {sys} system. I have to do some tweaks on my ship, but I'll meet you there. My ship is leaner and meaner since my radiators caught fire and I threw them out. I'll surely get there before you!"]]),
      {sys=mem.sys}))
   vn.na(_([[You're not sure if that was the best idea, but it does seem like she may be able to help this time around.]]))
   vn.func( function ()
      mem.state = 1
      misn.osdCreate( _("Sensor Anomaly"), {
         fmt.f(_("Meet Nelly at {sys}"),{sys=mem.sys}),
         _("Head to the sensor anomaly"),
      } )
      misn.npcRm( npc_nel )
   end )
   vn.done( tutnel.nelly.transition )

   vn.label("leave")
   vn.na(_([[You take your leave. It may be best not to get involved with Nelly right now.]]))
   vn.done( tutnel.nelly.transition )
   vn.run()
end

local nelly, fct_nelly, fct_nemesis, cutscene
function enter ()
   if cutscene then
      lmisn.fail(_("You abandoned the sensor anomaly and Nelly!"))
      return
   end

   if system.cur() ~= mem.sys or mem.state < 1 then
      return
   end

   misn.osdActive( 2 )

   pilot.clear()
   pilot.toggleSpawn( false )

   fct_nelly = faction.dynAdd( "Independent", _("Nelly") )
   fct_nemesis = faction.dynAdd( "Independent", _("Nemesis") )
   fct_nelly:dynEnemy( fct_nemesis )

   local shipname
   if player.misnDone( "Helping Nelly Out 2" ) then
      shipname = _("Llaminator MK3")
   else
      shipname = _("Llaminator MK2")
   end

   -- Spawn ship
   local jumpin, jdist
   jdist = math.huge -- Get closest jump to player, should be robust to player.teleport
   for k,v in ipairs(system.cur():jumps()) do
      local d = v:pos():dist( player.pos() )
      if d < jdist then
         jumpin = v
         jdist = d
      end
   end
   nelly = pilot.add( "Llama", fct_nelly, jumpin, shipname )
   nelly:setInvincible(true)
   nelly:setHilight(true)
   nelly:setVisplayer(true)
   nelly:setFriendly(true)

   cutscene = 1
   hook.timer( 5, "enter_delay" )
end

local pos
function enter_delay ()
   pos = poi.misnPos()

   nelly:control()
   nelly:moveto( pos )

   nelly:broadcast(_("There it is! Let's head towards the sensor anomaly!"))

   hook.timer( 1, "heartbeat" )
end

local nemesis, broadcasted
function heartbeat ()
   if cutscene == 1 and pos:dist( nelly:pos() ) < 1000 then
      nelly:taskClear()
      nelly:brake()
      cutscene = 2

   elseif cutscene == 2 then
      if nelly:pos():dist( player.pos() ) < 1000 then
         nemesis = pilot.add( "Koala", fct_nemesis, pos + vec2.new( 5000, rnd.angle() ), _("Nemesis") )
         nelly:broadcast(_("Wait? We were followed?"))
         cutscene = 3
         broadcasted = 0
         hook.timer( 5, "heartbeat" )
         player.autonavReset( 5 )

         misn.osdCreate( _("Sensor Anomaly"), {
            _("Eliminate the hostiles"),
         } )
         nelly:setHilight(false)
         nemesis:setHilight(true)
         return
      else
         broadcasted = (broadcasted or 0) - 1
         if broadcasted < 0 then
            local strlist = {
               _("Come over here!"),
               _("Get closer!"),
               _("I'm ready, come here!"),
            }
            nelly:broadcast( strlist[ rnd.rnd(1,#strlist) ] )
            broadcasted = 15
         end
      end

   elseif cutscene == 3 then
      if nemesis:exists() then
         nemesis:setHostile()
         nemesis:broadcast(_("You're as good as fried chicken!"))
         pilotai.guard( nemesis, pos )
         hook.pilot( nemesis, "death", "nemesis_dead" )
         cutscene = 4
         broadcasted = 5
      else
         cutscene = 5
      end

   elseif cutscene == 4 then
      broadcasted = broadcasted - 1
      if broadcasted < 0 then
         local strlist = {
            _("Save me!"),
            _("Get rid of them!"),
            _("Aaaaaaah!"),
         }
         nelly:broadcast( strlist[ rnd.rnd(1,#strlist) ] )
         broadcasted = 15
      end

   elseif cutscene == 5 then
      nelly:broadcast(_("Phew! Come over here and I'll activate my pulse scanner!"))
      cutscene = 6
      broadcasted = 15

   elseif cutscene == 6 then
      if nelly:pos():dist( player.pos() ) < 1e3 then
         nelly:broadcast(_("Time to activate my pulse scanner!"))
         hook.timer( 3, "heartbeat" )
         cutscene = 7
         return
      end
      broadcasted = broadcasted - 1
      if broadcasted < 0 then
         local strlist = {
            _("Come over here!"),
            _("Get closer!"),
            _("I'm ready, come here!"),
         }
         nelly:broadcast( strlist[ rnd.rnd(1,#strlist) ] )
         broadcasted = 15
      end

   elseif cutscene == 7 then
      luaspfx.pulse( nelly:pos(), nelly:vel() )
      naev.trigger( "poi_scan" )
      hook.timer( 5, "heartbeat" )
      cutscene = 8
      return

   elseif cutscene == 8 then
      nelly:broadcast(_("My engine stopped, you go on ahead!"))
      misn.osdCreate( _("Sensor Anomaly"), {
         _("Follow the trail"),
      } )
      return -- Done
   end

   hook.timer( 1, "heartbeat" )
end

function nemesis_dead ()
   cutscene = 5
end

-- luacheck: globals found (used in POI framework)
function found ()
   player.msg(_("You have found something!"),true)

   misn.osdCreate( _("Sensor Anomaly"), {
      _("Board the derelict"),
   } )

   local p = pilot.add( "Mule", "Derelict", mem.goal, _("Unusual Derelict"), {naked=true} )
   p:disable()
   p:setInvincible()
   p:setHilight()
   p:effectAdd( "Fade-In" )
   hook.pilot( p, "board", "board" )
end

function board( p )
   vn.clear()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.music( der.sfx.ambient )
   vn.transition()

   local nel = tutnel.vn_nelly{ shader=love_shaders.hologram(), pos="left" }

   vn.appear( nel, "electric" )
   nel(_([[You board the ship and immediately Nelly establishes a holo-link connection. She seems to be covered in ship grease.
"Whoa! This looks much fancier than I expected!"]]))

   vn.menu{
      {_([["Why so greasy?"]]), "grease"},
      {_([["Is this it?"]]), "it" },
      {_([[…]]), "cont01" },
   }

   vn.label("grease")
   nel(_([["When we got attacked by that scary ship, I slipped and accidentally pulled down my fish drying rack. The fish flew over all the ship and one got caught in my reactor because I forgot to close the maintenance panel. I managed to put out the fire, but the fish got lodged in the cooling tube. I had to grease myself up to slide in there and reach it, however, it's still a work in progress."]]))
   vn.menu{
      {_([[…]]), "cont01" },
   }

   vn.label("it")
   nel(_([["It sure looks like it! It's my first time seeing such a pretty derelict."]]))
   vn.jump("cont01")

   vn.label("cont01")
   nel(_([["Don't just stand there! Take a look around"]]))
   vn.na(_([[You attempt to go through the airlock when an authorization prompt opens up. Looking at the make of the ship, it seems heavily reinforced. It looks like you're going to have to break the code to gain complete access to the ship.]]))

   local sai = tut.vn_shipai{ pos="right" }
   vn.appear( sai, tut.shipai.transition )
   local fuzzy = "#o?#0"
   local exact = "#b!#0"
   sai(_([[Your ship AI materializes in front of you.
"This looks like a standard security mechanism. You will need to crack it to access the ship."]]))
   sai(fmt.f(_([["You have to guess the combination of symbols to open the password. This lock should have only {num} symbols. After each attempt, you will know how many symbols match exactly with {exact}. If a symbol is in the password, but is not in the correct position you will be shown {fuzzy}. You have to use the clues to attempt to match the true password."]]),
      {num=3, fuzzy=fuzzy, exact=exact}))
   nel(fmt.f(_([[She squints looking at {shipai}'s holographic projection.
"Say, do I know you?"]]),
      {shipai=tut.ainame()}))
   sai(fmt.f(_([["It is not possible that we have met. For I am {player}'s ship AI."
They turn quickly to you.
"Remember, {exact} for exact matches and {fuzzy} for correct symbol but not position. Best of luck!"
They dematerialize in a hurry.]]),
      {player=player.name(), exact=exact, fuzzy=fuzzy}))
   vn.disappear( sai, tut.shipai.transition )
   nel(_([[Thinking deeply to herself she murmurs "I definitely know them…".]]))
   nel(_([[She turns again to you.
"Try to crack the password, we need to see what's on the ship!"]]))

   vn.label("trycrack")
   local stringguess = require "minigames.stringguess"
   stringguess.vn()
   vn.func( function ()
      if stringguess.completed() then
         vn.jump("unlocked")
         return
      end
      vn.jump("unlock_failed")
   end )

   vn.label("unlock_failed")
   vn.na(_([[A brief '#rAUTHORIZATION DENIED#0' flashes on the screen, and you hear the ship internals groan as the emergency security protocol kicks in and everything starts to get locked down. However, it seems to jam and stops halfway. Then the authorization prompt reappears. Looks like you get another chance!]]))
   vn.jump("trycrack")

   vn.label("unlocked")
   vn.na(_([[You deftly crack the code and the screen flashes with '#gAUTHORIZATION GRANTED#0'. Time to see what goodness awaits you!]]))
   nel(_([[She seems to be banging on her reactor while talking to you.
"That was great! Go ahead!"]]))
   vn.na(_([[You proceed to enter and explore the interior of the derelict. Although there is a distinct lack of living life aboard, everything seems to be in pretty good condition. Eventually you reach the cargo hold and find a lone crate marked "cheese".]]))
   nel(_([[Even though she is apparently squeezing her greasy self through her reactor, her eyes light up the moment she sees the crate.
"Whoa! Is that what I think it is? Could you bring the crate over to my ship? I need to see that right away!"]]))
   vn.na(_([[You explore the rest of the ship but do not find anything else of interest. Although the ship is in very good condition, it is still not space-worthy, and there is not anything that you can do with it. You take the crate of cheese and leave the ship behind.]]))
   vn.func( function ()
      local c = commodity.new( N_("Cheese"), N_("A crate marked cheese. It is quite small and very, very smelly.") )
      misn.cargoAdd( c, 0 )
      nelly:setActiveBoard(true)
      nelly:setHilight()
      hook.pilot( nelly, "board", "board_nelly" )

      misn.osdCreate( _("Sensor Anomaly"), {
         _("Board Nelly's ship"),
      } )
   end )
   vn.sfx( der.sfx.unboard )
   vn.run()

   -- Clean up stuff
   poi.cleanup( false ) -- Can't fail
   p:setHilight(false)
   player.unboard()
end

function board_nelly ()
   local o = outfit.get("Pulse Scanner")

   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )

   vn.na(_([[The moment you get on Nelly's ship with the smelly cheese, she comes bouncing to you.]]))

   nel(_([["Oh! It's even better in person! Let me check the goods."]]))
   nel(_([[She takes the crate and deftly pries it open. Suddenly everything smells really strongly of cheese and you gag a bit.
"Let's see. This is an authentic sausage-infused matured cheese! The rumours were true!"]]))
   nel(_([["Here, take my Pulse Scanner, I'll take the cheese. I won't have to chase sensor anomalies for a while with this beauty!"
She rubs the cheese to her face.]]))
   vn.na(_([[Seeing as it doesn't seem like you'll be able to change anything and the Pulse Scanner is probably more useful than the cheese, you take the outfit and head back to your ship. Now you can explore sensor anomalies on your own!]]))

   vn.sfxVictory()
   vn.na( fmt.reward(o) )

   vn.done( tutnel.nelly.transition )
   vn.run()

   player.outfitAdd( o )

   -- Let her fly away
   nelly:setHilight(false)
   nelly:control(false)
   nelly:setActiveBoard(false)

   pilot.toggleSpawn(true) -- Re-enable spawns
   player.unboard()

   poi.log(_([[Nelly helped you explore a sensor anomaly. You found some smelly cheese and traded it with Nelly for a Pulse Scanner to be able to explore more sensor anomalies.]]))

   misn.finish(true) -- We're done here!
end
