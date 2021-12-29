local lg = require 'love.graphics'
local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local effects = {}

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
local alert_sound = audio.newSource('snd/sounds/alarm_warning.ogg')
local alert_meta = {}
function alert_meta.func( efx, x, y, z )
   if not effects.__alert_bg_shader then
      effects.__alert_bg_shader = lg.newShader(
         alert_bg_shader_frag,
         love_shaders.vertexcode
      )
   end
   effects.__alert_bg_shader:send( "u_time", efx.time )
   effects.__alert_bg_shader:send( "u_size", efx.params.size )

   local col = efx.params.col or {1, 1, 0, 0.5}
   local s = efx.params.size / z
   local old_shader = lg.getShader()
   lg.setShader( effects.__alert_bg_shader )
   lg.setColor( col )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end
function alert_meta.init( efx, _ttl, pos, vel )
   local dt_mod = player.dt_mod()
   local px, py = pos:get()
   local vx, vy = vel:get()
   efx.sound:setRelative( false )
   efx.sound:setPosition( px, py, 0 )
   efx.sound:setVelocity( vx, vy, 0 )
   efx.sound:setPitch( dt_mod )
   efx.sound.setAttenuationDistances( 500, 25e3 )
   efx.sound:play( pos )
end
function effects.alert( params )
   local efx = {}
   setmetatable( efx, { __index = alert_meta } )
   efx.sound = alert_sound:clone()
   efx.params = params
   return efx
end

return effects
