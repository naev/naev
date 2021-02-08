--[[
-- Run with `love trail`
--]]


local pixelcode = [[
#pragma language glsl3
#define M_PI 3.141592502593994140625

uniform float dt;
uniform int type;
uniform vec2 dimensions;

//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}
float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
      + i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}


/* Has a peak at 1/k */
float impulse( float x, float k )
{
   float h = x*k;
   return h * exp( 1.0 - h );
}

float fastdropoff( float x, float k )
{
   return 1. - pow( max(0.0, abs(1.-x) * 2.0 - 1.0 ), k );
}

/* k is the sharpness, more k == sharper.
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1. - pow( abs( sin( M_PI * x / 2. ) ), k );
}

float sharpbeam( float x, float k )
{
   return pow( min( cos( M_PI * x / 2. ), 1.0 - abs(x) ), k );
}

float random (vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

/* No animation. */
float trail_default( vec2 pos_tex, vec2 dim )
{
   float a, m;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   a *= smoothbeam( pos_tex.y, 3.*m );

   return a;
}

/* Pulsating motion. */
float trail_pulse( vec2 pos_tex, vec2 dim )
{
   float a, m, v;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   a *= smoothbeam( pos_tex.y, 3.*m );

   v = smoothstep( 0., 0.5, 1-pos_tex.x );
   a *=  0.8 + 0.2 * mix( 1, sin( 2*M_PI * (pos_tex.x * 0.03 * dim.x + dt * 3) ), v );

   return a;
}

/* Slow ondulating wave-like movement. */
float trail_wave( vec2 pos_tex, vec2 dim )
{
   float a, m, p, y;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   p = 2*M_PI * (pos_tex.x*5 + dt * 0.5);
   y = pos_tex.y + 0.2 * smoothstep(0, 0.8, 1-pos_tex.x) * sin( p );
   a *= smoothbeam( y, 2.*m );

   return a;
}

/* Flame-like periodic movement. */
float trail_flame( vec2 pos_tex, vec2 dim )
{
   float a, m, p, y;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 30. );

   // Modulate width
   // By multiplying two sine waves with different period it looks more like
   // a natural flame.
   p = 2*M_PI * (pos_tex.x*5 + dt * 5);
   y = pos_tex.y + 0.2 * smoothstep(0, 0.8, 1-pos_tex.x) * sin( p ) * sin( 2.7*p );
   a *= smoothbeam( y, 2.*m );

   return a;
}

/* Starts thin and gets wide. */
float trail_nebula( vec2 pos_tex, vec2 dim )
{
   float a, m;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1 );

   // Modulate alpha based on dispersion
   m = impulse( pos_tex.x, 0.3);

   // Modulate width
   m *= 2-smoothstep( 0., 0.2, 1.-pos_tex.x );
   a *= sharpbeam( pos_tex.y, 3*m );
   a *= 0.2 + 0.8*smoothstep( 0., 0.05, 1.-pos_tex.x );

   return a;
}

/* Somewhat like a lightning arc. */
float trail_arc( vec2 pos_tex, vec2 dim )
{
   float a, m, p, v, s;
   vec2 ncoord;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   //m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 1. );
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 1. );
   m *= 3;

   // Modulate width
   ncoord = vec2( pos_tex.x * 0.03*dim.x, 7*dt );
   s =  0.6 * smoothstep(0, 0.2, 1.-pos_tex.x);
   p = pos_tex.y + s * snoise( ncoord );
   v = sharpbeam( p, m );
   p = pos_tex.y + s * snoise( 1.5*ncoord );
   v += sharpbeam( p, 2*m );
   p = pos_tex.y + s * snoise( 2*ncoord );
   v += sharpbeam( p, 4*m );

   a *= v * 0.6;

   return min(1, a);
}

/* Bubbly effect. */
float trail_bubbles( vec2 pos_tex, vec2 dim )
{
   float a, m, p;
   vec2 coords;

   // Modulate alpha base on length
   a = fastdropoff( pos_tex.x, 1. );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.-pos_tex.x, 3. );

   coords = dim * pos_tex + vec2( 220*dt, 0 );
   p = 0.5 + min( 0.5, snoise( 0.08 * coords ));

   // Modulate width
   a *= p * smoothbeam( pos_tex.y, 3.*m );

   return a;
}


vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = color;
   vec2 pos = texture_coords;
   pos.y = 2*pos.y-1;
   pos.x = 1-pos.x;
   float t = pos.x;
   float a;

   if (type==1)
      a = trail_pulse( pos, dimensions );
   else if (type==2)
      a = trail_wave( pos, dimensions );
   else if (type==3)
      a = trail_flame( pos, dimensions );
   else if (type==4)
      a = trail_nebula( pos, dimensions );
   else if (type==5)
      a = trail_arc( pos, dimensions );
   else if (type==6)
      a = trail_bubbles( pos, dimensions );
   else
      a = trail_default( pos, dimensions );

   color_out.a *= a;

   return color_out;
}
]]

local vertexcode = [[
#pragma language glsl3

uniform float dt;
uniform int type;
uniform vec2 dimensions;

vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]


function love.load()
   love.window.setTitle( "Naev Trail Demo" )
   -- Set up the shader
   shader   = love.graphics.newShader(pixelcode, vertexcode)
   shader_type = 0
   shader:send( "type", shader_type )
   -- We need an image for the shader to work so we create a 1x1 px white image.
   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 1, 1, 1, 1 )
   img      = love.graphics.newImage( idata )
   -- Set the font
   love.graphics.setNewFont( 24 )
end

function love.keypressed(key)
   local num = tonumber(key)
   if num~=nil then
      shader_type = num
      shader:send( "type", shader_type )
   else
      love.event.quit()
   end
end

function love.draw ()
   local x = 20
   local y = 10
   local lg = love.graphics
   lg.setShader()
   lg.setColor( 1, 1, 1, 1 )
   lg.print( string.format("Use the number keys to change between shaders.\nCurrent Shader: %d", shader_type), x, y )
   y = y + 54 + 10
   for _,w in ipairs( {700, 500, 300, 100} ) do
      for _,h in ipairs( {30, 20, 10} ) do
         lg.setColor( 1, 1, 1, 0.5 )
         lg.rectangle( "line", x-2, y-2, w+4, h+4 )
         lg.setShader(shader)
         shader:send( "dimensions", {w,h} )
         lg.setColor( 0, 1, 1, 0.7 )
         lg.draw( img, x, y, 0, w, h)
         y = y + h + 20
      end
   end
end

function love.update( dt )
   global_dt = (global_dt or 0) + dt
   if shader:hasUniform("dt") then
      shader:send( "dt", global_dt )
   end
end

