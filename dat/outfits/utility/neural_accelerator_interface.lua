require 'outfits.shaders'

active = 5 -- active time in seconds
cooldown = 20 -- cooldown time in seconds
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


function turnon( p, po )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end
   po:state("on")
   po:progress(1)
   mem.timer = active
   mem.active = true

   -- Visual effect
   if mem.isp then shader_on() end
   return true
end

function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   mem.timer = cooldown
   mem.active = false
   shader_off()
   return true
end

function init( p, po )
   turnoff()
   mem.timer = nil
   po:state("off")
   po:clear() -- clear stat modifications
   mem.isp = (p == player.pilot())
   shader_force_off()
end

function update( p, po, dt )
   if not mem.timer then return end

   mem.timer = mem.timer - dt
   if mem.active then
      shader_update_on( dt )
      po:progress( mem.timer / active )
      if mem.timer < 0 then
         turnoff( p, po )
      end
   else
      shader_update_cooldown( dt )
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
         mem.timer = nil
         shader_force_off()
      end
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   else
      return turnoff( p, po )
   end
end
