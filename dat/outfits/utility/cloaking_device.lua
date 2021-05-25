local pp_shaders = require 'pp_shaders'

active = 10 -- active time in seconds
cooldown = 15 -- cooldown time in seconds
shielddrain = 2 -- How fast shield drains
energydrain = 0.1 -- How much energy per ton of mess
ppshader = pp_shaders.newShader([[
#include "lib/blend.glsl"
const vec3 colmod = vec3( 0.0, 0.5, 1.0 );
uniform float u_time = 0;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 color = texture( tex, texcoord );
   float opacity = min( u_time, 0.5 );
   color.rgb = blendGlow( color.rgb, colmod, opacity );
   return color;
}
]])


function turnon( p, po )
   -- Still on cooldown
   if mem.timer > 0 then
      return false
   end
   po:state("on")
   po:progress(1)
   mem.active = true

   local ps = p:stats()
   po:set( "shield_usage", ps.shield * shielddrain )
   po:set( "energy_loss", ps.mass * energydrain )
   mem.timer = active

   -- Make invisible
   p:setInvisible( true )
   if mem.isp then
      ppshader:send( "u_time", 0 )
      mem.shader = shader.addPPShader( ppshader, "game" )
   else
      p:setNoRender( true )
   end

   return true
end

function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   po:clear() -- clear stat modifications

   -- Make visible
   p:setInvisible( false )
   p:setNoRender( false )
  
   -- Turn off shader
   if mem.shader then
      shader.rmPPShader( ppshader )
   end
   mem.shader = nil

   mem.timer = cooldown
   mem.active = false
   return true
end

function init( p, po )
   turnoff()
   mem.timer = 0
   po:state("off")
   mem.isp = (p == player.pilot())
end

function update( p, po, dt )
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
      end
   end
end

-- Disable on hit
function onhit( p, po, armour, shield )
   if mem.active then
      turnoff( p, po )
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   else
      return turnoff( p, po )
   end
end
