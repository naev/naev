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

-- Action happens in the jump from Tepadania (Empire) to Ianella (Dvaered)
-- l337_b01 lives in Anubis (Scorpius)?
local spb1, sys1 = spob.getS("Tepdania Prime")
local sys2 = system.get("Ianella")
local jp = jump.get( sys1, sys2 )

--local money_reward = onion.rewards.misn03

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
   end
end

function enter ()
   if system.cur()==sys1 and mem.state==1 then
      hook.timer( 7, "prepare" )
   end
end

function prepare ()
   player.msg(_("l337_b01: head over to the jump, there should be a relay buoy there."), true)
   system.markerAdd( jp:pos() )
   hook.timer( 1, "wait" )
end

local distlim = 5e3
local dunit = naev.unit("distance")
function wait ()
   if player.pos():dist2( jp:pos() ) < 1e3^2 then
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
      return
   end
   hook.timer( 1, "wait" )
end

function heartbeat ()
   local d = player.pos():dist( jp:pos() )
   if d > distlim then
      lmisn.fail(_("You strayed too far from the jump!"))
   end
   misn.osdCreate( title, {
      fmt.f(_("Distance: {d} {dunit}"), {d=d, dunit=dunit}),
   } )
   hook.timer( 0.1, "heartbeat" )
end
