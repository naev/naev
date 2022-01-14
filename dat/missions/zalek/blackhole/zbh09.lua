--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 9">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <spob>Research Post Sigma-13</spob>
  <location>Bar</location>
  <done>Za'lek Black Hole 8</done>
 </avail>
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

-- luacheck: globals land enter zach_say heartbeat_wormhole heartbeat_ferals feral_hail (Hook functions passed by name)

local reward = zbh.rewards.zbh09

local inwormhole, insys = spob.getS( "Wormhole NGC-13674" )
local outwormhole, outsys = spob.getS( "Wormhole NGC-1931" )
local mainpnt, mainsys = spob.getS("Research Post Sigma-13")

local title = _("Black Hole Mystery")

function create ()
   misn.finish()
   if not misn.claim( {mainsys, insys, outsys} ) then
      misn.finish()
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

local wormholeknown = false
function accept ()
   wormholeknown = inwormhole:isKnown()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[]]))
   z(fmt.f(_([[""]])
      {}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(_([["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   z(_([[""]]))

   -- Change text a bit depending if known
   --[[
   if inwormhole:isKnown() then
   else
   end
   --]]

   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( title )
   misn.setReward( fmt.credits(reward) )
   misn.setDesc( _("") )

   mem.mrk = misn.markerAdd( insys )
   mem.state = 1

   misn.osdCreate( title, {
      fmt.f(_("Go track down the signal in the {sys} system"), {sys=insys})
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==3 and spob.cur() == mainpnt then

      vn.clear()
      vn.scene()
      local z = vn.newCharacter( zbh.vn_zach() )
      vn.transition( zbh.zach.transition )
      vn.na(_(""))
      z(_([[""]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zbh.zach.transition )
      vn.run()

      faction.modPlayer("Za'lek", zbh.fctmod.zbh09)
      player.pay( reward )
      zbh.log(_(""))
      misn.finish(true)
   end
end

local pack
function enter ()
   if mem.state==1 and system.cur() == insys then
      player.allowLand( false, _("Zach is analyzing the wormhole signal.") )

      if wormholeknown then
         system.mrkAdd( inwormhole:pos(), _("Wormhole") )
      else
         system.mrkAdd( inwormhole:pos(), _("Suspicious Signal") )
      end

      hook.timer( 5, zach_say, _("Weird that Icarus didn't follow us through the jump…") )
      hook.timer( 12, zach_say, _("I've marked the location on your system map.") )
      hook.timer( 1, "heartbeat_wormhole" )

   elseif mem.state==1 and system.cur() == outsys then
      pilot.clear()
      pilot.toggleSpawn(false)

      player.allowLand( false, _("The wormhole seems to have become too weak to go through.") )

      --local j = jump.get( outsys, "NGC-4771" )
      local pp = player.pilot()

      -- nohinohi, taitamariki, kauweke,
      local ships
      if pp:ship():size() >= 5 then
         ships = { "Kauweke", "Kauweke", "Taitamariki", "Taitamariki", "Taitamariki", "Taitamariki" }
      else
         ships = { "Kauweke", "Taitamariki", "Taitamariki", "Taitamariki" }
      end
      local pos = vec2.pos( -6000, 3000 ) -- Halfway towards NGC-4771
      pack = fleet.add( 1, ships, zbh.feralbioships(), pos )
      for k,p in ipairs(pack) do
         p:rename(_("Feral Bioship"))
         p:setNoDeath()
         p:setInvincible() -- in case the player does something silly like preemptively shoot torpedoes
         hook.pilot( p, "hail", feral_hail )
      end
      local l = pack[1]
      l:control()
      l:brake()

      hook.timer( 5, zach_say, _("Hot damn that was weird. Ugh, I feel sick.") )
      hook.timer( 12, zach_say, _("I'm getting some ship readings. Whait, what is that?") )
      hook.timer( 18, "heartbeat_ferals" )

   --elseif system.cur() == mainsys then

   end
end

function zach_say( msg )
   player.autonavReset( 3 )
   player.pilot():comm(fmt.f(_([[Zach: "{msg}"]]),{msg=msg}))
end

local zach_msg_known = {
   _("It's… mesmerizing!"),
   _("It's safe right? Maybe we should send a drone first."),
   _("I guess we might as well try it. Experimental physics at it's finest."),
   _("Try to go through it carefully!"),
}
local zach_msg_unknown = {
   _("Damn, that the hell is that?" ),
   _("One second, let me analyze the data." ),
   _("I can't make much sense out of this, but…"),
   _("It seems like some sort of space-time discontinuity…"),
   _("It doesn't seem dangerous, at least the readings seem strangely fine."),
   _("I guess we might as well try it. Experimental physics at it's finest."),
   _("Try to go through it carefully!"),
}

local wstate = 0
function heartbeat_wormhole ()
   local msglist = (wormholeknown and zach_msg_known) or zach_msg_unknown
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
   else
      wstate = wstate+1
      local msg = msglist[ wstate-2 ]
      if msg==nil then -- Out of messages
         player.allowLand()
         return
      end
      zach_say( msg )
   end

   -- Normalize time so it's independent of ship
   hook.timer( 5 / player.dt_mod(), "heartbeat_wormhole" )
end

local fstate = 0
local waitzone, fightstart, icarus
function heartbeat_ferals ()
   local nexttime = 5
   local l = pack[1]

   if fstate == 0 then
      player.cinematics( true )
      camera.set( l )
      l:taskClear()
      l:moveto( l:pos() + (player.pos()-l:pos()):normalize() * 1000 )
      nexttime = 10
      fstate = 1

   elseif fstate == 1 then
      nexttime = 10
      fstate = 2

   elseif fstate == 2 then
      player.cinematics( false )
      camera.set()
      nexttime = 3
      fstate = 3

   elseif fstate == 3 then
      zach_say( _("What are those ships over there? They look a lot like Icarus!") )
      l:setHighlight()
      l:setVisplayer()
      fstate = 4

   elseif fstate == 4 then
      zach_say( _("We should go greet them.") )
      misn.osdCreate( title, { _("Get near the feral bioships") } )
      fstate = 5

   elseif fstate == 5 and player.pos():dist( l:pos() ) < 3000 then

      zbh.sfx.spacewhale1:play()
      local pp = player.pilot()
      l:taskClear()
      l:brake()
      l:face( pp )
      pp:control()
      pp:brake()
      pp:face( l )

      player.cinematics( true )
      camera.set( (l:pos()+pp:pos())/2 )
      camera.setZoom( 3 )
      fstate = 6

   elseif fstate == 6 then
      zach_say( _("Look at the size of that thing!") )
      fstate = 7

   elseif fstate == 7 then
      zach_say( _("Wait, it looks like it's picking up some signal on us.") )
      fstate = 8

   elseif fstate == 8 then
      zbh.sfx.spacewhale2:play()
      l:broadcast(_("Son. Revenge. Die."))
      misn.osdCreate( title, { _("Survive!") } )

      zach_say( _("Watch out!") )
      local pp = player.pilot()
      l:control(false)
      pp:control(false)
      player.cinematics( false )
      camera.set()
      camera.setZoom()

      -- Where the defeated ships will wait
      waitzone = l:pos() + (l:pos() - pp:pos()):normalize()*1000
      fightstart = naev.ticksGame()

      for k,p in ipairs(pack) do
         p:setInvincible(false)
         p:setHostile(true)
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
      icarus:setHighlight(true)
      icarus:setFriendly(true)
      icarus:control()
      icarus:moveto( pp:pos() )
      hook.pilot( icarus, "hail", feral_hail )

      misn.osdCreate( title, { _("Go to Icarus!") } )

      fstate = 11

   elseif fstate == 11 and icarus:pos():dist( player.pos() ) < 3000 then

      -- It's over
      player.allowLand()

   end

   hook.timer( nexttime / player.dt_mod(), "heartbeat_ferals" )
end

local sfx_spacewhale = {
   zbh.sfx.spacewhale1,
   zbh.sfx.spacewhale2
}
function feral_hail ()
   local sfx = sfx_spacewhale[ rnd.rnd(1,#sfx_spacewhale) ]
   sfx:play()
   player.commClose()
end
