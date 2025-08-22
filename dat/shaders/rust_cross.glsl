layout( std140 ) uniform CrossData
{
   mat3  transform;
   vec4  colour;
   float radius;
};

#if defined( VERT )
layout( location = 0 ) in vec2 vertex;
out vec2 pos;

void main( void )
{
   pos = vertex;
   gl_Position = vec4( ( transform * vec3(vertex, 1.0) ).xy, 0.0, 1.0 );
}
#elif defined( FRAG )
#include "lib/sdf.glsl"

in vec2 pos;
layout( location = 0 ) out vec4 colour_out;

void main( void )
{
   float m     = 1.0 / ( 2.0 * radius );
   float rad   = 1.0 - radius * m;
   float d     = min( length( pos - vec2( clamp( pos.x, -rad, rad ), 0.0 ) ),
                     length( pos - vec2( 0.0, clamp( pos.y, -rad, rad ) ) ) );
   float alpha = smoothstep( -m, 0.0, -d );
   float beta  = smoothstep( -radius * m, -m, -d );
   colour_out  = colour * vec4( vec3( alpha ), beta );
}
#endif
