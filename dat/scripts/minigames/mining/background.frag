#pragma language glsl3

uniform float radius;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   uv = uv*2.0-1.0;
   float d = abs(length(uv)-0.9)-0.1;
   float m = 1.0 / radius;

   float alpha = smoothstep(    -m, 0.0, -d);
   return colour * vec4( vec3(1.0), alpha );
}
