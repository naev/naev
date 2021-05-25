local pp_shaders = require 'pp_shaders'

-- Global constant variables for the outfit
cooldown = 8 -- cooldown period in seconds
ontime = 3 -- powered on time in seconds (it gets modulated by time_mod)
ppshader = pp_shaders.newShader([[
#include "lib/blend.glsl"
#include "lib/colour.glsl"
const vec3 colmod = vec3( 1.0, 0.0, 0.0 );
uniform float u_time = 0;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 color     = texture( tex, texcoord );
   float opacity  = min( 2.0*u_time, 0.8 );
   vec3 grayscale = vec3(rgb2lum(color.rgb));
   color.rgb      = mix( color.rgb, grayscale, opacity );
   return color;
}
]])

-- Init function run on creation
function init( p, po )
   mem.timer = 0
   mem.active = false
   po:state( "off" )
   mem.isp = (p == player.pilot())
   if mem.shader then
      shader.rmPPShader( ppshader )
   end
   mem.shader = nil
end

function update( p, po, dt )
   mem.timer = mem.timer - dt
   -- If active, we run until end
   if mem.active then
      if mem.timer <= 0 then
         mem.timer = cooldown
         mem.active = false
         po:state( "cooldown" )
         po:progress(1)

         if mem.shader then
            shader.rmPPShader( ppshader )
         end
         mem.shader = nil
         return
      else
         po:progress( mem.timer / ontime )
      end
   else
      po:progress( mem.timer / cooldown )
      if mem.timer <= 0 then
         po:state( "off" )
      end
   end
end

function onhit( p, po, armour, shield )
   if not mem.active and armour > 0 then
      -- Don't run while cooling off
      if mem.timer > 0 then return end
      mem.timer = ontime
      mem.active = true
      po:state( "on" )
      po:progress(1)

      -- Visual effect
      if mem.isp then
         ppshader:send( "u_time", 0 )
         mem.shader = shader.addPPShader( ppshader, "game" )
      end
   end
end
