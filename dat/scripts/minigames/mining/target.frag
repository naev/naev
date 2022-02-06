#pragma language glsl3

uniform vec3 size;

#define M_PI 3.141592653

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   uv = uv*2.0-1.0;
   float m = 1.0 / size.y;

   vec2 p = (uv * size.xy / size.z + vec2(1.0,0.0) );
   float d = abs(length(p)-1.0)-0.2;

   float alpha = smoothstep( -m, 0.0, -d);
	return colour * vec4( vec3(1.0), alpha );
}
