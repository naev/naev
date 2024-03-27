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
   vn.na(_([[]]))
   l337(_([[""]]))
   vn.done("electric")
   vn.run()

   player.pay( money_reward )

   onion.log(_([[TODO]]))
   misn.finish(true)
end
