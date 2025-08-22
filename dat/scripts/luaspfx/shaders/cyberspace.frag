uniform float u_str;
uniform vec3 u_pos;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   uv = 2.0*uv-1.0;

   if (length(uv) > u_str * 1.5)
      discard;

   uv.y = 5.0 / (abs(uv.y+1.0) - 0.4);
   if (uv.y < 0.0)
      return vec4(0.0, 0.0, 0.0, 1.0);
   uv.x *= uv.y * 1.0;
   uv.y -= 25.0;
   uv.y *= 2.0;
   uv /= 10.0;

   vec2 nuv = -u_pos.xy*2.0 + uv*love_ScreenSize.xy*u_pos.z;
   nuv /= 350.0;
   vec2 duv = abs(nuv-round(nuv));
   float d = min(duv.x, duv.y);

   vec4 colour_out = colour;
   colour_out.rgb *= pow( smoothstep( -0.1, 0.0, -d ), 3.0);
   colour_out.rgb = mix( colour_out.rgb, colour.rgb, uv.y*0.01 );
   colour_out.a = 1.0;

   return colour_out;
}
