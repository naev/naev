--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 3">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <planet>Research Post Sigma-13</planet>
  <location>Bar</location>
  <done>Za'lek Black Hole 2</done>
 </avail>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 03

   Ward off enemy attack from evil PI
]]--
local vn = require "vn"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local fleet = require "fleet"

-- luacheck: globals land enter heartbeat cutscene_done welcome_back (Hook functions passed by name)

local reward = zbh.rewards.zbh03

local mainpnt, mainsys = planet.getS("Research Post Sigma-13")
local jumpsys = system.get("NGC-23")

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
   vn.na(_([[]]))
   z(_([[""]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(_([["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(_([[]]))
   vn.func( function () accepted = true end )
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Black Hole Scouting") )
   misn.setReward( fmt.reward(reward) )
   misn.setDesc( _("") )

   mem.mrk = misn.markerAdd( mem.destpnt )
   mem.state = 1

   misn.osdCreate( _("Repairing Sigma-13"), {
      fmt.f(_("Scout around ({sys} system)"), {sys=mainsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"), {pnt=mainpnt, sys=mainsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==2 and planet.cur() == mainpnt then

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

      zbh.unidiff( "sigma13_fixed2" )

      faction.modPlayer("Za'lek", zbh.fctmod.zbh02)
      player.pay( reward )
      zbh.log(_(""))
      misn.finish(true)
   end
end

-- Set up seeing the feral bioship on the way back
function enter ()
   if system.cur() ~= mainsys then
      return
   end

   hook.timer( 90, "heartbeat" )
end

local badguys
local heartbeat_state = 0
function heartbeat ()
   heartbeat_state = heartbeat_state + 1
   if heartbeat_state == 1 then
      pilot.broadcast( _("Sigma-13"), fmt.f(_("Zach: I've detected some incoming ships from {sys}!"), {sys=jumpsys}) )

      local fbadguys = faction.dynAdd( "Za'lek", "zbh_baddies", _("Za'lek"), {ai="baddiepos"} )

      local ships = {"Za'lek Sting", "Za'lek Heavy Drone", "Za'lek Light Drone"}
      if player.pilot():size() >= 5 then
         table.insert( ships, "Za'lek Demon", 1 )
      end
      badguys = fleet.add( 1, ships, fbadguys, jump.get(system.cur(),jumpsys) )
      for k,p in ipairs(badguys) do
         p:setHostile(true)
         local m = p:memory()
         m.guardpos = mainpnt:pos()
      end
      local l = badguys[1]
      l:setVisplayer(true)
      l:setHilight(true)

   --elseif heartbeat_state == 2 then
   end

   if heartbeat_state < 5 then
      hook.timer( 3, "heartbeat" )
   end
end
