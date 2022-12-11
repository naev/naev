const float NORMAL_OFFSET = 0.001;
const float NORMAL_STRENGTH = 0.01;

uniform sampler2D height;
uniform vec2 dimensions;

/* Compute normal map using the height texture. */
vec2 normal( vec2 uv )
{
   vec2 uv1 = fract( uv + vec2(  NORMAL_OFFSET, 0.0 ) );
   vec2 uv2 = fract( uv + vec2( -NORMAL_OFFSET, 0.0 ) );
   vec2 uv3 = fract( uv + vec2( 0.0,  NORMAL_OFFSET ) );
   vec2 uv4 = fract( uv + vec2( 0.0, -NORMAL_OFFSET ) );
   float h1 = texture( height, uv1 ).g;
   float h2 = texture( height, uv2 ).g;
   float h3 = texture( height, uv3 ).g;
   float h4 = texture( height, uv4 ).g;
   vec2 norm = NORMAL_STRENGTH * vec2( (h2 - h1)/(2.0*NORMAL_OFFSET), (h4 - h3)/(2.0*NORMAL_OFFSET));
   return norm;
}


vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
    // UV coordinates.
   vec2 uv = screen_coords / dimensions;
   vec4 color_out = vec4( normal( uv*vec2(0.5,1.0) ), 1.0, 1.0 );
   return color_out;
}
