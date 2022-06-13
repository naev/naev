local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

--local trail_bg_shader_frag = lf.read( "scripts/luaspfx/shaders/trail.frag" )
local trail_bg_shader_frag = lf.read( "scripts/luaspfx/shaders/alert.frag" )
local trail_shader

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   trail_shader:send( "u_time", d.timer )
   trail_shader:send( "u_size", d.size )

   local s = d.size / z
   local old_shader = lg.getShader()
   lg.setShader( trail_shader )
   lg.setColor( d.col )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function trail( pos, params )
   params = params or {}
   -- Lazy loading shader / sound
   if not trail_shader then
      trail_shader = lg.newShader( trail_bg_shader_frag )
   end

   local s = spfx.new( math.huge, update, nil, nil, render, pos )
   local d  = s:data()
   d.timer  = 0
   d.size   = params.size or 100
   d.col    = params.col or {1, 1, 0, 0.5}
   return s
end

return trail
