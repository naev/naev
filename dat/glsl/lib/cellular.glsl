// Cellular noise ("Worley noise") in 2D in GLSL.
// Copyright (c) Stefan Gustavson 2011-04-19. All rights reserved.
// This code is released under the conditions of the MIT license.
// See LICENSE file for details.
// https://github.com/stegu/webgl-noise

#ifndef _CELLULAR_GLSL
#define _CELLULAR_GLSL

#include "lib/math.glsl"

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
#ifndef CELLULAR_NOISE_ACCURATE
   // Cheat and pick only F1
   d.xy = min(d.xy, d.zw);
   d.x = min(d.x, d.y);
   return vec2(sqrt(d.x)); // F1 duplicated, F2 not computed
#else /* CELLULAR_NOISE_ACCURATE */
   // Do it right and find both F1 and F2
   d.xy = (d.x < d.y) ? d.xy : d.yx; // Swap if smaller
   d.xz = (d.x < d.z) ? d.xz : d.zx;
   d.xw = (d.x < d.w) ? d.xw : d.wx;
   d.y = min(d.y, d.z);
   d.y = min(d.y, d.w);
   return sqrt(d.xy);
#endif /* CELLULAR_NOISE_ACCURATE */
}
#undef K
#undef K2
#undef jitter

// Cellular noise, returning F1 and F2 in a vec2.
// Speeded up by using 2x2x2 search window instead of 3x3x3,
// at the expense of some pattern artifacts.
// F2 is often wrong and has sharp discontinuities.
// If you need a good F2, use the slower 3x3x3 version.
vec2 cellular2x2x2(vec3 P) {
#define K 0.142857142857 // 1/7
#define Ko 0.428571428571 // 1/2-K/2
#define K2 0.020408163265306 // 1/(7*7)
#define Kz 0.166666666667 // 1/6
#define Kzo 0.416666666667 // 1/2-1/6*2
#define jitter 0.8 // smaller jitter gives less errors in F2
	vec3 Pi = mod289(floor(P));
 	vec3 Pf = fract(P);
	vec4 Pfx = Pf.x + vec4(0.0, -1.0, 0.0, -1.0);
	vec4 Pfy = Pf.y + vec4(0.0, 0.0, -1.0, -1.0);
	vec4 p = permute(Pi.x + vec4(0.0, 1.0, 0.0, 1.0));
	p = permute(p + Pi.y + vec4(0.0, 0.0, 1.0, 1.0));
	vec4 p1 = permute(p + Pi.z); // z+0
	vec4 p2 = permute(p + Pi.z + vec4(1.0)); // z+1
	vec4 ox1 = fract(p1*K) - Ko;
	vec4 oy1 = mod7(floor(p1*K))*K - Ko;
	vec4 oz1 = floor(p1*K2)*Kz - Kzo; // p1 < 289 guaranteed
	vec4 ox2 = fract(p2*K) - Ko;
	vec4 oy2 = mod7(floor(p2*K))*K - Ko;
	vec4 oz2 = floor(p2*K2)*Kz - Kzo;
	vec4 dx1 = Pfx + jitter*ox1;
	vec4 dy1 = Pfy + jitter*oy1;
	vec4 dz1 = Pf.z + jitter*oz1;
	vec4 dx2 = Pfx + jitter*ox2;
	vec4 dy2 = Pfy + jitter*oy2;
	vec4 dz2 = Pf.z - 1.0 + jitter*oz2;
	vec4 d1 = dx1 * dx1 + dy1 * dy1 + dz1 * dz1; // z+0
	vec4 d2 = dx2 * dx2 + dy2 * dy2 + dz2 * dz2; // z+1

	// Sort out the two smallest distances (F1, F2)
#if 0
	// Cheat and sort out only F1
	d1 = min(d1, d2);
	d1.xy = min(d1.xy, d1.wz);
	d1.x = min(d1.x, d1.y);
	return vec2(sqrt(d1.x));
#else
	// Do it right and sort out both F1 and F2
	vec4 d = min(d1,d2); // F1 is now in d
	d2 = max(d1,d2); // Make sure we keep all candidates for F2
	d.xy = (d.x < d.y) ? d.xy : d.yx; // Swap smallest to d.x
	d.xz = (d.x < d.z) ? d.xz : d.zx;
	d.xw = (d.x < d.w) ? d.xw : d.wx; // F1 is now in d.x
	d.yzw = min(d.yzw, d2.yzw); // F2 now not in d2.yzw
	d.y = min(d.y, d.z); // nor in d.z
	d.y = min(d.y, d.w); // nor in d.w
	d.y = min(d.y, d2.x); // F2 is now in d.y
	return sqrt(d.xy); // F1 and F2
#endif
}
#undef K
#undef Ko
#undef K2
#undef Kz
#undef Kzo
#undef jitter

#endif /* _CELLULAR_GLSL */
