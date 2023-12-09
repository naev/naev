--[[--
A module containing a diversity of Love2D shaders for use in Naev. These are
designed to be used with the different aspects of the VN framework.

In general all shaders have a "strength" parameter indicating the strength
of the effect. Furthermore, those that have a temporal component have a
"speed" parameter. These are all normalized such that 1 is the default
value. Temporal component can also be inverted by setting a negative value.
@module love_shaders
--]]
local graphics = require "love.graphics"
local love_math = require "love.math"
local love_image = require "love.image"

local love_shaders = {}

--[[--
Shader common parameter table.
@tfield number strength Strength of the effect normalized such that 1.0 is the default value.
@tfield number speed Speed of the effect normalized such that 1.0 is the default value. Negative values run the effect backwards. Only used for those shaders with temporal components.
@tfield Colour colour Colour component to be used. Should be in the form of {r, g, b} where r, g, and b are numbers.
@tfield number size Affects the size of the effect.
@table shaderparams
--]]

-- Tiny image for activating shaders
local idata = love_image.newImageData( 1, 1 )
idata:setPixel( 0, 0, 1, 1, 1, 1 )
love_shaders.img = graphics.newImage( idata )

--[[--
Default fragment code that doesn't do anything fancy.
--]]
local _pixelcode = [[
vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 texcolour = Texel(tex, texture_coords);
   return texcolour * colour;
}
]]
--[[--
Default vertex code that doesn't do anything fancy.
--]]
local _vertexcode = [[
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]
-- Make default shaders visible.
love_shaders.pixelcode = _pixelcode
love_shaders.vertexcode = _vertexcode

local function _shader2canvas( shader, image, w, h, sx, sy )
   w = w or image.w
   h = h or image.h
   sx = sx or 1
   sy = sy or sx
   -- Render to image
   local newcanvas = graphics.newCanvas( w, h )
   local oldcanvas = graphics.getCanvas()
   local oldshader = graphics.getShader()
   graphics.setCanvas( newcanvas )
   graphics.clear( 0, 0, 0, 0 )
   graphics.setShader( shader )
   graphics.setColour( 1, 1, 1, 1 )
   image:draw( 0, 0, 0, sx, sy )
   graphics.setShader( oldshader )
   graphics.setCanvas( oldcanvas )

   return newcanvas
end

--[[--
Renders a shader to a canvas.

@tparam Shader shader Shader to render.
@tparam[opt=love.w] number width Width of the canvas to create (or nil for fullscreen).
@tparam[opt=love.h] number height Height of the canvas to create (or nil for fullscreen).
@treturn Canvas Generated canvas.
--]]
function love_shaders.shader2canvas( shader, width, height )
   local lw, lh = naev.gfx.dim()
   width = width or lw
   height = height or lh
   return _shader2canvas( shader, love_shaders.img, width, height, width, height )
end


--[[--
Renders an image with a shader to a canvas.

@tparam Shader shader Shader to user
@tparam Image image Image to render.
@tparam[opt=image.w] number width Width of the canvas to create.
@tparam[opt=image.h] number height Height of the canvas to create.
@tparam[opt=1] number sx Scale factor for width.
@tparam[opt=1] number sy Scale factor for height.
@treturn Canvas Generated canvas.
--]]
love_shaders.shaderimage2canvas = _shader2canvas


--[[--
Generates a paper-like image.

@tparam number width Width of the image to create.
@tparam number height Height of the image to create.
@tparam[opt=1] number sharpness How sharp to make the texture look.
@treturn Canvas A apper-like canvas image.
--]]
function love_shaders.paper( width, height, sharpness )
   sharpness = sharpness or 1
   local pixelcode = string.format([[
precision highp float;

#include "lib/simplex.glsl"

const float u_r = %f;
const float u_sharp = %f;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolour = colour * texture( tex, uv );

   float n = 0.0;
   for (float i=1.0; i<8.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( px * u_sharp * 0.003 * m + 1000.0 * u_r ) * (1.0 / m);
   }

   texcolour.rgb *= 0.68 + 0.3 * n;

   return texcolour;
}
]], love_math.random(), sharpness )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   return love_shaders.shader2canvas( shader, width, height )
end


--[[--
Blur shader applied to an image.

@tparam Drawable image A drawable to blur.
@tparam[opt=5] number kernel_size The size of the kernel to use to blur. This
   is the number of pixels in the linear case or the standard deviation in the
   Gaussian case.
@tparam[opt="gaussian"] string blurtype Either "linear" or "gaussian".
--]]
function love_shaders.blur( image, kernel_size, blurtype )
   kernel_size = kernel_size or 5
   blurtype = blurtype or "gaussian"
   local w, h = image:getDimensions()
   local pixelcode = string.format([[
precision highp float;
#include "lib/blur.glsl"
uniform vec2 blurvec;
const vec2 wh = vec2( %f, %f );
const float strength = %f;
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolour = blur%s( tex, uv, wh, blurvec, strength );
   return texcolour;
}
]], w, h, kernel_size, blurtype )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   -- Since the kernel is separable we need two passes, one for x and one for y
   shader:send( "blurvec", 1, 0 )
   local pass1 = _shader2canvas( shader, image, w, h )
   local mode, alphamode = graphics.getBlendMode()
   graphics.setBlendMode( "alpha", "premultiplied" )
   shader:send( "blurvec", 0, 1 )
   local pass2 = _shader2canvas( shader, pass1, w, h )
   graphics.setBlendMode( mode, alphamode )
   return pass2
end

--[[--
Creates an oldify effect, meant for full screen effects.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" field is used.
--]]
function love_shaders.oldify( params )
   params = params or {}
   local strength = params.strength or 1.0
   local pixelcode = string.format( [[
#include "lib/simplex.glsl"
#include "lib/perlin.glsl"
#include "lib/math.glsl"
#include "lib/blur.glsl"
#include "lib/blend.glsl"
#include "lib/colour.glsl"

uniform float u_time;

const float strength = %f;

float grain(vec2 uv, vec2 mult, float frame, float multiplier) {
   float offset = snoise(vec3(mult / multiplier, frame));
   float n1 = pnoise(vec3(mult, offset), vec3(1.0/uv * love_ScreenSize.xy, 1.0));
   return n1 / 2.0 + 0.5;
}
vec4 graineffect( vec4 bgcolour, vec2 uv, vec2 px ) {
   const float fps = 15.0;
   const float zoom = 0.2;
   float frame = floor(fps*u_time) / fps;
   const float tearing = 3.0; /* Tears "1/tearing" of the frames. */

   vec3 g = vec3( grain( uv, px * zoom, frame, 2.5 ) );

   // get the luminance of the image
   float luminance = rgb2lum( bgcolour.rgb );
   vec3 desaturated = vec3(luminance);

   // now blend the noise over top the backround
   // in our case soft-light looks pretty good
   vec4 colour;
   colour = vec4( vec3(1.2,1.0,0.4)*luminance, bgcolour.a );
   colour = vec4( blendSoftLight(colour.rgb, g), bgcolour.a );

   // and further reduce the noise strength based on some
   // threshold of the background luminance
   float response = smoothstep(0.05, 0.5, luminance);
   colour.rgb = mix(desaturated, colour.rgb, pow(response,2.0));

   // Vertical tears
   if (distance( love_ScreenSize.x * random(vec2(frame, 0.0)), px.x) < tearing*random(vec2(frame, 1000.0))-(tearing-1.0))
      colour.rgb *= vec3( random( vec2(frame, 5000.0) ));

   // Flickering
   colour.rgb *= 1.0 + 0.05*snoise( vec2(3.0*frame, M_PI) );

   return colour;
}
vec4 vignette( vec2 uv )
{
   uv *= 1.0 - uv.yx;
   float vig = uv.x*uv.y * 15.0; // multiply with sth for intensity
   vig = pow(vig, 0.3); // change pow for modifying the extend of the  vignette
   return vec4(vig);
}
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 screen_coords )
{
   vec4 texcolour = colour * texture( tex, uv );

   texcolour = graineffect( texcolour, uv, screen_coords );
   vec4 v = vignette( uv );
   texcolour.rgb *= v.rgb;
   //texcolour = mix( texcolour, v, v.a );

   return texcolour;
}
]], strength )

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000. * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
A hologram effect, mainly meant for VN characters.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" field is used.
--]]
function love_shaders.hologram( params )
   params = params or {}
   local strength = params.strength or 1.0
   local pixelcode = string.format([[
#include "lib/math.glsl"
#include "lib/blur.glsl"
#include "lib/blend.glsl"
#include "lib/simplex.glsl"
#include "lib/colour.glsl"

uniform float u_time;

float onOff(float a, float b, float c)
{
   return step(c, sin(u_time + a*cos(u_time*b)));
}

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 screen_coords )
{
   const float strength = %f; /* Simple parameter to control everything. */

   /* Strength-based constants. */
   const float BLURAMPLITUDE     = 6.0 * strength;
   const float BLURSPEED         = 0.15 * strength;
   const float HIGHLIGHTSPEED    = 0.05 + 0.05 * strength;
   const float HIGHLIGHTRANGE    = 0.1 + 0.05 * strength;
   const float HIGHLIGHTCOUNT    = 4.0 + 2.5 * strength;
   const float SHADOWSPEED       = 0.17 + 0.14 * strength;
   const float SHADOWRANGE       = 0.07 + 0.05 * strength;
   const float SHADOWCOUNT       = 4.7 + 2.3 * strength;
   const float GRAINSTRENGTH     = 32.0 + 48.0 * strength;
   const float WOBBLEAMPLITUDEX  = 0.0025 * strength;
   const float WOBBLEAMPLITUDEY  = 0.003 * strength;
   const float WOBBLESPEED       = 0.1 * strength;

   /* Truly constant values. */
   //const vec3 bluetint           = vec3( 0.075, 0.215, 0.604 );/* Gamma: vec3(0.3, 0.5, 0.8); */
   const vec3 bluetint           = vec3( 0.1, 0.3, 0.7 );/* Gamma: vec3(0.3, 0.5, 0.8); */
   const float BRIGHTNESS        = 0.1;
   const float CONTRAST          = 1.0/0.2;
   const float SCANLINEMEAN      = 0.9;
   const float SCANLINEAMPLITUDE = 0.2;
   const float SCANLINESPEED     = 1.0;

   /* Get the texture. */
   vec2 look      = uv;
   float tmod     = mod(u_time/4.0,1.0);
   float window   = 1.0 / (1.0+20.0*(look.y-tmod)*(look.y-tmod));
   look.x += WOBBLEAMPLITUDEX * sin(look.y*10.0 + u_time)*onOff(1.0,1.0,WOBBLESPEED)*(1.0+cos(u_time*80.0))*window;
   float vShift   = WOBBLEAMPLITUDEY * onOff(2.0,3.0,M_PI*WOBBLESPEED)*(sin(u_time)*sin(u_time*20.0)+(0.5 + 0.1*sin(u_time*200.0)*cos(u_time)));
   look.y = mod( look.y + vShift, 1.0 );

   /* Blur a bit. */
   vec4 texcolour  = colour * texture( tex, look );
   float blurdir  = snoise( vec2( BLURSPEED*u_time, 0.0 ) );
   vec2 blurvec   = BLURAMPLITUDE * vec2( cos(blurdir), sin(blurdir) );
   vec4 blurbg    = blur9( tex, look, love_ScreenSize.xy, blurvec );
   texcolour.rgb   = blendSoftLight( texcolour.rgb, blurbg.rgb );
   texcolour.a     = max( texcolour.a, blurbg.a );

   /* Drop to greyscale while increasing brightness and contrast */
   float greyscale= rgb2lum( texcolour.rgb ); // percieved
   texcolour.xyz   = CONTRAST * vec3(greyscale) + BRIGHTNESS;

   /* Shadows. */
   float shadow   = 1.0 + SHADOWRANGE * sin((uv.y + (u_time * SHADOWSPEED)) * SHADOWCOUNT);
   texcolour.xyz  *= shadow;

   /* Highlights */
   float highlight= HIGHLIGHTRANGE * (0.5 + 0.5 * sin((uv.y + (u_time * -HIGHLIGHTSPEED)) * HIGHLIGHTCOUNT) );
   texcolour.xyz  += highlight;

   // Other effects.
   float x = (uv.x + 4.0) * (uv.y + 4.0) * u_time * 10.0;
   float grain = 1.0 - (mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005) * GRAINSTRENGTH;
   float flicker = max(1.0, random(u_time * uv) * 1.5);
   float scanlines = SCANLINEMEAN + SCANLINEAMPLITUDE*step( 0.5, sin(0.5*screen_coords.y + SCANLINESPEED*u_time)-0.1 );

   texcolour.xyz *= grain * flicker * scanlines * bluetint;
   return texcolour * colour;
}
]], strength )

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
A corruption effect applies a noisy pixelated effect.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" field is used.
--]]
function love_shaders.corruption( params )
   params = params or {}
   local strength = params.strength or 1.0
   local pixelcode = string.format([[
#include "lib/math.glsl"

uniform float u_time;

const int fps        = 15;
const float strength = %f;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px ) {
   float time = u_time - mod( u_time, 1.0 / float(fps) );

   float glitchStep = mix(4.0, 32.0, random(vec2(time)));

   vec4 screenColour = texture( tex, uv );
   uv.x = round(uv.x * glitchStep ) / glitchStep;
   vec4 glitchColour = texture( tex, uv );
   return colour * mix(screenColour, glitchColour, vec4(0.03*strength));
}
]], strength )

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
A rolling steamy effect. Meant as/for backgrounds.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" and "speed" fields is used.
--]]
function love_shaders.steam( params )
   params = params or {}
   local strength = params.strength or 1.0
   local speed = params.speed or 1.0
   local pixelcode = string.format([[
#include "lib/math.glsl"
#include "lib/simplex.glsl"

uniform float u_time;

const float strength = %f;
const float speed    = %f;
const float u_r      = %f;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolour = colour * texture( tex, uv );

   vec2 offset = vec2( 50.0*sin( M_PI*u_time * 0.001 * speed ), -0.3*u_time*speed );

   float n = 0.0;
   for (float i=1.0; i<4.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( offset +  px * strength * 0.0015 * m + 1000.0 * u_r ) * (1.0 / m);
   }

   texcolour.a *= 0.68 + 0.3 * n;

   return colour * texcolour;
}
]], strength, speed, love_math.random() )

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
An electronic circuit-board like shader. Meant as/for backgrounds.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength" and "speed" fields is used.
--]]
function love_shaders.circuit( params )
   params = params or {}
   local strength = params.strength or 1.0
   local speed = params.speed or 1.0
   local pixelcode = string.format([[
#include "lib/math.glsl"

uniform float u_time = 0.0;
uniform vec3 u_camera = vec3( 0.0, 0.0, 1.0 );

const float strength = %f;
const float speed    = %f;
const float u_r      = %f;
const int  NUM_OCTAVES = 3;

/* 1D noise */
float noise1( float p )
{
   float fl = floor(p);
   float fc = fract(p);
   return mix(random(fl), random(fl + 1.0), fc);
}

/* Voronoi distance noise. */
float voronoi( vec2 x )
{
   vec2 p = floor(x);
   vec2 f = fract(x);

   vec2 res = vec2(8.0);
   for(int j = -1; j <= 1; j++) {
      for(int i = -1; i <= 1; i++) {
         vec2 b = vec2(i, j);
         vec2 r = vec2(b) - f + random(p + b);

         /* Chebyshev distance, one of many ways to do this */
         float d = max(abs(r.x), abs(r.y));

         if (d < res.x) {
            res.y = res.x;
            res.x = d;
         }
         else if (d < res.y)
            res.y = d;
      }
   }
   return res.y - res.x;
}

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   /* Calculate coordinates relative to camera. */
   vec2 uv = (texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy + u_r;
   uv *= strength / 500.0; /* Normalize so that strength==1.0 looks fairly good. */

   /* Minor flickering between 0.9 and 1.1. */
   float flicker = noise1( u_time * 2.0 * speed ) * 0.2 + 0.9;

   /* Add some noise octaves */
   float a = 0.6;
   float f = 1.0;

   /* 4 octaves also look nice, its getting a bit slow though */
   float v = 0.0;
   for (int i=0; i < NUM_OCTAVES; i ++) {
      float v1 = voronoi(uv * f + 5.0);
      float v2 = 0.0;

      /* Make the moving electrons-effect for higher octaves. */
      if (i > 0) {
         v2 = voronoi(uv * f * 0.5 + 50.0 + u_time * speed);

         float va = 1.0 - smoothstep(0.0, 0.1,  v1);
         float vb = 1.0 - smoothstep(0.0, 0.08, v2);
         v += a * pow(va * (0.5 + vb), 2.0);
      }

      /* Sharpen the edges. */
      v1 = 1.0 - smoothstep(0.0, 0.3, v1);

      /* Noise is used as intensity map */
      v2 = a * (noise1(v1 * 5.5 + 0.1));

      /* Octave 0's intensity changes a bit */
      if (i == 0)
         v += v2 * flicker;
      else
         v += v2;

      f *= 3.0;
      a *= 0.7;
   }

   /* Blueish colour set */
   vec3 cexp = vec3(6.0, 4.0, 2.0);

   /* Convert to colour, clamp and multiply by base colour. */
   vec3 col = vec3(pow(v, cexp.x), pow(v, cexp.y), pow(v, cexp.z)) * 2.0;
   return colour * vec4( clamp( col, 0.0, 1.0 ), 1.0);
}
]], strength, speed, love_math.random() )

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
A windy type shader. Meant as/for backgrounds, however, it is highly transparent.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength", "speed", and "density" fields is used.
--]]
function love_shaders.windy( params )
   params = params or {}
   local strength = params.strength or 1.0
   local speed = params.speed or 1.0
   local density = params.density or 1.0
   local pixelcode = string.format([[
#include "lib/simplex.glsl"

uniform float u_time = 0.0;
uniform vec3 u_camera = vec3( 0.0, 0.0, 1.0 );

const float strength = %f;
const float speed    = %f;
const float density  = %f;
const float u_r      = %f;

const float noiseScale = 0.001;
const float noiseTimeScale = 0.03;

float fbm3(vec3 v) {
   float result = snoise(v);
   result += snoise(v * 2.0) / 2.0;
   result += snoise(v * 4.0) / 4.0;
   result /= (1.0 + 1.0/2.0 + 1.0/4.0);
   return result;
}

float getNoise(vec3 v) {
   v.xy += vec2( fbm3(v), fbm3(vec3(v.xy, v.z + 1000.0)));
   return fbm3(v) / 2.0 + 0.5;
}

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float f = 0.0;
   vec3 uv;

   /* Calculate coordinates */
   uv.xy = (texture_coords - 0.5) * love_ScreenSize.xy * u_camera.z + u_camera.xy + u_r;
   uv.xy *= strength;
   uv.z  = u_time * speed;

   uv *= vec3( noiseScale, noiseScale, noiseTimeScale );

   float noise = getNoise( uv );
   noise = pow( noise, 4.0 / density ) * 2.0;  //more contrast
   return colour * vec4( 1.0, 1.0, 1.0, noise );
}
]], strength, speed, density, love_math.random() )

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
An aura effect for characters.

The default size is 40 and refers to the standard deviation of the Gaussian blur being applied.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "strength", "speed", "colour", and "size" fields are used.
--]]
function love_shaders.aura( params )
   params = params or {}
   local colour = params.colour or {1, 0, 0}
   local strength = params.strength or 1
   local speed = params.speed or 1
   local size = params.size or 40 -- Gaussian blur sigma
   local pixelcode = string.format([[
#include "lib/math.glsl"
#include "lib/simplex.glsl"
#include "lib/blend.glsl"

uniform float u_time;
uniform Image blurtex;

const vec3 basecolour = vec3( %f, %f, %f );
const float strength = %f;
const float speed = %f;
const float u_r = %f;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 blurcolour = texture( blurtex, uv );

   // Hack to hopefully speed up
   if (blurcolour.a <= 0.0)
      return vec4(0.0);

   vec4 texcolour = texture( tex, uv );
   vec2 offset = vec2( 50.0*sin( M_PI*u_time * 0.001 * speed ), -3.0*u_time*speed );

   float n = 0.0;
   for (float i=1.0; i<4.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( offset +  px * strength * 0.009 * m + 1000.0 * u_r ) * (1.0 / m);
   }
   n = 0.5*n + 0.5;

   blurcolour.a = 1.0-2.0*distance( 0.5, blurcolour.a );
   blurcolour.a *= n;

   texcolour.rgb = blendScreen( texcolour.rgb, basecolour, blurcolour.a );
   texcolour.a = max( texcolour.a, blurcolour.a );
   return colour * texcolour;
}
]], colour[1], colour[2], colour[3], strength, speed, love_math.random() )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader.prerender = function( self, image )
      self._blurtex = love_shaders.blur( image, size )
      self:send( "blurtex", self._blurtex )
      self.prerender = nil -- Run once
   end
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


--[[--
Simple colour modulation shader.

@see shaderparams
@tparam @{shaderparams} params Parameter table where "colour" field is used.
--]]
function love_shaders.colour( params )
   local colour = params.colour or {1, 1, 1, 1}
   colour[4] = colour[4] or 1
   local pixelcode = string.format([[
const vec4 basecolour = vec4( %f, %f, %f, %f );
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolour = Texel(tex, uv);
   return basecolour * colour * texcolour;
}
]], colour[1], colour[2], colour[3], colour[4] )
   local shader = graphics.newShader( pixelcode, _vertexcode )
   return shader
end


return love_shaders
