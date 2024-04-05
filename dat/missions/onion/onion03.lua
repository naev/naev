--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 03">
 <unique />
 <priority>3</priority>
 <chance>20</chance>
 <location>Bar</location>
 <done>Onion Society 02</done>
 <cond>
   if spob.cur() == spob.get("Tepdania Prime") then
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
   Onion03
--]]
local fmt = require "format"
local vn = require "vn"
local onion = require "common.onion"
local love_shaders = require "love_shaders"
local lmisn = require "lmisn"
local fleet = require "fleet"
local pltai = require "pilotai"
local lg = require "love.graphics"

-- Action happens in the jump from Tepadania (Empire) to Ianella (Dvaered)
-- l337_b01 lives in Anubis (Scorpius)?
local spb1, sys1 = spob.getS("Tepdania Prime")
local sys2 = system.get("Ianella")
local jmp = jump.get( sys1, sys2 )

local money_reward = onion.rewards.misn03

local title = _("Enter the Nexus")

--[[
   Mission States
   0: mission accepted
   1: plugged into grid
   2: defended the relay
--]]
mem.state = 0

-- Create the mission
function create()
   misn.finish(false) -- disabled for now
   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_onion() )

   misn.setNPC( _("l337_b01"), prt.t.tex, _([[You seem to have an incoming connection from the Onion Society.]]) )
   misn.setReward(_("???") )
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[Help l337_b01 short the Empire-Dvaered Nexus connection near the {sys} system.]]),
      {sys=sys1}))
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local l337 = vn.newCharacter( onion.vn_l337b01() )
   vn.transition("electric")
   vn.na(_([[You answer the incoming connection and a familiar hologram appears on-screen.]]))
   l337(fmt.f(_([["Hey {player}, how's it going? Anyway, enough small chat. One of my relay backdoors seems to be down, and I need you to short the Empire-Dvaered connection for me in {sys}. You up for the task, right?"]]),
      {player=player.name(), sys=sys1}))
   vn.menu{
      {_([["What does that even mean?"]]), "01_meaning"},
      {_([["Shouldn't you flim-flam the wurglebump?"]]), "01_flimflam"},
      {_([["Count on me!"]]), "01_ok"},
      {_([["Maybe next time."]]), "decline"},
   }

   vn.label("01_ok")
   vn.func( function () accepted = true end )
   vn.jump("01_cont")

   vn.label("01_flimflam")
   l337(_([["Flim-flam the wurglebump? Wait... I see. It all flew over your head, didn't it? Cute, but you could have just said you understood nothing."]]))
   vn.label("01_meaning")
   l337(_([["Let me see if I can explain this for you to understand. It's quite simple, actually."]]))
   vn.label("01_cont")
   l337(_([["You familiar with the Nexus right? It's the digital hodgepodge that connects everyone together. Lets you send messages and data all over a galaxy, it's how news and information gets around. Really nice to get in touch with people, as long as you can afford it."]]))
   l337(_([["The only way to go faster than the speed of light and actually make the Nexus possible is to use jumpgate Nexus relays, that accumulate local data, and then jump through the gate to transmit it to the other side. Voilà, super light speed communication."]]))
   l337(_([["Of course, everyone wants in with this, so naturally, unless you pay, you can spend ages trying to go through a relay. And normally, you want to go through more than one relay to get anywhere. Meant to connect humanity, it's now just another extension of our lovely class system."]]))
   l337(_([["That's the explanation for normal people anyway, Onion Society doesn't really play by the rules, and we just short the relays to communicate with everyone, all the time, as it should be. In practice, it's not that pretty though. Poorly maintained hardware is constantly failing, and nobody seems to give a rat's ass about reliability these days."]]))
   l337(_([["An Onion Society meeting is coming up in the Nexus, and just my bad luck, the Empire-Dvaered connection seems to have had some critical hardware repaired. I need you to be my hands in the, er, non-digital realm, and help me short it so that all is good and information flows free."]]))
   vn.func( function ()
      if accepted then
         vn.jump("accepted")
      end
   end )
   l337(fmt.f(_([["Should be an easy job in the {sys} system. You in?"]]),
      {sys=sys1}))
   vn.menu{
      {_([[Accept the job.]]), "accepted"},
      {_([[Decline for now.]]), "decline"},
   }

   vn.labelb("decline")
   vn.na(_([[You decline the work for now, and the hologram fades away.]]))
   vn.done("electric")

   vn.label("accepted")
   vn.func( function () accepted = true end )
   l337(fmt.f(_([["I've hooked up into your ship, so you don't have to worry about anything, just get me close and I'll jump through your ship systems. I should have enough bandwidth for that. Anyway, head to {spob} in the {sys} system, and we can fix this."]]),
      {spob=spb1, sys=sys1}))
   vn.done("electric")
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.markerAdd( spb1 )
   mem.state = 0
   hook.land("land")
   hook.enter("enter")
   misn.osdCreate( title, {
      fmt.f(_([[Land on {spb} ({sys} system)]]),
         {spb=spb1, sys=sys1}),
   } )
end

function land ()
   if spob.cur()==spb1 and mem.state==0 then
      vn.clear()
      vn.scene()
      local l337 = vn.newCharacter( onion.vn_l337b01() )
      vn.transition("electric")
      vn.na(_([[You land and immediately l337_b01's hologram appears on your ship console.]]))
      l337(_([["Time to get to work. Let's see, let's see."
You hear some clacking noises, but you have no idea what's going on as you only see l337_b01's avatar.]]))
      l337(_([["Mmmm, I see. Seems like the relay has been replaced. Most of the relays in Empire space are controlled by Nexus Shipyards. Going to have to check their databases. One second."]]))
      l337(_([["Humph. Looks like there's some mistake in the logs. Shoddy job as always. Can't say I blame them, Empire seems to just add more and more paperwork that nobody knows how to fill."]]))
      l337(_([["Looks like we are going to have to do some field work. If you can get near the jump to the {sys2} system, I can try to override the relay. Might be trouble though. They don't take kindly to sabotage."]]))
      vn.done("electric")
      vn.run()

      mem.state = 1
      misn.osdCreate( title, {
         fmt.f(_([[Land on {spb} ({sys} system)]]),
            {spb=spb1, sys=sys1}),
      } )
      misn.markerRm()
      misn.markerAdd( sys1 )
   elseif mem.state==2 then
      local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_onion() )
      misn.npcAdd( "meeting", _("l337_b01"), prt.t.tex, _("Establish communication with l337_b01?"), 2 )
   end
end

function enter ()
   if system.cur()==sys1 and mem.state==1 then
      hook.timer( 7, "prepare" )
   end
end

function prepare ()
   player.msg(_("l337_b01: head over to the jump, there should be a relay buoy there."), true)
   system.markerAdd( jmp:pos() )
   hook.timer( 1, "wait" )
end

local distlim = 5e3
local dunit = naev.unit("distance")
local timeend
local enemies

-- Nexus ships
-- hawking, pacifier, admonisher, lancelot, shark
local function spawn_baddie( ships )
   local fct = faction.dynAdd( "Dummy", "_onion_nexus", _("Nexus IT"), { ai="baddie" } )

   local loc = jmp
   local names = {}
   for k,s in ipairs(ships) do
      names[k] = fmt.f(_("Nexus {ship}"), {ship=s:name()})
   end
   local plts = fleet.add( 1, ships, fct, loc, names )
   for k,p in ipairs(plts) do
      p:changeAI( "guard" )
      local m = p:memory()
      m.guardpos = jmp:pos() + vec2.newP( 1e3 * rnd.rnd(), rnd.angle() )
      table.insert( enemies, p )
   end

   -- Give a message if new enemies coming in
   player.msg(_("l337_b01: jump signal detected!"), true)
end

local spawned = 1
local spawntable
function wait ()
   if player.pos():dist2( jmp:pos() ) < 1e3^2 then
      pilot.toggleSpawn(false) -- no more spawning
      pltai.clear() -- get rid of pilots

      vn.clear()
      vn.scene()
      local l337 = vn.newCharacter( onion.vn_l337b01() )
      vn.transition("electric")
      vn.na(_([[l337_b01 pops up on your system console.]]))
      l337(_([["OK, time to do a scan. Yup, there's the scanner. Let's see if it's configured like the late models... root root."]]))
      l337(_([["Mmmm, admin 1234. Humph, time to pull a dictionary attack. Whelp, looks like they tried to introduce some shorting countermeausers. It's going to take me some time to crack the relay."]]))
      l337(fmt.f(_([["Hate to bubble your boat, but, looks like some incoming Nexus tech support crew. Looks like it's your time to shine. Stay within {dist} {dstunit} of the jump!"]]),
         {dist=fmt.number(distlim), dstunit=dunit}))
      vn.na(fmt.f(_([[Looks like it's time to show what {shipname} is capable of!]]),
         {shipname=player.pilot():name()}))
      vn.done("electric")
      vn.run()

      -- Set up variables
      local now = naev.ticksGame()
      timeend = now + 150 -- 2.5 minutes
      spawned = 1
      spawntable = {
         { t=now+10, s={"Lancelot", "Lancelot", "Shark", "Shark"} },
         { t=now+30, s={"Admonisher", "Admonisher"} },
         { t=now+60, s={"Pacifier", "Lancelot", "Lancelot"} },
         { t=now+90, s={"Admonisher", "Shark", "Shark", "Shark" } },
         { t=now+120, s={"Pacifier"} },
      }
      enemies = {}

      spawn_baddie{ "Pacifier", "Lancelot", "Lancelot" }
      heartbeat()
      return
   end
   hook.timer( 1, "wait" )
end

function heartbeat ()
   local d = player.pos():dist( jmp:pos() )
   if d > distlim then
      lmisn.fail(_("You strayed too far from the jump!"))
   end
   local now = naev.ticksGame()
   if spawned < #spawntable and now >= spawntable[spawned].t then
      spawn_baddie( spawntable.s )
      spawned = spawned+1
   end
   local left = timeend-now
   if left <= 0 then
      misn.osdCreate( title, {
         _("You won?"),
      } )
      player.msg(_("l337_b01: Done! Wait, why isn't it working."), true)
      hook.timer( 5, "theend" )
      return
   end
   local dstr = fmt.number(d)
   if d > 4e3 then
      dstr = "#o"..dstr
   end
   misn.osdCreate( title, {
      fmt.f(_([[Defend the Jump
Distance: {d} {dunit}
Time left: {left:.1f}]]), {d=dstr, dunit=dunit, left=left}),
   } )
   hook.timer( 0.1, "heartbeat" )

   local capship = false
   for k,p in ipairs(enemies) do
      if p:exists() and p:size() >= 3 then
         capship = true
         break
      end
   end
   if not capship then
      spawn_baddie{ "Pacifier", "Lancelot", "Lancelot" }
   end
end

local bossname = _("Nexus RTFM")
function theend ()
   local plts = spawn_baddie{ "Hawking", "Admonisher", "Admonisher" }
   plts[1]:rename(bossname)
   hook.pilot( plts[1], "death", "rtfm_death" )
   hook.pilot( plts[1], "board", "rtfm_board" )
   player.msg(_("l337_b01: Another? Incoming jump signal. It's large!"), true)
   hook.timer( 5, "lastmsg" )
end

function lastmsg ()
   player.msg(fmt.f(_("l337_b01: That's the cause! Take down the {shipname}!"), {shipname=bossname}), true)
      misn.osdCreate( title, {
         fmt.f(_("Defeat the {shipname}"),{shipname=bossname}),
      } )
end

-- Make all the enemies go away
local function runaway ()
   for k,p in ipairs(enemies) do
      if p:exists() then
         p:control()
         p:hyperspace( jmp )
      end
   end
end

function rtfm_death ()
   hook.timer( 5, "won" )
   runaway()
end

function won ()
   vn.clear()
   vn.scene()
   local l337 = vn.newCharacter( onion.vn_l337b01() )
   vn.transition("electric")
   vn.na(_([[An easily recognizable hologram pops up.]]))
   l337(_([["Man, you make it look so easy. Sorry I wasn't able to lend a hand, bandwidth is still at a premium."]]))
   l337(fmt.f(_([["With the destruction of {shipname}, I've created enough issues and bugs on their internal trackers that they'll probably never notice the relay got shorted."]]),
      {shipname=bossname}))
   l337(_([["The relay connection looks stable, however, it's best to not use bandwidth out in open space. Too easy to track. Try landing somewhere, so I can mask the communication bandwidth."]]))
   vn.done("electric")
   vn.run()

   misn.osdCreate( title, {
      _("Land and talk communicate with l337_b01"),
   } )
   mem.state = 2
end

function rtfm_board( p )
   vn.clear()
   vn.scene()
   local l337 = vn.newCharacter( onion.vn_l337b01() )
   vn.transition("electric")
   vn.na(fmt.f(_([[Before you board {shipname}, l337_b01's hologram pops up.]]),
      {shipname=bossname}))
   l337(fmt.f(_([["Got to say that was some incredible flying. Not that often you see a {shipclass}-class ship's systems fried like that. So exciting I kept a video recording."]]),
      {shipclass=p:ship():name()}))
   l337(fmt.f(_([["Anyway, no need to worry more about {shipname} and the relay! With their ship disabled and the firewall down, I was able to pick clean their main data."]]),
      {shipname=bossname}))
   l337(_([["I've sent some anonymous tips to Nexus Shipyards HQ about the captain and some irregularities, and they'll be fired and maybe even tried for treason. The commotion will make it, so nobody will realize the relay was shorted."]]))
   l337(_([["The relay connection looks stable, however, it's best to not use bandwidth out in open space. Too easy to track. Try landing somewhere, so I can mask the communication bandwidth."]]))
   vn.done("electric")
   vn.run()

   misn.osdCreate( title, {
      _("Land and talk communicate with l337_b01"),
   } )
   mem.state = 2
   runaway() -- Others run away
   p:disable() -- Disable pilot permanently
end

-- Misison end cutscene
function meeting ()
   vn.clear()
   vn.scene()
   local l337 = vn.newCharacter( onion.vn_l337b01() )
   vn.transition("electric")
   vn.na(_([[You open up a channel with l337_b01.]]))
   if not mem.talked_l337 then
      vn.func( function () mem.talked_l337 = true end )
      l337(_([["Hey, I was benchmarking the relay connection, and it seems to be running quite smoothly. Not even hitting a packet loss of 20%! Seems like Nexus Shipyards has finally stepped up their hardware game."]]))
      l337(_([["With this, the Onion Society conclave should go without a hitch."
There is a slight pause.
"Say, you want in?"]]))
      vn.na(_([[You're not sure what happens, but before you can give a proper answer, l337_b01 manages to drag you into participating.]]))
      l337(_([["Just remember, don't give any personal or identifying information, don't encourage lonewolf4's antics, don't talk to notasockpuppet, wait, in fact, it's probably better for you not to talk at all."]]))
      l337(_([["Yeah, I'll just hook you up to a read-only feed with a default avatar. That should probably work. Weirder things have gone on in the past."]]))
      l337(_([["Are you ready?"]]))
   else
      l337(_([["So about the Onion Society conclave, you ready?"]]))
   end
   vn.menu{
      {_([["Hell yes!"]]), "01_yes"},
      {_([["Yes."]]), "01_yes"},
      {_([["Not yet."]]), "01_no"},
   }

   vn.label("01_no")
   vn.na(_([[You say you need to prepare. l337_b01 seems to understand and tells you to come back when you're ready for the Onion Society conclave.]]))
   vn.done("electric")

   vn.label("01_yes")
   l337(fmt.f(_([["{playername}, get strapped in, we're headed to the Nexus!"

Quite a fancy way of referring to connecting into a full sensorial holodeck.]]),
      {playername=player.name()}))
   vn.scene()
   local bg_cyberspace = love_shaders.shader2canvas( love_shaders.cyberspace() )
   vn.setBackground( function ()
      lg.setColour( 1, 1, 1, 1 )
      bg_cyberspace:draw( 0, 0 )
   end )
   vn.transition("fadeup")
   vn.scene()
   local l337er = vn.newCharacter( onion.vn_nexus_l337b01() )
   vn.transition()
   vn.na(_([[The world lurches around you, and you are plunged into the Nexus. Ugh, that's not a pleasant feeling.]]))
   l337er(_([["How are you feeling? Isn't it great? Nothing like home sweet home."]]))
   vn.menu{
      {_([["I feel sick."]]), "02_sick"},
      {_([["Home?"]]), "02_home"},
      {_([["Isn't this very empty?"]]), "02_empty"},
   }

   vn.label("02_sick")
   l337er(_([["Ah, the good old Nexus syndrome. You'll get used to it, and after a while, you'll look forward to it. It's not that bad."]]))
   vn.jump("02_cont")

   vn.label("02_home")
   l337er(_([["Well I, erhm, spend way more time here than not. You can do anything you want here, unburdened from physical shackles. The body is the tomb of the mind, but you're free of your tomb in here!"]]))
   vn.jump("02_cont")

   vn.label("02_empty")
   l337er(_([["Ah, yeah, it only seems empty to you because you don't have developer mode on, I wouldn't recommend it at first. It's usually too much information and can cause your last meal to find its way out your mouth."]]))
   vn.label("02_cont")
   l337er(_([["The Nexus habitat was originally designed by pure speculative greed. With normal goods and services, you have to do something right? However, if it's all digital, then there are no limits, and the gears of capitalism can go full speed. That's why it's so big and spacious."]]))
   l337er(_([["Of course, you can guess it didn't turn out as they expected. Ended up as abandonware, only recently was it rediscovered, and after a few decaperiod hacking sessions, we got some prototype running and onboard came the Onion Society."]]))
   l337er(_([["The codebase is humongous too, you can even find mix of paradigms, it's a true software archaeologists dream come true!"]]))
   l337er(_([["Did I tell you about the time... wait, we don't have time for this right now. Before we head off to the conclave, I wanted to meet up with Trixie first. They should be here anytime soon."]]))
   vn.move( l337er, "left" )
   local trixie = vn.newCharacter( onion.vn_nexus_trixie{pos="right"} )
   vn.appear( trixie )
   trixie(_([["Trixie always appears on time, never late, nor early!"]]))
   l337er(_([["Hey! Impeccable timing, as usual. Did you get the cat video I sent you?"]]))
   trixie(_([["It was entertaining as always. Cats never cease to amuse with their antics."]]))
   l337er(_([["Oh, this is... mmmmm... darkkazoo."
They gesture towards you.]]))
   vn.menu{
      {_([[...!]]), "03_cont"},
      {_([[... .. ...]]), "03_cont"},
      {_([[...]]), "03_cont"},
   }

   vn.label("03_cont")
   l337er(_([["Ah, yes. Forgot the read-only mode. Oh well, it shouldn't be long."]]))
   trixie(_([["darkkazoo, that's a curious name. Quite ominous."]]))
   l337er(_([["Thanks! Before I forget, let me wire you the files I promised. Opening secure channel."]]))
   trixie(_([["One sec... ... ...there, got it, thanks!"]]))
   trixie(_([["I see, so Ogre wasn't the one we were looking for. Just another fraud. Seems to be many these days."]]))
   l337er(_([["Yeah, taught them a lesson though. Could be someone closer, maybe in the inner circle."]]))
   trixie(_([["Not something we can rule out yet. Wait... How much does darkkazoo know? Are they to be trusted."]]))
   l337er(_([["They've proven their worth, but I haven't explained it yet. Just wanted to drag them along a bit for now, see if we could get a better 3rd party opinion. Us Nexus dwellers tend to be a bit of an echo chamber."]]))
   trixie(_([["You and your wild ideas. You should be more careful, for all we know, darkkazoo could be one of them."]]))
   l337er(_([["Come on, life is too short to be stuck in an ivory tower all day! Anyway, we've got to hurry, the colloquium is going to start soon, and we should get there to set up some protection before it starts."]]))
   l337er(_([["Um, darkkazoo, remember what I told you? Just watch, and try not to stick your neck out, or it'll get chopped away. I'll be lifting you in my seal, so unless someone pries too hard, you should be mainly out of sight."]]))
   trixie(_([["Let's go!"]]))

   vn.scene()
   local offset = 1/7
   l337er = vn.newCharacter( onion.vn_nexus_l337b01{pos=1*offset, flip=true} )
   local underworlder = vn.newCharacter( onion.vn_nexus_underworlder{pos=2*offset, flip=true} )
   local puppet = vn.newCharacter( onion.vn_nexus_notasockpuppet{pos=3*offset, flip=true} )
   trixie = vn.newCharacter( onion.vn_nexus_trixie{pos=4*offset, flip=false} )
   local dog = vn.newCharacter( onion.vn_nexus_dog{pos=5*offset, flip=false} )
   local lonewolf4 = vn.newCharacter( onion.vn_nexus_lonewolf4{pos=6*offset, flip=false} )
   vn.transition()
   vn.na(_([[The world lurches once again, although this time it feels slightly duller than before.

You find yourself in a seemingly infinite field full of odd looking avatars. Is that a sock?]]))

   l337er()
   underworlder()
   puppet()
   trixie()
   dog()
   lonewolf4()

   -- Leave the Nexus
   vn.scene()
   vn.transition()
   vn.scene()
   vn.setBackground()
   vn.transition("fadedown")

   vn.run()

   player.pay( money_reward )

   onion.log(_([[TODO]]))
   misn.finish(true)
end
