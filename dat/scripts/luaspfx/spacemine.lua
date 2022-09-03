local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local spacemine_bg_shader_frag = lf.read( "scripts/luaspfx/shaders/pulse.frag" )
local spacemine_shader

local function explode( s, d )
   local sp = s:pos()
   local plt = d.pilot
   if plt and not plt:exists() then
      plt = nil
   end
   for k,p in ipairs(pilot.getInrange( sp, d.explosion )) do
      --local dst = p:dist( sp )
      p:damage( 1000, 0, 50, "normal", plt )
      p:knockback( 500, (p:pos()-sp)*10, nil, 0.5 )
   end
   s:rm() -- Remove
end

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt

   local sp = s:pos()
   local mod, angle = s:vel():polar()
   if mod > 1e-3 then
      s:setVel( vec2.newP( math.max(0,mod-100*dt), angle ) )
   end

   local triggers
   if d.fct then
      triggers = pilot.getHostiles( d.fct, d.range, sp, false, true )
   else
      triggers = pilot.getInrange( sp, d.range )
   end

   for k,p in ipairs(triggers) do
      local ew = p:evasion()
      -- if perfectly tracked, we don't have to do fancy computations
      if ew <= d.track then
         explode( s, d )
         return
      end
      -- Have to see if it triggers now
      local dst = p:pos():dist( sp )
      if d.range * d.track < dst * ew then
         explode( s, d )
         return
      end
   end
end

local function render( sp, x, y, z )
   local d = sp:data()
   spacemine_shader:send( "u_time", d.timer )

   local s = d.size * z
   local old_shader = lg.getShader()
   --lg.setShader( spacemine_shader )
   lg.setColor( d.col )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function spacemine( pos, vel, fct, params )
   params = params or {}
   -- Lazy loading shader / sound
   if not spacemine_shader then
      spacemine_shader = lg.newShader( spacemine_bg_shader_frag )
   end

   -- Sound is handled separately in outfit
   local s = spfx.new( 90, update, nil, nil, render, pos, vel )
   local d  = s:data()
   d.timer  = 0
   d.size   = params.size or 100
   d.range  = 300
   d.explosion = 500
   d.col    = params.col or {1, 1, 1, 1}
   d.fct    = fct
   d.track  = params.track or 3000
   d.pilot  = params.pilot
   return s
end

return spacemine
