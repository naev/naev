uniform float u_time = 0.0;
const float PERIOD = 4.0;

vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   colour.a *= clamp( 10.0*sin( u_time * PERIOD ), 0.0, 1.0 );
   if (colour.a <= 0.0)
      discard;
   vec2 uv = texture_coords*2.0-1.0;
   float d = length(uv)-0.3;
   colour.a *= smoothstep( -0.7, 0.0, -d );
   return colour;
}
