#include "lib/sdf.glsl"

float marker_func( vec2 pos, float rad )
{
   return min( abs( sdCircle( pos, rad ) ), sdCircle( pos, rad*0.2 ) );
}

#include "spobmarker_base.glsl"
