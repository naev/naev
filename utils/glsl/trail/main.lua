--[[
-- Run with `love trail`
--]]


local pixelcode = [[
#pragma language glsl3
#define M_PI 3.141592502593994140625

#define TRAIL_FUNC_PROTOTYPE

uniform float dt;
uniform int type;
uniform vec2 pos1;
uniform vec2 pos2;
uniform float r;

const vec3 nebu_col = vec3( 1.0, 1.0, 1.0 );

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
vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec4 permute(vec4 x) {
  return mod289(((x*34.0)+1.0)*x);
}
vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(vec2 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
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

// Cellular noise ("Worley noise") in 2D in GLSL.
// Copyright (c) Stefan Gustavson 2011-04-19. All rights reserved.
// This code is released under the conditions of the MIT license.
// See LICENSE file for details.
// https://github.com/stegu/webgl-noise


// Modulo 7 without a division
vec4 mod7(vec4 x) {
  return x - floor(x * (1.0 / 7.0)) * 7.0;
}

// Cellular noise, returning F1 and F2 in a vec2.
// Speeded up by using 2x2 search window instead of 3x3,
// at the expense of some strong pattern artifacts.
// F2 is often wrong and has sharp discontinuities.
// If you need a smooth F2, use the slower 3x3 version.
// F1 is sometimes wrong, too, but OK for most purposes.
vec2 cellular2x2(vec2 P) {
#define K 0.142857142857 // 1/7
#define K2 0.0714285714285 // K/2
#define jitter 0.8 // jitter 1.0 makes F1 wrong more often
   vec2 Pi = mod289(floor(P));
   vec2 Pf = fract(P);
   vec4 Pfx = Pf.x + vec4(-0.5, -1.5, -0.5, -1.5);
   vec4 Pfy = Pf.y + vec4(-0.5, -0.5, -1.5, -1.5);
   vec4 p = permute(Pi.x + vec4(0.0, 1.0, 0.0, 1.0));
   p = permute(p + Pi.y + vec4(0.0, 0.0, 1.0, 1.0));
   vec4 ox = mod7(p)*K+K2;
   vec4 oy = mod7(floor(p*K))*K+K2;
   vec4 dx = Pfx + jitter*ox;
   vec4 dy = Pfy + jitter*oy;
   vec4 d = dx * dx + dy * dy; // d11, d12, d21 and d22, squared
   // Sort out the two smallest distances
#if 1
   // Cheat and pick only F1
   d.xy = min(d.xy, d.zw);
   d.x = min(d.x, d.y);
   return vec2(sqrt(d.x)); // F1 duplicated, F2 not computed
#else
   // Do it right and find both F1 and F2
   d.xy = (d.x < d.y) ? d.xy : d.yx; // Swap if smaller
   d.xz = (d.x < d.z) ? d.xz : d.zx;
   d.xw = (d.x < d.w) ? d.xw : d.wx;
   d.y = min(d.y, d.z);
   d.y = min(d.y, d.w);
   return sqrt(d.xy);
#endif
}

//
// GLSL textureless classic 2D noise "cnoise",
// with an RSL-style periodic variant "pnoise".
// Author:  Stefan Gustavson (stefan.gustavson@liu.se)
// Version: 2011-08-22
//
// Many thanks to Ian McEwan of Ashima Arts for the
// ideas for permutation and gradient selection.
//
// Copyright (c) 2011 Stefan Gustavson. All rights reserved.
// Distributed under the MIT license. See LICENSE file.
// https://github.com/stegu/webgl-noise
//

// Classic Perlin noise
float cnoise(vec2 P)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod289(Pi); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;

  vec4 i = permute(permute(ix) + iy);

  vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
  vec4 gy = abs(gx) - 0.5 ;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;

  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);

  vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;

  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));

  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
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
TRAIL_FUNC_PROTOTYPE
vec4 trail_default( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.0-pos_tex.x, 30.0 );

   // Modulate width
   color.a *= smoothbeam( pos_tex.y, m );

   return color;
}

/* Pulsating motion. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_pulse( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, v;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.0-pos_tex.x, 30.0 );

   // Modulate width
   color.a *= smoothbeam( pos_tex.y, 2.0*m );

   v = smoothstep( 0.0, 0.5, 1.0-pos_tex.x );
   color.a *=  0.8 + 0.2 * mix( 1.0, sin( 2.0*M_PI * (0.06 * pos_px.x + dt * 3.0) ), v );

   return color;
}

/* Slow ondulating wave-like movement. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_wave( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p, y;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.0-pos_tex.x, 30.0 );

   // Modulate width
   p = 2.0*M_PI * (pos_tex.x*5.0 + dt * 0.5 + r);
   y = pos_tex.y + 0.2 * smoothstep(0.0, 0.8, 1.0-pos_tex.x) * sin( p );
   color.a *= smoothbeam( y, m );

   return color;
}

/* Flame-like periodic movement. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_flame( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p, y;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.0-pos_tex.x, 30.0 );

   // Modulate width
   // By multiplying two sine waves with different period it looks more like
   // a natural flame.
   p = 2.0*M_PI * (pos_tex.x*5.0 + dt * 5.0 + r);
   y = pos_tex.y + 0.2 * smoothstep(0, 0.8, 1.0-pos_tex.x) * sin( p ) * sin( 2.7*p );
   color.a *= smoothbeam( y, m );

   return color;
}

/* Starts thin and gets wide. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_nebula( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   const float SCALAR = pow(2., 4.0/3.0 );
   float m, f;
   vec2 coords;

   color.rgb = nebu_col;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = impulse( pos_tex.x, 0.3);

   // Modulate width
   m *= 2.0-smoothstep( 0.0, 0.2, 1.0-pos_tex.x );
   color.a *= sharpbeam( pos_tex.y, 5.0*m );
   color.a *= 0.2 + 0.8*smoothstep( 0.0, 0.05, 1.0-pos_tex.x );

   // We only do two iterations here (turbulence noise)
   coords = 0.02 * pos_px + vec2( dt, 0.0 ) + 1000.0*r;
   f  = abs( cnoise( coords * SCALAR ) );
   f += abs( cnoise( coords * pow(SCALAR,2.0) ) );
   color.a *= 0.5 + 0.7*f;

   // Smoother end trails
   if (pos_tex.x < 0.1) {
      float offy = pow(abs(pos_tex.y),2.0);
      color.a *= smoothstep( offy, 1.0, pow(pos_tex.x / 0.1, 0.5) );
   }

   return color;
}

/* Somewhat like a lightning arc. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_arc( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p, v, s;
   vec2 ncoord;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 1.5 + 5.0*impulse( 1.0-pos_tex.x, 1.0 );

   // Create three beams with varying parameters
   ncoord = vec2( 0.03 * pos_px.x, 7.0*dt ) + 1000.0 * r;
   s =  0.6 * smoothstep(0.0, 0.2, 1.0-pos_tex.x);
   p = clamp( pos_tex.y + s * snoise( ncoord ), -1.0, 1.0 );
   v = sharpbeam( p, m );
   p = clamp( pos_tex.y + s * snoise( 1.5*ncoord ), -1.0, 1.0 );
   v += sharpbeam( p, 2.0*m );
   p = clamp( pos_tex.y + s * snoise( 2.0*ncoord ), -1.0, 1.0 );
   v += sharpbeam( p, 4.0*m );

   v = abs(v);
   s = s + 0.1;
   color.rgb  = mix( color.rgb, vec3(1.0), pow(s*v*0.8, 3.0) );
   color.a   *= min(1.0, v);

   return color;
}

/* Bubbly effect. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_bubbles( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   const float speed = 16.0;   // How fast the trail moves
   const float scale = 0.13;  // Noise scaling (sharpness)
   float m, p;

   // Modulate alpha base on length
   color.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 0.5 + 0.5*impulse( 1.0-pos_tex.x, 3.0 );

   // Compute the noise
   p = 1.0 - 0.7*cellular2x2( scale * pos_px + vec2( speed*dt, 0.0 ) + 1000.0 * r ).x;

   // Modulate width
   color.a   *= p * smoothbeam( pos_tex.y, 2.0*m );
   color.rgb *= 1.0 + max(0.0, 10.0*(p-0.8));

   return color;
}

/* Modulated with multiple wavy beams. */
TRAIL_FUNC_PROTOTYPE
vec4 trail_split( vec4 colour, vec2 pos_tex, vec2 pos_px )
{
   float m, v, y, p, s;
   vec2 ncoord;

   // Modulate alpha base on length
   colour.a *= fastdropoff( pos_tex.x, 1.0 );

   // Modulate alpha based on dispersion
   m = 1.5 + 5.0*impulse( 1.0-pos_tex.x, 1.0 );

   // Create three beams with varying parameters
   ncoord = vec2( 0.03 * pos_px.x, 5.0*dt ) + 1000.0 * r;
   s =  0.6 * smoothstep(0.0, 0.175, 1.0-pos_tex.x);
   p = 2.0*M_PI * (pos_tex.x*5.0 + dt * 0.5 + r + (snoise( ncoord ) - 0.5)/8.0);
   y = clamp( pos_tex.y + s * sin( p ), -1.0, 1.0 );
   v += sharpbeam( y, m );
   p = 2.0*M_PI * (pos_tex.x*5.0 + dt * 0.5 + r + 1.0/3.0 + (snoise( 1.5*ncoord ) - 0.5)/8.0);
   y = clamp( pos_tex.y + s * sin( p ), -1.0, 1.0 );
   v += sharpbeam( y, m );
   p = 2.0*M_PI * (pos_tex.x*5.0 + dt * 0.5 + r + 2.0/3.0 + (snoise( 2.0*ncoord ) - 0.5)/8.0);
   y = clamp( pos_tex.y + s * sin( p ), -1.0, 1.0 );
   v += sharpbeam( y, m );

   // Modulate width
   v = abs(v);
   colour.a *= min(1.0, smoothbeam( pos_tex.y, m ) * (0.5 + 0.5*v));
   colour.rgb  = mix( colour.rgb, vec3(1.0), min(pow((1.0-s)*v*0.4, 2.0), 0.25) );

   return colour;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out;
   vec2 pos = texture_coords;
   vec2 pos_px = mix( pos1, pos2, pos );
   pos.y = 2*pos.y-1;
   pos.x = 1-pos.x;

   color = gammaToLinear( color );

   if (type==1)
      color_out = trail_pulse( color, pos, pos_px );
   else if (type==2)
      color_out = trail_wave( color, pos, pos_px );
   else if (type==3)
      color_out = trail_flame( color, pos, pos_px );
   else if (type==4)
      color_out = trail_nebula( color, pos, pos_px );
   else if (type==5)
      color_out = trail_arc( color, pos, pos_px );
   else if (type==6)
      color_out = trail_bubbles( color, pos, pos_px );
   else if (type==7)
      color_out = trail_split( color, pos, pos_px );
   else
      color_out = trail_default( color, pos, pos_px );

   return color_out;
}
]]

local vertexcode = [[
#pragma language glsl3

vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

local img, scaling, shader, shader_color, shader_type

local function set_shader( num )
   shader_type = num
   shader:send( "type", shader_type )

   if shader_type == 1 then -- zalek
      shader_color = { 1, 0.3, 0.3, 0.7 }
   elseif shader_type == 2 then -- sirius
      shader_color = { 1, 0.9, 0.6, 0.7 }
   elseif shader_type == 3 then -- fire
      shader_color = { 1, 0.84, 0, 0.7 }
   elseif shader_type == 4 then -- nebula
      shader_color = { 0.4, 0.4, 0.9, 0.7 }
   elseif shader_type == 5 then -- pirate
      shader_color = { 0.9, 0.1, 0.4, 0.7 }
   elseif shader_type == 6 then -- soromid
      shader_color = { 0.5, 0.9, 0.2, 0.7 }
   else -- default
      shader_color = { 0, 1, 1, 0.7 }
   end
end

function love.load()
   love.window.setTitle( "Naev Trail Demo" )
   -- Set up the shader
   shader   = love.graphics.newShader(pixelcode, vertexcode)
   set_shader( 0 )
   -- We need an image for the shader to work so we create a 1x1 px white image.
   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 1, 1, 1, 1 )
   img      = love.graphics.newImage( idata )
   -- Set the font
   love.graphics.setNewFont( 24 )
   -- Scaling
   scaling = 2
   love.graphics.setBackgroundColor( 0.5, 0.5, 0.5, 1 )
end

function love.keypressed(key)
   local num = tonumber(key)
   if num~=nil then
      set_shader( num )
   elseif key=="q" or key=="escape" then
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
   local s = scaling
   love.graphics.scale( s, s )
   y = y/s
   for _,w in ipairs( {350, 250, 150, 50} ) do
      for _,h in ipairs( {15, 10, 5} ) do
         lg.setColor( 1, 1, 1, 0.5 )
         lg.rectangle( "line", x-2, y-2, w+4, h+4 )
         lg.setShader(shader)
         shader:send( "r", 0 )
         shader:send( "pos1", {w,h} )
         shader:send( "pos2", {0,0} )
         lg.setColor( shader_color )
         lg.draw( img, x, y, 0, w, h)
         y = y + h + 20/s
      end
   end
end

local global_dt = 0
function love.update( dt )
   global_dt = global_dt + dt
   if shader:hasUniform("dt") then
      shader:send( "dt", global_dt )
   end
end
