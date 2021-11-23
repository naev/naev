--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Particle Physics 3">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <planet>Katar I</planet>
  <location>Bar</location>
  <done>Za'lek Particle Physics 2</done>
 </avail>
 <notes>
  <campaign>Za'lek Particle Physics</campaign>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Za'lek Particle Physics 03

   Player has to go check a droid malfunction. There will be two drones one
   disable and one hostile. Player must destroy hostile and "hack" the disabled
   one to get the black box.
]]--
local vn = require "vn"
local fmt = require "format"
local zpp = require "common.zalek_physics"
local sokoban = require "minigames.sokoban"

-- luacheck: globals land enter drone_board (Hook functions passed by name)

local reward = zpp.rewards.zpp03
local mainpnt, mainsys = planet.getS("Katar I")

function create ()
   vn.finish(true)
   misn.setNPC( _("Noona"), zpp.noona.portrait, zpp.noona.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_([[]]))
   n(_([[""]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   n(_([[""]]))
   vn.done( zpp.noona.transition )

   vn.label("accept")
   n(_([[""]]))
   vn.func( function () accepted = true end )
   vn.done( zpp.noona.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Particle Physics") )
   misn.setReward( fmt.reward(reward) )
   misn.setDesc(fmt.f(_("Investigate the issue with the drones near the particle physics testing site at {sys}."),
      {sys=mainsys}))

   mem.mrk = misn.markerAdd( system.cur() )

   misn.osdCreate( _("Particle Physics"), {
      _("Investigate the drones"),
      _("Return to Katar I"),
   } )

   mem.state = 1

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==1 or planet.cur() ~= mainpnt then
      return
   end

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_([[]]))
   n(_([[""]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zpp.noona.transition )
   vn.run()

   player.pay( reward )
   zpp.log(_(""))
   misn.finish(true)
end

function enter ()
   if mem.state~=1 or system.cur() ~= mainsys then
      return
   end

   -- Temp faction
   local fdrone = faction.dynAdd( "Za'lek", "haywire_drone", _("Za'lek") )

   -- Spawn the drones
   -- TODO better location once testing center object is created
   local pkatar = planet.get("Katar"):pos()
   local pkatari = mainpnt:pos()
   local pos = (pkatar - pkatari)*3 + pkatar
   local pdis = pilot.add( "Za'lek Light Drone", fdrone, pos, _("Haywire Drone") )
   pdis:setFriendly(true)
   pdis:setInvincible(true)
   pdis:setActiveBoard(true)
   pdis:control(true)
   pdis:brake()
   hook.pilot( pdis, "disable", "drone_board" )

   pos = pos + vec2.newP( 100*rnd.rnd(), rnd.angle() )
   local phost = pilot.add( "Za'lek Heavy Drone", fdrone, pos, _("Haywire Drone") )
   phost:setHostile(true)
end

local hacked = false
function drone_board ()
   if hacked then
      player.msg(_("The drone's black box has already been extracted."))
      return
   end

   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(_([[You access the drone control panel and jack into the black box.]]))

   sokoban.vn{ levels={4,5}, header="Drone Black Box"}
   vn.func( function ()
      if sokoban.completed() then
         mem.state = 2
         vn.jump("sokoban_done")
         hacked = true
         return
      end
      vn.jump("sokoban_fail")
   end )

   vn.label([[sokoban_done]])
   vn.na(_([[You manage recover the entire black box intact and load the information on your ship.]]))

   vn.label("sokoban_fail")
   vn.na(_([[You failed to access the black box.]]))

   vn.run()

   if hacked then
      local c = misn.cargoNew( N_("Drone Black Box"), N_("The recovered black box of a Za'lek drone.") )
      misn.cargoAdd(c, 0)
      mem.state = 2
   end

   player.unboard()
end
