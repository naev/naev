--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Enlightenment">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Enlightenment</system>
</event>
--]]
local textoverlay = require "textoverlay"
local chakra = require "luaspfx.chakra_explosion"
local srs = require "common.sirius"

local prevship, markers
local reward = outfit.get("Seeking Chakra")
local obelisk = spob.get("Kal Atok Obelisk")

local function marker_set( n, state )
   local m = markers[n]
   m.p:effectClear()
   if state then
      m.p:effectAdd("Psychic Orb On")
   else
      m.p:effectAdd("Psychic Orb Off")
   end
   m.on = state
   m.t = naev.ticksGame()
end

local function marker_toggle( n )
   marker_set( n, not markers[n].on )
end

local hook_done
function create ()
   srs.obeliskEnter( obelisk )

   -- Swap player's ship
   local player_ship = player.shipAdd( "Astral Projection Lesser", _("Psyche"), _("Psychic powers."), true )
   prevship = player.pilot():name() -- Ship to go back to
   player.shipSwap( player_ship, true )

   -- Set up position
   local pp = player.pilot()
   pp:weapsetSetInrange(nil,false)
   pp:effectAdd("Astral Projection")
   pp:setDir( math.pi*0.5 )
   pp:setPos( vec2.new(0,-500) )
   pp:intrinsicSet( { -- Ship is too fast otherwise
      accel_mod     = -50,
      speed_mod      = -50,
      turn_mod       = -50,
   }, true ) -- overwrite all

   -- First puzzle
   markers = {}
   local n = 5
   for i=1,n do
      local dir = math.pi*0.5 + (i-1)/n*math.pi*2.0
      local pos = vec2.newP( 200, dir )
      local m = pilot.add("Psychic Orb", "Independent", pos, nil, {ai="dummy"} )
      m:setNoDeath(true)
      m:setNoDisable(true)
      m:setHostile(true)
      m:setInvisible(true)
      local h = hook.pilot( m, "attacked", "puzzle01" )
      markers[i] = { p=m, h=h }
   end
   marker_set( 1, true )
   marker_set( 2, false )
   marker_set( 3, true )
   marker_set( 4, true )
   marker_set( 5, false )

   textoverlay.init( "#y".._("Test of Enlightenment").."#0",
      "#y".._("Activate the Orbs").."#0" )

   -- Anything will finish the event
   hook_done = hook.enter( "done" )
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

local puzzle02_pos
function puzzle01( p )
   local n = 0
   for i,m in ipairs(markers) do
      if m.p==p then
         n = i
         break
      end
   end
   assert( n~=0 )
   local mm = markers[n]
   mm.p:setHealth( 100, 100 )
   mm.p:setVel( vec2.new() )

   marker_toggle( n )
   marker_toggle( math.fmod( n,   5 )+1 ) -- One up
   marker_toggle( math.fmod( n+3, 5 )+1 ) -- One below

   -- Check if done
   local allon = true
   for i,m in ipairs(markers) do
      if not m.on then
         allon = false
         break
      end
   end
   if allon then
      hook.safe("puzzle02_start")
   end
end

function puzzle02_start ()
   puzzle02_pos = {}
   for i,m in ipairs(markers) do
      hook.rm( m.h )
      m.p:intrinsicSet( {
         accel      = 200,
         speed      = 100,
         turn       = 900,
      }, true ) -- overwrite all
      m.p:control(true)
      m.t = 1
      hook.pilot( m.p, "idle", "puzzle02_idle" )
      local lastpos = m.p:pos()
      local posm, posa = lastpos:polar()
      local function rndpos ()
         lastpos = lastpos + vec2.newP( posm+50+50*rnd.rnd(), posa+math.pi*0.5*(rnd.rnd()-0.5) )
         return lastpos
      end
      puzzle02_pos[i] = { rndpos(), rndpos(), rndpos(), }
      m.p:moveto( puzzle02_pos[i][m.t] )
      m.h = hook.pilot( m.p, "attacked", "puzzle02" )
   end
end

function puzzle02_idle( p )
   local n = 0
   for i,m in ipairs(markers) do
      if m.p and m.p==p then
         n = i
         break
      end
   end
   assert( n~=0 )
   local mm = markers[n]
   local mp = puzzle02_pos[n]
   mm.t = math.fmod( mm.t, #mp )+1
   mm.p:moveto( mp[mm.t] )
end

function puzzle02( p )
   local n = 0
   for i,m in ipairs(markers) do
      if m.p and m.p==p then
         n = i
         break
      end
   end
   assert( n~=0 )
   local mm = markers[n]
   chakra( mm.p:pos(), vec2.new(), 100 )
   mm.p:rm()
   mm.p = nil

   -- Check if done
   local allon = true
   for i,m in ipairs(markers) do
      if m.p and m.p:exists() then
         allon = false
         break
      end
   end
   if allon then
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
end

function cleanup ()
   hook.rm( hook_done )
   srs.obeliskCleanup( cleanup_player, evt.finish )
end
