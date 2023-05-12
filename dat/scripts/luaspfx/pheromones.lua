local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local pheromones_shader

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   pheromones_shader:send( "u_time", d.timer )

   local s = d.size * z
   local old_shader = lg.getShader()
   lg.setShader( pheromones_shader )
   lg.setColor( d.col )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function pheromones( pos, vel, params )
   params = params or {}
   -- Lazy loading shader / sound
   if not pheromones_shader then
      local pheromones_bg_shader_frag = lf.read( "scripts/luaspfx/shaders/pheromones.frag" )
      pheromones_shader = lg.newShader( pheromones_bg_shader_frag )
   end

   local size   = params.size or 6000

   -- Sound is handled in outfit
   local s = spfx.new( 8, update, nil, nil, render, pos, vel, nil, size )
   local d  = s:data()
   d.timer  = 0
   d.size   = size
   d.col    = params.col or {1.0, 0.5, 0.8, 0.5}
   return s
end

return pheromones
