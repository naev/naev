#include "lib/math.glsl"

uniform float u_time;
uniform float u_r;

uniform vec3 u_colour;
uniform float u_duration;

const float NPARTICLES = 19.0;

vec4 effect( vec4 unused, sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
	vec2 p = (texture_coords - 0.5)*0.667;
	float t = fract( u_time/u_duration );
   t = sqrt(t);

	if ( dot(p,p) > 0.002 + t*0.13 )
		discard;

	vec4 c = vec4(0.0);
	for (float i = 0.0; i < NPARTICLES; i += 1.0) {
		float angle = 2.0*M_PI*random( vec2(i, u_r*100.0 )) + t*sin(i*43768.5453);
		float dist = 0.15 + 0.2 * random(vec2(i*351.0, i*135.0 + u_r*1000.0));

		vec2 pt = p + vec2(dist*sin(angle), dist*cos(angle));
		pt = mix(p, pt, t);

		float r = 0.03 * (1.0 - t) * t + 0.002*t*t*(1.0 - max(0.0, t - 0.9)*10.0);

		float d = 1.0 - smoothstep(pow(dot(pt, pt), 0.6), 0.0, r);
		c += vec4( u_colour*d, d);
	}
	return c - 0.1;
}

#ifndef _LOVE
in vec2 pos;
out vec4 colour_out;
uniform sampler2D dummy;
void main (void)
{
   colour_out = effect( vec4(0.0), dummy, pos, vec2(0.0) );
}
#endif /* _LOVE */
