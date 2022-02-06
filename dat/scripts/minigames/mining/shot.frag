#pragma language glsl3

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec2 pos = (uv*2.0-1.0);
   float d = length(pos)-1.0;
   float alpha = smoothstep( 0.0, 0.8, -d);
   vec4 colout = vec4( colour.rgb, alpha );
   colout.rgb += pow( alpha, 2.0 );
   colout.a *= colour.a;
   return colout;
}
