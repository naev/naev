local pixelcode_noise = [[
#pragma language glsl3
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

/* Taken from /usr/include/math.h */
const float M_E         = 2.7182818284590452354;   /* e */
const float M_LOG2E     = 1.4426950408889634074;   /* log_2 e */
const float M_LOG10E    = 0.43429448190325182765;  /* log_10 e */
const float M_LN2       = 0.69314718055994530942;  /* log_e 2 */
const float M_LN10      = 2.30258509299404568402;  /* log_e 10 */
const float M_PI        = 3.14159265358979323846;  /* pi */
const float M_PI_2      = 1.57079632679489661923;  /* pi/2 */
const float M_PI_4      = 0.78539816339744830962;  /* pi/4 */
const float M_1_PI      = 0.31830988618379067154;  /* 1/pi */
const float M_2_PI      = 0.63661977236758134308;  /* 2/pi */
const float M_2_SQRTPI  = 1.12837916709551257390;  /* 2/sqrt(pi) */
const float M_SQRT2     = 1.41421356237309504880;  /* sqrt(2) */
const float M_SQRT1_2   = 0.70710678118654752440;  /* 1/sqrt(2) */

// Modulo 289 without a division (only multiplications)
vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

// Modulo 7 without a division
vec4 mod7(vec4 x) {
  return x - floor(x * (1.0 / 7.0)) * 7.0;
}

// Permutation polynomial: (34x^2 + x) mod 289
vec4 permute(vec4 x) {
  return mod289((34.0 * x + 1.0) * x);
}
vec3 permute(vec3 x) {
  return mod289((34.0 * x + 1.0) * x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec3 fade(vec3 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}
vec2 fade(vec2 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}

// http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float random(vec2 co)
{
   const highp float seed = 12.9898;
   highp float a = seed;
   highp float b = 78.233;
   highp float c = 43758.5453;
   highp float dt= dot(co.xy ,vec2(a,b));
   highp float sn= mod(dt,M_PI);
   return fract(sin(sn) * c);
}

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

// Classic Perlin noise, periodic variant
float pnoise(vec2 P, vec2 rep)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, rep.xyxy); // To create noise with explicit period
  Pi = mod289(Pi);        // To avoid truncation effects in permutation
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

// Classic Perlin noise
float cnoise(vec3 P)
{
   vec3 Pi0 = floor(P); // Integer part for indexing
   vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
   Pi0 = mod289(Pi0);
   Pi1 = mod289(Pi1);
   vec3 Pf0 = fract(P); // Fractional part for interpolation
   vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
   vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
   vec4 iy = vec4(Pi0.yy, Pi1.yy);
   vec4 iz0 = Pi0.zzzz;
   vec4 iz1 = Pi1.zzzz;

   vec4 ixy = permute(permute(ix) + iy);
   vec4 ixy0 = permute(ixy + iz0);
   vec4 ixy1 = permute(ixy + iz1);

   vec4 gx0 = ixy0 * (1.0 / 7.0);
   vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
   gx0 = fract(gx0);
   vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
   vec4 sz0 = step(gz0, vec4(0.0));
   gx0 -= sz0 * (step(0.0, gx0) - 0.5);
   gy0 -= sz0 * (step(0.0, gy0) - 0.5);

   vec4 gx1 = ixy1 * (1.0 / 7.0);
   vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
   gx1 = fract(gx1);
   vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
   vec4 sz1 = step(gz1, vec4(0.0));
   gx1 -= sz1 * (step(0.0, gx1) - 0.5);
   gy1 -= sz1 * (step(0.0, gy1) - 0.5);

   vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
   vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
   vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
   vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
   vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
   vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
   vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
   vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

   vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
   g000 *= norm0.x;
   g010 *= norm0.y;
   g100 *= norm0.z;
   g110 *= norm0.w;
   vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
   g001 *= norm1.x;
   g011 *= norm1.y;
   g101 *= norm1.z;
   g111 *= norm1.w;

   float n000 = dot(g000, Pf0);
   float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
   float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
   float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
   float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
   float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
   float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
   float n111 = dot(g111, Pf1);

   vec3 fade_xyz = fade(Pf0);
   vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
   vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
   float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
   return 2.2 * n_xyz;
}

// Classic Perlin noise, periodic variant
float pnoise(vec3 P, vec3 rep)
{
   vec3 Pi0 = mod(floor(P), rep); // Integer part, modulo period
   vec3 Pi1 = mod(Pi0 + vec3(1.0), rep); // Integer part + 1, mod period
   Pi0 = mod289(Pi0);
   Pi1 = mod289(Pi1);
   vec3 Pf0 = fract(P); // Fractional part for interpolation
   vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
   vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
   vec4 iy = vec4(Pi0.yy, Pi1.yy);
   vec4 iz0 = Pi0.zzzz;
   vec4 iz1 = Pi1.zzzz;

   vec4 ixy = permute(permute(ix) + iy);
   vec4 ixy0 = permute(ixy + iz0);
   vec4 ixy1 = permute(ixy + iz1);

   vec4 gx0 = ixy0 * (1.0 / 7.0);
   vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
   gx0 = fract(gx0);
   vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
   vec4 sz0 = step(gz0, vec4(0.0));
   gx0 -= sz0 * (step(0.0, gx0) - 0.5);
   gy0 -= sz0 * (step(0.0, gy0) - 0.5);

   vec4 gx1 = ixy1 * (1.0 / 7.0);
   vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
   gx1 = fract(gx1);
   vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
   vec4 sz1 = step(gz1, vec4(0.0));
   gx1 -= sz1 * (step(0.0, gx1) - 0.5);
   gy1 -= sz1 * (step(0.0, gy1) - 0.5);

   vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
   vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
   vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
   vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
   vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
   vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
   vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
   vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

   vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
   g000 *= norm0.x;
   g010 *= norm0.y;
   g100 *= norm0.z;
   g110 *= norm0.w;
   vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
   g001 *= norm1.x;
   g011 *= norm1.y;
   g101 *= norm1.z;
   g111 *= norm1.w;

   float n000 = dot(g000, Pf0);
   float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
   float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
   float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
   float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
   float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
   float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
   float n111 = dot(g111, Pf1);

   vec3 fade_xyz = fade(Pf0);
   vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
   vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
   float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
   return 2.2 * n_xyz;
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

float snoise(vec3 v)
{
   const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
   const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

   // First corner
   vec3 i  = floor(v + dot(v, C.yyy) );
   vec3 x0 =   v - i + dot(i, C.xxx) ;

   // Other corners
   vec3 g = step(x0.yzx, x0.xyz);
   vec3 l = 1.0 - g;
   vec3 i1 = min( g.xyz, l.zxy );
   vec3 i2 = max( g.xyz, l.zxy );

   //   x0 = x0 - 0.0 + 0.0 * C.xxx;
   //   x1 = x0 - i1  + 1.0 * C.xxx;
   //   x2 = x0 - i2  + 2.0 * C.xxx;
   //   x3 = x0 - 1.0 + 3.0 * C.xxx;
   vec3 x1 = x0 - i1 + C.xxx;
   vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
   vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

   // Permutations
   i = mod289(i);
   vec4 p = permute( permute( permute(
               i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
         + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

   // Gradients: 7x7 points over a square, mapped onto an octahedron.
   // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
   float n_ = 0.142857142857; // 1.0/7.0
   vec3  ns = n_ * D.wyz - D.xzx;

   vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

   vec4 x_ = floor(j * ns.z);
   vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

   vec4 x = x_ *ns.x + ns.yyyy;
   vec4 y = y_ *ns.x + ns.yyyy;
   vec4 h = 1.0 - abs(x) - abs(y);

   vec4 b0 = vec4( x.xy, y.xy );
   vec4 b1 = vec4( x.zw, y.zw );

   //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
   //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
   vec4 s0 = floor(b0)*2.0 + 1.0;
   vec4 s1 = floor(b1)*2.0 + 1.0;
   vec4 sh = -step(h, vec4(0.0));

   vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
   vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

   vec3 p0 = vec3(a0.xy,h.x);
   vec3 p1 = vec3(a0.zw,h.y);
   vec3 p2 = vec3(a1.xy,h.z);
   vec3 p3 = vec3(a1.zw,h.w);

   //Normalise gradients
   vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
   p0 *= norm.x;
   p1 *= norm.y;
   p2 *= norm.z;
   p3 *= norm.w;

   // Mix final noise value
   vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
   m = m * m;
   return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
            dot(p2,x2), dot(p3,x3) ) );
}
]]


local pixelcode_nebula = [[
const int ITERATIONS = 3;
const float SCALAR = pow(2., 4./3.);

uniform vec4 color;
uniform mat4 projection;
uniform float eddy_scale = 50.0;
uniform float u_time = 0.0;
out vec4 color_out;

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   uv.xy = screen_coords / eddy_scale;
   uv.z = u_time / 5.0;

   // Create the noise
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
      f += abs( cnoise( uv * scale ) ) / scale;
   }
   return mix( vec4(0.0,0.0,0.0,1.0), color, .1 + f );
}
]]

local pixelcode_wind = [[
uniform float u_time = 0.0;
uniform float eddy_scale = 50.0;
out vec4 color_out;

const int noiseSwirlSteps = 2; // 2
const float noiseSwirlValue = 1.; // 1.
const float noiseSwirlStepValue = noiseSwirlValue / float(noiseSwirlSteps);

const float noiseScale = 0.5; // 0.5
const float noiseTimeScale = 0.1; // 0.1

float fbm3(vec3 v) {
   float result = snoise(v);
   result += snoise(v * 2.) / 2.;
   result += snoise(v * 4.) / 4.;
   result /= (1. + 1./2. + 1./4.);
   return result;
}

float fbm5(vec3 v) {
   float result = snoise(v);
   result += snoise(v * 2.) / 2.;
   result += snoise(v * 4.) / 4.;
   result += snoise(v * 8.) / 8.;
   result += snoise(v * 16.) / 16.;
   result /= (1. + 1./2. + 1./4. + 1./8. + 1./16.);
   return result;
}

float getNoise(vec3 v) {
   //  make it curl
   for (int i=0; i<noiseSwirlSteps; i++) {
      v.xy += vec2(fbm3(v), fbm3(vec3(v.xy, v.z + 1000.))) * noiseSwirlStepValue;
   }
   //  normalize
   return fbm5(v) / 2. + 0.5;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float f = 0.0;
   vec3 uv;

   // Calculate coordinates
   uv.xy = screen_coords / eddy_scale;
   uv.z = u_time;

   uv *= vec3( noiseScale, noiseScale, noiseTimeScale );

   float noise = getNoise( uv );
   noise = pow( noise, 4.0 ) * 2.0;  //more contrast
   return vec4( 0.2, 0.9, 1.0, noise );
   //return vec4(noise, noise, noise, 1.0);
}
]]

local pixelcode_digital = [[
uniform float u_time = 0.0;
uniform float eddy_scale = 50.0;
out vec4 color_out;
// by srtuss, 2013
// https://www.shadertoy.com/view/4sl3Dr

// rotate position around axis
vec2 rotate(vec2 p, float a)
{
	return vec2(p.x * cos(a) - p.y * sin(a), p.x * sin(a) + p.y * cos(a));
}

// 1D random numbers
float rand(float n)
{
    return fract(sin(n) * 43758.5453123);
}

// 2D random numbers
vec2 rand2(in vec2 p)
{
	return fract(vec2(sin(p.x * 591.32 + p.y * 154.077), cos(p.x * 391.32 + p.y * 49.077)));
}

// 1D noise
float noise1(float p)
{
	float fl = floor(p);
	float fc = fract(p);
	return mix(rand(fl), rand(fl + 1.0), fc);
}

// voronoi distance noise, based on iq's articles
float voronoi(in vec2 x)
{
	vec2 p = floor(x);
	vec2 f = fract(x);
	
	vec2 res = vec2(8.0);
	for(int j = -1; j <= 1; j ++)
	{
		for(int i = -1; i <= 1; i ++)
		{
			vec2 b = vec2(i, j);
			vec2 r = vec2(b) - f + rand2(p + b);
			
			// chebyshev distance, one of many ways to do this
			float d = max(abs(r.x), abs(r.y));
			
			if(d < res.x)
			{
				res.y = res.x;
				res.x = d;
			}
			else if(d < res.y)
			{
				res.y = d;
			}
		}
	}
	return res.y - res.x;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv;

   // Calculate coordinates

   float flicker = noise1(u_time * 2.0) * 0.8 + 0.4;

   uv = screen_coords / eddy_scale / 10.;
	uv = (uv - 0.5) * 2.0;
	vec2 suv = uv;
	//uv.x *= iResolution.x / iResolution.y;
	
	
	float v = 0.0;
	
	// that looks highly interesting:
	//v = 1.0 - length(uv) * 1.3;
	
	
	// a bit of camera movement
   /*
	uv *= 0.6 + sin(u_time * 0.1) * 0.4;
	uv = rotate(uv, sin(u_time * 0.3) * 1.0);
	uv += u_time * 0.4;
   */
	
	// add some noise octaves
	float a = 0.6, f = 1.0;
	
	for(int i = 0; i < 3; i ++) // 4 octaves also look nice, its getting a bit slow though
	{	
		float v1 = voronoi(uv * f + 5.0);
		float v2 = 0.0;
		
		// make the moving electrons-effect for higher octaves
		if(i > 0)
		{
			// of course everything based on voronoi
			v2 = voronoi(uv * f * 0.5 + 50.0 + u_time);
			
			float va = 0.0, vb = 0.0;
			va = 1.0 - smoothstep(0.0, 0.1, v1);
			vb = 1.0 - smoothstep(0.0, 0.08, v2);
			v += a * pow(va * (0.5 + vb), 2.0);
		}
		
		// make sharp edges
		v1 = 1.0 - smoothstep(0.0, 0.3, v1);
		
		// noise is used as intensity map
		v2 = a * (noise1(v1 * 5.5 + 0.1));
		
		// octave 0's intensity changes a bit
		if(i == 0)
			v += v2 * flicker;
		else
			v += v2;
		
		f *= 3.0;
		a *= 0.7;
	}

	// slight vignetting
	//v *= exp(-0.6 * length(suv)) * 1.2;
	
	// use texture channel0 for color? why not.
	//vec3 cexp = texture(iChannel0, uv * 0.001).xyz * 3.0 + texture(iChannel0, uv * 0.01).xyz;//vec3(1.0, 2.0, 4.0);
	//cexp *= 1.4;
	
	// old blueish color set
	vec3 cexp = vec3(6.0, 4.0, 2.0);
	
	vec3 col = vec3(pow(v, cexp.x), pow(v, cexp.y), pow(v, cexp.z)) * 2.0;
	
	return vec4(col, 1.0);
}
]]

local pixelcode_marble = [[
// https://www.shadertoy.com/view/Xs3fR4
uniform float u_time = 0.0;
uniform float eddy_scale = 50.0;
out vec4 color_out;
// variant of Vorocracks: https://shadertoy.com/view/lsVyRy
// integrated with cracks here: https://www.shadertoy.com/view/Xd3fRN

#define MM 0

#define VARIANT 1              // 1: amplifies Voronoi cell jittering
#if VARIANT
      float ofs = .5;          // jitter Voronoi centers in -ofs ... 1.+ofs
#else
      float ofs = 0.;
#endif

//int FAULT = 1;                 // 0: crest 1: fault

float RATIO = 1.,              // stone length/width ratio
 /*   STONE_slope = .3,        // 0.  .3  .3  -.3
      STONE_height = 1.,       // 1.  1.  .6   .7
      profile = 1.,            // z = height + slope * dist ^ prof
 */
      CRACK_depth = 3.,
      CRACK_zebra_scale = 1.,  // fractal shape of the fault zebra
      CRACK_zebra_amp = .67,
      CRACK_profile = 1.,      // fault vertical shape  1.  .2
      CRACK_slope = 50.,       //                      10.  1.4
      CRACK_width = .0;


// std int hash, inspired from https://www.shadertoy.com/view/XlXcW4
vec3 hash3( uvec3 x )
{
#   define scramble  x = ( (x>>8U) ^ x.yzx ) * 1103515245U // GLIB-C const
    scramble; scramble; scramble;
    return vec3(x) / float(0xffffffffU) + 1e-30; // <- eps to fix a windows/angle bug
}

// === Voronoi =====================================================
// --- Base Voronoi. inspired by https://www.shadertoy.com/view/MslGD8

#define hash22(p)  fract( 18.5453 * sin( p * mat2(127.1,311.7,269.5,183.3)) )
#define disp(p) ( -ofs + (1.+2.*ofs) * hash22(p) )

vec3 voronoi( vec2 u )  // returns len + id
{
    vec2 iu = floor(u), v;
	float m = 1e9,d;
#if VARIANT
    for( int k=0; k < 25; k++ ) {
        vec2  p = iu + vec2(k%5-2,k/5-2),
#else
    for( int k=0; k < 9; k++ ) {
        vec2  p = iu + vec2(k%3-1,k/3-1),
#endif
            o = disp(p),
      	      r = p - u + o;
		d = dot(r,r);
        if( d < m ) m = d, v = r;
    }

    return vec3( sqrt(m), v+u );
}

// --- Voronoi distance to borders. inspired by https://www.shadertoy.com/view/ldl3W8
vec3 voronoiB( vec2 u )  // returns len + id
{
    vec2 iu = floor(u), C, P;
	float m = 1e9,d;
#if VARIANT
    for( int k=0; k < 25; k++ ) {
        vec2  p = iu + vec2(k%5-2,k/5-2),
#else
    for( int k=0; k < 9; k++ ) {
        vec2  p = iu + vec2(k%3-1,k/3-1),
#endif
              o = disp(p),
      	      r = p - u + o;
		d = dot(r,r);
        if( d < m ) m = d, C = p-iu, P = r;
    }

    m = 1e9;

    for( int k=0; k < 25; k++ ) {
        vec2 p = iu+C + vec2(k%5-2,k/5-2),
		     o = disp(p),
             r = p-u + o;

        if( dot(P-r,P-r)>1e-5 )
        m = min( m, .5*dot( (P+r), normalize(r-P) ) );
    }

    return vec3( m, P+u );
}

// === pseudo Perlin noise =============================================
#define rot(a) mat2(cos(a),-sin(a),sin(a),cos(a))
int MOD = 1;  // type of Perlin noise

// --- 2D
#define hash21(p) fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453123)
float nnoise2(vec2 p) {
   vec2 i = floor(p);
   vec2 f = fract(p); f = f*f*(3.-2.*f); // smoothstep

   float v= mix( mix(hash21(i+vec2(0,0)),hash21(i+vec2(1,0)),f.x),
         mix(hash21(i+vec2(0,1)),hash21(i+vec2(1,1)),f.x), f.y);
   return   MOD==0 ? v
      : MOD==1 ? 2.*v-1.
      : MOD==2 ? abs(2.*v-1.)
      : 1.-abs(2.*v-1.);
}

float fbm2(vec2 p) {
    float v = 0.,  a = .5;
    mat2 R = rot(.37);

    for (int i = 0; i < 9; i++, p*=2.,a/=2.)
        p *= R,
        v += a * nnoise2(p);

    return v;
}
#define nnoise22(p) vec2(nnoise2(p),nnoise2(p+17.7))
vec2 fbm22(vec2 p) {
    vec2 v = vec2(0);
    float a = .5;
    mat2 R = rot(.37);

    for (int i = 0; i < 6; i++, p*=2.,a/=2.)
        p *= R,
        v += a * nnoise22(p);

    return v;
}
vec2 mfbm22(vec2 p) {  // multifractal fbm
    vec2 v = vec2(1);
    float a = .5;
    mat2 R = rot(.37);

    for (int i = 0; i < 6; i++, p*=2.,a/=2.)
        p *= R,
        //v *= 1.+nnoise22(p);
          v += v * a * nnoise22(p);

    return v-1.;
}

/*
// --- 3D
#define hash31(p) fract(sin(dot(p,vec3(127.1,311.7, 74.7)))*43758.5453123)
float noise3(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p); f = f*f*(3.-2.*f); // smoothstep

    float v= mix( mix( mix(hash31(i+vec3(0,0,0)),hash31(i+vec3(1,0,0)),f.x),
                       mix(hash31(i+vec3(0,1,0)),hash31(i+vec3(1,1,0)),f.x), f.y),
                  mix( mix(hash31(i+vec3(0,0,1)),hash31(i+vec3(1,0,1)),f.x),
                       mix(hash31(i+vec3(0,1,1)),hash31(i+vec3(1,1,1)),f.x), f.y), f.z);
	return   MOD==0 ? v
	       : MOD==1 ? 2.*v-1.
           : MOD==2 ? abs(2.*v-1.)
                    : 1.-abs(2.*v-1.);
}

float fbm3(vec3 p) {
    float v = 0.,  a = .5;
    mat2 R = rot(.37);

    for (int i = 0; i < 9; i++, p*=2.,a/=2.)
        p.xy *= R, p.yz *= R,
        v += a * noise3(p);

    return v;
}
*/

// ======================================================

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 O;
   vec2 U;

   U = screen_coords / eddy_scale / 10.;

    //U *= 4./iResolution.y;
    //U.x += iTime;                                     // for demo
    U *= 4.;
    U.x += u_time;
 // O = vec4( 1.-voronoiB(U).x,voronoi(U).x, 0,0 );   // for tests
    vec2 I = floor(U/2.);
    bool vert = mod(I.x+I.y,2.)==0.; //if (vert) U = U.yx;
    vec3 H0;
    O-=O;

    for(float i=0.; i<CRACK_depth ; i++) {
        vec2 V =  U / vec2(RATIO,1),                  // voronoi cell shape
             D = CRACK_zebra_amp * fbm22(U/CRACK_zebra_scale) * CRACK_zebra_scale;
        vec3  H = voronoiB( V + D ); if (i==0.) H0=H;
        float d = H.x;                                // distance to cracks
   /*         r = voronoi(V).x,                       // distance to center
              s = STONE_height-STONE_slope*pow(r,profile);// stone interior
    */                                                // cracks
        d = min( 1., CRACK_slope * pow(max(0.,d-CRACK_width),CRACK_profile) );

        O += vec4(1.-d) / exp2(i);
        U *= 1.5 * rot(.37);
    }
    /*
    O = vec4(
        FAULT==1 ? d * s                              // fault * stone
                 : mix(1.,s, d)                       // crest or stone
            ); */
    if (vert) O = 1.-O; O *= vec4(.9,.85,.85,1);      // for demo

#if MM
    O.g = hash3(uvec3(H0.yz,1)).x;
#endif
    return O;
}
]]

local pixelcode_electric = [[
uniform float u_time = 0.0;
uniform float eddy_scale = 50.0;
out vec4 color_out;
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// https://www.shadertoy.com/view/4tySWK

#define time_         u_time
#define detail_steps_ 1 // 13

#define mod3_      vec3(.1031, .11369, .13787)

vec3  hash3_3(vec3 p3);
float perlin_noise3(vec3 p);
float noise_sum_abs3(vec3 p);

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   //vec2 p = screen_coords / eddy_scale;
   // re2.*uv.xy-res_.xy) / res_.y*s;
   //vec2 p = screen_coords / eddy_scale * 3.;
   //p += -10;
   //vec2 p = (texture_coords-0.5) * 5.;
   vec2 p = screen_coords / 150.;

   float electric_density = 1.1; // 0.9
   float electric_radius  = length(texture_coords*2.-1.) - 0.1;
   float velocity = 0.5;

   vec3  electric_local_domain = vec3(p, u_time * velocity);
   float electric_field = electric_density * noise_sum_abs3(electric_local_domain);

   vec3 col = vec3(107, 148, 196) / 255.;
   col += (1. - (electric_field + electric_radius));
   //col += (1. - (electric_field));
   for(int i = 0; i < detail_steps_; i++) {
      if(length(col) >= 2.1 + float(i) / 2.)
         col -= .3;
   }
   col += 1. - 4.2*electric_field;

   float alpha = 1.;
   return vec4(col, alpha);
}


vec3 hash3_3(vec3 p3) {
	p3 = fract(p3 * mod3_);
    p3 += dot(p3, p3.yxz + 19.19);
    return -1. + 2. * fract(vec3((p3.x + p3.y) * p3.z, (p3.x+p3.z) * p3.y, (p3.y+p3.z) * p3.x));
}

float perlin_noise3(vec3 p) {
    vec3 pi = floor(p);
    vec3 pf = p - pi;

    vec3 w = pf * pf * (3. - 2. * pf);

    return 	mix(
    	mix(
            mix(
                dot(pf - vec3(0, 0, 0), hash3_3(pi + vec3(0, 0, 0))),
                dot(pf - vec3(1, 0, 0), hash3_3(pi + vec3(1, 0, 0))),
                w.x),
            mix(
                dot(pf - vec3(0, 0, 1), hash3_3(pi + vec3(0, 0, 1))),
                dot(pf - vec3(1, 0, 1), hash3_3(pi + vec3(1, 0, 1))),
                w.x),
    	w.z),
        mix(
            mix(
                dot(pf - vec3(0, 1, 0), hash3_3(pi + vec3(0, 1, 0))),
                dot(pf - vec3(1, 1, 0), hash3_3(pi + vec3(1, 1, 0))),
                w.x),
            mix(
                dot(pf - vec3(0, 1, 1), hash3_3(pi + vec3(0, 1, 1))),
                dot(pf - vec3(1, 1, 1), hash3_3(pi + vec3(1, 1, 1))),
                w.x),
     	w.z),
	w.y);
}


float noise_sum_abs3(vec3 p) {
    float f = 0.;
    p = p * 3.;
    f += 1.0000 * abs(perlin_noise3(p)); p = 2. * p;
    f += 0.5000 * abs(perlin_noise3(p)); p = 3. * p;
	f += 0.2500 * abs(perlin_noise3(p)); p = 4. * p;
	f += 0.1250 * abs(perlin_noise3(p)); p = 5. * p;
	f += 0.0625 * abs(perlin_noise3(p)); p = 6. * p;

    return f;
}
]]

local vertexcode = [[
#pragma language glsl3
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

function set_shader( num )
   shader_type = num
   --shader:send( "type", shader_type )
end

function love.load()
   love.window.setTitle( "Naev Overlay Demo" )
   love.window.setMode( 800, 450 )
   --love.window.setMode( 0, 0, {fullscreen = true} )
   -- Set up the shader
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_wind, vertexcode)
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_digital, vertexcode)
   --shader   = love.graphics.newShader( pixelcode_noise..pixelcode_marble, vertexcode)
   shader   = love.graphics.newShader( pixelcode_noise..pixelcode_electric, vertexcode)
   set_shader( 0 )
   -- We need an image for the shader to work so we create a 1x1 px white image.
   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 1, 1, 1, 1 )
   img      = love.graphics.newImage( idata )
   -- Set the font
   love.graphics.setNewFont( 24 )
end

function love.keypressed(key)
   if key=="q" or key=="escape" then
      love.event.quit()
   end
end

function love.draw ()
   local lg = love.graphics

   local w, h = love.graphics.getDimensions()
   lg.setColor( 0, 0, 1, 1 )
   lg.setShader(shader)
   lg.draw( img, 0, 0, 0, w, h )
   lg.setShader()
end

function love.update( dt )
   global_dt = (global_dt or 0) + dt
   if shader:hasUniform("u_time") then
      shader:send( "u_time", global_dt )
   end
end

