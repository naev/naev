--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 9">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Research Post Sigma-13</spob>
 <location>Bar</location>
 <done>Za'lek Black Hole 8</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 09

   Have to find the wormhole, go through it, and have an encounter with feral bioships
   1. Go to insys and find  wormhole (cutscene)
   2. Jump into outsys, have cutscene with feral bioships
   3. After some criteria, Icarus jumps in and brings peace
   4. Icarus leaves and the player goes back to sigma-13
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local fleet = require "fleet"
local cinema = require "cinema"

local reward = zbh.rewards.zbh09

local inwormhole, insys = spob.getS( "Wormhole NGC-13674" )
local outwormhole, outsys = spob.getS( "Wormhole NGC-1931" )
local mainpnt, mainsys = spob.getS("Research Post Sigma-13")

local title = _("Black Hole Mystery")

function create ()
   if not misn.claim( {mainsys, insys, outsys} ) then
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
   vn.na(_([[You once again approach Zach at the bar.]]))
   z(fmt.f(_([["Hey. I've finally set up the new sensors you brought back. The station was running on the damaged back-up sensor system that I patched up to have some minimal functionality. The new sensors basically change everything, and are a strict upgrade to the standard functionality in these research posts. Anyway, I was trying out the sensors and looking at the readings of the black hole, when I noticed there seems to be an anomalous amount of tachyon readings nearby. I haven't seen anything like this before, and I think we should go check it out. Will you help me investigate the signal? "]]),{}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(p_("Zach", [["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   z(fmt.f(_([["Great! The anomaly has been detected in the nearby system of {sys}, It seems to be almost like some sort of ship jumping signal, but unlike jump signals, it's much more… how do I say this… consistent? It seems to just flow constantly like a river or something like that. I've never seen anything like this."]]),
      {sys=insys}))

   -- Change text a bit depending if known
   vn.func( function ()
      if not inwormhole:known() then
         vn.jump("unknown")
      end
   end )
   vn.menu{
      {_("Tell them about the wormhole"),"known"},
      {_("Stay silent"),"unknown"},
   }

   vn.label("known")
   z(_([["Wait, you knew about this incredible space phenomena outside our doorstep already!? Why didn't you tell me about it! We should have published a peer-reviewed paper on this instead of all the shenanigans we did! Oh well, I still want to see it with my own eyes."]]))
   vn.func( function ()
      mem.wormholeknown = true
   end )
   vn.jump("cont")

   vn.label("unknown")
   z(fmt.f(_([["I've run some standard tests and tried to match it with existing known space objects, but it doesn't seem to be anything like them. We could be on the edge of an amazing discovery! I can imagine it now, the Xiao-{name} space phenomena! I might even get tenure!"]]),
      {name=player.name()}))

   vn.label("cont")
   z(_([["Normally I would stay behind and monitor, but I'm a bit worried about my safety here and will accompany you this time. It's not like I want to see what's out there with my own eyes…
OK, maybe a little. Let's get going!"]]))

   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( title )
   misn.setReward(reward)
   misn.setDesc(fmt.f(_("Investigate the mysterious signal coming from the {sys} system with Zach."),{sys=insys}))

   local c = commodity.new( N_("Zach"), N_("A Za'lek scientist.") )
   misn.cargoAdd(c, 0)

   mem.mrk = misn.markerAdd( insys )
   mem.state = 1

   misn.osdCreate( title, {
      fmt.f(_("Go track down the signal in the {sys} system"), {sys=insys})
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==2 and spob.cur() == mainpnt then

      vn.clear()
      vn.scene()
      local z = vn.newCharacter( zbh.vn_zach() )
      vn.transition( zbh.zach.transition )
      vn.na(_("Zach remains silent as you land, completely engrossed in thought. You make your ship replay the land completed announcement to get him out of his stupor."))
      z(_([[He seems fairly concentrated on something, as he stumbles out of the ship without really paying attention where he is going, you can hear him mumble something to you.
"…meet me up at the bar later. I have to think…"]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zbh.zach.transition )
      vn.run()

      faction.modPlayer("Za'lek", zbh.fctmod.zbh09)
      player.pay( reward )
      zbh.log(_("You travelled through a wormhole with Zach and met a family of feral bioships. After a brief and intense exchange, Icarus came to make peace and decided to return to their family."))
      misn.finish(true)
   end
end

local function icarus_talk ()
   vn.clear()
   vn.scene()
   local i = vn.newCharacter( zbh.vn_icarus{ pos="left"} )
   local z = vn.newCharacter( zbh.vn_zach{ pos="right" } )
   vn.transition()
   vn.na(_([[Icarus opens up a communication channel with you. It seems like he figured out how to do that by himself.]]))
   i(_([["Peace. Friend."]]))
   z(_([["It looks like there's been a misunderstanding. Could this be from where Icarus came from? I have no idea where we are, but the rough coordinates seem to indicate we are near Soromid space."]]))
   i(_([["Elders. Ahaeosrc."
The translation system struggles with what Icarus is saying.]]))
   z(_([["Mmmm, this seems to be a word that wasn't in the original notes. I'm not sure how we're going to be able to translate it."]]))
   i(_([["Ahaeosrc. Zach."]]))
   z(_([["Wait, what? How did my name get picked up in the translation system?… Does this mean that…"
He goes silent, lost in thought.]]))
   i(_([["Leave. Elders."
Icarus slightly motions to the other feral bioships. Does this mean he won't be coming back?]]))
   vn.menu{
      {_("Try to convince Icarus to stay."),"stay"},
      {_("Wave goodbye to Icarus"),"wave"},
   }

   vn.label("stay")
   i(_([[Icarus seems to shake its head. Where did it learn to do that?
"Elders. Leave."]]))
   vn.jump("cont")

   vn.label("wave")
   i(_([[Icarus seems to do a motion that can only be interpreted as waving back at you.]]))
   vn.jump("cont")

   vn.label("cont")
   vn.na(_([[Icarus turns around to leave when suddenly Zach seems to break out of his trance.]]))
   z(_([["What did they tell you Icarus? What happened out there!?]]))
   i(_([[Icarus sends a last message before joining the pack of feral ships.
"Dark. Wet. Eye."]]))
   vn.disappear( i )
   z(_([[Zach slumps down as if all the energy left his body leaving a shrivelled husk behind. In a small voice he mumbles
"What could this mean…"]]))
   vn.na(fmt.f(_([[You should probably get Zach back to {pnt}.]]),{pnt=mainsys}))
   vn.done()
   vn.run()
end

local pack
function enter ()
   if mem.state==1 and system.cur() == mainsys then
      local feral = zbh.plt_icarus( mainpnt:pos() + vec2.newP(300,rnd.angle()) )
      feral:setFriendly(true)
      feral:setInvincible(true)
      feral:control(true)
      feral:follow( player.pilot() )
      hook.pilot( feral, "hail", "feral_hail" )

   elseif mem.state==2 and system.cur() == mainsys then
      local p = pilot.add( "Za'lek Scout Drone", zbh.evilpi(), mainpnt:pos(), nil, {ai="baddie"} )
      p:intrinsicSet( "ew_hide", 100 ) -- Easier to spot
      p:control(true)
      p:stealth()
      hook.pilot( p, "discovered", "scout_discovered" )

   elseif mem.state==1 and system.cur() == insys then
      player.landAllow( false, _("Zach is analyzing the wormhole signal.") )

      if mem.wormholeknown then
         system.markerAdd( inwormhole:pos(), _("Wormhole") )
      else
         system.markerAdd( inwormhole:pos(), _("Suspicious Signal") )
      end

      hook.timer( 5, "zach_say", _("Weird that Icarus didn't follow us through the jump…") )
      hook.timer( 12, "zach_say", _("I've marked the location on your system map.") )
      hook.timer( 1, "heartbeat_wormhole" )

   elseif mem.state==1 and system.cur() == outsys then
      pilot.clear()
      pilot.toggleSpawn(false)

      player.landAllow( false, _("The wormhole seems to have become too weak to go through.") )

      --local j = jump.get( outsys, "NGC-4771" )
      local pp = player.pilot()
      pp:setNoJump(true)

      -- nohinohi, taitamariki, kauweke,
      local ships
      if pp:ship():size() >= 5 then
         ships = { "Kauweke", "Kauweke", "Taitamariki", "Taitamariki", "Taitamariki", "Taitamariki" }
      else
         ships = { "Kauweke", "Taitamariki", "Taitamariki", "Taitamariki" }
      end
      local pos = vec2.new( -6000, 3000 ) -- Halfway towards NGC-4771
      pack = fleet.add( 1, ships, zbh.feralbioship(), pos )
      for k,p in ipairs(pack) do
         p:rename(_("Feral Bioship"))
         p:setNoDeath()
         p:setInvincible() -- in case the player does something silly like preemptively shoot torpedoes
         hook.pilot( p, "hail", "feral_hail" )
      end
      local l = pack[1]
      l:control()
      l:brake()

      misn.markerRm( mem.mrk )

      hook.timer( 5, "zach_say", _("Hot damn, that was weird. Ugh, I feel sick.") )
      hook.timer( 12, "zach_say", _("I'm getting some ship readings. Wait, what is that?") )
      hook.timer( 18, "heartbeat_ferals" )

   elseif mem.state == 2 and system.cur() == mainsys then
      hook.timer( 15, "zach_say", _("It's a bit quiet without Icarus around any more…") )

   end
end

function scout_discovered( scout )
   player.autonavReset(3)
   scout:taskClear()
   scout:hyperspace( system.get("NGC-23") )
end

function zach_say( msg )
   player.autonavReset( 3 )
   player.msg(fmt.f(_([[Zach: "{msg}"]]),{msg=msg}),true)
end

local zach_msg_known = {
   _("It's… mesmerizing!"),
   _("It's safe right? Maybe we should send a drone first."),
   _("I guess we might as well try it. Experimental physics at its finest."),
   _("Try to go through it carefully!"),
}
local zach_msg_unknown = {
   _("Damn, that the hell is that?" ),
   _("One second, let me analyze the data." ),
   _("I can't make much sense out of this, but…"),
   _("It seems like some sort of space-time discontinuity…"),
   _("It doesn't seem dangerous, at least the readings seem strangely fine."),
   _("I guess we might as well try it. Experimental physics at its finest."),
   _("Try to go through it carefully!"),
}

local wstate = 0
function heartbeat_wormhole ()
   local msglist = (mem.wormholeknown and zach_msg_known) or zach_msg_unknown
   local pp = player.pilot()
   local d = pp:pos():dist( inwormhole:pos() )
   if wstate==0 and d < 5000 then
      wstate = 1
      zach_say( _("The sensor readings are off the charts!") )
   elseif wstate==1 and d < 2500 then
      wstate = 2
      zach_say( _("I think I see something!") )
   elseif wstate==2 and d < 500 then
      wstate = 3
      zach_say( msglist[wstate-2] )
   elseif wstate >= 3 then
      wstate = wstate+1
      local msg = msglist[ wstate-2 ]
      zach_say( msg )
      if msglist[ wstate-1 ]==nil then -- Was the last message
         player.landAllow()
         misn.osdCreate( title, { _("Go through the wormhole") } )
         return
      end
   end

   -- Normalize time so it's independent of ship
   hook.timer( 5 / player.dt_mod(), "heartbeat_wormhole" )
end

local fstate = 0
local waitzone, icaruszone, fightstart, icarus
function heartbeat_ferals ()
   local nexttime = 5
   local l = pack[1]

   if fstate == 0 then
      cinema.on()
      camera.set( l )
      l:taskClear()
      l:moveto( l:pos() + (player.pos()-l:pos()):normalize() * 1000 )
      nexttime = 10
      fstate = 1

   elseif fstate == 1 then
      nexttime = 10
      fstate = 2

   elseif fstate == 2 then
      cinema.off()
      camera.set()
      nexttime = 3
      fstate = 3

   elseif fstate == 3 then
      zach_say( _("What are those ships over there? They look a lot like Icarus!") )
      l:setHilight()
      l:setVisplayer()
      fstate = 4

   elseif fstate == 4 then
      zach_say( _("We should go greet them.") )
      misn.osdCreate( title, { _("Get near the feral bioships") } )
      fstate = 5

   elseif fstate == 5 and player.pos():dist( l:pos() ) < 3000 then

      local pp = player.pilot()
      cinema.on()
      camera.set( (l:pos()+pp:pos())/2 )
      camera.setZoom( 3 )

      zbh.sfx.spacewhale1:play()
      l:taskClear()
      l:brake()
      l:face( pp )
      pp:face( l )

      local lp = l:pos()
      for k,p in ipairs(pack) do
         if k > 1 then
            p:control(true)
            p:moveto( lp + vec2.newP( 200+300*rnd.rnd(), rnd.angle() ) )
            p:face( pp )
         end
      end
      fstate = 6

   elseif fstate == 6 then
      zach_say( _("Look at the size of that thing!") )
      fstate = 7

   elseif fstate == 7 then
      zach_say( _("Wait, it looks like it's picking up some signal on us.") )
      fstate = 8

   elseif fstate == 8 then
      local pp = player.pilot()
      zbh.sfx.spacewhale2:play()
      if pp:ship():tags().bioship then
         l:broadcast(_("Son. Revenge. Imposter. Die."), true)
      else
         l:broadcast(_("Son. Revenge. Die."), true)
      end
      misn.osdCreate( title, { _("Survive!") } )

      cinema.off()
      camera.set()
      camera.setZoom()

      zach_say( p_("Zach", "Watch out!") )
      l:control(false)
      pp:control(false)

      -- Where the defeated ships will wait
      waitzone = l:pos() + (l:pos() - pp:pos()):normalize()*1000
      fightstart = naev.ticksGame()

      for k,p in ipairs(pack) do
         p:setInvincible(false)
         p:setHostile(true)
         p:control(false)
      end
      fstate = 9

   elseif fstate == 9 then
      local defeated, total = 0, 0
      -- Check ending criteria
      for k,p in ipairs(pack) do
         local ps = p:ship():size()
         if not p:flags("invincible") then
            local pa = p:health()
            if pa < 30 then
               p:setInvincible(true)
               p:setHostile(false)
               p:setInvisible(true)
               p:control()
               p:moveto( waitzone + vec2.newP( 500*rnd.rnd(), rnd.angle() ) )
               defeated = defeated + ps
            end
         else
            defeated = defeated + ps
         end
         total = total + ps
      end
      nexttime = 0.1

      -- End criteria
      if (naev.ticksGame() - fightstart > 90) or (defeated > 0.5*total) or l:flags("invincible") then
         fstate = 10
      end

   elseif fstate == 10 then
      zbh.sfx.spacewhale1:play()
      local pp = player.pilot()

      zach_say(_("Wait, is that Icarus? Run to him!"))

      icarus = zbh.plt_icarus( outwormhole )
      icarus:setInvincible(true)
      icarus:setHilight(true)
      icarus:setVisplayer(true)
      icarus:setFriendly(true)
      icarus:control()
      icarus:moveto( pp:pos() )
      hook.pilot( icarus, "hail", "feral_hail" )

      misn.osdCreate( title, { _("Go to Icarus!") } )

      fstate = 11

   elseif fstate == 11 and icarus:pos():dist( player.pos() ) < 3000 then
      local pp = player.pilot()
      player.landAllow()
      pp:setNoJump(false)

      pp:control()
      pp:brake()
      pp:face( icarus )

      camera.set( icarus )
      camera.setZoom( 2 )

      for k,p in ipairs(pack) do
         if not p:flags("invincible") then
            p:setInvincible(true)
            p:setHostile(false)
            p:setInvisible(true)
            p:control()
            p:moveto( l:pos() + vec2.newP( 500*rnd.rnd(), rnd.angle() ) )
         end
      end

      l:taskClear()
      l:brake()

      icaruszone = l:pos() + (icarus:pos() - l:pos()):normalize()*1000

      icarus:taskClear()
      icarus:moveto( icaruszone )
      fstate = 12

   elseif fstate == 12 then
      if icarus:pos():dist( icaruszone ) < 500 then
         icarus:taskClear()
         icarus:brake()
         icarus:face( l )

         for k,p in ipairs(pack) do
            if p:exists() then
               p:setInvisible(false)
               p:taskClear()
               p:brake()
               p:face( icarus )
            end
         end

         nexttime = 3
         fstate = 13

      else
         nexttime = 0.1
      end

   elseif fstate == 13 then
      zbh.sfx.spacewhale1:play()
      fstate = 14

   elseif fstate == 14 then
      camera.set( l )
      zbh.sfx.spacewhale2:play()
      fstate = 15

   elseif fstate == 15 then
      camera.set( icarus )
      zbh.sfx.spacewhale1:play()
      fstate = 16

   elseif fstate == 16 then

      icarus_talk ()

      icarus:setLeader( l )
      icarus:control(false)
      icarus:setHilight(false)
      for k,p in ipairs(pack) do
         p:control( false )
      end
      l:control()
      l:hyperspace()
      l:setHilight(false)

      local pp = player.pilot()
      pp:control(false)
      camera.set( pp )
      camera.setZoom()

      misn.osdCreate( title, { fmt.f(_("Return to {pnt} ({sys} system)"),{pnt=mainpnt,sys=mainsys}) } )
      misn.markerAdd( mainpnt )

      mem.state = 2
      return

   end

   hook.timer( nexttime / player.dt_mod(), "heartbeat_ferals" )
end

local sfx_spacewhale = {
   zbh.sfx.spacewhale1,
   zbh.sfx.spacewhale2,
}
function feral_hail ()
   local sfx = sfx_spacewhale[ rnd.rnd(1,#sfx_spacewhale) ]
   sfx:play()
   player.commClose()
end
