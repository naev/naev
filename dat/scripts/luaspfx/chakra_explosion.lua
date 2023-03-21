local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local exp_shader, exp_sfx

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   exp_shader:send( "u_time",  d.timer )
   exp_shader:send( "u_speed",  d.speed )
   exp_shader:send( "u_grain",  d.grain )
   exp_shader:send( "u_r", d.r )

   local s = d.size * z
   local old_shader = lg.getShader()
   lg.setShader( exp_shader )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function spfx_chakra( pos, vel, size, params )
   size = size * 1.5 -- Chakra look a bit smaller in reality, so we increase in size
   local speed = params.speed or math.max(1.5-(size/350)^0.5, 0.4)
   local sfx
   if not params.silent then
      sfx = exp_sfx[ rnd.rnd(1,#exp_sfx) ]
   end
   local s  = spfx.new( 1/speed, update, nil, nil, render, pos, vel, sfx, size*0.5 )
   local d  = s:data()
   d.timer  = 0
   d.size   = size
   d.speed  = speed
   d.grain  = params.grain or (size/30)^0.5
   d.r      = rnd.rnd()
   if params.volume then
      local ss = s:sfx()
      ss:setVolume( params.volume )
   end
   return s
end

local function exp( pos, vel, radius, params )
   params = params or {}

   -- Lazy loading shader / sound
   if not exp_shader then
      local exp_shader_frag = lf.read( "glsl/chakra_exp.frag" )
      exp_shader = lg.newShader( exp_shader_frag )
      exp_sfx = {
         -- TODO sound
         audio.new( "snd/sounds/empexplode.ogg" ),
      }
   end

   -- Create the exps
   return spfx_chakra( pos, vel, radius, params )
end

return exp
