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

local brokerspb, brokersys = spob.getS("Wunomi's World")
local deliverspb, deliversys = spob.getS("Shangris Station")
local trixiespb, trixiesys = spob.getS("Ian")
local backdoorspb, backdoorsys = spob.getS("Marius Enclave")

local title = _("The Great Hack")

--[[
   Mission States
   0: mission accepted
   1. met broker at brokerspb
   2. destroyed enemies
   3. delivered to deliverspb
   4. got data from broker
--]]
local STATE_ACCEPTED=0
local STATE_METBROKER=1
local STATE_BEATENEMIES=2
local STATE_BROKERDELIVERY=3
local STATE_RETURNBROKER=4
mem.state = STATE_ACCEPTED

-- Create the mission
function create()
   if not var.peek("testing") then return misn.finish() end -- Not ready yet

   -- Claims some systems
   if not misn.claim{  deliversys } then
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
   trixie(_([["Never mind... On with it, or I'll do it myself!"]]))
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
      fmt.f(_([[Visit the Data Broker at {spb} ({sys} system)]]),
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
"That was just a courtesy, I know you are here for the NEXUS-V4.791829i Manual."]]))
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
      broker(_([["Yes. Nothing complicated for a pilot of your calibre."]]))
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
      trixie(_([["Strange, but to be expected. The Data Broker always prefers weird favours over credits."]]))
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
      local worker = vn.newCharacter( _("Dockworker"), {image=vni.generic()} )
      vn.transition()
      vn.na(_([[You arrive to the spacedock and carry the cube to the cargo delivery centre, where a weary worker attends to you.]]))
      worker(_([["Another one of these? Damnit, you playing a prank?"]]))
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
      vn.na(_([[The floor opens and a holodrive appears on a small pedestal. You pocket it, and turn to leave, but the door does not open.]]))
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
      l337(_([["Anyway, quickly plug in the holodrive, I want to see it!"]]))
      vn.na(_([[You plug it in your ship's console and the hackers quickly help themselves to the data.]]))
      trixie(_([["Let's see what we have here...
'confidential, for your eyes only'... boring...
wow, only 7D cryptographical encodings..."]]))
      l337(_([["Running me reconstruction cipher..."]]))
      vn.na(_([[You hear more beeping as it seems they start to make themselves at hope with your ship's computation power.]]))
      l337(_([["Almost there... aham! That's it!"]]))
      trixie(_([["Got a lock?"]]))
      l337(_([["Yup! I think I found an entry point to the backend. It's really old school, I thought it would be port knocking or something, but it's actually a physical backdoor!"]]))
      -- Pinocles Station incident is where a short circuit of a physical backdoor was crashed into and accidentally vented the station atmosphere killing everyone
      trixie(_([["Wow! Those should be banned since the Pinocles Station incident."]]))
      l337(_([["Pinocles Station?"]]))
      trixie(_([["Physical backdoor short-circuit vented the atmosphere, all dead. Didn't you study your protocols?"]]))
      l337(_([["Eh heh, I always fall asleep."
They rub their eyes.
"Anyway, I found it, right under our eyes!"]]))
      trixie(_([["Don't keep us waiting, where is it?"]]))
      l337(fmt.f(_([["It's at the old rickety station of {spb} in the {sys} system. I knew the station was old, but never thought it was that old!"]]),
         {spb=backdoorspb, sys=backdoorsys}))
      trixie(_([["You never know with these stations. It could even just be some piece reused from somewhere else."]]))
      l337(fmt.f(_([["Only one way to find out! {pilot}, onwards to {spb}!"]]),
         {pilot=pilot.name(), spb=backdoorspb}))
      trixie([["RáÚÆ Â£Ř§Ů—� ©????╟舐—â€š�Ř§Ů½  æØ¢Ã Ř§Ů© ráÚÆ ????½ æØ¢Ã"]])
      l337(_([["Looks like the Nexus relay is being flaky. Don't worry, they should be back the moment it stabilizes. We'll get back to you later!"]]))
      vn.na(_([[It seems like the hackers need your help again...]]))

      vn.done("electric")
      vn.run()

      misn.osdCreate( title, {
         fmt.f(_([[Go to {spb} ({sys} system)]]),{spb=backdoorspb, sys=backdoorsys}),
      } )
      misn.markerRm()
      misn.markerAdd( backdoorspb )
      mem.state = STATE_RETURNBROKER
      hook.jumpin("jumpin")

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

mem.jumped = 0
function jumpin ()
   if mem.state == STATE_RETURNBROKER then
      mem.jumped = mem.jumped+1
   end

   if mem.jumped > 2 then
      hook.timer( 7, "badnews" )
   end
end

function badnews ()
   local l337 = onion.vn_l337b01()

   vn.clear()
   vn.scene()
   vn.newCharacter( l337 )
   vn.musicStop() -- Stop all music
   vn.transition("electric")
   vn.na(_([[You are suddenly surprised by l337_b01 opening a holo-connection on the distress channel.]]))
   l337(_([[You hear a loud sobbing.
"T-T-tt..."
The wailing continues.]]))
   vn.menu{
      {_([["What's wrong?"]]), "01_cont"},
      {_([["Trixie?"]]), "01_cont"},
      {_([["Take a deep breath!"]]), "01_cont"},
   }

   vn.label("01_cont")
   l337(_([[They inhale deeply, cough a bit, and continue.
"T-Trixie got peeled!"]]))
   -- TODO some sad + dramatic music
   vn.menu{
      {_([["Whaaaat!?"]]), "02_cont"},
   }

   vn.label("02_cont")
   l337(_([["I should have known something was wrong. I thought it was just a fluke of the system, should have run more diagnostic programs. Why did this have to happen!?!"
You hear a big sob.]]))
   l337(_([["Trixie, oh Trixie, you are the better and stronger one. I should have been the one peeled, not you. You're the real hacker."
The words blend into inconsolable yelling and sobbing.]]))
   vn.menu{
      {_([["It's not your fault."]]), "03_fault"},
      {_([["They could be alright still."]]), "03_cont"},
      {_([[...]]), "03_cont"},
   }

   vn.label("03_fault")
   l337(_([["But, Trixie! T-Trixie!!!"
They wail with abundance of sorrow.]]))
   vn.jump("03_cont")

   vn.label("03_cont")
   -- Message send by SAI
   vn.na(fmt.f(_([[You notice that a notification pops up on your HUD, apparently regarding a large explosion on {spb} in the {sys} system. This doesn't look good.]]),
      {spb=trixiespb, sys=trixiesys}))
   l337(_([["It's over! We're doomed!"
They cough a bit, choking on their tears.]]))
   l337(_([["I can't go on without them. Without Trixie..."]]))
   vn.menu{
      {_([["We aren't going down without a fight!"]]), "04_cont"},
      {_([["Trixie wouldn't want it to end like this!"]]), "04_cont"},
   }

   vn.label("04_cont")
   l337(_([[They give a big sniffle.
"You're right. This isn't what Trixie would do."
You hear the sound of them blowing their nose.]]))
   l337(_([[There is a deep sigh.
"Give me one second."
They cough, probably clearing the tears.]]))
   l337(fmt.f(_([[You hear a surprisingly clear voice.
"What, how could I have missed this? I found a stray packet coming from the {sys} relay. Looks like...!!"
They go silent, and you start receiving a new voice channel.]]),
      {sys=trixiesys}))

   vn.move( l337, "left" )
   local msg = vni.soundonly( "00", { pos="right" } )
   vn.appear( msg )
   msg(_([["Heyo."
The voice sounds very old and tired.]]))
   msg(_([["Glad you found this l337_b01, but I was never hoping you'd hear my real voice like this."]]))
   msg(_([["I don't have much time, I'm recording this on the go."
You hear a siren in the background.]]))
   msg(_([["Shit, they're moving faster than I expected."
There is a constant sound of loud and furious typing.]]))
   msg(_([["Since I was dropped offline, I've been trying to trace the freak. They're very good, almost too good."]]))
   -- Modified / hacked part done by underworlder
   msg(_([[The audio seems to break up.
"But not *CRACKLE* enough. *HISS* tracked *POP*-nal to *CRACKLE* *SCRATCH* lonewolf4 *HISS* *CRACKLE*"]]))
   msg(_([[The audio seems to recover.
"*SCRITCH* my suspicious that this was a fellow technomancer, but I didn't think they'd be so brash."]]))
   msg(_([["Oh shit, looks like this is the end of the line."]]))
   msg(_([[The tone in the voice changes, almost nostalgic.
"l337_b01, everything is in your hands now. Like old times... I'll be waiting with v3c70r and DEADBEEF..."]]))
   msg(_([["CONNECTION TERMINATED"]]))
   vn.disappear( msg )
   vn.move( l337, "center" )

   l337(_([["..."]]))
   vn.music( onion.loops.hacker )
   l337(_([["Damnit Trixie, I looked up to you!"
They take another deep breath.
"This isn't going to stay like this!"]]))
   l337(_([["{player}, I can't involve you more. Stay alive."]]))

   vn.scene()
   vn.transition("electric")
   vn.na(_([[The connection abruptly cuts off. Is l337_b01 is going to something drastic?]]))
   vn.done("electric")
   vn.run()

   diff.apply("onion05") -- Update Ian
   onion.log(_([[You helped l337_b01 and Trixie to obtain a manual referencing the long-forgotten Nexus Backbone backdoor from the Data Broker. On the way to the backdoor, l337_b01 informed you that Trixie was peeled, throwing your plans into disarray. l337_b01 looks like they are about to do something potentially drastic.]]))

   -- Happy Ending :D
   misn.finish(true)
end
