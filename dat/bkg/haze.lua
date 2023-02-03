--[[
   Some sort of hazy background.
--]]
local graphics = require "love.graphics"
local bgshaders = require "bkg.lib.bgshaders"
local love_shaders = require 'love_shaders'
-- We use the default background too!
require "bkg.default"

local shader, sf, shaze

local background_default = background
function background ()
   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   sf = naev.conf().nebu_scale

   -- Initialize shader
   local pixelcode = string.format([[
#include "lib/simplex.glsl"

const int ITERATIONS = 5;
const float SCALAR = 2.0;
const float SCALE = 1.0/900.0;
const float TIME_SCALE = 1.0/50.0;
const float VISIBILITY = 400.0;

uniform float u_time = 0.0;
uniform vec3 u_camera;

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec3 uv = 100.0 * vec3( %f, %f, %f );

   /* Calculate coordinates */
   uv.xy += ((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy) * SCALE;
   uv.z += u_time * TIME_SCALE;

   /* Create the noise */
   float f = 0.0;
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
      f += (snoise( uv * scale )*0.5 + 0.2) / scale;
   }

   /* Give more transparency around the player. */
   float d = min( 1.0, length( (texture_coords-0.5)*love_ScreenSize.xy )*u_camera.z/VISIBILITY );

   return mix( vec4(0.0), color, f*d );
}
]], rnd.rnd(), rnd.rnd(), rnd.rnd() )
   shader = graphics.newShader( pixelcode, love_shaders.vertexcode )
   shader._dt = -1000*rnd.rnd()
   shader.update = function( self, dt )
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   shaze = bgshaders.init( shader, sf )

   -- Default nebula background
   background_default()

   -- Set some fancy effects
   audio.setEffect( "haze", require("reverb_preset").forest() )
   audio.setGlobalEffect( "haze" )
   audio.setGlobalAirAbsorption( 3000, 1 )
   audio.setGlobalDopplerFactor( 0.6 ) -- More than normal
end

function renderov( dt )
   -- Get camera properties
   local x, y = camera.get():get()
   local z = camera.getZoom()
   local m = 1
   shader:send( "u_camera", x*m/sf, -y*m/sf, z*sf )

   --shaze:render( dt, {0.9, 0.1, 0.4, 1.0} )
   shaze:render( dt, {0xE5/0xFF, 0x1A/0xFF, 0x4C/0xFF, 1.0} )
end
