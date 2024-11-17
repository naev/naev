#include "lib/sdf.glsl"

float marker_func( vec2 pos, float rad )
{
   return abs( sdRhombus( pos, vec2(0.8,1.0)*rad ) );
}

#include "spobmarker_base.glsl"
