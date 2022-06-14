const float LENGTH   = 3.0;
uniform float u_time = 0.0;

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float progress = fract( u_time / LENGTH );
   vec2 uv = texture_coords*2.0-1.0;
   float r = (progress*2.0-0.3);
   float d = abs(length(uv)-r);
   color.a *= smoothstep( -0.2, 0.0, -d );
   color.a -= length(uv);
   return color;
}
