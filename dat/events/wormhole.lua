--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Wormhole">
 <trigger>none</trigger>
 <chance>0</chance>
</event>
--]]
local audio = require 'love.audio'
local pp_shaders = require "pp_shaders"

local pixelcode = [[
#include "lib/simplex.glsl"

uniform float u_time = 0.0;
uniform float u_progress = 0.0;
uniform int u_invert = 0;

const vec3 col_outter = vec3( 0.0, 0.8, 1.0 );

float fbm2( vec3 x )
{
   float v = 0.0;
   float a = 0.5;
   const vec3 shift = vec3(100.0);
   for (int i=0; i<2; i++) {
      v += a * snoise(x);
      x  = x * 2.0 + shift;
      a *= 0.5;
   }
   return v;
}

float fbm3( vec3 x )
{
   float v = 0.0;
   float a = 0.5;
   const vec3 shift = vec3(100.0);
   for (int i=0; i<3; i++) {
      v += a * snoise(x);
      x  = x * 2.0 + shift;
      a *= 0.5;
   }
   return v;
}

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   float t = u_time * 0.2;

   vec2 uv = texture_coords-0.5;
   vec2 st = vec2( 1.5 * length(uv), atan(uv.y,uv.x) );

   st.y += 1.1 * st.x;
   float x = fbm2( vec3( sin(st.y), cos(st.y), pow(st.x, 0.3) + 0.1*t ) ); // 3
   float y = fbm2( vec3( cos(1.0-st.y), sin(1.0-st.y), pow(st.x, 0.5) + 0.1*t ) ); // 4

   float r = fbm3( vec3( x, y, st.x + 0.3*t ) ); // 5
   r = fbm3( vec3( r-x, r-y, r + 0.3*t ) ); // 6

   float c = (r + 5.0*st.x) / 6.0;

   float a = 1.0-smoothstep( 0.7*u_progress, 1.4*u_progress, c );
   if (u_invert!=0)
      a = 1.0 - a;
   return vec4( mix( texture( tex, texture_coords ).rgb, col_outter, a ), 1.0 );
}
]]

-- luacheck: globals update wormhole (Hook functions passed by name)

local target, shader, r
local sfx = audio.newSource( 'snd/sounds/wormhole.ogg' )
function create ()

   target = var.peek("wormhole_target")
   if not target then
      warn(_("Wormhole event run with no target!"))
      return
   end
   hook.update( "update" )
   sfx:play()

   shader = pp_shaders.newShader( pixelcode )
   shader.addPPShader( shader, "final" )
   r = -rnd.rnd()*1000
end

local timer = 0
local jumped = false
local jumptime = 2.0
function update( _dt, real_dt )
   timer = timer + real_dt
   shader:send( "u_time", timer+r )
   shader:send( "u_progress", math.min(timer/jumptime,1.0) )
   if timer >= jumptime then
      if not jumped then
         jumped = true
         r = r + timer
         timer = 0
         shader:send( "u_progress", 0 ) -- avoids visual artefact
         shader:send( "u_invert", 1 )
         hook.safe( "wormhole" )
      else
         shader.rmPPShader( shader )
         var.pop("wormhole_target")
         evt.finish()
      end
   end
end

function wormhole ()
   player.teleport( target )
   local pp = player.pilot()
   pp:setPos( pp:pos() + vec2.newP( 100+100*rnd.rnd(), rnd.angle() ) )
end
