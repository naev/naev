local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'
local explosion = require 'luaspfx.explosion'
local luasfx = require "luaspfx.sfx"

local spacemine_shader, highlight_shader, spacemine_sfx

local CHECK_INTERVAL = 0.1 -- How often to check activations in seconds
local ACTIVATE_DELAY = 1.5 -- Time to blow up once activated in seconds
local RANGE          = 300 -- Range of detection
local RANGE_MIN      = 100 -- Minimum range of detection
local RANGE_EXP      = 500 -- Range of the explosion
local DAMAGE         = 1000 -- Default damage
local PENETRATION    = 50 -- Default penetration
local TRACKMIN       = 0 -- Default minimum tracking
local TRACKMAX       = 5e3 -- Default maximum tracking
local PRIME          = 5 -- Default time to prime

local function trigger( s, d )
   d.triggered = d.timer + ACTIVATE_DELAY
   luasfx( s:pos(), s:vel(), spacemine_sfx )
end

local function get_range( ew, d )
   local mod = (ew - d.trackmin) / (d.trackmax - d.trackmin)
   return math.max( d.rangemin, d.range*mod )
end

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt

   local sp = s:pos()
   local mod, angle = s:vel():polar()
   if mod > 1e-3 then
      s:setVel( vec2.newP( math.max(0,mod-100*dt), angle ) )
   end

   -- Not primed yet
   if d.timer < d.primed then
      return
   end

   -- Has been triggered
   if d.triggered then
      -- Time to go boom
      if d.timer > d.triggered then
         explosion( sp, s:vel(), d.explosion, d.damage, {
            parent = d.pilot,
            penetration = d.penetration,
            dmgtype = "kinetic",
         } )
         s:rm() -- Remove

         -- Notify pilots in rangea
         -- TODO make detection affect this
         pilot.msg( nil, pilot.getInrange( sp, 10e3 ), "explosion" )
      end
      return
   end

   -- Only check once per second
   if d.timer < d.check then
      return
   end
   d.check = d.timer + CHECK_INTERVAL

   -- See what can trigger it
   local triggers
   if d.fct then
      triggers = pilot.getEnemies( d.fct, d.range, sp, true, true )
   else
      triggers = pilot.getInrange( sp, d.range )
   end
   if d.hostile then
      local pp = player.pilot()
      if pp:exists() and pp:pos():dist2( sp ) < d.range*d.range then
         table.insert( triggers, pp )
      end
   end

   -- Detect nearby enemies
   for k,p in ipairs(triggers) do
      local ew = p:signature()
      -- if perfectly tracked, we don't have to do fancy computations
      if ew > d.trackmax then
         trigger( s, d )
         return
      end
      -- Have to see if it triggers now
      local dst = p:pos():dist2( sp )
      if dst < get_range(ew,d)^2 then
         trigger( s, d )
         return
      end
   end
end

local function render( sp, x, y, z )
   local d = sp:data()
   local old_shader = lg.getShader()
   highlight_shader:send( "u_time", d.timer )
   if d.triggered then
      -- Basically speeds up animation by a constant factor
      spacemine_shader:send( "u_time", d.timer + 2*(d.timer - d.triggered + ACTIVATE_DELAY)  )
   else
      spacemine_shader:send( "u_time", d.timer )
   end

   -- Render for player
   local pp = player.pilot()
   if pp:exists() and (d.hostile or not d.fct or d.fct:areEnemies(pp:faction())) then
      local ew = pp:signature()
      if ew > d.trackmin then
         local r = get_range( ew, d ) * z
         lg.setShader( highlight_shader )
         lg.setColour( {1, 0, 0, 0.1} )
         love_shaders.img:draw( x-r, y-r, 0, 2*r )
      end
      lg.setColour( {1, 0, 0, 1} )
   else
      if d.triggered then
         lg.setColour( {1, 0, 0, 1} )
      else
         lg.setColour( {0, 1, 0, 1} )
      end
   end

   -- TODO render something nice that blinks
   local s = 10 * z
   lg.setShader( spacemine_shader )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )

   lg.setShader( old_shader )
end

local function spacemine( pos, vel, fct, params )
   params = params or {}
   -- Lazy loading shader / sound
   if not spacemine_shader then
      local spacemine_shader_frag = lf.read( "scripts/luaspfx/shaders/spacemine_blink.frag" )
      local highlight_shader_frag = lf.read( "scripts/luaspfx/shaders/spacemine_highlight.frag" )
      spacemine_shader = lg.newShader( spacemine_shader_frag )
      highlight_shader = lg.newShader( highlight_shader_frag )
      spacemine_sfx = audio.new( "snd/sounds/detonation_alarm.ogg" )
   end

   -- Other params
   local duration = params.duration or 90

   -- Sound is handled separately in outfit
   local s  = spfx.new( duration, update, render, nil, nil, pos, vel, nil, RANGE )
   local d  = s:data()
   d.timer     = 0
   d.check     = rnd.rnd() * CHECK_INTERVAL
   d.range     = RANGE
   d.rangemin  = RANGE_MIN
   d.explosion = RANGE_EXP
   d.fct       = fct
   d.damage    = params.damage or DAMAGE
   d.penetration = params.penetration or PENETRATION
   d.trackmax  = params.trackmax or TRACKMAX
   d.trackmin  = params.trackmin or TRACKMIN
   d.pilot     = params.pilot
   d.primed    = params.primed or PRIME
   d.hostile   = params.hostile
   return s
end

return spacemine
