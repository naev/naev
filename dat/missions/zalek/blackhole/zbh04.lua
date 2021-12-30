--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 4">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <spob>Research Post Sigma-13</spob>
  <location>Bar</location>
  <done>Za'lek Black Hole 3</done>
 </avail>
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

-- luacheck: globals land enter feral_idle feral_move feral_discovered zach_msg spacewhale (Hook functions passed by name)

local reward = zbh.rewards.zbh04

local mainpnt, mainsys = spob.getS("Research Post Sigma-13")

function create ()
   misn.finish()
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
   vn.na(_([[You find Zach deep in thought.]]))
   z(fmt.f(_([[""]]),{}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(_([["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(fmt.f(_([[""]]),{}))
   vn.func( function () accepted = true end )
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Black Hole Mystery") )
   misn.setReward( fmt.reward(reward) )
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
   if mem.state~=3 or spob.cur() ~= mainpnt then
      lmisn.fail(_("You were supposed to follow the signals!"))
      return
   end

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[]]))
   z(_([[""]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zbh.zach.transition )
   vn.run()

   faction.modPlayer("Za'lek", zbh.fctmod.zbh04)
   player.pay( reward )
   zbh.log(fmt.f(_("{sys} {pnt}"),{sys=mainsys,pnt=mainpnt}))
   misn.finish(true)
end

local function sample_point ()
   local pos = player.pos()
   local v
   repeat
      v = vec2.newp( system.cur():radius(), rnd.angle()  )
   until v:dist2( pos ) > 3000
   return v
end

local feral, points
local function feral_map( first )
   system.mrkClear()
   for k,v in ipairs(points) do
      local pos = v + vec2.newP( 300+1500*rnd.rnd(), rnd.angle() )
      system.mrkAdd( pos, _("Detected Motion")  )
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
      lmisn.fail(_("You were supposed to follow the signals!"))
      return
   end

   -- Get random points and sort so the bioship starts furthest away
   points = {}
   for i=1,3 do
      points[i] = sample_point()
   end
   local ppos = player.pos()
   table.sort( points, function( a, b ) return ppos:dist2(a) > ppos:dist2(b) end )

   feral = zbh.feralkid( points[1] )
   feral:setInvincible(true)
   feral:control(true)
   feral:stealth()

   hook.pilot( feral, "discovered", "feral_discovered" )
   hook.pilot( feral, "idel", "feral_idle" )

   luaspfx.init()

   hook.timer(  5, "spacewhale" )
   hook.timer(  7, "zach_msg", _("Zach: What the hell was that noise?") )
   hook.timer( 15, "zach_msg", _("Zach: One second, let me calibrate the instruments.") )
   hook.timer( 23, "zach_msg", _("Zach: OK, looks like we're ready to do a scan pulse.") )
   hook.timer( 30, "feral_move" )
end

function zach_msg( msg )
   pilot.comm( _("Sigma-13"), msg )
end

function spacewhale ()
   local sfx = zbh.sfx[ rnd.rnd(1,#zbh.sfx) ]
   luaspfx.addfg( luaspfx.effects.sfx{ sfx=sfx, dist_ref=2500, dist_max=25e3 }, 10, feral:pos() )
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

   hook.timer(  3, "zach_msg", _("Zach: What the hell is that?") )
   hook.timer( 10, "zach_msg", _("Zach: ") )
end
