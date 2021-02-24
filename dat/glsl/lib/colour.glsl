#ifndef _COLOUR_GLSL
#define _COLOUR_GLSL

   
float rgbToLuminance( vec3 color ) {
   return dot( color, vec3( 0.2989, 0.5870, 0.1140 ) );
}


#endif /* _COLOUR_GLSL */
