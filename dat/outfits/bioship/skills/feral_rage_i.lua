--local osh = require 'outfits.shaders'
local audio = require 'love.audio'
local luaspfx = require 'luaspfx'

-- Global constant variables for the outfit
local ontime = 5 -- ontime period
-- TODO should probably have an affect on the ship, not the screen
--[=[
local oshader = osh.new([[
#include "lib/blend.glsl"
const vec3 colmod = vec3( 1.0, 0.0, 0.0 );
uniform float progress;
vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord )
{
   vec4 color     = texture( tex, texcoord );
   float opacity  = clamp( progress, 0.0, 1.0 );
   color.rgb      = blendSoftLight( color.rgb, colmod, opacity );
   return color;
}
]])
--]=]

local sfx = audio.newSource( 'snd/sounds/activate3.ogg' )

-- Init function run on creation
function init( p, po )
   mem.timer = nil
   mem.active = false
   po:state( "off" )
   mem.isp = (p == player.pilot())
   --oshader:force_off()
end

function cleanup( _p, _po )
   --oshader:force_off()
end

function update( _p, po, dt )
   if not mem.timer then return end
   mem.timer = mem.timer - dt
   if mem.active then
      --oshader:update_on(dt)
      if mem.timer <= 0 then
         po:state( "off" )
         --oshader:force_off()
         return
      else
         po:progress( mem.timer / ontime )
      end
   end
end

function onhit( p, po, armour, _shield )
   if not mem.active and armour > 0 then
      -- Already running, so just reset timer
      if mem.timer and mem.timer > 0 then
         mem.timer = ontime
         return
      end
      mem.active = true
      mem.timer = ontime
      po:state( "on" )
      po:progress(1)

      -- Visual effect
      if mem.isp then
         --oshader:on()
         luaspfx.sfx( true, nil, sfx )
      else
         luaspfx.sfx( p:pos(), p:vel(), sfx )
      end
   end
end
