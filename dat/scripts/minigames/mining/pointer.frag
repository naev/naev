#pragma language glsl3

uniform float radius;
uniform float pointer;

#define M_PI 3.141592653

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   uv = uv*2.0-1.0;
   float d = abs(length(uv)-0.9)-0.1;
   float m = 1.0 / radius;

   float a = atan( uv.y, uv.x );
   if ((a < 0.0) && (pointer > M_PI))
      a += 2.0*M_PI;
   if (pointer < a)
      d = 1.0;
   else
      colour.a *= 1.0-0.5*(pointer-a);

   float alpha = smoothstep( -m, 0.0, -d);
	return colour * vec4( vec3(1.0), alpha );
}
