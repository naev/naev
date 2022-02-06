#pragma language glsl3

uniform vec4 size;

#define M_PI 3.141592653

float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
   vec2 pa = p-a, ba = b-a;
   float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
   return length( pa - ba*h );
}

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   uv = uv*2.0-1.0;
   float m = 1.0 / size.x;

   vec2 p = (uv * size.xy / size.z + vec2(1.0,0.0) );
   float d = abs(length(p)-1.0)-0.25;
   d = max( d, (abs(p.y)-size.w/size.z*p.x) );
   d = max( d, -(d+3.0*m) ); /* Hollow out inside. */

   d = max( d, -(abs(length(p)-1.0)-0.15) );
   d = max( d, -(abs(p.y)-size.w/size.z*p.x*0.5) );

   float alpha = smoothstep( -m, 0.0, -d);
	return colour * vec4( vec3(1.0), alpha );
}
