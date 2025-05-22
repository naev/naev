/*
 * Magic Kernel Sharp for Image Resizing
 * https://johncostella.com/magic/
 *
 * Single pass solution
 */

uniform float u_scale = 0.25; // Amount of scaling being done
uniform float u_radius = 8.0; /// Radius of the window
uniform sampler2D tex;

in vec2 pos;
out vec4 colour_out;

#if 0
// mks2013
float resizeFilter(float x)
{
   x = abs(x);
   if (x >= 2.5)
      return 0.0;
   else if (x >= 1.5)
      return -0.125 * (x - 2.5) * (x - 2.5);
   else if (x >= 0.5)
      return (1.0 - x) * (1.75 - x);
		//return 0.25 * (4.0 * x * x - 11.0 * x + 7.0);
   return 1.0625 - 1.75 * x * x;
}
#else
// mks2021
float resizeFilter(float x)
{
   x = abs(x);
   if (x >= 4.5)
      return 0.0;
   else if (x >= 3.5)
      return -1.0/288.0 * (x - 4.5) * (x - 4.5);
   else if (x >= 2.5)
		return 1.0/36.0 * (x - 3.0) * (x - 3.75);
	else if (x >= 1.5)
		return 1.0/6.0 * (x - 2.0) * (65.0 / 24.0 - x);
   else if (x >= 0.5)
      return 35.0/36.0 * (x - 1.0) * (x - 239.0/140.0);
   return 577.0/576.0 - 239.0/144.0 * x * x;
}
#endif

void main()
{
   vec2 dims = textureSize( tex, 0 );
   vec2 pixsize = 1.0 / dims;

   vec2 src = pos * dims;
   vec2 startf = src - u_radius;
   vec2 endf = src + u_radius;
   ivec2 start = ivec2( floor(startf) );
   ivec2 end = ivec2( ceil(endf) );

   float sum = 0.0;
   vec4 colour = vec4(0.0);
   for(int i = start.x; i <= end.x; i++) {
      float fi = float(i) + 0.5;
      for (int j = start.y; j <= end.y; j++) {
         float fj = float(j) + 0.5;
         vec2 fij  = vec2(fi,fj);

         float weight = resizeFilter( length( (fij-src) * u_scale ) );
         vec2 texpos = fij * pixsize;

         vec4 pixel = texture( tex, texpos );
         pixel.rgb *= pixel.a;
         colour += pixel * weight;
         sum += weight;
      }
   }
	colour /= sum;
   colour.rgb /= colour.a;
	colour_out = colour;
}
