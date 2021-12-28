#include "lib/sdf.glsl"

const float M_SQRT3 = sqrt(3.0);

float marker_func( vec2 pos, float rad )
{
   return abs( sdTriangleEquilateral( p * rad / M_SQRT3 ) * M_SQRT3 );
}

#include "spobmarker_base.frag"
