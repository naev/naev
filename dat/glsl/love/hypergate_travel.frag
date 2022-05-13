uniform float u_progress = 0.0;
uniform sampler2D u_prevtex;

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 old = texture( u_prevtex, texture_coords * vec2(1.0,-1.0) + vec2(0.0,1.0) );
   vec4 new = texture( tex, texture_coords );
   return  mix( old, new, u_progress );
}
