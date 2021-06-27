#ifndef _MATH_GLSL
#define _MATH_GLSL

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

/* Returns a value in the [0,1] range. */
float random(float n)
{
   return fract(sin(n) * 43758.5453123);
}

/* Returns a value in the [0,1] range.
 http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
 */
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

#endif /* _MATH_GLSL */
