uniform vec3 u_pos = vec3(0.0);
uniform float u_size = 0.0;
uniform float u_timer = 0.0;

vec4 effect( sampler2D tex, vec2 texcoord, vec2 screen_coords )
{
   vec4 colour = texture( tex, texcoord );
   float f = smoothstep( 0.0, 1.0, u_timer*5.0 );
   float d = length(screen_coords-u_pos.xy) - u_size * u_pos.z * f;
   float a = smoothstep( -5.0, 5.0, -d );
   if (a > 0.0) {
      colour.rgb = mix( colour.rgb, 1.0-colour.rgb, a );
   }
   return colour;
}
