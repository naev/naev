#include "lib/math.glsl"

uniform float u_progress = 0.0;
uniform sampler2D u_prevtex;

vec2 off( float progress, float x, float theta )
{
	float shifty = 0.03 * progress * cos(15.0*(progress+x));
	return vec2( 0, shifty );
}

vec4 oldtex( vec2 p )
{
   return texture( u_prevtex, p * vec2(1.0,-1.0) + vec2(0.0,1.0));
}

vec4 effect( sampler2D tex, vec2 p, vec2 screen_coords )
{
   float v = smoothstep( 0.0, 1.0, u_progress );
	return mix(
		oldtex(       p + off( v,     p.x, 0.0  ) ),
      texture( tex, p + off( 1.0-v, p.x, M_PI ) ),
      u_progress);
}
