#include "lib/sdf.glsl"

const float M_SQRT3_2 = sqrt(3.0) / 2.0;

float marker_func( vec2 pos, float rad )
{
   return abs( sdTriangleEquilateral( vec2(pos.x,-pos.y) / M_SQRT3_2 / rad ) * rad * M_SQRT3_2 );
}

#include "spobmarker_base.glsl"
