const vec3 COLOUR_BASE = vec3( %f, %f, %f );

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   uv = 2.0*uv-1.0;
   uv.y = 5.0 / (abs(uv.y+1.0) - 0.4);
   if (uv.y < 0.0)
      return vec4( vec3(-0.001*uv.y), 1.0);
   uv.x *= uv.y * 1.0;
   uv.y -= 25.0;
   uv.y *= 2.0;
   uv *= %f;

   vec2 duv = abs(uv-round(uv));
   float d = min(duv.x, duv.y);
   vec4 colour_out = vec4( COLOUR_BASE, 1.0 );
   colour_out.rgb *= pow( smoothstep( -0.1, 0.0, -d ), 3.0);
   colour_out.rgb = mix( colour_out.rgb, vec3(1.0), uv.y*0.001 );

   return colour_out;
}
