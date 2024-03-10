#include "lib/sdf.glsl"

float marker_func( vec2 pos, float rad )
{
   /* TODO figure out a proper constant (not 1.3) to make it perfectly fit and such. */
   return abs(sdVesica( vec2(pos.x,-pos.y), rad*1.3, 0.5 ));
}

#include "spobmarker_base.glsl"
