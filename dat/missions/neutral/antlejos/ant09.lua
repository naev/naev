--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 9">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>3</priority>
  <chance>100</chance>
  <location>Bar</location>
  <spob>Antlejos V</spob>
  <cond>require('common.antlejos').unidiffLevel() &gt;= 9</cond>
  <done>Terraforming Antlejos 8</done>
 </avail>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Deal the final major blow get rid of the PUAAA mothership harassing Antlejos
--]]
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local ant = require "common.antlejos"
local lmisn = require "lmisn"

local reward = ant.rewards.ant09

local retpnt, retsys = spob.getS("Antlejos V")
local mainsys = system.get("Klintus")

-- luacheck: globals approaching enter land mothershipdeath mothershipboard (Hook functions passed by name)

function create ()
   misn.finish(false)
   if not misn.claim(mainsys) then misn.finish() end
   misn.setNPC( _("Verner"), ant.verner.portrait, ant.verner.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_([[TODO]]))
   v(_([["TODO"]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(_([[""]]))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end
   local title = _("Eliminate the PUAAA Mothership")

   misn.accept()
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("Eliminate the PUAAA mothership located in the {sys} system. You have been provided explosives to detonate the mothership if you are able to get close enough to it."),{sys=mainsys}))
   misn.setReward( fmt.credits(reward) )
   misn.osdCreate( title , {
      fmt.f(_("Go to {sys}"),{sys=mainsys}),
      _("Destroy the PUAAA mothership"),
      fmt.f(_("Return to {pnt} ({sys} system)"),{pnt=retpnt, sys=retsys}),
   })
   mem.mrk = misn.markerAdd( mainsys )
   mem.state = 0

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==2 and spob.cur() == retpnt then
      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      vn.na(_([[TODO]]))
      v(_([["TODO"]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      player.pay( reward )
      ant.log(fmt.f(_("You eliminated the PUAAA mothership bringing more security to {pnt}."),{pnt=retpnt}))
      misn.finish(true)
   end
end

local mothership
function enter ()
   if mem.state==2 then
      -- Nothing to do here
      return
   end
   -- Wrong system
   if mem.state==1 and system.cur() ~= mainsys then
      lmisn.fail(fmt.f(_("You were not supposed to leave {sys}!"),{sys=mainsys}))
      return
   end
   if system.cur() ~= mainsys then
      return
   end

   pilot.clear()
   pilot.toggleSpawn(false)

   mem.state = 1
   misn.osdActive(2)

   -- Main positions
   local pos_mothership = vec2.new( -4e3, 0 )

   -- Define the routes
   local route0 = {
      jump.get( mainsys, "Senara" ):pos(),
      jump.get( mainsys, "NGC-3219" ):pos(),
      jump.get( mainsys, "Antlejos" ):pos(),
      jump.get( mainsys, "Kobopos" ):pos(),
   }
   local route1 = treverse( route0 )
   local route2 = {
      jump.get( mainsys, "Senara" ):pos(),
      jump.get( mainsys, "NGC-3219" ):pos(),
      vec2.new( -14e3, 0 ),
   }
   local route3 = {
      vec2.new( -14e3,  6e3 ),
      vec2.new(  10e3,  6e3 ),
      vec2.new(  10e3, -6e3 ),
      vec2.new( -14e3, -6e3 ),
   }
   local route4 = {
      vec2.new( -10e3,    0 ),
      vec2.new(  -4e3,  4e3 ),
      vec2.new(   4e3,  4e3 ),
      vec2.new(   4e3, -4e3 ),
      vec2.new(  -4e3, -4e3 ),
   }
   local route5 = {
      vec2.new(  -14e3, 4e3 ),
      pos_mothership,
      vec2.new(  -14e3, -4e3 ),
   }
   local route6 = {
      vec2.new(  6e3, -14e3 ),
      vec2.new(  6e3,  10e3 ),
      vec2.new( -6e3,  10e3 ),
      vec2.new( -6e3, -14e3 ),
   }

   -- Initialize ship stuff
   local puaaa = ant.puaaa()

   mothership = pilot.add( "Zebra", puaaa, pos_mothership, _("Planet Saver"), {ai="baddiepos"} )
   mothership:setVisplayer(true)
   mothership:setHilight(true)
   mothership:control()
   mothership:brake()
   mothership:setHostile(true)
   -- Can't detect anything
   mothership:intrinsicSet( "ew_hide", -75 )
   mothership:intrinsicSet( "ew_detect", -1000 )
   -- Much more bulky than normal
   mothership:intrinsicSet( "shield", 500 )
   mothership:intrinsicSet( "armour", 1500 )
   mothership:intrinsicSet( "absorb", 30 )
   mothership:setActiveBoard(true) -- Can board them to blow them up
   hook.pilot( mothership, "death", "mothershipdeath" )
   hook.pilot( mothership, "board", "mothershipboard" )

   local function spawn_ship( s, pos )
      local p = pilot.add( s, puaaa, pos )
      p:setHostile(true)
      return p
   end

   local function add_patrol_group( route, ships, start )
      start = start or rnd.rnd(1,#route)
      local pos = route[ start ]
      local l
      for k, s in ipairs( ships ) do
         local p = spawn_ship( s, pos )
         if k==1 then
            l = p
            local aimem = p:memory()
            aimem.waypoints = route
            aimem.loiter = math.huge -- patrol forever
         else
            p:setLeader( l )
         end
      end
   end

   local small_group = {
      "Lancelot",
      "Shark",
      "Shark",
   }
   local medium_group = {
      "Admonisher",
      "Ancestor",
      "Ancestor",
   }
   local large_group = {
      "Pacifier",
      "Vendetta",
      "Vendetta",
      "Ancestor",
   }

   -- route0 and route1 can't be random or it might spawn directly on the player
   add_patrol_group( route0, small_group, 1 )
   add_patrol_group( route1, small_group, 1 ) -- route0 in reverse
   add_patrol_group( route2, large_group )
   add_patrol_group( route3, medium_group )
   add_patrol_group( route4, small_group )
   add_patrol_group( route5, medium_group )
   add_patrol_group( route6, large_group )
end

function mothershipdeath ()
   if mem.state == 2 then return end
   player.msg("#g".._("You eliminated the mothership!").."#0")
   mem.state = 2
   misn.osdActive(3)
   misn.markerMove( mem.mrk, retpnt )
end

function mothershipboard ()
   vntk.msg(_("TODO"), _([[TODO]]))
   mothership:disable() -- Permanently disable
   mem.state = 2
   misn.osdActive(3)
   misn.markerMove( mem.mrk, retpnt )
   player.unboard()
end
