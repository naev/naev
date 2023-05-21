--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Renewal">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Renewal</system>
</event>
--]]
local textoverlay = require "textoverlay"
local chakra = require "luaspfx.chakra_explosion"
local srs = require "common.sirius"
local fmt = require "format"

local prevship
local reward = outfit.get("Astral Projection")
local obelisk = spob.get("Kal Niut Obelisk")

local hook_done, start_marker
function create ()
   srs.obeliskEnter( obelisk )

   -- Swap player's ship
   local player_ship = player.shipAdd( "Astral Projection", _("Psyche"), _("Psychic powers."), true )
   prevship = player.pilot():name() -- Ship to go back to
   player.shipSwap( player_ship, true )

   -- Set up position
   local pp = player.pilot()
   pp:weapsetSetInrange(nil,false)
   pp:effectAdd("Astral Projection")
   pp:setDir( math.pi*0.5 )
   pp:setPos( vec2.new(0,-200) )
   pp:intrinsicSet( { -- Ship is too fast otherwise
      thrust_mod     = -30,
      speed_mod      = -30,
      turn_mod       = -30,
   }, true ) -- overwrite all
   local _n1, seeking_chakra = pp:outfitAdd( "Seeking Chakra" )
   local _n2, feather_drive = pp:outfitAdd( "Feather Drive" )
   pp:outfitAddIntrinsic( "Astral Flow Amplifier" )
   pp:weapsetAdd( 1, seeking_chakra )
   pp:weapsetAdd( 2, feather_drive )

   -- First puzzle
   local m = pilot.add("Psychic Orb", "Independent", vec2.new(), nil, {ai="dummy"} )
   m:effectAdd("Psychic Orb On")
   m:setNoDeath(true)
   m:setNoDisable(true)
   m:setHostile(true)
   m:setVisible(true)
   m:setInvincible(true)
   start_marker = m

   textoverlay.init( "#y".._("Test of Renewal").."#0",
      "#y".._("Feather Drive through All the Orbs").."\n"..
      fmt.f(_("{key1}: use seeking chakra\n{key2}: use feather drive"),
         {key1=naev.keyGet("weapset1"),key2=naev.keyGet("weapset2")}).."#0",
      { length=8 } )

   -- Player lost hooks
   hook.pilot( pp, "disable", "player_lost_disable" )
   hook.pilot( pp, "exploded", "player_lost" )

   -- TODO proper puzzle
   hook.custom( "feather_drive", "puzzle01" )

   -- Anything will finish the event
   hook_done = hook.enter( "done" )
end

local end_hook
function player_lost_disable ()
   local pp = player.pilot()
   pp:setInvincible( true )
   pp:setInvisible( true )
   if not end_hook then
      textoverlay.init( "#r".._("Test Failed").."#0", nil, {length=6})
      end_hook = hook.timer( 6, "cleanup" )
   end
end
function player_lost ()
   local pp = player.pilot()
   pp:setHealth( 100, 100 ) -- Heal up to avoid game over if necessary
   pp:setHide( true )
   pp:setInvincible( true )
   if not end_hook then
      textoverlay.init( "#r".._("Test Failed").."#0", nil, {length=6})
      end_hook = hook.timer( 6, "cleanup" )
   end
end

function puzzle01_addship ()
   -- Spawn an enemy
   local pos = player.pos() + vec2.newP( 800+400*rnd.rnd(), rnd.angle() )
   local e = pilot.add( "Astral Projection Lesser", _("Independent"), pos, nil, {ai="baddie"})
   e:effectAdd("Psychic Orb On")
   e:setHostile(true)
   e:setVisible(true)
   e:intrinsicSet( { -- Ship is too fast otherwise
      thrust_mod     = -30,
      speed_mod      = -30,
      turn_mod       = -30,
      fwd_damage     = -60, -- Don't instagib player
      armour_mod     = -50,
      shield_mod     = -50,
   }, true ) -- overwrite all
   local emem = e:memory()
   emem.control_no = _("No response.")

   -- Readd ship when dead
   hook.pilot( e, "exploded", "puzzle01_shipdeath" )
end

function puzzle01_shipdeath ()
   hook.timer( 5, "puzzle01_addship" )
end

local markers
local marker_ship = ship.get("Psychic Orb")
function puzzle01( p )
   if start_marker and p==start_marker then
      start_marker:rm()
      start_marker = nil

      markers = {}
      local n = 5
      for i=1,n do
         local mm = {}

         local mp = {}
         local lastpos = vec2.newP( 350+300*rnd.rnd(), math.pi*0.5*(i-1)/n*math.pi*2 )
         table.insert( mp, lastpos )
         for j=1,3 do
            lastpos = lastpos + vec2.newP( 50*50*rnd.rnd(), rnd.angle() )
            table.insert( mp, lastpos )
         end
         mm.pos = mp

         local m = pilot.add(marker_ship, "Independent", vec2.new(), nil, {ai="dummy"} )
         m:setNoDeath(true)
         m:setNoDisable(true)
         m:setHostile(true)
         m:setVisible(true)
         m:control(true)
         m:moveto( mp[1] )
         mm.p = m
         mm.t = 1
         m.h = hook.pilot( m.p, "idle", "puzzle01_idle" )
         markers[i] = mm

         -- Add an enemy
         puzzle01_addship()
      end
      return
   end

   -- Hit a "normal" ship
   if p:ship() ~= marker_ship and p ~= player.pilot() then
      chakra( p:pos(), vec2.new(), 100 )
      p:rm()
      hook.timer( 8, "puzzle01_addship" )
      return
   end

   -- Hit a marker maybe?
   if markers then
      local n = 0
      for i,m in ipairs(markers) do
         if m.p and m.p==p then
            n = i
            break
         end
      end
      if n==0 then
         return
      end
      local mm = markers[n]
      chakra( mm.p:pos(), vec2.new(), 100 )
      mm.p:rm()
      mm.p = nil

      for i,m in ipairs(markers) do
         if m.p and m.p:exists() then
            return -- Not done yet
         end
      end
   else
      return -- ??
   end

   -- All done, so give ability
   srs.sfxGong()
   if player.outfitNum( reward ) > 0 then
      textoverlay.init( "#y".._("Test Completed").."#0", nil, {length=6})
   else
      player.outfitAdd( reward )
      textoverlay.init( "#y"..reward:name().."#0",
         "#y".._("New Flow Ability Unlocked").."#0",
         {length=6})
   end
   hook.timer( 6, "cleanup" )
end

function puzzle01_idle( p )
   local n = 0
   for i,m in ipairs(markers) do
      if m.p and m.p==p then
         n = i
         break
      end
   end
   assert( n~=0 )
   local mm = markers[n]
   local mp = mm.pos
   mm.t = math.fmod( mm.t, #mp )+1
   mm.p:moveto( mp[mm.t] )
end

local function cleanup_player ()
   -- Restore previous ship
   player.shipSwap( prevship, true, true )
end

function done ()
   cleanup_player()
   srs.obeliskExit()

   evt.finish()
end

function cleanup ()
   hook.rm( hook_done )
   srs.obeliskCleanup( cleanup_player, evt.finish )
end
