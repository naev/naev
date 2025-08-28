#include "lib/sdf.glsl"

float marker_func( vec2 pos, float rad )
{
   return abs( sdCircle( pos, rad ) );
}

#include "spobmarker_base.glsl"
