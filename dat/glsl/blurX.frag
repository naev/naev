#ifndef SHADOWMAP_SIZE
#  define SHADOWMAP_SIZE 512
#endif

uniform sampler2D sampler;

in vec2 tex_coord;

vec4 blurgaussian( sampler2D image, vec2 uv, vec2 resolution, vec2 direction, float sigma ) {
   const float threshold = 0.01; // Threshold to ignore pixels at
   vec4 color = texture( image, uv );
   float dstep = 1.0 / dot( resolution, direction );
   float g = 1.0; // exp( 0.0 )
   float sum = g;
   float den = 1.0 / (2.0 * pow(sigma, 2.0));
   for (float i = 1.0; (g=exp( -pow(i,2.0)*den )) > threshold; i++ ) {
      vec2 off = direction*i*dstep;
      color += texture( image, uv+off ) * g;
      color += texture( image, uv-off ) * g;
      sum += 2.0 * g;
   }
   color /= sum;
   return color;
}

void main (void)
{
   //float r = texture( sampler, tex_coord ).r;
   float r = blurgaussian( sampler, tex_coord, vec2(SHADOWMAP_SIZE), vec2(1.0,0.0), 3.0 ).r;
   gl_FragDepth = r;
}
