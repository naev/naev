--[[
   Some sort of hazy background.
--]]
local graphics = require "love.graphics"
local bgshaders = require "bkg.lib.bgshaders"
local love_shaders = require 'love_shaders'

local shader_bg, shader_ov, sf, sbg, sov

function background ()
   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   sf = naev.conf().nebu_scale

   -- Initialize bg shader
   local pixelcode = string.format([[
#include "lib/cellular.glsl"

const int ITERATIONS = 3;
const float SCALAR = pow(2.0, 3.0/3.0);
const float SCALE = 1.0/500.0;
const float TIME_SCALE = 1.0/50.0;

uniform float u_time = 0.0;
uniform vec3 u_camera;

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec3 uv = 100.0 * vec3( %f, %f, %f );

   /* Calculate coordinates */
   uv.xy += ((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy) * SCALE;
   uv.z += u_time * TIME_SCALE;

   /* Create the noise */
   float f = 0.0;
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
		f += abs(0.8-cellular2x2x2( uv * scale ).x) / scale;
   }

   return mix( vec4(vec3(0.0),1.0), colour, f );
}
]], rnd.rnd(), rnd.rnd(), rnd.rnd() )
   shader_bg = graphics.newShader( pixelcode, love_shaders.vertexcode )
   shader_bg._dt = -1000*rnd.rnd()
   shader_bg.update = function( self, dt )
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   sbg = bgshaders.init( shader_bg, sf )

   -- Initialize overlay shader
   pixelcode = string.format([[
#include "lib/cellular.glsl"

const int ITERATIONS = 2;
const float SCALAR = pow(2.0, 3.0/3.0);
const float SCALE = 1.0/1000.0;
const float TIME_SCALE = 1.0/50.0;
const float VISIBILITY = 900.0;

uniform float u_time = 0.0;
uniform vec3 u_camera;

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec3 uv = 100.0 * vec3( %f, %f, %f );

   /* Calculate coordinates */
   uv.xy += ((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy) * SCALE;
   uv.z += u_time * TIME_SCALE;

   /* Create the noise */
   float f = 0.0;
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
		f += abs(0.8-cellular2x2x2( uv * scale ).x) / scale;
   }

   float dist = length( (texture_coords-0.5)*love_ScreenSize.xy * u_camera.z );
   vec4 colout = mix( vec4(0.0), colour, smoothstep( 0.0, 2.0*VISIBILITY, dist ) );
   colout.a *= smoothstep( 0.0, VISIBILITY, dist );
	return colout;
}
]], rnd.rnd(), rnd.rnd(), rnd.rnd() )
   shader_ov = graphics.newShader( pixelcode, love_shaders.vertexcode )
   shader_ov._dt = -1000*rnd.rnd()
   shader_ov.update = function( self, dt )
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   sov = bgshaders.init( shader_ov, sf )

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

   local x, y = camera.get():get()
   local z = camera.getZoom()
   local m = 1
   shader_ov:send( "u_camera", x*m/sf, -y*m/sf, z*sf )

   sov:render( dt, {224/256, 110/256, 22/256, 1.0} )
end

function renderbg( dt )
   local x, y = camera.get():get()
   local z = camera.getZoom()
   local m = 1
   shader_bg:send( "u_camera", x*m/sf, -y*m/sf, z*sf )

   sbg:render( dt, {224/256, 110/256, 22/256, 1.0} )
end
