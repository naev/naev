local pp_shaders = require 'pp_shaders'

active = 5 -- active time in seconds
cooldown = 20 -- cooldown time in seconds
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
   if mem.isp then
      ppshader:send( "u_time", 0 )
      mem.shader = shader.addPPShader( ppshader, "game" )
   end
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
   if mem.shader then
      shader.rmPPShader( ppshader )
   end
   mem.shader = nil
   return true
end

function init( p, po )
   turnoff()
   mem.timer = nil
   po:state("off")
   po:clear() -- clear stat modifications
   mem.isp = (p == player.pilot())
end

function update( p, po, dt )
   if not mem.timer then
      return
   end

   mem.timer = mem.timer - dt
   if mem.active then
      po:progress( mem.timer / active )
      if mem.timer < 0 then
         turnoff( p, po )
      end
   else
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
         mem.timer = nil
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
