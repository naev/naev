local osh = require 'outfits.shaders'
local audio = require 'love.audio'
local luaspfx = require 'luaspfx'

-- Global constant variables for the outfit
local cooldown = 8 -- cooldown period in seconds
local ontime = 3 -- powered on time in seconds (it gets modulated by time_mod)
local oshader = osh.new([[
#include "lib/blend.glsl"
#include "lib/colour.glsl"
const vec3 colmod = vec3( 1.0, 0.0, 0.0 );
uniform float progress;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 color     = texture( tex, texcoord );
   float opacity  = 0.8 * clamp( progress, 0.0, 1.0 );
   vec3 grayscale = vec3(rgb2lum(color.rgb));
   color.rgb      = mix( color.rgb, grayscale, opacity );
   return color;
}
]])

local sfx = audio.newSource( 'snd/sounds/activate4.ogg' )

-- Init function run on creation
function init( p, po )
   mem.timer = nil
   mem.active = false
   po:state( "off" )
   mem.isp = (p == player.pilot())
   oshader:force_off()
end

function cleanup( _p, _po )
   oshader:force_off()
end

function update( p, po, dt )
   if not mem.timer then return end
   mem.timer = mem.timer - dt
   -- If active, we run until end
   if mem.active then
      oshader:update_on(dt)
      if mem.timer <= 0 then
         mem.timer = cooldown * p:shipstat("cooldown_mod",true)
         mem.active = false
         po:state( "cooldown" )
         po:progress(1)

         oshader:force_off()
         return
      else
         po:progress( mem.timer / ontime )
      end
   else
      po:progress( mem.timer / cooldown )
      oshader:update_cooldown(dt)
      if mem.timer <= 0 then
         po:state( "off" )
         mem.timer = nil
         oshader:force_off()
      end
   end
end

function onhit( p, po, armour, _shield )
   if not mem.active and armour > 0 then
      -- Don't run while cooling off
      if mem.timer and mem.timer > 0 then return end
      mem.timer = ontime
      mem.active = true
      po:state( "on" )
      po:progress(1)

      -- Visual effect
      if mem.isp then
         oshader:on()
         luaspfx.sfx( true, nil, sfx )
      else
         luaspfx.sfx( p:pos(), p:vel(), sfx )
      end
   end
end
