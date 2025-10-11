--[[
   Some sort of hazy background.
--]]
local graphics = require "love.graphics"
local bgshaders = require "bkg.lib.bgshaders"
local love_shaders = require 'love_shaders'

local shader_bg, shader_ov, sf, sbg, sov
local nonuninformity

local rgba, r, g, b
function background ()
   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   local nc = naev.conf()
   sf = bgshaders.scale_factor_default()
   nonuninformity = nc.nebu_nonuniformity

   -- Initialize bg shader
   local pixelcode = string.format([[
#include "lib/cellular.glsl"

const int ITERATIONS = 3;
const float SCALE = 1.0/250.0;
const float TIME_SCALE = 1.0/30.0;
const float nonuniformity = %f;

uniform float u_time = 0.0;
uniform vec3 u_camera;

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   if (nonuniformity <= 0.0)
      return colour;

   vec3 uv = 100.0 * vec3( %f, %f, %f );

   /* Calculate coordinates */
   uv.xy += ((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy) * SCALE;
   uv.z += u_time * TIME_SCALE;

   /* Create the noise */
   float f;
   f  = (1.0-cellular( uv     ).x) * 0.5;
   f += (1.0-cellular( uv*1.5 ).x) * 0.5;

   vec4 colout = mix( vec4(vec3(0.0),1.0), colour, f );
   if (nonuniformity < 1.0) {
      colout = mix( colour, colout, nonuniformity );
   }
   return colout;
}
]], nonuninformity, rnd.rnd(), rnd.rnd(), rnd.rnd() )
   shader_bg = graphics.newShader( pixelcode, love_shaders.vertexcode )
   shader_bg._dt = -1000*rnd.rnd()
   if nonuninformity > 0.0 then
      shader_bg.update = function( self, dt )
         self._dt = self._dt + dt
         self:send( "u_time", self._dt )
      end
   end
   sbg = bgshaders.init( shader_bg, sf, {nobright=true} )

   -- Initialize overlay shader
   pixelcode = string.format([[
#include "lib/cellular.glsl"

const int ITERATIONS = 2;
const float SCALE = 1.0/500.0;
const float TIME_SCALE = 1.0/30.0;
const float VISIBILITY_INNER = 600.0;
const float VISIBILITY_OUTTER = 1700.0;
const float nonuniformity = %f;

uniform float u_time = 0.0;
uniform vec3 u_camera;

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float dist = length( (texture_coords-0.5)*love_ScreenSize.xy * u_camera.z );
   if (dist > VISIBILITY_OUTTER)
      return colour;

   vec3 uv = 100.0 * vec3( %f, %f, %f );

   /* Calculate coordinates */
   uv.xy += ((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy) * SCALE;
   uv.z += u_time * TIME_SCALE;

   vec4 colout;
   if (nonuniformity <= 0.0) {
      colout = colour;
   }
   else {
      float f;
      /* Create the noise */
      f  = (1.0-cellular2x2x2( uv     ).x);
      colout = colour * (0.1+0.9*f);
   }

   if (nonuniformity < 1.0) {
      colout = mix( colour, colout, nonuniformity );
   }
   colout = mix( colout, colour, smoothstep( VISIBILITY_INNER, VISIBILITY_OUTTER, dist ) );
   colout.a *= smoothstep( 0.0, VISIBILITY_OUTTER, dist );
   return colout;
}
]], nonuninformity, rnd.rnd(), rnd.rnd(), rnd.rnd() )
   shader_ov = graphics.newShader( pixelcode, love_shaders.vertexcode )
   shader_ov._dt = -1000*rnd.rnd()
   if nonuninformity > 0.0 then
      shader_ov.update = function( self, dt )
         self._dt = self._dt + dt
         self:send( "u_time", self._dt )
      end
   end
   sov = bgshaders.init( shader_ov, sf, {nobright=true} )

   local h, s, v = colour.rgb_to_hsv( 92/255, 230/255, 23/255 )
   r, g, b = colour.hsv_to_rgb( h, s * nc.nebu_saturation, v )
   rgba = {r, g, b, 1}

   gfx.lightAmbient( r, g, b, 3 )
   gfx.lightIntensity( 0.3 * gfx.lightIntensityGet() )

   -- Set some fancy effects
   --[[
   audio.setEffect( "haze", require("reverb_preset").forest() )
   audio.setGlobalEffect( "haze" )
   audio.setGlobalAirAbsorption( 3000, 1 )
   audio.setGlobalDopplerFactor( 0.6 ) -- More than normal
   --]]
end

function renderov( dt )
   if not player.name() then return end

   local x, y, z = camera.get()
   local m = 1
   shader_ov:send( "u_camera", x*m/sf*0.5, y*m/sf*0.5, z*sf )

   sov:render( dt, rgba )
end

function renderbg( dt )
   local x, y, z = camera.get()
   local m = 1
   if nonuninformity > 0.0 then
      shader_bg:send( "u_camera", x*m/sf*0.5, y*m/sf*0.5, z*sf )
   end

   sbg:render( dt, rgba )
end
