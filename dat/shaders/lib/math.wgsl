/* Taken from /usr/include/math.h */
const M_E: f32         = 2.7182818284590452354;   /* e */
const M_LOG2E: f32     = 1.4426950408889634074;   /* log_2 e */
const M_LOG10E: f32    = 0.43429448190325182765;  /* log_10 e */
const M_LN2: f32       = 0.69314718055994530942;  /* log_e 2 */
const M_LN10: f32      = 2.30258509299404568402;  /* log_e 10 */
const M_PI: f32        = 3.14159265358979323846;  /* pi */
const M_PI_2: f32      = 1.57079632679489661923;  /* pi/2 */
const M_PI_4: f32      = 0.78539816339744830962;  /* pi/4 */
const M_1_PI: f32      = 0.31830988618379067154;  /* 1/pi */
const M_2_PI: f32      = 0.63661977236758134308;  /* 2/pi */
const M_2_SQRTPI: f32  = 1.12837916709551257390;  /* 2/sqrt(pi) */
const M_SQRT2: f32     = 1.41421356237309504880;  /* sqrt(2) */
const M_SQRT1_2: f32   = 0.70710678118654752440;  /* 1/sqrt(2) */

// Modulo 289 without a division (only multiplications)
fn mod289(x: vec4f) -> vec4f {
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}
fn mod289(x: vec3f) -> vec3f {
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}
fn mod289(x: vec2f) -> vec2f {
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}
fn mod289(x: f32) -> f32 {
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}

// Modulo 7 without a division
fn mod7(x: vec4f) -> vec4f {
   return x - floor(x * (1.0 / 7.0)) * 7.0;
}
fn mod7(x: vec3f) -> vec3f {
   return x - floor(x * (1.0 / 7.0)) * 7.0;
}

// Permutation polynomial: (34x^2 + x) mod 289
fn permute(x: vec4f) -> vec4f {
   return mod289((34.0 * x + 10.0) * x);
}
fn permute(x: vec3f) -> vec3f {
   return mod289((34.0 * x + 10.0) * x);
}

fn taylorInvSqrt(r: vec4f) -> vec4f {
   return 1.79284291400159 - 0.85373472095314 * r;
}

fn fade(t: vec3f) -> vec3f {
   return t*t*t*(t*(t*6.0-15.0)+10.0);
}
fn fade(t: vec2f) -> vec2f {
   return t*t*t*(t*(t*6.0-15.0)+10.0);
}

// Returns a value in the [0,1] range.
fn random(n: f32) -> f32 {
   return fract(sin(n) * 43758.5453123);
}

// Returns a value in the [0,1] range.
// http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
fn random(co: vec2f) -> f32 {
   let seed: f32 = 12.9898;
   let a = seed;
   let b = 78.233;
   let c = 43758.5453;
   let dt= dot(co.xy ,vec2(a,b));
   let sn= mod(dt,M_PI);
   return fract(sin(sn) * c);
}
*/
