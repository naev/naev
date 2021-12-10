local osh = require 'outfits.shaders'

local duration = 3 -- time to try to bite
local cooldown = 8 -- cooldown time in seconds
local oshader = osh.new([[
#include "lib/blend.glsl"
const vec3 colmod = vec3( 1.0, 0.0, 0.0 );
uniform float progress = 0;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 color     = texture( tex, texcoord );
   float opacity  = clamp( progress, 0.0, 1.0 );
   color.rgb      = blendSoftLight( color.rgb, colmod, opacity );
   return color;
}
]])


local function turnon( p, po )
   -- Needs a target
   if p:target()==nil then
      return false
   end
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end
   po:state("on")
   po:progress(1)
   mem.active = true

   -- Visual effect
   if mem.isp then oshader:on() end

   return true
end

local function turnoff( _p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   mem.timer = cooldown
   mem.active = false
   oshader:off()
   return true
end

function init( p, po )
   turnoff()
   mem.timer = nil
   po:state("off")
   po:clear() -- clear stat modifications
   mem.isp = (p == player.pilot())
   oshader:force_off()
end

function update( p, po, dt )
   if not mem.timer then return end
   mem.timer = mem.timer - dt
   if mem.active then
      oshader:update_on(dt)
      if mem.timer <= 0 then
         return turnoff( p, po )
      else
         po:progress( mem.timer / duration )
      end
   else
      oshader:update_cooldown(dt)
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
         mem.timer = nil
         oshader:force_off()
      end
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   --else
   --   return turnoff( p, po )
   end
end
