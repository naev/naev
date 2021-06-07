require 'outfits.shaders'

-- Global constant variables for the outfit
cooldown = 8 -- cooldown period in seconds
ontime = 3 -- powered on time in seconds (it gets modulated by time_mod)
ppshader = shader_new([[
#include "lib/blend.glsl"
#include "lib/colour.glsl"
const vec3 colmod = vec3( 1.0, 0.0, 0.0 );
uniform float progress = 0;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 color     = texture( tex, texcoord );
   float opacity  = 0.8 * clamp( progress, 0.0, 1.0 );
   vec3 grayscale = vec3(rgb2lum(color.rgb));
   color.rgb      = mix( color.rgb, grayscale, opacity );
   return color;
}
]])

-- Init function run on creation
function init( p, po )
   mem.timer = nil
   mem.active = false
   po:state( "off" )
   mem.isp = (p == player.pilot())
   shader_force_off()
end

function cleanup( p, po )
   shader_force_off()
end

function update( p, po, dt )
   if not mem.timer then return end
   mem.timer = mem.timer - dt
   -- If active, we run until end
   if mem.active then
      shader_update_on(dt)
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
      shader_update_cooldown(dt)
      if mem.timer <= 0 then
         po:state( "off" )
         mem.timer = nil
         shader_force_off()
      end
   end
end

function onhit( p, po, armour, shield )
   if not mem.active and armour > 0 then
      -- Don't run while cooling off
      if mem.timer and mem.timer > 0 then return end
      mem.timer = ontime
      mem.active = true
      po:state( "on" )
      po:progress(1)

      -- Visual effect
      if mem.isp then shader_on() end
   end
end
