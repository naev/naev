local osh = require 'outfits.shaders'
local audio = require 'love.audio'
local luaspfx = require 'luaspfx'

local active = 10 -- active time in seconds
local cooldown = 15 -- cooldown time in seconds
local shielddrain = 2 -- How fast shield drains
local energydrain = 0.1 -- How much energy per ton of mess
local oshader = osh.new([[
#include "lib/blend.glsl"
const vec3 colmod = vec3( 0.0, 0.5, 1.0 );
uniform float progress = 0;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 color     = texture( tex, texcoord );
   float opacity  = 0.5 * clamp( progress, 0.0, 1.0 );
   color.rgb      = blendGlow( color.rgb, colmod, opacity );
   return color;
}
]])
oshader.fade = 1

local sfx = audio.newSource( 'snd/sounds/activate5.ogg' )

local function turnon( p, po )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end
   po:state("on")
   po:progress(1)
   mem.active = true

   local ps = p:stats()
   po:set( "shield_regen_malus", ps.shield * shielddrain )
   po:set( "energy_loss", ps.mass * energydrain )
   mem.timer = active

   -- Make invisible
   p:setInvisible( true )
   if mem.isp then
      oshader:on()
      luaspfx.sfx( true, nil, sfx )
   else
      luaspfx.sfx( p:pos(), p:vel(), sfx );
      p:setNoRender( true )
   end

   return true
end

local function turnoff( p, po )
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
   oshader:off()

   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   mem.active = false
   return true
end

function init( p, po )
   turnoff()
   mem.timer = nil
   po:state("off")
   mem.isp = (p == player.pilot())
   oshader:force_off()
end

function update( p, po, dt )
   if not mem.timer then return end
   mem.timer = mem.timer - dt
   if mem.active then
      oshader:update_on(dt)
      po:progress( mem.timer / active )
      if mem.timer < 0 then
         turnoff( p, po )
      end
   else
      oshader:update_cooldown(dt)
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
         mem.timer = nil
      end
   end
end

-- Disable on hit
function onhit( p, po, _armour, _shield )
   if mem.active then
      turnoff( p, po )
   end
end

-- Disable on shoot
function onshoot( p, po )
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
