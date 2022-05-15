local lg = require 'love.graphics'
local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local alert_bg_shader_frag = [[
#include "lib/sdf.glsl"

uniform float u_time = 0.0;
uniform float u_size = 100.0;

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   color.a *= sin(u_time*20.0) * 0.1 + 0.9;

   /* Base Alpha */
   float a = step( sin( u_size * (uv.x + uv.y) * 0.3), 0.0);

   /* Signed Distance Function Exclamation Point */
   vec2 p = 2.0*uv-1.0;
   p.y *= -1.0;
   float dc = sdCircle( p, 1.0 );
   p *= 1.2;
   float d = min( sdCircle( p+vec2(0.0,0.65), 0.15), sdUnevenCapsuleY( p+vec2(0,0.15), 0.1, 0.25, 0.7 ));

   /* Add border and make center solid. */
   a *= step( 0.0, d-20.0/u_size );
   a += step( d, 0.0 );

   /* Second border. */
   float off = 15.0 / u_size;
   a *= step( dc+off, 0.0 );
   a += step( -(dc+off), 0.0 );
   a *= step( dc, 0.0 );

   color.a *= a;
   return color;
}
]]
local alert_shader, alert_sound

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   alert_shader:send( "u_time", d.timer )
   alert_shader:send( "u_size", d.size )

   local s = d.size / z
   local old_shader = lg.getShader()
   lg.setShader( alert_shader )
   lg.setColor( d.col )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function alert( pos, params )
   params = params or {}
   -- Lazy loading shader / sound
   if not alert_sound then
      alert_sound = audio.newSource('snd/sounds/alarm_warning.ogg')
   end
   if not alert_shader then
      alert_shader = lg.newShader(
         alert_bg_shader_frag,
         love_shaders.vertexcode
      )
   end

   local s = spfx.new( 2.2, update, nil, nil, render, pos, nil, alert_sound )
   local d  = s:data()
   d.timer  = 0
   d.size   = params.size or 100
   d.col    = params.col or {1, 1, 0, 0.5}
end

return alert
