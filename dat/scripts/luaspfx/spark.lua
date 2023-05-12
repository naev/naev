local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local spark_shader
--local spark_sfx

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   spark_shader:send( "u_time",  d.timer )
   spark_shader:send( "u_speed", d.speed )
   spark_shader:send( "u_r", d.r )

   local s = d.size * z
   local old_shader = lg.getShader()
   lg.setShader( spark_shader )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function spfx_spark( pos, vel, size, params )
   size = size * 2.5
   local speed = params.speed or math.max(3.0-size/150, 2.0)
   local sfx = nil
   --[[
   if not params.silent then
      sfx = spark_sfx[ rnd.rnd(1,#spark_sfx) ]
   end
   --]]
   local s  = spfx.new( 1/speed, update, nil, nil, render, pos, vel, sfx, size*0.5 )
   local d  = s:data()
   d.timer  = 0
   d.size   = size
   d.speed  = speed
   d.r      = rnd.rnd()
   if params.volume then
      local ss = s:sfx()
      ss:setVolume( params.volume )
   end
end

local function spark( pos, vel, radius, _disable, params )
   params = params or {}

   -- Lazy loading shader / sound
   if not spark_shader then
      local spark_shader_frag = lf.read( "glsl/spark.frag" )
      spark_shader = lg.newShader( spark_shader_frag )
      --[[
      spark_sfx = {
         audio.new( "snd/sounds/empexplode.ogg" ),
      }
      --]]
   end

   -- Create the sparks
   spfx_spark( pos, vel, radius, params )
end

return spark
