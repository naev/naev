local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local emp_shader, emp_sfx

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   emp_shader:send( "u_time",  d.timer )
   emp_shader:send( "u_speed", d.speed )
   emp_shader:send( "u_r", d.r )
   --emp_shader:send( "u_grain", d.grain )

   local s = d.size * z
   local old_shader = lg.getShader()
   lg.setShader( emp_shader )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function spfx_emp( pos, vel, size, params )
   size = size * 1.5 -- EMP look a bit smaller in reality, so we increase in size
   local speed = params.speed or math.max(1.2-size/600, 0.4)
   local sfx
   if not params.silent then
      sfx = emp_sfx[ rnd.rnd(1,#emp_sfx) ]
   end
   local s  = spfx.new( 1/speed, update, nil, nil, render, pos, vel, sfx, size*0.5 )
   local d  = s:data()
   d.timer  = 0
   d.size   = size
   d.speed  = speed
   --d.grain  = params.grain or math.max( 1.0, size/70 )
   d.r      = rnd.rnd()
   if params.volume then
      local ss = s:sfx()
      ss:setVolume( params.volume )
   end
end

local function emp_blast( pos, vel, radius, _disable, params )
   params = params or {}

   -- Lazy loading shader / sound
   if not emp_shader then
      local emp_shader_frag = lf.read( "glsl/emp_blast.frag" )
      emp_shader = lg.newShader( emp_shader_frag )
      emp_sfx = {
         audio.new( "snd/sounds/empexplode.ogg" ),
      }
   end

   -- Create the emps
   spfx_emp( pos, vel, radius, params )
end

return emp_blast
