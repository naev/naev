
#version 3.7;

global_settings{
   assumed_gamma 1.6
   ambient_light 1.0
}

#include "colors.inc"

camera{
   location    5*<0,13,-7>
   right       x*image_width/image_height
   direction   z*5
   look_at     0
}

light_source{
   <2000,700,-2000>
   2.5*White
}

height_field {
   "pot.png" /*smooth*/
   pigment { rgb<1,1,1>  }
   translate <-.5, 0, -.5>
   scale <17,1.0, 17>
}
