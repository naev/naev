--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 4">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Research Post Sigma-13</spob>
 <location>Bar</location>
 <done>Za'lek Black Hole 3</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 04

   Try to win the trust and coax the feral bioship
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local lmisn = require "lmisn"
local luaspfx = require "luaspfx"
local love_shaders = require "love_shaders"
local tut = require "common.tutorial"
local cinema = require "cinema"

local reward = zbh.rewards.zbh04

local mainpnt, mainsys = spob.getS("Research Post Sigma-13")

function create ()
   if not misn.claim( mainsys ) then
      misn.finish()
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You find Zach deep in thought, who takes a while to notice your presence.]]))
   z(fmt.f(_([["Ah, didn't notice you. Sometimes I get lost in here. Too many things remind me of them."
He chugs his drink and seems to sober up.
"I've been organizing the notes I've found around the station, and it seems like there was anomalies or something going on around the station. Would you be willing to take a look for me?"]]),{}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(p_("Zach", [["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(_([["Thanks again. So, looking into the notes, I found mentions to some sort of object around the station. It seems like it was a ship or some sort of drone, but I haven't been able to restore all the data yet, so the initial encounters are missing. I only mainly know the codename they gave it, which is 'Icarus', although it's not clear why."]]))
   z(_([["I have been able to recover lots of data regarding some sort of language, which is pretty strange given that all standard ship AI have universal translators, and languages haven't really been an issue outside of pure academic settings in ages. Quite a few details haven't really been recovered yet, but hopefully the data recovery drone will get something useful in a bit."]]))
   z(fmt.f(_([["Other than that, there's still the mystery of the ships that tried to attack {pnt}. I have a feeling that they're not finished with business here. You should keep an eye out for any suspicious ships. There shouldn't be anyone here other than us."]]),{pnt=mainpnt}))
   vn.na(fmt.f(_([[Suddenly, one of Zach's drone starts flashing and announces that movement detected in {sys}.]]),{sys=mainsys}))
   z(_([["Shit, it looks like something is out there already! From the readings it doesn't look like it's very large. Quick, go see what is out there!"
He gets up and starts running to the command center.
"Looks like it's time to try the new scanner! I'll give you instructions once you're out there!"]]))
   vn.func( function () accepted = true end )
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Black Hole Mystery") )
   misn.setReward(reward)
   misn.setDesc( fmt.f(_("Patrol the {sys} system and report your observations to Zach at {pnt}."), {pnt=mainpnt, sys=mainsys}) )

   mem.mrk = misn.markerAdd( mainsys )
   mem.state = 1

   misn.osdCreate( _("Black Hole Mystery"), {
      fmt.f(_("Scout around ({sys} system)"), {sys=mainsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"), {pnt=mainpnt, sys=mainsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state~=2 or spob.cur() ~= mainpnt then
      lmisn.fail(_("You were supposed to follow the detected motion!"))
      return
   end

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You land while the feral bioship waits cautiously outside the space docks. It seems like it's not entirely sure what to do.]]))
   z(_([[Zach is peeking out of the airlock.
"Damn, it's more impressive when it's up close. We have to be careful though, it's not clear what its intentions are, although it does not seem to be aggressive. Let me see if my drones can scan it and give us more details."]]))
   vn.na(_([[A couple of drones fly past you and head towards the feral bioship, which reacts in fear and runs away until you can barely see it in the distance.]]))
   z(_([["Fear of drones? Something must have happened it to react this way. Very interesting how it seems to behave almost like a normal animal. Let's see if I can get it back here."]]))
   z(_([[He sits cross-legged on the floor and sets his cyberdeck on his lap, spending a second to stretch, before engaging in a furious typing spree. The drones come back and hover nearby him, patiently awaiting orders.]]))
   z(_([[He begins muttering to himself.
"Let's see…  …  …maybe if I…  …  …not that line…
…that shouldn't be there…  …who the hell wrote this code?…
…oh wait, that was me wasn't it?…  …  …damn."]]))
   vn.na(_([[The muttering goes on for quite a long time, until he suddenly slams the enter key, You can see a brief flicker around the drones as the station's holographic project starts to overlay a hologram of cute cats on them. However, given the size of the drones, they seem to be more like large tigers that are rather quite intimidating. The cat hologram drones fly once again out into space, and while staying near the station begin to broadcast a message towards the feral bioship. ]]))
   vn.sfx( zbh.sfx.spacewhale1 )
   vn.na(_([["Friend. Peace. Play."
The message resonates throughout the station and out into space…]]))
   vn.na(_([[After what seems like ages, the feral bioship starts approaching slowly, apparently intrigued by the holographic projections. As the drones have been programmed to keep their distance, they back away as the feral bioship approaches. This makes the bioship more intrigued and it begins to chase the cat drones around the exterior of the ship.]]))
   vn.na(_([[As you are enjoying the playful spectacle, you see Zach staring intently at all the collected data flashing on his cyberdeck screen.]]))
   z(_([["Mmmm… this isn't looking very good. From a preliminary analysis of the feral bioship, or shall I say #oIcarus#0, we can see there's some pretty major structural damage that hasn't quite healed fully. Furthermore, I would venture to say that it even seems to be what you would call 'malnourished'. I'm much more familiar with normal technology than this biotechnology, so I'll have to double check with databases, but that's the only conclusion I can come to right now."]]))
   z(_([["I'm going to continue running a more in-depth scan, while I think of our next steps. Meet up with me at the bar when you're ready to help."]]))
   vn.na(_([[You see Icarus still playing with the cat drones, although it looks more like it is trying to eat them. Are those fangs?…]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zbh.zach.transition )
   vn.run()

   faction.modPlayer("Za'lek", zbh.fctmod.zbh04)
   player.pay( reward )
   zbh.log(fmt.f(_("You and Zach found a feral bioship wandering around {sys}. After Zach managed to create a simple program to communicate with it, you were able to get it to follow you back to {pnt}."),{sys=mainsys, pnt=mainpnt}))
   misn.finish(true)
end

local function sample_point ()
   local pos = player.pos()
   local v
   repeat
      v = vec2.newP( system.cur():radius(), rnd.angle()  )
   until v:dist2( pos ) > 3000
   return v
end

local feral, points
local function feral_map( first )
   system.markerClear()
   for k,v in ipairs(points) do
      local pos = v + vec2.newP( 300+1500*rnd.rnd(), rnd.angle() )
      system.markerAdd( pos, _("Detected Motion")  )
   end

   local msg
   if first then
      msg = _("Zach: I've detected some motion and sent you the coordinates!")
   else
      msg = _("Zach: It looks like the positions changed. I've updated your coordinates!")
   end

   pilot.comm( _("Sigma-13"), msg )
end

function enter ()
   if system.cur() ~= mainsys then
      if mem.state == 2 then
         lmisn.fail(fmt.f(_("You were supposed to lead the feral bioship to {pnt}!"),{pnt=mainpnt}))
      else
         lmisn.fail(_("You were supposed to follow the detected motion!"))
      end
      return
   end

   -- Get random points and sort so the bioship starts furthest away
   points = {}
   for i=1,3 do
      points[i] = sample_point()
   end
   local ppos = player.pos()
   table.sort( points, function( a, b ) return ppos:dist2(a) > ppos:dist2(b) end )

   feral = zbh.plt_icarus( points[1] )
   feral:rename( _("Feral Bioship") )
   feral:setInvincible(true)
   feral:control(true)
   feral:stealth()

   hook.pilot( feral, "discovered", "feral_discovered" )
   hook.pilot( feral, "idle", "feral_idle" )

   hook.timer(  5, "spacewhale" )
   hook.timer(  7, "zach_msg", _("Zach: What the hell was that noise?") )
   hook.timer( 15, "zach_msg", _("Zach: One second, let me calibrate the instruments.") )
   hook.timer( 23, "zach_msg", _("Zach: OK, looks like we're ready to do a scan pulse.") )
   hook.timer( 30, "feral_move" )
end

function zach_msg( msg )
   pilot.comm( _("Sigma-13"), msg )
end

local sfx_spacewhale = {
   zbh.sfx.spacewhale1,
   zbh.sfx.spacewhale2
}
function spacewhale ()
   local sfx = sfx_spacewhale[ rnd.rnd(1,#sfx_spacewhale) ]
   -- Don't use velocity to avoid sound deformations
   luaspfx.sfx( feral:pos(), nil, sfx, { dist_ref=2500, dist_max=25e3 } )
   player.autonavReset(5)
end

function feral_idle ()
   if mem.state~=1 then return end

   hook.timer( 15, "feral_move" )
end

function feral_move ()
   if mem.state~=1 then return end

   local newpoints = {}
   for i=1,3 do
      newpoints[i] = sample_point()
   end
   local ppos = player.pos()
   table.sort( newpoints, function( a, b ) return ppos:dist2(a) > ppos:dist2(b) end )

   points = newpoints

   feral:taskClear()
   feral:moveto( points[1] )

   spacewhale()

   feral_map()
end

function feral_discovered ()
   mem.state = 2
   feral:taskClear()
   feral:brake()
   feral:face( player.pilot() )

   player.autonavAbort(_("You found something!"))
   cinema.on{ gui=true }
   camera.set( feral )

   -- Have to do this AFTER aborting autonav
   local pp = player.pilot()
   pp:control(true)
   pp:brake()
   pp:face(feral)

   system.markerClear()

   hook.pilot( feral, "hail", "feral_hail" )

   hook.timer(  3, "zach_msg", _("Zach: What the hell is that?") )
   hook.timer( 10, "zach_msg", _("Zach: Try opening a communication channel with it.") )
   hook.timer( 10, "feral_hailstart" )
end

local feral_canhail = false
function feral_hailstart ()
   feral_canhail = true
   cinema.off()
   camera.set()
end

function feral_hail ()
   if not feral_canhail then
      feral:comm( player.pilot(), _("No response") )
      player.commClose()
      return
   end

   vn.clear()
   vn.scene()
   local f = vn.newCharacter( zbh.vn_icarus{ pos="left"} )
   local z = vn.newCharacter( zbh.vn_zach{ pos="right", shader=love_shaders.hologram() } )
   local ai = tut.vn_shipai()
   f:rename(_("Feral Bioship"))
   vn.transition()
   vn.na(fmt.f(_("You open a joint communication channel with the ship in front of you and Zach back at {pnt}."),{pnt=mainpnt}))
   z(_([["Hey, this looks a lot like a Soromid Bioship. Ship AI, what is that?"]]))

   vn.appear( ai, "electric" )
   ai(fmt.f(_([[Your ship AI materializes infront of you.
"This appears to be a {shipname}. Although they are loosely based on a Soromid Reaver Bio-Fighter, they have gone back to a more wild biological state, rejecting most synthetic components. {shipname} are the smallest of what are commonly referred to as #oferal bioships#0. I advise caution with dealing with such ships as they lack any sort of ship AI."]]),{shipname=feral:ship()}))
   vn.disappear( ai, "electric" )
   z(_([["I see. However, this doesn't explain how the hell it got down here, There's no way it could have made the entire trip unnoticed."]]))
   vn.sfx( zbh.sfx.spacewhale1 )
   f(_("You can tell the ship is trying to convey something, but don't understand the meaning."))
   z(_([["Wait wait, this wouldn't be Icarus? Wouldn't it? That would explain lots of things. One second, I may be able to make use of these notes."]]))
   vn.sfx( zbh.sfx.spacewhale2 )
   f(_("The feral bioship once again lets out a stream of electromagnetic radiation that your ship AI translates as a sound."))
   z(_([["One second, I'm getting there…"
He starts muttering to himself.
"…do an Laplace transform, carry the s over, then compute the envelope…"]]))
   vn.sfxBingo()
   z(_([["I think I got it! Let us see now."
The communication software flickers a second as it reboots.]]))
   vn.sfx( zbh.sfx.spacewhale1 )
   f(_([[The feral bioship lets another cry and you can see whatever modification Zach made to the communication kick into action.
"Athaen9a. Ihnatsoeu. Xllaudtohennnoaehustoa."]]))
   z(_([["Needs some more adjustments, one second. You don't get to translate an unknown language from scratch notes every day. My minor in linguistics is finally paying off!"
The communication software once more flickers and reboots.]]))
   f(_([["Elders. Alone… Scared. Elders. Where."]]))
   z(fmt.f(_([["I guess we won't be able to do much more than this for now. Let me see if we can transmit something to get it to follow you back to {pnt}."]]),
      {pnt=mainpnt}))
   vn.sfx( zbh.sfx.spacewhale2 )
   z(_([[Zach hits some buttons and transmits something similar to what you heard coming from the feral bioship.
"Follow. Safe."]]))
   f(_([[Eventually, it seems like necessity overcomes fear, and the feral bioship begins to follow your ship.]]))
   z(fmt.f(_([["OK, it seems like it's following you. Try leading it back to {pnt}.]]),{pnt=mainpnt}))
   vn.done()
   vn.run()

   feral:taskClear()
   feral:follow( player.pilot() )
   misn.osdActive(2)
   mem.state = 2
   misn.markerAdd(mainsys)
   player.commClose()
end
