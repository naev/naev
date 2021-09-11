#ifndef _COFACTOR_GLSL
#define _COFACTOR_GLSL

mat4 cofactor( const mat4 m ) {
	vec4 r0 = m[0];
	vec4 r1 = m[1];
	vec4 r2 = m[2];
	vec4 r3 = m[3];
	return mat4(
		 (r1.y * (r2.z * r3.w - r3.z * r2.w) - r1.z * (r2.y * r3.w - r3.y * r2.w) + r1.w * (r2.y * r3.z - r3.y * r2.z)),
		-(r1.x * (r2.z * r3.w - r3.z * r2.w) - r1.z * (r2.x * r3.w - r3.x * r2.w) + r1.w * (r2.x * r3.z - r3.x * r2.z)),
		 (r1.x * (r2.y * r3.w - r3.y * r2.w) - r1.y * (r2.x * r3.w - r3.x * r2.w) + r1.w * (r2.x * r3.y - r3.x * r2.y)),
		-(r1.x * (r2.y * r3.z - r3.y * r2.z) - r1.y * (r2.x * r3.z - r3.x * r2.z) + r1.z * (r2.x * r3.y - r3.x * r2.y)),
		-(r0.y * (r2.z * r3.w - r3.z * r2.w) - r0.z * (r2.y * r3.w - r3.y * r2.w) + r0.w * (r2.y * r3.z - r3.y * r2.z)),
		 (r0.x * (r2.z * r3.w - r3.z * r2.w) - r0.z * (r2.x * r3.w - r3.x * r2.w) + r0.w * (r2.x * r3.z - r3.x * r2.z)),
		-(r0.x * (r2.y * r3.w - r3.y * r2.w) - r0.y * (r2.x * r3.w - r3.x * r2.w) + r0.w * (r2.x * r3.y - r3.x * r2.y)),
		 (r0.x * (r2.y * r3.z - r3.y * r2.z) - r0.y * (r2.x * r3.z - r3.x * r2.z) + r0.z * (r2.x * r3.y - r3.x * r2.y)),
		 (r0.y * (r1.z * r3.w - r3.z * r1.w) - r0.z * (r1.y * r3.w - r3.y * r1.w) + r0.w * (r1.y * r3.z - r3.y * r1.z)),
		-(r0.x * (r1.z * r3.w - r3.z * r1.w) - r0.z * (r1.x * r3.w - r3.x * r1.w) + r0.w * (r1.x * r3.z - r3.x * r1.z)),
		 (r0.x * (r1.y * r3.w - r3.y * r1.w) - r0.y * (r1.x * r3.w - r3.x * r1.w) + r0.w * (r1.x * r3.y - r3.x * r1.y)),
		-(r0.x * (r1.y * r3.z - r3.y * r1.z) - r0.y * (r1.x * r3.z - r3.x * r1.z) + r0.z * (r1.x * r3.y - r3.x * r1.y)),
		-(r0.y * (r1.z * r2.w - r2.z * r1.w) - r0.z * (r1.y * r2.w - r2.y * r1.w) + r0.w * (r1.y * r2.z - r2.y * r1.z)),
		 (r0.x * (r1.z * r2.w - r2.z * r1.w) - r0.z * (r1.x * r2.w - r2.x * r1.w) + r0.w * (r1.x * r2.z - r2.x * r1.z)),
		-(r0.x * (r1.y * r2.w - r2.y * r1.w) - r0.y * (r1.x * r2.w - r2.x * r1.w) + r0.w * (r1.x * r2.y - r2.x * r1.y)),
		 (r0.x * (r1.y * r2.z - r2.y * r1.z) - r0.y * (r1.x * r2.z - r2.x * r1.z) + r0.z * (r1.x * r2.y - r2.x * r1.y))
	);
}

#endif /* _COFACTOR_GLSL */
