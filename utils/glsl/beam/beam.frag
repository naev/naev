#pragma language glsl3
#define M_PI 3.141592502593994140625

#define BEAM_FUNC_PROTOTYPE

uniform vec4 color;
uniform vec2 dimensions;
uniform float dt;
uniform float r;
uniform int type;

const float ANIM_SPEED   = 1.0/1.5; /**< Controls the global speed of the animations. */

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

float snoise(vec2 v) {
   const vec4 C = vec4(
      0.211324865405187,  // (3.0-sqrt(3.0))/6.0
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

// All components are in the range [0…1], including hue.
vec3 hsv2rgb( vec3 c )
{
   vec4 k = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
   vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
   c = c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
   c = gammaToLinear(c);
   return c;
}

/* k is the sharpness, more k == smoother
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1. - pow( abs( sin( M_PI * x / 2. ) ), k );
}

float sharpbeam( float x, float k )
{
   return pow( min( cos( M_PI * x / 2. ), 1.0 - abs(x) ), k );
}

float beamfade( float p, float x )
{
   const float fadein_pixels = 7.0;
   if (p < fadein_pixels)
      return p / fadein_pixels;
   return 1.0 - smoothstep( 0.0, 0.2, x-0.8 );
}

BEAM_FUNC_PROTOTYPE
vec4 beam_default( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float m;
   const float range = 0.3;

   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Normal beam
   coords = pos_px / 500.0 + vec2( -3.0*ANIM_SPEED*dt, 0 );
   m = 1.5 + 0.5*snoise( coords );
   float a = smoothbeam( pos_tex.y, m );
   color.rgb = mix( color.rgb, vec3(1.0), 3.0*smoothbeam( pos_tex.y, 0.1 ) );
   color.a *= a;

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_unstable( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float a, m, y;
   const float range = 0.3;

   // Normal beam
   coords = vec2( 3.0*ANIM_SPEED*(dt+r), 0.0 );
   m = 0.3 + 0.3*snoise( coords );

   coords = pos_px/2000.0 + vec2( -3.0*ANIM_SPEED*dt, 1000.0 );
   y = pos_tex.y + 0.2*snoise( coords );
   a = smoothbeam( y, m );
   coords += vec2( 1000., 0.0 );
   y = pos_tex.y + 0.7*snoise( coords );
   a += 0.5*smoothbeam( y, 0.5 );
   coords += vec2( 1000.0, 0.0 );
   y = pos_tex.y + 0.7*snoise( coords );
   a += 0.5*smoothbeam( y, 0.5 );
   a = min( a, 1.0 );
   color.rgb = mix( color.rgb, vec3(1.0), 3.0*smoothbeam( pos_tex.y, 0.1 ) );
   color.a *= a;
   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_fuzzy( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float m, f;
   const float range = 0.3;

   // Normal beam
   coords = vec2( 3.0*ANIM_SPEED*dt, 0.0 );
   m = 4.0 + snoise( coords );
   float a = smoothbeam( pos_tex.y, m );
   color.rgb += 3.0 * a * smoothbeam( pos_tex.y, 0.1 );
   color.a *= a;
   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Perlin noise
   coords = 0.2 * pos_px + vec2( -45.0*ANIM_SPEED*dt, 0.0 ) + 1000.0*r;
   f = cnoise( coords );
   color.a *= 1.0 + 1.0 * f * smoothstep( 0.0, 1.0, abs(pos_tex.y) );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_wave( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float y, p, m;
   vec2 coords;
   const float range = 0.3;

   color.a *= beamfade( pos_px.x, pos_tex.x );

   coords = pos_px / 500.0 + vec2( -3.0*ANIM_SPEED*dt, 0.0 );
   m = 1.5 + 0.5*snoise( coords );
   p = 2.0*M_PI * (pos_px.x/20.0 - 8.0*ANIM_SPEED*dt + r);
   y = pos_tex.y + 0.2 * sin( p );
   color.a *= smoothbeam( y, m );
   color.rgb = mix( color.rgb, vec3(1.0), 4.0*smoothbeam( y, 0.1 ) );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_arc( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, y, v;
   vec2 ncoord;

   // Modulate alpha base on length
   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Modulate alpha based on dispersion
   m = 4.0;

   // Modulate width
   ncoord = vec2( 0.03 * pos_px.x, 7.0*ANIM_SPEED*dt ) + 1000.0 * r;
   y = clamp( pos_tex.y + 0.7 * snoise( ncoord ), -1.0, 1.0 );
   v = sharpbeam( y, m );
   y = clamp( pos_tex.y + 0.7 * snoise( 1.5*ncoord ), -1.0, 1.0 );
   v += sharpbeam( y, 2.0*m );
   y = clamp( pos_tex.y + 0.7 * snoise( 2.0*ncoord ), -1.0, 1.0 );
   v += sharpbeam( y, 4.0*m );

   v = abs(v);
   color.rgb  = mix( color.rgb, vec3(1.0), v*0.5 );
   color.rgb  = pow( color.rgb, vec3(3.0) );
   color.a   *= min(1.0, v);

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_helix( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float y, p, a, c, m, v;
   vec2 coords;
   const float range = 0.3;

   color.a *= beamfade( pos_px.x, pos_tex.x );

   m = 0.6 * (0.8 + 0.2*sin( 2.0*M_PI * 0.5*ANIM_SPEED*dt ) );
   p = 2.0*M_PI * (pos_px.x/40.0 - dt * 6.0 + r);
   // First sine
   y = pos_tex.y + m * sin( p );
   a = smoothbeam( y, 0.8 );
   v = 3.0*smoothbeam( y, 0.1 );
   // Second sine
   y  = pos_tex.y + m * sin( p + M_PI );
   a += smoothbeam( y, 0.8 );
   v += 3.0*smoothbeam( y, 0.1 );
   color.rgb = mix( color.rgb, vec3(1.0), v );
   color.a *= a;

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_organic( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   float m, p;
   vec2 coords;
   const float range = 0.3;

   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Modulate alpha based on dispersion
   m = 3.0;

   coords = pos_px + vec2( -200.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   p = 1.0 - 0.7*cellular2x2( 0.13 * coords ).x;

   // Modulate width
   color.a   *= p * smoothbeam( pos_tex.y, m );
   color.rgb = mix( color.rgb, vec3(1.0), max(0.0, 10.0*(p-0.9)) );
   color.a    = pow( color.a, 1.5 );

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_reverse( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float m;
   const float range = 0.3;

   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Normal beam
   coords = pos_px / 500.0 + vec2( 3.0*ANIM_SPEED*dt, 0 );
   m = 1.5 + 0.5*snoise( coords );
   float a = smoothbeam( pos_tex.y, m );
   color.rgb = mix( color.rgb, vec3(0.0), 3.0*smoothbeam( pos_tex.y, 0.2 ) );
   color.a *= a;

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( 10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

BEAM_FUNC_PROTOTYPE
vec4 beam_rainbow( vec4 color, vec2 pos_tex, vec2 pos_px )
{
   vec2 coords;
   float m;
   const float range = 0.3;

   color.rgb = hsv2rgb( vec3( fract(pos_px.x/200.0 - dt), 0.5, 1.0 ) );
   color.a *= beamfade( pos_px.x, pos_tex.x );

   // Normal beam
   coords = pos_px / 500.0 + vec2( -3.0*ANIM_SPEED*dt, 0 );
   m = 1.5 + 0.5*snoise( coords );
   float a = smoothbeam( pos_tex.y, m );
   color.rgb = mix( color.rgb, vec3(1.0), 3.0*smoothbeam( pos_tex.y, 0.1 ) );
   color.a *= a;

   // Do fancy noise effect
   coords = pos_px * vec2( 0.03, 5.0 ) + vec2( -10.0*ANIM_SPEED*dt, 0.0 ) + 1000.0 * r;
   float v = snoise( coords );
   v = max( 0.0, v-(1.0-range) ) * (2.0/range) - 0.1;
   color.a += v * (1.0 - smoothstep( 0.0, 0.05, pos_tex.x-0.95 ) );

   return color;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out;
   vec2 pos = texture_coords;
   vec2 pos_px = pos * dimensions;
   pos.y = 2*pos.y-1;

   color = gammaToLinear( color );

   if (type==1)
      color_out = beam_wave( color, pos, pos_px );
   else if (type==2)
      color_out = beam_arc( color, pos, pos_px );
   else if (type==3)
      color_out = beam_helix( color, pos, pos_px );
   else if (type==4)
      color_out = beam_organic( color, pos, pos_px );
   else if (type==5)
      color_out = beam_unstable( color, pos, pos_px );
   else if (type==6)
      color_out = beam_fuzzy( color, pos, pos_px );
   else if (type==7)
      color_out = beam_reverse( color, pos, pos_px );
   else if (type==8)
      color_out = beam_rainbow( color, pos ,pos_px );
   else
      color_out = beam_default( color, pos, pos_px );

   return color_out;
}
