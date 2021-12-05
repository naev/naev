--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 2">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <planet>Research Post Sigma-13</planet>
  <location>Bar</location>
  <done>Za'lek Black Hole 1</done>
 </avail>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 02

   Player has to bring back supplies to Zach
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local lmisn = require "lmisn"

-- luacheck: globals land enter heartbeat (Hook functions passed by name)

local reward = zbh.rewards.zbh02
--local cargo_name = _("Repair Supplies and Assistants")
--local cargo_amount = 50 -- Amount of cargo to take

local retpnt, retsys = planet.getS("Research Post Sigma-13")

function create ()
   misn.finish()
   if not misn.claim( retsys ) then
      misn.finish()
   end

   mem.destpnt, mem.destsys = lmisn.getRandomPlanetAtDistance( system.cur(), 3, 8, "Za'lek", false, function( p )
      return p:tags().industrial
   end )
   if not mem.destpnt then
      misn.finish()
      return
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[You approach Zach who seems a bit tired out.]]))
   z(fmt.f(_([[]]),
      {pnt=mem.destpnt, sys=mem.destsys, credits=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(_([["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(_([[""]]))
   vn.func( function () accepted = true end )
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Black Hole Research") )
   misn.setReward( fmt.reward(reward) )
   misn.setDesc( fmt.f(_("Pick up the necessary supplies at {pnt} in the {sys} system and bring them back to Zach at {retpnt}."),
      {pnt=mem.destpnt, sys=mem.destsys, retpnt=retpnt} ))

   misn.markerAdd( mem.destpnt )
   mem.state = 1

   misn.osdCreate( _("Black Hole Research"), {
      fmt.f(_("Pick up cargo at {pnt} ({sys} system)"), {pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"), {pnt=retpnt, sys=retsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==1 and planet.cur() == mem.destpnt then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.run()

      mem.state = 2

   elseif mem.state==2 and planet.cur() == retpnt then

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

      zbh.unidiff( "sigma13_fixed1" )

      faction.modPlayer("Za'lek", zbh.fctmod.zbh02)
      player.pay( reward )
      zbh.log(_(""))
      misn.finish(true)
   end
end

local firsttime = true
function enter ()
   if system.cur() ~= retsys or not firsttime then
      return
   end

   firsttime = false
   hook.timer( 3, "heartbeat" )
end

function heartbeat ()
   --local pp = player.pilot()
   --local d = mem.destpnt:pos():dist( pp:pos() )
   hook.timer( 3, "heartbeat" )
end
