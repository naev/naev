--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 05">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <done>Onion Society 04</done>
 <cond>
   local c = spob.cur()
   local f = c:faction()
   if not f or not f:tags("generic") then
      return false
   end
   return true
 </cond>
 <notes>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Onion05

   TODO explanation
--]]
local fmt = require "format"
local vn = require "vn"
local vni = require "vnimage"
local onion = require "common.onion"
local love_shaders = require "love_shaders"
local fleet = require "fleet"
local pilotai = require "pilotai"
--[[
local lmisn = require "lmisn"
local audio = require "love.audio"
--]]

local brokerspb, brokersys = spob.getS("Wunomi's World")
local deliverspb, deliversys = spob.getS("Shangris Station")
local _trixiespb, trixiesys = spob.getS("Ian")

--local money_reward = onion.rewards.misn05

local title = _("The Great Hack")


--[[
   Mission States
   0: mission accepted
   1. met broker at brokerspb
   2. destroyed enemies
   3. delivered to deliverspb
   4. got data from broker
   5. message about trixie
--]]
local STATE_ACCEPTED=0
local STATE_METBROKER=1
local STATE_BEATENEMIES=2
local STATE_BROKERDELIVERY=3
local STATE_RETURNBROKER=4
--local STATE_TRIXIEMESSAGE=5
mem.state = STATE_ACCEPTED

-- Create the mission
function create()
   misn.finish() -- Not ready yet

   -- Claims some systems
   if not misn.claim{ trixiesys, deliversys } then
      return misn.finish(false)
   end

   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_l337b01() )
   misn.setNPC( _("l337_b01"), prt.t.tex, _([[You seem to have an incoming connection from the Onion Society.]]) )
   misn.setReward(_("???") )
   misn.setTitle( title )
   misn.setDesc(_([[Help l337_b01 and Trixie hack the Nexus backend.]]))
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local l337 = onion.vn_l337b01{pos="left"}
   local trixie = onion.vn_trixie{pos="right"}
   vn.newCharacter( l337 )
   vn.newCharacter( trixie )
   vn.music( onion.loops.hacker )
   vn.transition("electric")
   vn.na(_([[You answer the incoming connection and some familiar holograms appear on-screen.]]))
   trixie(fmt.f(_([["Yo, {player}."]]),
      {player=player.name()}))
   l337(_([["Heyo!"]]))
   vn.menu{
      {_([["Heyo."]]), "01_cont"},
      {_([["Yo."]]), "01_cont"},
      {_([["Hello."]]), "01_cont"},
   }

   vn.label("01_cont")
   l337(_([["You ready to go down in the history books? Change the Nexus, and by extension, the Universe forever? Become truly l337!?! Go on to..."]]))
   trixie(_([[Trixie cuts them off.
"C'mon no need to keep them waiting!"]]))
   l337(_([["It's called building hype! Got to give the people what they love!"]]))
   -- Vnexer is basically like a vtuber (virtual youtuber) but on the Nexus
   trixie(_([["Have you been binging on self-help Vnexers again?"]]))
   l337(_([["Err, no? W-Why w-w-would you think that?"O
You believe you see the avatar blush.]]))
   trixie(_([["Nevermind... On with it, or I'll do it myself!"]]))
   l337(_([["Ahem! So anyway, remember the talk about the Nexus backbone hack?"]]))
   vn.menu{
      {_([["Remind me again."]]), "02_cont"},
      {_([["Of course!"]]), "02_ofcourse"},
   }

   vn.label("02_ofcourse")
   trixie(_([[Trixie's avatar seems to squint their eyes at you.
"Let's just go over it again so we are all in sync."]]))
   vn.label("02_cont")
   l337(_([["So, I managed to get my hands on some old design blueprints of the Nexus, like really old stuff, like maybe even before the Imperial Proclamation."]]))
   -- Hecacycle would be like century (a tad longer though)
   trixie(_([["l337_b01 and I were looking over it, and trying to poke the Nexus to see if there are similarities, and it seems like it is truly still there, somewhere under all the layers of abstractions and code that have piled on top over the hecacycles."]]))
   l337(_([["If we get access to it, we become Gods! All the information! All the power! We could finally bring about change for the better!"]]))
   trixie(_([["You're getting ahead of ourselves, this is still mainly all speculation."]]))
   l337(_([["Ah, sorry."]]))
   trixie(fmt.f(_([["We still need more documents. We need you to visit the Data Broker at {spb} in the {sys} system."]]),
      {spb=brokerspb, sys=brokersys}))
   l337(_([["They're really weird! They're completely offline! Only way to go there is in person, and you are our best bet!"]]))
   vn.menu{
      {_([["Count on me!"]]), "03_yes"},
      {_([[Maybe later.]]), "03_later"},
   }

   vn.label("03_later")
   vn.na(_([[You decline the work for now, and the holograms fade away.]]))
   vn.done("electric")

   vn.label("03_yes")
   vn.func( function () accepted = true end )
   trixie(fmt.f(_([["Excellent. We have made all the arrangements to meet the Data Broker, we'll brief you more when you get to {spb}."]]),
      {spb=brokerspb}))
   l337(_([["Time to make history!"]]))

   vn.done("electric")
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.markerAdd( brokerspb )
   mem.state = STATE_ACCEPTED
   hook.land("land")
   hook.enter("enter")
   misn.osdCreate( title, {
      fmt.f(_([[Visit the Data Broker at {spb} ({sys} system))]]),
         {spb=brokerspb, sys=brokersys}),
   } )
end

function land ()
   local cspb = spob.cur()
   if mem.state==STATE_ACCEPTED and cspb==brokerspb then
      -- Characters
      local l337 = onion.vn_l337b01{pos="left"}
      local trixie = onion.vn_trixie{pos="right"}

      vn.clear()
      vn.scene()
      -- Broker briefing
      vn.newCharacter( l337 )
      vn.newCharacter( trixie )
      vn.music( onion.loops.hacker )
      vn.transition("electric")
      vn.na(_([[As you land, your holographic system flares up with two familiar holograms.]]))
      l337(_([["Spot on!"]]))
      trixie(_([[Trixie's avatar nods.]]))
      l337(_([["So, the Data Broker, he's paranoid, like really really paranoid. So he's sitting on an enormous pile of data, but he never connects to the Nexus, he just keeps it offline."]]))
      trixie(_([["Risk-adverse."]]))
      l337(_([["Yeah, so you have to deal with him in person, which is where you come in."]]))
      trixie(_([["He has a ton of eccentric data, stuff you can't even find in the nooks and crannies of the Nexus. Nobody really knows where he gets his stuff, but he's pretty much a go to if you need specific information."]]))
      l337(_([["Yeah, the clever bastard also has a killswitch so that if anything happens to him, it all goes poof! So all the pirates and governmental agencies have to bow their heads to him if they need something."]]))
      trixie(_([["What we need from him is the 'NEXUS-v4.791829i Manual', which should shed the light on the connection protocol, the one we have failed to handshake."]]))
      l337(_([["Here, I'll send you the specification so you can take a hardcopy with you. We won't be able to be with you when you see the broker because of his damn electromagnetic shielding, but try your best to get it."]]))

      vn.scene()
      vn.music() -- Restore music
      vn.transition("electric")
      vn.na(_([[The two holograms fade out and you are left on your own.]]))
      vn.na(_([[You print a hardcopy of the data you were asked to get, and start to make your way to the location specified by the hackers.]]))
      vn.na(_([[You head to some residential quarters, on the way there you can't help shake the feeling you are getting watched.]]))
      vn.na(_([[Eventually you get to the location and as you approach the door it opens to you, leading you towards a very pristine room with nothing but a chair.]]))
      vn.scene()
      local broker = vn.newCharacter( _("Data Broker") )
      vn.transition()
      broker(fmt.f(_([[A voice echoes throughout the room.
"Have a seat, {player}."]]),
         {player=player.name()}))
      vn.menu{
         {_([["How did you know my name?"]]), "01_name"},
         {_([[Take a seat.]]), "01_sit"},
         {_([[Stay standing.]]), "01_stand"},
      }

      vn.label("01_name")
      broker(_([["How would such trivial data escape me, the Data Broker?"]]))
      vn.menu{
         {_([[Take a seat.]]), "01_sit"},
         {_([[Stay standing.]]), "01_stand"},
      }

      vn.label("01_stand")
      vn.na(_([[You stay standing.]]))
      broker(_([["As you wish."]]))
      vn.jump("01_cont")

      vn.label("01_sit")
      vn.na(_([[You sit down in the chair, which is surprisingly comfortable.]]))
      vn.func( function ()
         mem.sat_down = true
      end )
      vn.jump("01_cont")

      vn.label("01_cont")
      broker(_([[The voice continues.
"So what are you here for?"]]))
      vn.menu{
         {_([["The NEXUS-v4.791829i Manual."]]), "02_manual"},
         {_([["The Amulet of Yendor"]]), "02_silly"},
         {_([["Philosopher's Stone"]]), "02_silly"},
      }

      vn.label("02_silly")
      broker(_([[You hear a chuckle.
"That was just a courtesy, I know you are here for the NEXUS-V4.791829i Manual.]]))
      vn.jump("02_manual")

      vn.label("02_manual")
      broker(_([["I do have a copy of the data you seek. A fine manual it is."]]))
      broker(fmt.f(_([["Let us do a deal. I need some data delivered to {spb} in the {sys} system. You do the simple delivery for me, and I would be glad to part with a copy of the manual."]]),
         {spb=deliverspb, sys=deliversys}))

      vn.menu{
         {_([["Sure thing."]]), "03_cont"},
         {_([["Only a delivery?"]]), "03_question"},
         {_([["What about credits instead?"]]), "03_credits"},
      }

      vn.label("03_question")
      broker(_([["Yes. Nothing complicated for a pilot of your caliber."]]))
      vn.menu{
         {_([["Sure thing."]]), "03_cont"},
         {_([["What about credits instead?"]]), "03_credits"},
      }

      vn.label("03_credits")
      -- Obviously knows how many credits the player has, and will make sure they do the delivery
      broker(fmt.f(_([["Credits work too. I will sell it to you for exactly {credits}."]]),
         {credits=fmt.credits(player.credits()+1000)}))
      vn.menu{
         {_([[#r(Not enough credits)#0 "Delivery sounds fine."]]), "03_cont"},
      }

      vn.label("03_cont")
      broker(_([["Then we have a deal."]]))
      vn.na(_([[The floor opens up and a small package levitates up in front of you. It seems to be a smooth perfect white cube with no imperfections on the surface.]]))
      vn.na(_([[The door behind you opens, and you make your way back to the spaceport. On your way, you think you notice something out of the corner of you eye, but when you turn around you see nothing out of place. How odd.]]))

      vn.scene()
      vn.music( onion.loops.hacker )
      vn.newCharacter( l337 )
      vn.newCharacter( trixie )
      vn.transition("electric")
      vn.na(_([[You get back to your ship and two holograms pop up.]]))
      l337(_([["Did you get the manual?"]]))
      vn.na(_([["You explain them your encounter with the Data Broker."]]))
      trixie(_([["Strange, but to be expected. The Data Broker always prefer weird favours over credits."]]))
      l337(_([["Guess there's not much we can do. {player}, it's up to you to do the physical stuff. We'll keep an eye out for when you get back."]]))
      vn.na(_([[Time to deliver.]]))
      vn.done("electric")
      vn.run()

      local c = commodity.new( N_("Perfect Cube"), N_("A smooth perfect white cube. It doesn't make any noise, and sensors don't seem to detect anything in it. You have no idea what it is.") )
      mem.carg_id = misn.cargoAdd( c, 0 )

      misn.osdCreate( title, {
         fmt.f(_([[Deliver a package to {spb} ({sys} system)]]),
            {spb=deliverspb, sys=deliversys}),
         fmt.f(_([[Return to the Data Broker at {spb} ({sys} system)]]),
            {spb=brokerspb, sys=brokersys}),
      } )
      misn.markerRm()
      misn.markerAdd( deliverspb )
      mem.state = STATE_METBROKER

   elseif mem.state==STATE_BEATENEMIES and cspb==deliverspb then
      vn.clear()
      vn.scene()
      local worker = vn.newCharacter( _("Dock Worker"), {image=vni.generic()} )
      vn.transition()
      vn.na(_([[You arrive to the spacedock and carry the cube to the cargo delivery center, where a weary worker attends to you.]]))
      worker(_([["Anotehr one of these? Damnit, you playing a prank?"]]))
      vn.na(_([[You explain your plight and that you were asked to deliver this here.]]))
      worker(_([["That's quite a pretty cube, but it's junk. I checked and nobody is expecting it, we can't take it."]]))
      vn.menu{
         {_([[Throw the cube away.]]), "01_throw"},
         {_([[Take the cube with you.]]), "01_take"},
      }

      vn.label("01_throw")
      vn.func( function ()
         mem.cube_trashed = true
      end )
      vn.na(_([[Dejected, you chuck the cube into the nearest bin and head back to your ship. Seems like the Data Broker has explaining to do.]]))
      vn.done()

      vn.label("01_take")
      vn.na(_([[Dejected, you take the cube back with you to your ship. Seems like the Data Broker has explaining to do.]]))
      vn.run()

      if not mem.cube_trashed then
         misn.cargoRm( mem.carg_id ) -- Remove cargo
         mem.carg_id = nil
      end

      -- Advance internal state
      misn.osdActive(2)
      misn.markerRm()
      misn.markerAdd( brokerspb )
      mem.state = STATE_BROKERDELIVERY

   elseif mem.state==STATE_BROKERDELIVERY and cspb==brokerspb then
      -- Characters
      local l337 = onion.vn_l337b01{pos="left"}
      local trixie = onion.vn_trixie{pos="right"}

      vn.clear()
      vn.scene()
      local broker = vn.newCharacter( _("Data Broker") )
      vn.transition()
      vn.na(_([[You land and head back to the Data Broker's headquarters. You still feel watched on the way there, but it is different than last time.]]))
      vn.na(_([[When you get to the location, the door discretely opens and you once again find yourself in the room with the chair.]]))
      broker(fmt.f(_([["Welcome back, {player}.]]),
         {player=player.name()}))
      vn.menu( function ()
         local opts = {
            {_([["You tried to kill me!"]]), "01_setup"},
            {_([["What was that all about!"]]), "01_setup"},
            {_([[Sit down.]]), "01_sit"},
            {_([[Stand silently.]]), "01_stand"},
         }
         if not mem.cube_trashed then
            table.insert( opts, 1, {_([[Throw the cube.]])}, "01_throw" )
         end
         return opts
      end )

      vn.label("01_throw")
      vn.func( function ()
         mem.cube_thrown = true
      end )
      vn.na(_([[The cube bounces around before laying still on the ground. After a few seconds, the floor quickly opens around the cube and it disappears from sight.]]))
      broker(_([["That is not going to get you anywhere."]]))
      vn.jump("01_setup")

      vn.label("01_setup")
      broker(_([["It seems like you do not understand the data. I did not try to kill you, I just had you do me a favour by eliminating certain nuisances."]]))
      broker(_([["The delivery was necessary to set up the conditions for success. And you completed the task most excellently."]]))
      broker(_([["Here, let me provide you with the data that you require."]]))
      vn.label("01_data")

      vn.label("01_sit")
      broker(_([["So it seems you understood our deal. Let me provide you with the data that you required."]]))
      vn.jump("01_data")

      vn.label("01_stand")
      if not mem.sat_down then
         broker(_([["Quite the rebel are we."]]))
      end
      broker(_([["It seems like you held your part of the bargain. Let me provide you with the data that you require."]]))
      vn.jump("01_data")

      vn.label("01_givecube")
      broker(_([["But first, set the cube down."]]))
      vn.na(_([[You set the cube on the ground in front of you, and the floor quickly opens under it. Oh well, it was a nice cube.]]))
      broker(_([["Excellent."]]))
      vn.jump("01_datagive")

      vn.label("01_data")
      vn.func( function ()
         if not mem.cube_thrown and not mem.cube_trashed then
            vn.jump("01_givecube")
         end
      end )

      vn.label("01_datagive")
      vn.na(_([[The floor opens and a holodrive appears on a small pedastel. You pocket it, and turn to leave, but the door does not open.]]))
      broker(_([["Some free advice from the Data Broker. Be careful with what you seek. I do not think this manual will be of much use to you."]]))
      vn.na(_([[The door opens and you make your way back to the spaceport.]]))

      vn.scene()
      vn.newCharacter( l337 )
      vn.newCharacter( trixie )
      vn.music( onion.loops.hacker )
      vn.transition("electric")

      vn.na(_([[You get to your ship and the two familiar holograms appear.]]))
      trixie(_([["Did you get the..."]]))
      l337(_([[l337_b01 butts in.
"Was the Data Broker as cool as he sounds like?"]]))
      vn.menu{
         {_([["He's an asshole."]]), "02_asshole"},
         {_([["Cooler than ice."]]), "02_cool"},
      }

      vn.label("02_asshole")
      l337(_([["Whaaat!?"
You detect the shock in their voice.]]))
      trixie(_([[Trixie snickers.]]))
      vn.jump("02_cont")

      vn.label("02_cool")
      l337(_([["I knew it!"]]))
      trixie(_([["That's a surprise. I would have expected him to be an asshole."]]))
      vn.jump("02_cont")

      vn.label("02_cont")

      vn.done("electric")
      vn.run()

      misn.osdCreate( title, {
         "TODO"
      } )
      misn.markerRm()
      misn.markerAdd( brokerspb )
      mem.state = STATE_RETURNBROKER

      if mem.carg_id then
         misn.cargoRm( mem.carg_id ) -- Remove cargo
         mem.carg_id = nil
      end
   end
end

local enemies = {}
local boss
function enter ()
   local csys = system.cur()
   if mem.state==STATE_METBROKER and csys==deliversys then
      local pos = deliverspb:pos() + vec2.newP( 300, rnd.angle() )
      local fct = faction.get("Mercenary")
      enemies = fleet.add( 1, {
         "Starbridge",
         "Admonisher",
         "Admonisher",
         "Ancestor",
         "Ancestor",
         "Ancestor",
      }, fct, pos, _("Mercenary"), {ai="baddiepos"} )
      for k,p in ipairs(enemies) do
         p:setHostile(true)
      end
      boss = enemies[1]
      boss:rename(_("Mercenary Boss"))
      boss:setNoDisable(true)
      hook.pilot( boss, "death", "bossdead" )
      -- TODO actually handling aliveness? Meh, just let them kill the player
      pilotai.setTaunt( boss, _("That's the ship! Try to take them alive!") )
      deliverspb:landDeny()
   end
end

function bossdead ()
   mem.state = STATE_BEATENEMIES
   deliverspb:landAllow()

   -- Have one say something about running away
   local say = false
   for k,p in ipairs(enemies) do
      if p:exists() then
         if not say then
            p:broadcast(_("Shit! Get out of here!"))
            say = true
         end
         p:memory().aggressive = false -- No longer aggressive when they run away
         pilotai.hyperspace( p )
      end
   end
end
