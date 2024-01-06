#include "lib/sdf.glsl"

float marker_func( vec2 pos, float rad )
{
   float dplus = min(
      length( pos - vec2( clamp( pos.x, -rad, rad ), 0.0 ) ),
      length( pos - vec2( 0.0, clamp( pos.y, -rad, rad ) ) )
   );
   float dcirc = abs( sdCircle( pos, rad ) );
   return min( dcirc, dplus );
}

#include "spobmarker_base.glsl"
