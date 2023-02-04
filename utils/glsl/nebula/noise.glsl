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
vec3 mod7(vec3 x) {
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

vec4 blur5(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
   vec4 color = texture(image, uv) * 0.29411764705882354;
   vec2 off1 = vec2(1.3333333333333333) * direction;
   color += texture(image, uv + (off1 / resolution)) * 0.35294117647058826;
   color += texture(image, uv - (off1 / resolution)) * 0.35294117647058826;
   return color;
}
vec4 blur5( sampler2D image, vec2 uv, vec2 resolution, float strength ) {
   vec4 color = texture(image, uv) * 0.2941176470588234;
   vec2 off1 = vec2(1.3333333333333333,0.0) * strength;
   /* Horizontal. */
   vec2 off1xy = off1.xy / resolution;
   color += texture(image, uv + off1xy) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1xy) * 0.35294117647058826 * 0.25;
   /* Vertical. */
   vec2 off1yx = off1.yx / resolution;
   color += texture(image, uv + off1yx) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1yx) * 0.35294117647058826 * 0.25;
   /* BL-TR Diagonal. */
   vec2 off1d1 = off1.xx * M_SQRT2 / resolution;
   color += texture(image, uv + off1d1) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1d1) * 0.35294117647058826 * 0.25;
   /* TL-BR Diagonal. */
   vec2 off1d2 = vec2(off1d1.x, -off1d1.y);
   color += texture(image, uv + off1d2) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1d2) * 0.35294117647058826 * 0.25;
   return color;
}
vec4 blur9( sampler2D image, vec2 uv, vec2 resolution, vec2 direction ) {
   vec4 color = texture(image, uv) * 0.2270270270;
   vec2 off1 = vec2(1.3846153846) * direction;
   vec2 off2 = vec2(3.2307692308) * direction;
   color += texture(image, uv + (off1 / resolution)) * 0.3162162162;
   color += texture(image, uv - (off1 / resolution)) * 0.3162162162;
   color += texture(image, uv + (off2 / resolution)) * 0.0702702703;
   color += texture(image, uv - (off2 / resolution)) * 0.0702702703;
   return color;
}
vec4 blur9( sampler2D image, vec2 uv, vec2 resolution, float strength ) {
   vec4 color = texture(image, uv) * 0.2270270270;
   vec2 off1 = vec2(1.3846153846,0.0) * strength;
   vec2 off2 = vec2(3.2307692308,0.0) * strength;
   /* Horizontal. */
   vec2 off1xy = off1.xy / resolution;
   vec2 off2xy = off2.xy / resolution;
   color += texture(image, uv + off1xy) * 0.3162162162 * 0.25;
   color += texture(image, uv - off1xy) * 0.3162162162 * 0.25;
   color += texture(image, uv + off2xy) * 0.0702702703 * 0.25;
   color += texture(image, uv - off2xy) * 0.0702702703 * 0.25;
   /* Vertical. */
   vec2 off1yx = off1.yx / resolution;
   vec2 off2yx = off2.yx / resolution;
   color += texture(image, uv + off1yx) * 0.3162162162 * 0.25;
   color += texture(image, uv - off1yx) * 0.3162162162 * 0.25;
   color += texture(image, uv + off2yx) * 0.0702702703 * 0.25;
   color += texture(image, uv - off2yx) * 0.0702702703 * 0.25;
   /* BL-TR Diagonal. */
   vec2 off1d1 = off1.xx * M_SQRT2 / resolution;
   vec2 off2d1 = off2.xx * M_SQRT2 / resolution;
   color += texture(image, uv + off1d1) * 0.3162162162 * 0.25;
   color += texture(image, uv - off1d1) * 0.3162162162 * 0.25;
   color += texture(image, uv + off2d1) * 0.0702702703 * 0.25;
   color += texture(image, uv - off2d1) * 0.0702702703 * 0.25;
   /* TL-BR Diagonal. */
   vec2 off1d2 = vec2(off1d1.x, -off1d1.y);
   vec2 off2d2 = vec2(off2d1.x, -off2d1.y);
   color += texture(image, uv + off1d2) * 0.3162162162 * 0.25;
   color += texture(image, uv - off1d2) * 0.3162162162 * 0.25;
   color += texture(image, uv + off2d2) * 0.0702702703 * 0.25;
   color += texture(image, uv - off2d2) * 0.0702702703 * 0.25;
   return color;
}
vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
   vec4 color = texture(image, uv) * 0.1964825501511404;
   vec2 off1 = vec2(1.411764705882353) * direction;
   vec2 off2 = vec2(3.2941176470588234) * direction;
   vec2 off3 = vec2(5.176470588235294) * direction;
   color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
   color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
   color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
   color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
   color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
   color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
   return color;
}
vec4 blur13( sampler2D image, vec2 uv, vec2 resolution, float strength ) {
   vec4 color = texture(image, uv) * 0.1964825501511404;
   vec2 off1 = vec2(1.411764705882353) * strength;
   vec2 off2 = vec2(3.2941176470588234) * strength;
   vec2 off3 = vec2(5.176470588235294) * strength;
   /* Horizontal. */
   vec2 off1xy = off1.xy / resolution;
   vec2 off2xy = off2.xy / resolution;
   vec2 off3xy = off3.xy / resolution;
   color += texture(image, uv + off1xy) * 0.2969069646728344 * 0.25;
   color += texture(image, uv - off1xy) * 0.2969069646728344 * 0.25;
   color += texture(image, uv + off2xy) * 0.09447039785044732 * 0.25;
   color += texture(image, uv - off2xy) * 0.09447039785044732 * 0.25;
   color += texture(image, uv + off3xy) * 0.010381362401148057 * 0.25;
   color += texture(image, uv - off3xy) * 0.010381362401148057 * 0.25;
   /* Vertical. */
   vec2 off1yx = off1.yx / resolution;
   vec2 off2yx = off2.yx / resolution;
   vec2 off3yx = off3.yx / resolution;
   color += texture(image, uv + off1yx) * 0.2969069646728344 * 0.25;
   color += texture(image, uv - off1yx) * 0.2969069646728344 * 0.25;
   color += texture(image, uv + off2yx) * 0.09447039785044732 * 0.25;
   color += texture(image, uv - off2yx) * 0.09447039785044732 * 0.25;
   color += texture(image, uv + off3yx) * 0.010381362401148057 * 0.25;
   color += texture(image, uv - off3yx) * 0.010381362401148057 * 0.25;
   /* BL-TR Diagonal. */
   vec2 off1d1 = off1.xx * M_SQRT2 / resolution;
   vec2 off2d1 = off2.xx * M_SQRT2 / resolution;
   vec2 off3d1 = off3.xx * M_SQRT2 / resolution;
   color += texture(image, uv + off1d1) * 0.2969069646728344 * 0.25;
   color += texture(image, uv - off1d1) * 0.2969069646728344 * 0.25;
   color += texture(image, uv + off2d1) * 0.09447039785044732 * 0.25;
   color += texture(image, uv - off2d1) * 0.09447039785044732 * 0.25;
   color += texture(image, uv + off3d1) * 0.010381362401148057 * 0.25;
   color += texture(image, uv - off3d1) * 0.010381362401148057 * 0.25;
   /* TL-BR Diagonal. */
   vec2 off1d2 = vec2(off1d1.x, -off1d1.y);
   vec2 off2d2 = vec2(off2d1.x, -off2d1.y);
   vec2 off3d2 = vec2(off3d1.x, -off3d1.y);
   color += texture(image, uv + off1d2) * 0.2969069646728344 * 0.25;
   color += texture(image, uv - off1d2) * 0.2969069646728344 * 0.25;
   color += texture(image, uv + off2d2) * 0.09447039785044732 * 0.25;
   color += texture(image, uv - off2d2) * 0.09447039785044732 * 0.25;
   color += texture(image, uv + off3d2) * 0.010381362401148057 * 0.25;
   color += texture(image, uv - off3d2) * 0.010381362401148057 * 0.25;
   return color;
}

/*
 * These two implementations below are computed per pixel and much slower.
 * Meant for offline use (e.g., to create canvases)
 */
vec4 blurlinear( sampler2D image, vec2 uv, vec2 resolution, vec2 direction, float strength ) {
   vec4 color = texture( image, uv );
   float dstep = 1.0 / dot( resolution, direction );
   float sum = 1.0;
   for (float i = 1.0; i<=strength; i++ ) {
      vec2 off = direction*i*dstep;
      color += texture( image, uv+off );
      color += texture( image, uv-off );
      sum += 2.0;
   }
   color /= sum;
   return color;
}
vec4 blurgaussian( sampler2D image, vec2 uv, vec2 resolution, vec2 direction, float sigma ) {
   const float threshold = 0.01; // Threshold to ignore pixels at
   vec4 color = texture( image, uv );
   float dstep = 1.0 / dot( resolution, direction );
   float g = 1.0; // exp( 0.0 )
   float sum = g;
   float den = 1.0 / (2.0 * pow(sigma, 2.0));
   for (float i = 1.0; (g=exp( -pow(i,2.0)*den )) > threshold; i++ ) {
      vec2 off = direction*i*dstep;
      color += texture( image, uv+off ) * g;
      color += texture( image, uv-off ) * g;
      sum += 2.0 * g;
   }
   color /= sum;
   return color;
}

// Cellular noise, returning F1 and F2 in a vec2.
// 3x3x3 search region for good F2 everywhere, but a lot
// slower than the 2x2x2 version.
// The code below is a bit scary even to its author,
// but it has at least half decent performance on a
// modern GPU. In any case, it beats any software
// implementation of Worley noise hands down.
vec2 cellular(vec3 P) {
#define K 0.142857142857 // 1/7
#define Ko 0.428571428571 // 1/2-K/2
#define K2 0.020408163265306 // 1/(7*7)
#define Kz 0.166666666667 // 1/6
#define Kzo 0.416666666667 // 1/2-1/6*2
#define jitter 1.0 // smaller jitter gives more regular pattern

	vec3 Pi = mod289(floor(P));
 	vec3 Pf = fract(P) - 0.5;

	vec3 Pfx = Pf.x + vec3(1.0, 0.0, -1.0);
	vec3 Pfy = Pf.y + vec3(1.0, 0.0, -1.0);
	vec3 Pfz = Pf.z + vec3(1.0, 0.0, -1.0);

	vec3 p = permute(Pi.x + vec3(-1.0, 0.0, 1.0));
	vec3 p1 = permute(p + Pi.y - 1.0);
	vec3 p2 = permute(p + Pi.y);
	vec3 p3 = permute(p + Pi.y + 1.0);

	vec3 p11 = permute(p1 + Pi.z - 1.0);
	vec3 p12 = permute(p1 + Pi.z);
	vec3 p13 = permute(p1 + Pi.z + 1.0);

	vec3 p21 = permute(p2 + Pi.z - 1.0);
	vec3 p22 = permute(p2 + Pi.z);
	vec3 p23 = permute(p2 + Pi.z + 1.0);

	vec3 p31 = permute(p3 + Pi.z - 1.0);
	vec3 p32 = permute(p3 + Pi.z);
	vec3 p33 = permute(p3 + Pi.z + 1.0);

	vec3 ox11 = fract(p11*K) - Ko;
	vec3 oy11 = mod7(floor(p11*K))*K - Ko;
	vec3 oz11 = floor(p11*K2)*Kz - Kzo; // p11 < 289 guaranteed

	vec3 ox12 = fract(p12*K) - Ko;
	vec3 oy12 = mod7(floor(p12*K))*K - Ko;
	vec3 oz12 = floor(p12*K2)*Kz - Kzo;

	vec3 ox13 = fract(p13*K) - Ko;
	vec3 oy13 = mod7(floor(p13*K))*K - Ko;
	vec3 oz13 = floor(p13*K2)*Kz - Kzo;

	vec3 ox21 = fract(p21*K) - Ko;
	vec3 oy21 = mod7(floor(p21*K))*K - Ko;
	vec3 oz21 = floor(p21*K2)*Kz - Kzo;

	vec3 ox22 = fract(p22*K) - Ko;
	vec3 oy22 = mod7(floor(p22*K))*K - Ko;
	vec3 oz22 = floor(p22*K2)*Kz - Kzo;

	vec3 ox23 = fract(p23*K) - Ko;
	vec3 oy23 = mod7(floor(p23*K))*K - Ko;
	vec3 oz23 = floor(p23*K2)*Kz - Kzo;

	vec3 ox31 = fract(p31*K) - Ko;
	vec3 oy31 = mod7(floor(p31*K))*K - Ko;
	vec3 oz31 = floor(p31*K2)*Kz - Kzo;

	vec3 ox32 = fract(p32*K) - Ko;
	vec3 oy32 = mod7(floor(p32*K))*K - Ko;
	vec3 oz32 = floor(p32*K2)*Kz - Kzo;

	vec3 ox33 = fract(p33*K) - Ko;
	vec3 oy33 = mod7(floor(p33*K))*K - Ko;
	vec3 oz33 = floor(p33*K2)*Kz - Kzo;

	vec3 dx11 = Pfx + jitter*ox11;
	vec3 dy11 = Pfy.x + jitter*oy11;
	vec3 dz11 = Pfz.x + jitter*oz11;

	vec3 dx12 = Pfx + jitter*ox12;
	vec3 dy12 = Pfy.x + jitter*oy12;
	vec3 dz12 = Pfz.y + jitter*oz12;

	vec3 dx13 = Pfx + jitter*ox13;
	vec3 dy13 = Pfy.x + jitter*oy13;
	vec3 dz13 = Pfz.z + jitter*oz13;

	vec3 dx21 = Pfx + jitter*ox21;
	vec3 dy21 = Pfy.y + jitter*oy21;
	vec3 dz21 = Pfz.x + jitter*oz21;

	vec3 dx22 = Pfx + jitter*ox22;
	vec3 dy22 = Pfy.y + jitter*oy22;
	vec3 dz22 = Pfz.y + jitter*oz22;

	vec3 dx23 = Pfx + jitter*ox23;
	vec3 dy23 = Pfy.y + jitter*oy23;
	vec3 dz23 = Pfz.z + jitter*oz23;

	vec3 dx31 = Pfx + jitter*ox31;
	vec3 dy31 = Pfy.z + jitter*oy31;
	vec3 dz31 = Pfz.x + jitter*oz31;

	vec3 dx32 = Pfx + jitter*ox32;
	vec3 dy32 = Pfy.z + jitter*oy32;
	vec3 dz32 = Pfz.y + jitter*oz32;

	vec3 dx33 = Pfx + jitter*ox33;
	vec3 dy33 = Pfy.z + jitter*oy33;
	vec3 dz33 = Pfz.z + jitter*oz33;

	vec3 d11 = dx11 * dx11 + dy11 * dy11 + dz11 * dz11;
	vec3 d12 = dx12 * dx12 + dy12 * dy12 + dz12 * dz12;
	vec3 d13 = dx13 * dx13 + dy13 * dy13 + dz13 * dz13;
	vec3 d21 = dx21 * dx21 + dy21 * dy21 + dz21 * dz21;
	vec3 d22 = dx22 * dx22 + dy22 * dy22 + dz22 * dz22;
	vec3 d23 = dx23 * dx23 + dy23 * dy23 + dz23 * dz23;
	vec3 d31 = dx31 * dx31 + dy31 * dy31 + dz31 * dz31;
	vec3 d32 = dx32 * dx32 + dy32 * dy32 + dz32 * dz32;
	vec3 d33 = dx33 * dx33 + dy33 * dy33 + dz33 * dz33;

	// Sort out the two smallest distances (F1, F2)
#ifndef CELLULAR_NOISE_ACCURATE
	// Cheat and sort out only F1
	vec3 d1 = min(min(d11,d12), d13);
	vec3 d2 = min(min(d21,d22), d23);
	vec3 d3 = min(min(d31,d32), d33);
	vec3 d = min(min(d1,d2), d3);
	d.x = min(min(d.x,d.y),d.z);
	return vec2(sqrt(d.x)); // F1 duplicated, no F2 computed
#else
	// Do it right and sort out both F1 and F2
	vec3 d1a = min(d11, d12);
	d12 = max(d11, d12);
	d11 = min(d1a, d13); // Smallest now not in d12 or d13
	d13 = max(d1a, d13);
	d12 = min(d12, d13); // 2nd smallest now not in d13
	vec3 d2a = min(d21, d22);
	d22 = max(d21, d22);
	d21 = min(d2a, d23); // Smallest now not in d22 or d23
	d23 = max(d2a, d23);
	d22 = min(d22, d23); // 2nd smallest now not in d23
	vec3 d3a = min(d31, d32);
	d32 = max(d31, d32);
	d31 = min(d3a, d33); // Smallest now not in d32 or d33
	d33 = max(d3a, d33);
	d32 = min(d32, d33); // 2nd smallest now not in d33
	vec3 da = min(d11, d21);
	d21 = max(d11, d21);
	d11 = min(da, d31); // Smallest now in d11
	d31 = max(da, d31); // 2nd smallest now not in d31
	d11.xy = (d11.x < d11.y) ? d11.xy : d11.yx;
	d11.xz = (d11.x < d11.z) ? d11.xz : d11.zx; // d11.x now smallest
	d12 = min(d12, d21); // 2nd smallest now not in d21
	d12 = min(d12, d22); // nor in d22
	d12 = min(d12, d31); // nor in d31
	d12 = min(d12, d32); // nor in d32
	d11.yz = min(d11.yz,d12.xy); // nor in d12.yz
	d11.y = min(d11.y,d12.z); // Only two more to go
	d11.y = min(d11.y,d11.z); // Done! (Phew!)
	return sqrt(d11.xy); // F1, F2
#endif
}
#undef K
#undef Ko
#undef K2
#undef Kz
#undef Kzo
#undef jitter
