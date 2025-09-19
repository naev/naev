--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 04">
 <unique />
 <priority>3</priority>
 <chance>50</chance>
 <location>Bar</location>
 <done>Onion Society 03</done>
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
   Onion04

   Player has to take a one-time pad from a convoy, and then swap the cargo
   with another convoy on the way there. Afterwards, they have to swap the
   cargo with another another ship, and finally infiltrate the final place
   before getting away.
--]]
local fmt = require "format"
local vn = require "vn"
local vni = require "vnimage"
local onion = require "common.onion"
local love_shaders = require "love_shaders"
local lmisn = require "lmisn"
local fleet = require "fleet"
local pilotai = require "pilotai"
local der = require 'common.derelict'
local lg = require "love.graphics"
local mg = require "minigames.flip"
local audio = require "love.audio"

-- Action happens on the jump from Action happens Overture to Dohriabi on the Overture side
local ambushsys = system.get("Overture")
local swapspb, swapsys = spob.getS("Fuzka")
local targetspb, targetsys = spob.getS("Nexus Shipyards HQ")
local jmpsys = system.get("Dohriabi")
local jmp = jump.get( ambushsys, jmpsys )
local runjmp = jump.get( ambushsys, system.get("Nartur") )

local money_reward = onion.rewards.misn04

local title = _("Onion Heist")

--[[
   Mission States
   0: mission accepted
   1: took one-time pad
   2: (optional) landed once before swapspb
   3: ship cargo swapped at swapspb
   4: heist done and took off
   5: left system
--]]
mem.state = 0

-- Create the mission
function create()
   -- Claim the ambush and escape stuff
   if not misn.claim{ ambushsys, targetsys } then
      return misn.finish(false)
   end

   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_l337b01() )

   misn.setNPC( _("l337_b01"), prt.t.tex, _([[You seem to have an incoming connection from the Onion Society.]]) )
   misn.setReward(_("???") )
   misn.setTitle( title )
   misn.setDesc(_([[Help l337_b01 and Trixie do a bank heist.]]))
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
   l337(fmt.f(_([["Heyo, how's it going {player}?"]]),
      {player=player.name()}))
   trixie(_([["Yo."]]))
   vn.menu{
      {_([["Yo."]]), "01_cont"},
      {_([["Heyo."]]), "01_cont"},
      {_([["Hello."]]), "01_cont"},
   }

   vn.label("01_cont")
   l337(_([["Was poking at the Nexus Shipyards security with Trixie, and it seems like we've got a path into the mainframe, although it's a bit convoluted."]]))
   trixie(fmt.f(_([["It seems like there's an important convoy coming from {startspb} to {targetspb} with some of the new encryption protocols and one-time pads. However, we can't really directly mess with them, or they'll redo the entire encryption, and it's moot."]]),
      {startspb=spob.get("Emperor's Wrath"), targetspb=targetspb}))
   l337(fmt.f(_([[l337_b01 butts in, "but if we manage to swap the one-time pad with another, we can get access there. We've found another suitable convoy that will be going through the {sys} system."]]),
      {sys=ambushsys}))
   trixie(fmt.f(_([[Trixie counter-butts in, "Yes, so we need you to raid the convoy in {sys} system, and then deliver that to {swapspb} in the {swapsys} system, and we can handle the rest!"]]),
      {sys=ambushsys, swapspb=swapspb, swapsys=swapsys}))
   vn.menu{
      {_([["Count on me!"]]), "02_yes"},
      {_([[Maybe later.]]), "02_later"},
   }

   vn.label("02_later")
   vn.na(_([[You decline the work for now, and the holograms fade away.]]))
   vn.done("electric")

   vn.label("02_yes")
   vn.func( function () accepted = true end )
   l337(_([["Great! Trixie and I will be using your ship as a beacon, and should be able to help you as bandwidth allows."]]))
   trixie(fmt.f(_([["The convoy will be jumping from the {jmpsys} system to the {ambushsys} system. It's our best bet due to the high bandwidth and few ships to capture the cargo."]]),
      {jmpsys=jmpsys, ambushsys=ambushsys}))
   l337(fmt.f(_([["No need to fret, {player} is an excellent pilot and this will be no challenge for the {shipname}!"]]),
      {player=player.name(), shipname=player.pilot():name()}))

   vn.done("electric")
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.markerAdd( ambushsys )
   mem.state = 0
   hook.land("land")
   hook.enter("enter")
   misn.osdCreate( title, {
      fmt.f(_([[Steal one-time pads from the convoy in the {sys} system]]),
         {sys=ambushsys}),
      fmt.f(_([[Land on {spb} ({sys} system)]]),
         {spb=swapspb, sys=swapsys}),
   } )
end

function land ()
   local cspb = spob.cur()
   if (mem.state==1 or mem.state==2) and cspb==swapspb then
      -- Characters
      local l337 = onion.vn_l337b01{pos="left"}
      local trixie = onion.vn_trixie{pos="right"}
      local staffimg = vni.faction( cspb:faction() )

      -- Cargo swap cutscene
      vn.clear()
      vn.scene()
      vn.newCharacter( l337 )
      vn.newCharacter( trixie )
      local staff = vn.Character.new( _("Spacedock Staff"), { image=staffimg, shader=love_shaders.hologram() } )
      vn.music( onion.loops.hacker )
      vn.transition("electric")
      vn.na(_([[Your ship enters the docks, and the two Onion Society hackers make their timely holographic appearance.]]))
      trixie(_([["Quick, l337_b01, to action!"]]))
      if mem.state==2 then
         l337(_([["Aye aye captain!"]]))
         trixie(_([[You think you see what amounts to a grimace from Trixie's avatar.]]))
      else
         l337(_([["I'm on it!"]]))
      end
      trixie(fmt.f(_([["Damn, it seems denser than expected. We need a distraction. Quick, {player}, do something!"]]),
         {player=player.name()}))
      vn.menu{
         {_([[Activate your ship's security alarm.]]), "01_alarm_early"},
         {_([[Bump the ship into the station structure.]]), "01_bump_early"},
         {_([["What should I do!?"]]), "01_stumped"},
      }

      vn.label("01_stumped")
      trixie(_([["I don't know! Run over a maintenance robot or something!"]]))
      l337(_([["Time to get busy!"]]))
      vn.disappear( {l337, trixie}, "electric" )
      vn.menu{
         {_([[Run over a maintenance robot.]]), "01_robot"},
         {_([[Activate your ship's security alarm.]]), "01_alarm"},
         {_([[Bump the ship into the station structure.]]), "01_bump"},
      }
      local badthing

      vn.label("01_bump_early")
      vn.disappear( {l337, trixie}, "electric" )
      vn.label("01_bump")
      vn.na(_([[You fumble the controls of the ship a bit, just enough to make it barely scratch the inner hull of the space dock. Although you expected it to make some noise, you didn't foresee the ear-splitting screech caused by the metal on metal interaction. With all the heads of everyone at the spacedocks turned your way, it seems like you managed to get the attention of the inter spacedock.]]))
      vn.func( function () badthing = "bump" end )
      vn.jump("01_cont")

      vn.label("01_robot_early")
      vn.disappear( {l337, trixie}, "electric" )
      vn.label("01_robot")
      vn.na(_([[You begrudgingly take the controls of the ship, and wait until a cleaner robot strays near your landing gear. Wasting no time on the opportunity, you do a precise manoeuvre to crush it with a resounding *CRUNCH*. Given the amount of heads turned, it looks like you got the attention of the entire spacedock.]]))
      vn.func( function () badthing = "robot" end )
      vn.jump("01_cont")

      vn.label("01_alarm_early")
      vn.disappear( {l337, trixie}, "electric" )
      vn.label("01_alarm")
      vn.sfx( audio.newSource( 'snd/sounds/loops/alarm.ogg' ) ) -- Just one loop
      vn.na(_([[You open the command console and trigger the ship's emergency alarm system. The alarm system, designed for long-range planetary signalling, seems to be amplified and echoed by the spacedock structure, resulting in an onslaught of audible chaos. Everyone in the spacedock turns towards your ship: it looks like you got the attention of the entire floor.]]))
      vn.func( function () badthing = "alarm" end )
      vn.jump("01_cont")

      vn.label("01_cont")
      vn.appear( staff, "electric" )
      vn.na(_([[Almost immediately, a high priority communication channel with the spacedock command opens.]]))
      staff(_([["What the hell are you thinking! I've dealt with a ton of pilots in my time, but none as reckless as you!"]]))
      vn.menu{
         {_([["Sorry! It's my first time landing."]]), "02_cont"},
         {_([["What do you mean?"]]), "02_cont"},
      }

      vn.label("02_cont")
      staff( function ()
         local msg1 = _([[You call that landing? Who the hell taught you to fly?]])
         local msg2
         if badthing=="bump" then
            msg2 = _([[It's a bloody spacedock, not a percussive instrument you have to bash your ship against!]])
         elseif badthing=="robot" then
            msg2 = _([[The robot you crushed only had one cycle left until retirement!]])
         else
            msg2 = _([[You can't just blast your alarm at the spacedocks! You know how much tympanic reconstruction costs!]])
         end
         local msg = fmt.f(_([["{msg1} {msg2}"]]),{
            msg1=msg1, msg2=msg2 } )
         return msg
      end )
      vn.appear( l337, "electric" )
      l337(_([[l337_b01 appears on a separate channel.
"We're almost there, keep them distracted a bit longer."]]))
      vn.menu( function ()
         local opts = {
            {_([["I have no idea of what you are talking about."]]),"03_cont"},
            {_([["What you say?"]]),"03_cont"},
         }
         if badthing=="bump" then
            table.insert( opts, 1, {_([["It was only a scratch!"]]),"03_bump"} )
         elseif badthing=="robot" then
            table.insert( opts, 1, {_([["It was only a scratch!"]]),"03_robot"} )
         end
         return opts
      end )

      vn.label("03_bump")
      staff(_([["The spacedock hull integrity may not be compromised, but I assure you the traumatic auditive memories of the resulting caterwaul is not going to be easy to recover from!"]]))
      vn.jump("03_cont")

      vn.label("03_robot")
      staff(_([["Only a scratch? Scrubby is flat as a pancake! You've turned a great cleaning robot into a stain on the spacedock floor!"]]))
      vn.jump("03_cont")

      vn.label("03_cont")
      staff(_([["Show me your credentials. I'm filing a report to the Empire. You do know what happens if you don't comply, do you?"]]))
      vn.menu{
         {_([["Wait, I can explain!"]]), "04_explain"},
         {_([["Credentials? I don't even know who I am!"]]), "04_cont"},
         {_([["No, *I'm* filing a complaint to the Empire!"]]), "04_counter"},
      }

      vn.label("04_explain")
      staff(_([["Too late now! The spacedock recordings are all the explanation anyone will need!"]]))
      vn.jump("04_cont")

      vn.label("04_counter")
      staff(_([["File a complaint for what? Upholding the law? Taking care of the spacedocks? Dealing with horrible pilots?"]]))
      vn.jump("04_cont")

      vn.label("04_cont")
      staff(fmt.f(_([["Your flying days will be over. The Empire does not take kindly to this kind of infraction. If you get lucky, you'll only be given a life sentence on the lovely prison planet {spb}!"]]),
         {spb=spob.get("Finn")})) -- Finn is an Empire prison world
      staff(_([["Not going to be so cocky when you are..."]]))

      l337(_([[Suddenly l337_b01 shouts through the other channel.
"Got them!"]]))
      vn.scene()
      vn.newCharacter( l337 )
      vn.setBackground( function ()
         local nw, nh = lg.getDimensions()
         lg.setColour( 0, 0, 0, 1 )
         lg.rectangle( "fill", 0, 0, nw, nh )
      end )
      vn.transition()
      vn.na(_([[Suddenly the stations power goes out and the connection with spacedock command is cut.]]))
      vn.appear( trixie, "electric" )
      trixie(_([["Yo."]]))
      vn.menu{
         {_([["What happened?"]]), "05_cont"},
         {_([["Yo."]]), "05_cont"},
      }

      vn.label("05_cont")
      l337(_([["Trixie, that was top-notch! I didn't even think of reversing the polarization protocol to create a bandwidth backflow and overflow the router!"]]))
      trixie(_([["There are a lot more tricks where that came from!"]]))
      l337(_([["Totally yoinking that one."]]))
      trixie(fmt.f(_([["{player}, to fill you out, we managed to swap the one-time pad onto the local convoy. We've rebooted the station systems to cover our track, but the job here is done!"]]),
         {player=player.name()}))
      vn.menu{
         {_([["What about the spacedock command?"]]), "06_cont"},
         {_([["Will I be alright?"]]), "06_cont"},
         {_([["Great job!"]]), "06_cont"},
      }

      vn.label("06_cont")
      trixie(_([["No need to worry about anything."]]))
      l337(_([["Just watch and see."]]))

      vn.scene()
      vn.newCharacter( l337 )
      vn.newCharacter( trixie )
      vn.setBackground()
      vn.transition()
      vn.appear( staff, "electric" )
      vn.na(_([[The station lights flicker on and an angrier than ever spacedock command appears promptly on your holoscreen.]]))
      staff(_([["You! You have something to do with this. When I add this to your list of crimes, you'll be lucky to even breath oxygen again!"]]))
      staff(fmt.f(_([["Security! Seize the {shipname}!"]]),
         {shipname=player.pilot():name()}))
      staff(_([["Wait, what are you doing? Criminal fraud? I haven't done anything."]]))
      vn.disappear( staff, "electric" )
      vn.na(_([[The connection with spacedock command abruptly cuts out.]]))

      trixie(_([["And that was that."]]))
      l337(_([["Nice clean job. I could get used to this."]]))
      trixie(fmt.f(_([["OK, now comes for the final part. {player}, we need you to go to {spb} in the {sys} system, and we'll break into the system using the new security code they'll be using."]]),
         {player=player.name(), spb=targetspb, sys=targetsys}))
      l337(_([["If we've gotten this far, it should be a piece of cake. "]]))
      vn.func( function ()
         if mem.didoptscene then
            return vn.jump("07_didscene")
         end
         return vn.jump("07_noscene")
      end )

      vn.label("07_didscene" )
      trixie(_([["Again? I told you to stop making me hungry!"]]))
      l337(_([["Didn't you just have cake?"]]))
      trixie(_([["First cake yes, but what about second cake?"]]))
      l337(_([["You should watch out for your blood sugar."]]))
      trixie(_([[You hear a muffled eating sound.]]))
      l337(fmt.f(_([["Ah well. I should get some cake too."
The pause a second.
"Before I forget, {player}, on to {spb}!"]]),
         {player=player.name(), spb=targetspb}))
      vn.na(_([[Maybe you should get some cake too.]]))
      vn.done("electric")

      vn.label("07_noscene" )
      trixie(_([["Stop making me hungry!"]]))
      l337(_([["Sorry, forgot you had such a sweet tooth."]]))
      trixie(_([[You hear Trixie muttering something about cake.]]))
      l337(fmt.f(_([["{player}, on to {spb}!"]]),
         {player=player.name(), spb=targetspb}))
      vn.done("electric")
      vn.run()

      misn.osdCreate( title, {
         fmt.f(_([[Break into {spb} ({sys} system)]]),
            {spb=targetspb, sys=targetsys}),
      } )
      misn.markerRm()
      misn.markerAdd( targetspb )
      mem.state = 3
   elseif mem.state==1 then
      -- Small extra optional cutscene
      vn.clear()
      vn.scene()
      local l337 = vn.newCharacter( onion.vn_l337b01{pos="left"} )
      local trixie = vn.newCharacter( onion.vn_trixie{pos="right"} )
      vn.music( onion.loops.hacker )
      vn.transition("electric")
      vn.na(_([[You land and go inspect the cargo you pulled off of the Nexus Convoy. It seems like the two hackers hitch-hiking on your ship's systems take notice and their holograms appear before you.]]))
      l337(fmt.f(_([["Nice flying out there! See, Trixie, I told you it would be a piece of cake for {player}!"]]),
         {player=player.name()}))
      trixie(_([["Stop making me hungry!"
They pause for a second.
"I'm just worried that it seems like their security was really week... Too weak."]]))
      l337(_([["Really? Aren't you just being overly paranoid?"]]))
      trixie(_([["The devil's in the details! At least it should be a while until they notice that the convoy went missing while your script keeps running."]]))
      l337(fmt.f(_([["Yeah, the simulation is emulating the convoy, so we should have some time to finish pulling off the plan. {player}, you ready to do the swap at {spb}?"]]),
         {player=player.name(), spb=swapspb}))
      vn.menu{
         {_([["Piece of cake."]]), "01_cake"},
         {_([["Security felt tough though..."]]), "01_cont"},
      }

      vn.label("01_cake")
      trixie(_([["That does it! I'm ordering cake!"
You hear Trixie muttering to themselves.]]))
      l337(_([["You already ate the last one you got?"]]))
      trixie(_([["Not my fault I have a sweet tooth!"]]))
      vn.jump("01_cont")

      vn.label("01_cont")
      l337(_([["Today, with our fairly limited bandwidth, we were able to break into and override quite a few of their ships. Usually, it's hard enough to access a spaceship's operating system, let alone override it."]]))
      trixie(_([["This has to be related to the weird patterns we're seeing on the Nexus!"]]))
      l337(_([["Correlation doesn't imply causation!"]]))
      trixie(_([["Indeed it does not, but coincidences are not very good omens."]]))
      l337(_([["Superstitions! However, you tend to have good hunches, so let me see if I can scrap some bandwidth to launch some monitoring spells."]]))
      trixie(_([["Never can be too careful. There is too much at stake here to mess up."
You hear a beep.
"Ah, diagnostics finished, let's see..."]]))
      trixie(fmt.f(_([["Cargo looks OK! Seems to be lots of encrypted documents, but there also is a nice one-time pad. Exactly what we need. l337_b01, if you can spare the resources, try to decrypt some of the documents. {player}, head to {swapspb} in the {swapsys}. We proceed with the plan!"]]),
         {player=player.name(), swapspb=swapspb, swapsys=swapsys}))
      l337(_([["Aye aye captain!"]]))
      trixie(fmt.f(_([["Stop mocking me, the captain here is {player}!"]]),
         {player=player.name()}))

      vn.scene()
      vn.transition("electric")
      vn.na(_([[The holograms fade out, probably to conserve bandwidth, and you are left with the task at hand.]]))
      vn.run()
      -- Advance internal state
      mem.didoptscene = true
      mem.state = 2

   elseif mem.state==3 and cspb==targetspb then
      misn.npcAdd( "breakin", _("Onion Society"), onion.img_onion().tex, _([[Break into the Nexus Shipyards systems.]]) )

   elseif mem.state==5 and cspb~=targetspb then
      -- last cutscene
      vn.clear()
      vn.scene()
      local l337 = vn.newCharacter( onion.vn_l337b01{pos="left"} )
      local trixie = vn.newCharacter( onion.vn_trixie{pos="right"} )
      vn.music( onion.loops.hacker )
      vn.transition("electric")
      vn.na(_([[Your ship enters the docks, and the two Onion Society hackers make their timely holographic appearance.]]))
      trixie(fmt.f(_([["Glad to see you made it out alive {player}. That did not go as expected."]]),
         {player=player.name()}))
      l337(_([["Sorry about that, I panicked when they started upstreaming to me and cut the connection. I should have listened to you Trixie, I guess I got cocky."]]))
      trixie(_([["No sweat. There's always a next time."]]))
      if mem.failedhack then
         l337(_([["Shame we weren't able to break into the system, we were so close."]]))
         trixie(_([["At least we all made it out alive. There's always more opportunities as long as we're still alive and kicking."]]))
         l337(_([["Can't win them all."]]))
      else
         l337(_([["What were you able to get?"]]))
         trixie(_([["I didn't have much time to choose, so I just randomly downloaded as much as bandwidth allowed, still going through it. From the little I've seen, there's some interesting account credentials, and some encrypted stuff that we have to unlock. All in all, a pretty good haul."]]))
         l337(_([["Cool. Def share the titbits. I've got some spare computational power to crack codes."]]))
      end
      l337(_([["What are our next steps?"]]))
      trixie(_([["I've been thinking about what we talked about almost a cycle ago. We might be able to pull off something big, like really big."]]))
      l337(_([["Oh, is this about the Nexus backbone hack?"]]))
      trixie(_([["Yes, I think we have a good opportunity to pull it off."]]))
      l337(_([["Awesome!"]]))
      trixie(fmt.f(_([["{player}, if you're interested in becoming part of history, send us a message in a bit. I'll give the details."]]),
         {player=player.name()}))
      if mem.failedhack then
         trixie(_([["Oh, and to encourage you to participate, here's some credits courtesy of Nexus Shipyards. Not much, but next time we should be able to get more!"]]))
         vn.func( function ()
            player.pay( 0.4*money_reward )
         end )
         vn.sfxVictory()
         vn.na( fmt.reward(0.4*money_reward) )
      else
         trixie(_([["Oh, and to encourage you to participate, here's some credits courtesy of Nexus Shipyards."]]))
         vn.func( function ()
            player.pay( money_reward )
         end )
         vn.sfxVictory()
         vn.na( fmt.reward(money_reward) )
      end

      vn.done("electric")
      vn.run()

      if mem.failedhack then
         onion.log(fmt.f(_([[You helped l337_b01 and Trixie perform a heist at {spb}, although it was not a complete success.]]),
            {spb=targetspb}))
      else
         onion.log(fmt.f(_([[You helped l337_b01 and Trixie perform a heist at {spb}.]]),
            {spb=targetspb}))
      end
      misn.finish(true)
   end
end

function breakin ()
   player.save() -- Save the game

   vn.clear()
   vn.scene()
   local l337 = vn.newCharacter( onion.vn_l337b01{pos="left"} )
   local trixie = vn.newCharacter( onion.vn_trixie{pos="right"} )
   vn.music( onion.loops.hacker )
   vn.transition("electric")
   vn.na(_([[You signal to the two Onion Society hackers that it is time for them to make their timely holographic appearance.]]))
   trixie(fmt.f(_([["Oh boy, {spb}, it's been a while since I've been here. Not in person though. Reminds me of old times."]]),
      {spb=targetspb}))
   l337(fmt.f(_([["No time to get all old-timey on us now. It's time to get to business. {player}, see if you can jack us in to a local console. Let me send you a map of the nearest one."]]),
      {player=player.name()}))
   vn.na(_([[You head off to the terminal l337_b01 pointed you to. The installation is a bit of a mess, but eventually you find it and jack in, giving the hackers more direct access.]]))
   l337(_([["Let's see what we have here... seems like the one-time pad passed the verification test."]]))
   trixie(_([["Wait, that's a bit weird, I'm seeing some inconsistencies in the object system."]]))
   l337(_([["You sure? My scripts haven't picked up anything strange yet."]]))
   trixie(_([["Something feels a bit off. Wait, let me double-check running processes."]]))
   l337(_([["Mmm, what was that?"]]))
   trixie(_([["Shit, it seems like there's someone else hijacking the system, pattern blue!"]]))
   l337(_([["I see them now! They're running a reverse packet analysis, I don't think I can keep up the connection much longer."]]))
   trixie(_([["I'll try to draw them off!"]]))
   trixie(_([["l337_b01? Shit, not responding to ping, they're down."]]))
   vn.disappear( l337, "electric" )
   trixie(fmt.f(_([["Scripts are down. {player}, I'm going to need you to manually override the last security protocol! I'll be running diversions!"]]),
      {player=player.name()}))

   vn.label("mg_hack")
   mg.vn()
   vn.func( function ()
      if mg.completed() then
         vn.jump("mg_success")
      else
         vn.jump("mg_fail")
      end
   end )

   vn.label("mg_fail")
   vn.na(_([[You fail to gain access of the terminal. Try again?]]))
   vn.menu{
      {_([[Try again.]]), "mg_hack"},
      {_([[Abort.]]), "mg_abort"},
   }

   vn.label("mg_abort")
   vn.func( function ()
      mem.failedhack = true
   end )
   vn.music( 'snd/sounds/loops/alarm.ogg' )
   trixie(_([["Shit, get out of there now!"]]))
   vn.jump("end")

   vn.label("mg_success")
   trixie(_([["Got it? Great! Let me access try to access the data."]]))
   vn.music( 'snd/sounds/loops/alarm.ogg' )
   trixie(_([["Shit, I'll try to get what I can, but you have to get out of there now!"]]))
   vn.jump("end")

   vn.label("end")
   vn.disappear( trixie, "electric" )
   vn.na(_([[You unplug from the terminal and make a wild dash for your ship. On the way there, an armed Nexus guard appears before you, but you just plow through them before they can arm their weapon, sending them sprawling on the ground.]]))
   vn.na(fmt.f(_([[You finally make it to the spacedocks and manage to slide in before taking control. You quickly activate full thrusters and smash your way out of the spacedocks as Nexus security pummels your ship with weapon fire. You barely make it out in one piece. Looks like you're going to have a bumpy ride out of {sys}!]]),
      {sys=targetsys}))
   vn.run()

   misn.osdCreate( title, {
      fmt.f(_([[Get away from {sys} and land on a planet or station]]),
         {sys=targetsys}),
   } )
   misn.markerRm()
   mem.state = 4
   player.allowSave(false) -- Don't save, as we saved before this event started
   player.takeoff() -- off we go
   player.allowSave(true) -- restore save flag
end

local function fct_baddie ()
   return faction.dynAdd( "Dummy", "_onion_nexus", _("Nexus IT"), { ai="mercenary" } )
end

function enter ()
   hook.timerClear()
   -- Ambush set up
   if system.cur()==ambushsys and mem.state==0 then
      hook.timer( 7, "prepare" )

   elseif system.cur()==targetsys and mem.state==4 then
      -- Took weapon fire when taking off, be kinder if they hacked
      if mem.failedhack then
         player.pilot():setHealth( 50, 10 )
      else
         player.pilot():setHealth( 90, 30 )
      end
      player.landAllow( false ) -- Don't let the player land until they jump out

      -- Small patrol fleet to annoy the player
      local ships = {
         "Hawking",
         "Admonisher",
         "Admonisher",
         "Shark",
         "Shark",
         "Shark",
         "Shark",
      }
      local baddies1 = fleet.add( 1, ships, fct_baddie(), spob.get("Wellen"):pos() )
      local wp=targetsys:waypoints()

      pilotai.apply( baddies1, function (p)
         p:rename( fmt.f(_("Nexus {ship}"), {ship=p:ship():name()}))
         p:setHostile(true)
      end )
      pilotai.patrol( baddies1, {
         wp["onion04_1"],
         wp["onion04_2"],
         wp["onion04_3"],
      } )

      -- Player has to run away
      hook.timer( 5, "baddiechase1" )

   -- Player got away
   elseif mem.state==4 then
      mem.state = 5 -- done with last part
   end
end

local convoyspawn
local distlim = 5e3
local mrk
function prepare ()
   -- Skip talk if jumping in from jmpsys
   if player.pos():dist( jmp:pos() ) < distlim then
      convoyspawn()
      return
   end
   player.msg(_("l337_b01: Head towards the jump. They should be here soon!"), true)
   player.autonavReset( 5 )
   mrk = system.markerAdd( jmp:pos() )
   hook.timer( 1, "wait" )
end

function wait ()
   if player.pos():dist( jmp:pos() ) < distlim then
      convoyspawn()
      return
   end
   hook.timer( 1, "wait" )
end

local enemies, spam
function convoyspawn ()
   if mrk then
      system.markerRm( mrk )
      mrk = nil
   end
   player.msg(_("Trixie: Oh boy, that's a lot of ships. Get close and we'll hack!"), true )
   player.autonavReset( 5 )

   -- Clear up
   pilotai.clear()
   pilot.toggleSpawn(false)

   local ships = {
      "Gawain", -- has the cargo
      "Pacifier",
      "Pacifier",
      "Pacifier",
      "Admonisher",
      "Admonisher",
      "Admonisher",
      "Admonisher",
      "Admonisher",
      "Shark",
      "Shark",
      "Shark",
      "Shark",
      "Shark",
      "Shark",
   }
   local fct = fct_baddie()
   local names = {}
   for k,s in ipairs(ships) do
      names[k] = fmt.f(_("Nexus {ship}"), {ship=ship.name(s)})
   end
   enemies = fleet.add( 1, ships, fct, jmp, names )

   local minspeed = math.huge
   for k,p in ipairs(enemies) do
      minspeed = math.min( p:stats().speed_max * 0.95, minspeed )
   end

   local l = enemies[1]
   hook.pilot( l, "board", "board" )
   hook.pilot( l, "death", "death" )
   hook.pilot( l, "land", "gawain_lost" )
   hook.pilot( l, "jump", "gawain_lost" )
   l:setHilight(true)
   l:setVisplayer(true)
   l:control(true)
   l:hyperspace(runjmp)
   l:setSpeedLimit( minspeed )
   hook.timer( 8, "heartbeat" )
   spam = 0
end

function gawain_lost ()
   if mem.state > 0 then return end
   lmisn.fail(_([[You lost track of the target!!!]]))
end

function heartbeat ()
   local l = enemies[1]
   local dis = l:disabled()
   if dis then
      -- Player already disabled, nothing to do
      return
   end
   if l:pos():dist( player.pos() ) < 1500 then
      local fct = faction.dynAdd( "Dummy", "_onion_nexus_hacked", _("Nexus IT (Hacked)"), { ai="baddie" } )
      fct:dynEnemy( fct_baddie() )
      l:setDisable()
      local dohack = { 3, 6, 8, 11, 14 } -- IDs of ships to hack
      for k,p in ipairs(enemies) do
         if p:exists() then
            local hacked = inlist( dohack, k )
            if hacked then
               p:effectAdd("Onionized")
               p:setFaction( fct )
               p:setFriendly(true)
            else
               p:setHostile(true)
            end
         end
      end
      player.msg(_("l337_b01: Script deployed! Time to cry!"), true )
      player.autonavReset( 5 )
      return
   end
   spam = spam-1
   if spam < 0 then
      player.msg(fmt.f(_("Trixie: See the {p}? Get closer!"),{p=l}), true )
      player.autonavReset( 5 )
   end
   hook.timer( 1, "heartbeat" )
end

function board( p )
   vn.clear()
   vn.scene()
   vn.transition()
   vn.sfx( der.sfx.board )
   vn.na(fmt.f(_([[Your ship approaches the {shp}, and you rip off the cargo pod and make your way.]]),
      {shp=p}))
   vn.sfx( der.sfx.unboard )
   vn.run()

   -- Give the player the cargo
   local c = commodity.new( N_("Nexus Cargo Pod"), N_("Cargo taken from Nexus Shipyards.") )
   mem.cargo = misn.cargoAdd( c, 0 )

   mem.state = 1
   misn.osdActive(2)
   misn.markerRm()
   misn.markerAdd( swapspb )

   hook.timer( 5, "postboard" )
   pilot.toggleSpawn(true) -- spawn again
end

function postboard ()
   player.msg(fmt.f(_("l337_b01: Got the goods, on to {spb}!"),{spb=swapspb}), true )
   player.autonavReset( 5 )
end

function death ()
   if mem.state > 0 then return end
   pilot.toggleSpawn(true) -- spawn again
   lmisn.fail(_([[You were supposed to capture the cargo, not destroy the ship!]]))
end

function baddiechase1 ()
   player.msg(fmt.f(_("Trixie: {player}, you have to scram! I'll try to cover."), {player=player.name()}), true )
   player.autonavReset( 5 )
   hook.timer( 6, "baddiechase2" )
end

function baddiechase2 ()
   player.msg(_("l337_b01: Shit, here they come!"), true )
   player.autonavReset( 5 )
   hook.timer( 3, "baddiechase3" )
end

function baddiechase3 ()
   local ships = {
      "Pacifier",
      "Admonisher",
      "Shark",
      "Shark",
   }
   local baddies = fleet.add( 1, ships, fct_baddie(), targetspb )
   pilotai.apply( baddies, function (p)
      p:setHostile(true)
      p:rename( fmt.f(_("Nexus {ship}"), {ship=p:ship():name()}))
   end )
   hook.timer( 7, "baddiechase4" )
end

function baddiechase4 ()
   local ships = {
      "Hawking",
      "Hawking",
      "Shark",
      "Shark",
      "Shark",
      "Shark",
   }
   local baddies = fleet.add( 1, ships, fct_baddie(), targetspb )
   pilotai.apply( baddies, function (p)
      p:setHostile(true)
      p:rename( fmt.f(_("Nexus {ship}"), {ship=p:ship():name()}))
   end )
end
