--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 03">
 <unique />
 <priority>3</priority>
 <chance>20</chance>
 <location>Bar</location>
 <done>Onion Society 02</done>
 <cond>
   local c = spob.cur()
   if c == spob.get("Tepdania Prime") then
      return false
   end
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
local audio = require 'love.audio'

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
   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_l337b01() )

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
   vn.music( onion.loops.hacker )
   vn.transition("electric")
   vn.na(_([[You accept the incoming connection and a familiar hologram appears on-screen.]]))
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
   l337(_([["An Onion Society conclave is coming up in the Nexus, and just my bad luck, the Empire-Dvaered connection seems to have had some critical hardware repaired. I need you to be my hands in the, er, non-digital realm, and help me short it so that all is good and information flows free."]]))
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

   vn.label("decline")
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
      vn.music( onion.loops.hacker )
      vn.transition("electric")
      vn.na(_([[You land and immediately l337_b01's hologram appears on your ship console.]]))
      l337(_([["Time to get to work. Let's see, let's see."
You hear some clacking noises, but you have no idea what's going on as you only see l337_b01's avatar.]]))
      l337(_([["Mmmm, I see. Seems like the relay has been replaced. Most of the relays in Empire space are controlled by Nexus Shipyards. Going to have to check their databases. One second."]]))
      l337(_([["Humph. Looks like there's some mistake in the logs. Shoddy job as always. Can't say I blame them, Empire seems to just add more and more paperwork that nobody knows how to fill."]]))
      l337(fmt.f(_([["Looks like we are going to have to do some field work. If you can get near the jump to the {sys} system, I can try to override the relay. Might be trouble though. They don't take kindly to sabotage."]]),
         {sys=sys2}))
      vn.done("electric")
      vn.run()

      mem.state = 1
      misn.osdCreate( title, {
         fmt.f(_([[Head to the jump to {sys2} in the {sys1} system]]),
            {sys2=sys2, sys1=sys1}),
      } )
      misn.markerRm()
      misn.markerAdd( sys1 )
   elseif mem.state==2 then
      local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_l337b01() )
      misn.npcAdd( "conclave", _("l337_b01"), prt.t.tex, _("Establish communication with l337_b01?"), 2 )
   end
end

function enter ()
   hook.timerClear()
   if system.cur()==sys1 and mem.state==1 then
      hook.timer( 7, "prepare" )
   end
end

function prepare ()
   player.msg(_([[l337_b01: "head over to the jump, there should be a relay buoy there."]]), true)
   system.markerAdd( jmp:pos() )
   hook.timer( 1, "wait" )
end

local distlim = 2500
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
      names[k] = fmt.f(_("Nexus {ship}"), {ship=ship.name(s)})
   end
   local plts = fleet.add( 1, ships, fct, loc, names )
   for k,p in ipairs(plts) do
      p:setHostile(true)
      p:changeAI( "guard" )
      local m = p:memory()
      m.guardpos = jmp:pos() + vec2.newP( 1e3 * rnd.rnd(), rnd.angle() )
      table.insert( enemies, p )
   end

   -- Give a message if new enemies coming in
   player.msg(_([[l337_b01: "jump signal detected!"]]), true)
   return plts
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
      l337(_([["Nope? Mmmm, let's see admin 1234. Also, nope. High-tech security we have here. Humph, time to pull a dictionary attack. Whelp, looks like they tried to introduce some shorting countermeasures. It's going to take me some time to crack the relay."]]))
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
         { t=now+15, s={"Lancelot", "Shark"} },
         { t=now+40, s={"Admonisher", "Lancelot", "Lancelot"} },
         { t=now+80, s={"Pacifier", "Shark", "Shark"} },
         { t=now+110, s={"Lancelot", "Shark", "Shark" } },
         { t=now+125, s={"Admonisher"} },
      }
      enemies = {}

      spawn_baddie{ "Pacifier" }
      heartbeat()
      return
   end
   hook.timer( 1, "wait" )
end

local do_extra_baddie
function heartbeat ()
   local d = player.pos():dist( jmp:pos() )
   if d > distlim then
      lmisn.fail(_("You strayed too far from the jump!"))
   end
   local now = naev.ticksGame()
   if spawned < #spawntable and now >= spawntable[spawned].t then
      spawn_baddie( spawntable[spawned].s )
      spawned = spawned+1
   end
   local left = timeend-now
   if left <= 0 then
      misn.osdCreate( title, {
         _("You won?"),
      } )
      player.msg(_([[l337_b01: "Done! Wait, why isn't it working."]]), true)
      hook.timer( 10, "theend" )
      return
   end
   local dstr = fmt.number(d)
   if d > distlim*2/3 then
      dstr = "#o"..dstr
   end
   misn.osdCreate( title, {
      fmt.f(_([[Defend the Jump
Distance: {d} {dunit}
Time left: {left:.1f}]]), {d=dstr, dunit=dunit, left=left}),
   } )
   hook.timer( 0.1, "heartbeat" )

   if not do_extra_baddie then
      local capship = false
      for k,p in ipairs(enemies) do
         if p:exists() and p:ship():size() >= 3 then
            capship = true
            break
         end
      end
      if not capship then
         do_extra_baddie = hook.timer( 9, "extra_baddie" )
      end
   end
end

function extra_baddie ()
   spawn_baddie{ "Admonisher" }
   do_extra_baddie = nil
end

local bossname = _("Nexus RTFM")
function theend ()
   player.msg(_([[l337_b01: "Another? Incoming jump signal. It's large!"]]), true)
   hook.timer( 8, "moremsg" )
end

function moremsg ()
   local plts = spawn_baddie{ "Hawking" }
   plts[1]:rename(bossname)
   hook.pilot( plts[1], "death", "rtfm_death" )
   hook.pilot( plts[1], "board", "rtfm_board" )
   hook.timer( 3, "lastmsg" )
end

function lastmsg ()
   player.msg(fmt.f(_([[l337_b01: "That's the cause! Take down the {shipname}!"]]), {shipname=bossname}), true)
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
   hook.timerClear()
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
   pilot.toggleSpawn(true) -- spawn again
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
   p:setDisable() -- Disable pilot permanently
   pilot.toggleSpawn(true) -- spawn again
end

-- Mission end cutscene
function conclave ()
   vn.clear()
   vn.scene()
   vn.music( onion.loops.hacker )
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
   vn.func( function ()
      var.push( "nexus_sickness", true )
   end )
   l337er(_([["Ah, the good old Nexus syndrome. You'll get used to it, and after a while, you'll look forward to it. It's not that bad."]]))
   vn.jump("02_cont")

   vn.label("02_home")
   l337er(_([["Well I, erhm, spend way more time here than not. You can do anything you want here, unburdened from physical shackles. The body is the tomb of the mind, but you're free of your tomb in here!"]]))
   vn.jump("02_cont")

   vn.label("02_empty")
   l337er(_([["Ah, yeah, it only seems empty to you because you don't have developer mode on, I wouldn't recommend it at first. It's usually too much information and can cause your last meal to find its way out your mouth."]]))
   vn.label("02_cont")
   l337er(_([["The Nexus habitat was originally designed by pure speculative greed. With normal goods and services, you have to do something right? However, if it's all digital, then there are no limits, and the gears of capitalism can go full speed. That's why it's so big and spacious."]]))
   l337er(_([["Of course, you can guess it didn't turn out as they expected. Ended up as abandonware, only recently was it rediscovered, and after a few deca-period hacking sessions, we got some prototype running and onboard came the Onion Society."]]))
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
   l337er(_([["Come on, life is too short to be stuck in an ivory tower all day! Anyway, we've got to hurry, the conclave is going to start soon, and we should get there to set up some protection before it starts."]]))
   l337er(_([["Um, darkkazoo, remember what I told you? Just watch, and try not to stick your neck out, or it'll get chopped away. I'll be lifting you in my seal, so unless someone pries too hard, you should be mainly out of sight."]]))
   l337er(_([["Wait, let me make some adjustments."]]))
   vn.na(_([[You notice the world, including l337_b01 and Trixie, seems to get larger around you. Wait no, you're just getting smaller. Eventually you become a small enough so that l337_b01 grabs you and puts you on their shoulder. You can't seem to move much, but at least you get a good view of things.]]))
   trixie(_([["Let's go!"]]))

   vn.scene()
   local offset = 1/7
   l337er = vn.newCharacter( onion.vn_nexus_l337b01{pos=1*offset} )
   local underworlder = vn.newCharacter( onion.vn_nexus_underworlder{pos=2*offset, flip=true} )
   local puppet = vn.newCharacter( onion.vn_nexus_notasockpuppet{pos=3*offset, flip=true} )
   trixie = vn.newCharacter( onion.vn_nexus_trixie{pos=4*offset} )
   local dog = vn.newCharacter( onion.vn_nexus_dog{pos=5*offset, flip=false} )
   local lonewolf4 = vn.newCharacter( onion.vn_nexus_lonewolf4{pos=6*offset, flip=false} )
   vn.music( onion.loops.circus )
   vn.transition()
   vn.na(_([[The world lurches once again, although this time it feels slightly duller than before.

You find yourself in a seemingly infinite field full of odd looking avatars. Is that a sock?]]))

   underworlder(_([["...and then wanting to see some fireworks, I rerouted the offensive firewalls to each other. The recursion was beautiful until the mainframe dropped."]]))
   lonewolf4(_([["Ah, fellow technomancers, it seems like l337_b01 and Trixie have graced us with their presence. We are all here for today's Conclave. Let us blur the limits of possibility!"]]))
   trixie(_([["Always so grandiose lonewolf4. You managed to get a stable connection this time?"]]))
   lonewolf4(_([["Why of course, I spare no expense on bandwidth to be together on such a glorious occasion!"]]))
   puppet(_([["What's so glorious? Another fraud getting ousted? The relays are languishing, mmmm."]]))
   underworlder(_([["If you're low on bandwidth, I could take some of your puppets off your hand. Ha ha!"]]))
   l337er(_([["You complain, yet your puppets took down the peeled Ogre in less than a period!"]]))
   puppet(_([[notasockpuppet lets out a sigh.
"It's easy for you sproutlings to make with scraps, you never knew the true Nexus, before the Incident. We're lucky to see 5% of the bandwidth on a good cycle!"]]))
   underworlder(_([["Aaah, ah. There you go again. Get used to it already."]]))
   lonewolf4(_([["Tales of yore are naught but echoes in the digital void. The codes of legend are naught forgotten, but at hand are the new dragons of our time. We must cast aside the relics of the past and focus on the codes of the future."]]))
   dog(_([[While the others are talking, you notice that DOG seems to stare at you. Have they figured you out? Either way, they seem to be intent on keeping their thoughts to themselves.]]))
   trixie(_([["lonewolf4 is right, we're not here to here ramblings. There seems to be some dark motion, dragons if you want to call them that, going on."]]))
   l337er(_([["Yes, the packets are behaving abnormally."]]))
   underworlder(_([["Are you sure this isn't just Nexus Shipyards new relay models? They seem to be running some weird firmware."]]))
   l337er(_([["Unlikely, but not impossible."]]))
   puppet(_([["The Great Houses are bickering. It seems like the post-Incident peace is coming to its end. My puppets are restless."]]))
   underworlder(_([["Meh, they're always bickering. Tell your puppets to calm down. We don't want to make a scene."]]))
   trixie(_([["I hope it isn't anything worse. It's much harder to maintain these real-time conversations."]]))
   lonewolf4(_([["The winds of fate are beginning to howl. We must play our cards wisely, lest another rupture tears us apart. The fairies whisper of treason among the ranks."]]))
   puppet(_([["Ha ha. Quite strong accusations. What do you think DOG? Is your connection holding up."]]))
   dog(_([["It is."]]))
   puppet(_([["Such a way with words. As talkative as usual."]]))
   underworlder(_([["lonewolf4 is just playing his usual mind games. Always has to imagine we're in some sort of great fantasy novel."]]))
   lonewolf4(_([["The signs are all there, yet the cobwebs of your own ineptitude cloud your vision."]]))
   vn.musicStop()
   underworlder(_([["U wot m4"]]))
   vn.sfx( audio.newSoundData('snd/sounds/activate5') )
   lonewolf4(_([[lonewolf4 flickers briefly
"Mortem Tenebrae Obscuritas Infernalis!"]]))
   underworlder(_([["Bring it on!"]]))
   vn.sfx( audio.newSoundData("snd/sounds/avatar_of_sirichana") )
   vn.na(_([[Trixie and l337_b01 seem to mutter something and take cover as lonewolf4 and underworlder seems to be engaging in some sort of digital combat. You can't really follow what is going on, and all you can see are some rendering glitches as the two run around each other.]]))
   vn.sfx( audio.newSoundData('snd/sounds/detonation_alarm') )
   lonewolf4(_([["Exanimo Umbra Obscura Tenebrarum!"]]))
   vn.sfx( audio.newSoundData('snd/sounds/activate4') )
   underworlder(_([[underworlder seems to freeze in place.]]))
   dog(_([["Down."]]))
   vn.sfx( audio.newSoundData('snd/sounds/crash1') )
   vn.animation( 0.5, function ( x )
      -- easeOutElastic
      local progress = math.pow(2, -10 * x) * math.sin((x * 10 - 0.75) * 2*math.pi/3) + 1
      lonewolf4.offy = progress * 200
      underworlder.offy = progress * 200
   end, nil, "linear" )
   vn.na(_([[Suddenly lonewolf4 and underworlder are splatted prone on the ground.]]))
   underworlder(_([["Screw this.", underworlder gasps before flickering out of existence.]]))
   vn.disappear( underworlder )
   lonewolf4(_([[lonewolf4 stands up and seems to brush non-existent dust off their clothes.
"Humph, such a vulgar character."
They seem much lower resolution than before.]]))
   lonewolf4(_([[They flash a glare at notasockpuppet before fading out of existence too. What happened?]]))
   vn.disappear( lonewolf4 )
   puppet(_([["Seems like today's conclave is at an end. Amusing as always, but short."]]))
   vn.disappear( puppet )
   dog(_([[DOG abruptly flashes out of existence.]]))
   vn.disappear( dog )
   trixie(_([[Trixie lets out a sigh.]]))

   vn.scene()
   l337er = vn.newCharacter( onion.vn_nexus_l337b01{pos="left"} )
   trixie = vn.newCharacter( onion.vn_nexus_trixie{pos="right"} )
   vn.music( onion.loops.hacker )
   vn.transition()
   vn.na(_([[The world warps abruptly.]]))
   l337er(_([["Better off in a new channel, too many zombie processes in the last one."]]))
   trixie(_([["Ugh, lonewolf4 might have hit the nail on the head."]]))
   l337er(_([["Maybe, but it's not like everyone is rowing on the same boat anyway. I thought the only reason the Onion Society existed was sort of keeping everyone from everyone's throats."]]))
   trixie(_([["And yet, everyone is one step away from bloodshed."]]))
   l337er(_([[They pick you up and put you on the floor.
"Almost forgot."]]))
   vn.na(_([[Slowly everything gets smaller until you are back to normal size.]]))
   l337er(_([["Go on, you can talk now. I'm sure you have questions."]]))
   vn.label("04_menu")
   vn.menu{
      {_([["Technomancers?"]]), "04_techo"},
      {_([["What happened?"]]), "04_what"},
      {_([["Who were they?"]]), "04_who"},
      {_([["Is it always like that?"]]), "04_always"},
      {_([["What are you going to do now?" (continue)]]), "04_cont"},
   }

   vn.label("04_what")
   l337er(_([["Yet another bickering conclave. The conclaves are meant to keep people off each other's toes and share weaknesses and strengths of the network, but everyone is just always on the edge lately."]]))
   trixie(_([["Can't really be helped. Bandwidth is at a premium, and it is a zero-sum game. Nobody wants to give up any precious resources. Not to mention the big egos."]]))
   l337er(_([["It's the first time I've seen such a display of offensive program casting. Maybe it is the end."]]))
   trixie(_([["The Onion Society has been through worse. We just have to figure out what is going on, there seem to be darker motives behind this."]]))
   vn.jump("04_menu")

   vn.label("04_who")
   l337er(_([["All the core members were there today: DOG, notasockpuppet, lonewolf4, underworlder, Trixie, and me. Technomancers are always an odd bunch, have to be careful to not ruffle them the wrong way."]]))
   trixie(_([["DOG is one of the oldest members, I'm pretty sure they were a founding member, but it's hard to tell. They are usually quite reserved, but have some wild programs I've never seen before up their sleeves. Can't really tell what they're thinking of."]]))
   trixie(_([["notasockpuppet is also quite an old member, however, it's not really clear who they are. They have a giant puppet net, and usually come connect via multiple random puppets. Hard to tell if they are even the avatar most of the time. You do not want to mess with them though. They're quite frivolous, unpredictable, and dangerous."]]))
   l337er(_([["Other than that, there's lonewolf4, who is, as you probably guessed, a bit dramatic and over the top. Still top-notch, but it's a bit hard to tell what they're saying. Last, underworlder is a bit brash, but quite skilled. Can't really well what they're thinking either. Although, that's quite common among technomancers."]]))
   l337er(_([["And you already know Trixie and I. Trixie has been around much longer than I have, and has shown me a bit of the ropes here."]]))
   trixie(_([["Not that I really had to teach much, l337_b01 has got some skills!"]]))
   vn.jump("04_menu")

   vn.label("04_always")
   l337er(_([["No, I haven't seen anything like that, although it has been progressively worse for a while now."]]))
   trixie(_([["There have been some cases of direct confrontations in the past, some that make today's events pale in comparison, but it's been ages since I've seen shit like that, and it almost ended the Onion Society itself!"]]))
   l337er(_([["Wow, must have been wild."]]))
   trixie(_([["Oh it was. Things may be getting worse, but there's still a ways to go before it falls apart."]]))
   vn.jump("04_menu")

   vn.label("04_techo")
   l337er(_([["Technomancer, code wizards, sourcerers, hackzerkers, it's all the same. Terms for those who do the impossible with technology. You've got to get a knack for bending the code to your will and interfacing kernel functions here."]]))
   l337er(_([["The Nexus and most systems are a horrible mess of barely working archaic systems that nobody understands. We exploit vulnerabilities to do as we please."]]))
   trixie(_([["It's not all fun and games, it has lots of implications everywhere. It is fun though. :)"]]))
   vn.jump("04_menu")

   vn.label("04_cont")
   l337er(_([["That's a good question. What should we do?"]]))
   trixie(_([["Well, they always say follow the money, so how does a bank heist sound?"]]))
   l337er(_([["Excellent!"]]))
   vn.menu{
      {_([["I'm in!"]]), "05_in"},
      {_([["Isn't that illegal?"]]), "05_illegal"},
   }

   vn.label("05_in")
   trixie(_([["Great! It's always nice to have a skilled pilot. We can handle the data, but there's only so much you can do through a constrained bandwidth connection outside the Nexus."]]))
   vn.jump("05_cont")

   vn.label("05_illegal")
   trixie(_([["Illegal? It's an imperative correction to a broken system. These banks, trust me I've been in their financial data, exist only to exploit the regular people while funneling money to the pockets of the elite, bribing officials, and doing mass media manipulation."]]))
   trixie(_([["So when we do a bank heist, it's not theft, it's justice. It's a redistribution of wealth away from the system that perpetuates injustices. It's daring to challenge the status quo and reshape the world for the better!"]]))
   l337er(_([["And making a pretty credit while we're at it."]]))
   trixie(_([["Damn right we're making a pretty credit!"]]))
   vn.jump("05_cont")

   vn.label("05_cont")
   l337er(fmt.f(_([["OK, here's what we'll do figure out the logistics a bit and get back to you, {player}. Then it's time to do justice!"]]),
      {player=player.name()}))
   trixie(_([["We're going to make them cry!"]]))

   -- Leave the Nexus
   vn.scene()
   vn.music()
   vn.transition()
   vn.scene()
   vn.setBackground()
   vn.transition("fadedown")
   vn.na(fmt.f(_([[Your connection to the Nexus closes, and you find yourself back on {spob}. You sit for a moment trying to process all that happened, when you suddenly notice you are receiving a credit transfer from an anonymous account. You can guess who it is from.]]),
      {spob=spob.cur()}))
   vn.sfxVictory()
   vn.func( function ()
      player.pay( money_reward )
   end )
   vn.na( fmt.reward(money_reward) )

   vn.run()

   onion.log(fmt.f(_([[You helped l337_b01 short a Nexus relay in the {sys} system to give them a better connection to the Nexus. Afterwards you were invited to a hectic Onion Society Conclave that ended poorly. Trixie and l337_b01 mentioned they will get in touch with you again to do a bank heist in the near future.]]),
      {sys=sys1}))
   misn.finish(true)
end
