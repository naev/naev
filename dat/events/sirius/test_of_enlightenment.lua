--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Enlightenment">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Enlightenment</system>
</event>
--]]
local textoverlay = require "textoverlay"
local audio = require 'love.audio'
local lf = require "love.filesystem"
local pp_shaders = require "pp_shaders"
local chakra = require "luaspfx.chakra_explosion"

local ssys, sysr, spos, sdir
local prevship
local markers
local reward = outfit.get("Seeking Chakra")

local sfx = audio.newSource( 'snd/sounds/gamelan_gong.ogg' )

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
   ssys = system.cur()
   sysr = ssys:radius()

   -- Hide rest of the universe
   for k,s in ipairs(system.getAll()) do
      s:setHidden(true)
   end
   ssys:setHidden(false)

   -- Stop and play different music
   music.stop()
   -- TODO sound

   -- Swap player's ship
   local player_ship = player.shipAdd( "Astral Projection Lesser", _("Psyche"), _("Psychic powers."), true )
   prevship = player.pilot():name() -- Ship to go back to
   player.shipSwap( player_ship, true )

   -- Set up position
   local pp = player.pilot()
   spos = pp:pos()
   sdir = pp:dir()
   pp:weapsetSetInrange(nil,false)
   pp:effectAdd("Astral Projection")
   pp:setDir( math.pi*0.5 )
   pp:setPos( vec2.new(0,-500) )
   pp:intrinsicSet( { -- Ship is too fast otherwise
      thrust_mod     = -50,
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

   hook.update( "update_limits" )

   textoverlay.init( "#y".._("Test of Enlightenment").."#0",
      "#y".._("Activate the Orbs").."#0" )

   -- Anything will finish the event
   hook_done = hook.enter( "done" )
end

-- Forces the player (and other ships) to stay in the radius of the system
function update_limits ()
   for k,p in ipairs(pilot.get()) do
      local pos = p:pos()
      local d = pos:dist()
      if d > sysr then
         local _m, dir = pos:polar()
         p:setPos( vec2.newP( sysr, dir ) )
      end
   end
end

function done ()
   -- Restore previous ship
   player.shipSwap( prevship, true, true )

   ssys:setKnown(false)
   for k,s in ipairs(system.getAll()) do
      s:setHidden(false)
   end
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
      puzzle02_pos = {}
      for i,m in ipairs(markers) do
         hook.rm( m.h )
         m.p:intrinsicSet( {
            thrust     = 200,
            speed      = 100,
            turn       = 900,
         }, true ) -- overwrite all
         m.p:control()
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
         m.p:setInvincible(true)
         m.h = hook.pilot( m.p, "attacked", "puzzle02" )
      end
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
   mm.p:setInvincible(false)
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
      sfx:play()
      player.outfitAdd( reward )
      textoverlay.init( "#y"..reward:name().."#0",
         "#y".._("New Flow Ability Unlocked").."#0",
         {length=6})
      hook.timer( 10, "cleanup" )
   end
end

local pixelcode_enter = lf.read( "glsl/love/obelisk_enter.frag" )
local pixelcode_exit = lf.read( "glsl/love/obelisk_exit.frag" )
local shader
function cleanup ()
   -- Played backwards from entering
   shader = pp_shaders.newShader( pixelcode_exit )
   shader.addPPShader( shader, "gui" )
   hook.update( "update_end" )
   hook.rm( hook_done )
end

local end_timer = 2.0
local jumped = false
function update_end( _dt, real_dt )
   end_timer = end_timer - real_dt
   shader:send( "u_progress", end_timer/2.0 )
   if end_timer < 0 then
      if not jumped then
         jumped = true
         end_timer = 2.0
         shader.rmPPShader( shader )
         shader = pp_shaders.newShader( pixelcode_enter )
         shader.addPPShader( shader, "gui" )
         hook.safe( "return_obelisk" )
      else
         shader.rmPPShader( shader )
         done()
      end
   end
end

function return_obelisk ()
   local _spb,sys = spob.getS("Kal Atok Obelisk")
   time.inc( time.new( 0, 0, 1000 + 2000*rnd.rnd() ) )
   player.teleport( sys, true, true )
   local pp = player.pilot()
   pp:setDir( sdir )
   pp:setPos( spos )
   pp:setVel( vec2.new() )
   sfx:play()
   music.stop()
end
