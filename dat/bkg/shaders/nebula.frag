#pragma language glsl3

#include "lib/perlin.glsl"
#include "lib/simplex.glsl"
#include "lib/colour.glsl"

/* Constants be here. */
const int STEPS         = 48;
const float RADIUS      = 1.0;
const float HUE_INNER   = 1.0;
const float HUE_OUTTER  = 0.67;
const float ABSORPTION  = 45.0;
const float OPACITY     = 60.0;
const vec2 RESOLUTION   = vec2( %f, %f );

/* Our nebula function (static version of the 2D turbulence noise in 3D) */
float nebula( in vec3 p )
{
   const float SCALAR = pow(2., 4./3.);
   float f, scale;
   p = p + vec3( %f, %f, %f );
   scale = 1.0;
   f  = abs( cnoise( p * scale ) ) / scale;
   scale *= SCALAR;
   f += abs( cnoise( p * scale ) ) / scale;
   scale *= SCALAR;
   f += abs( cnoise( p * scale ) ) / scale;
   return f;
}

/* Since it is density it is minus the SDF value. */
float density( in vec3 pos )
{
   //return (1.0 - length(pos));
   //return pow(1.0-length(pos),1.0) * 0.5*(nebula( pos*3.0 )-0.1);
   //return smoothstep(0.0,1.0,1.0-length(pos)) * (2.0*(nebula( pos*5.0 )-0.5));
   //return smoothstep(0.0,1.0,1.0-length(pos)) * (1.0*(nebula( pos*3.0 )-0.3));
   return smoothstep(0.0,1.0,1.0-length(pos)) * (-length(pos)+1.5*nebula( pos*4.0 )-0.1);
   //return smoothstep(0.0,1.0,1.0-length(pos)) * (-length(pos)+1.5*nebula( pos*7.0 )-0.1);
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   /* Normalize coordinates. */
   vec2 uv = texture_coords*2.0-1.0;

   /* Set up rays. */
   vec3 ro = vec3(0.0, 0.0, sqrt(2.0));
	vec3 rd = normalize(vec3(uv, -1.0));

   /* Frustrum. */
   const float znear = (sqrt(2.0)-1.0)/2.0;
   const float zfar  = sqrt(2.0)+1.0;
   const float zstep = RADIUS / float( STEPS );

   /* Some initial parameters. */
	float T = 1.0;
   color = vec4(0.0);

   float depth = znear;
	for (int i=0; i < STEPS; i++) {
      /* Out of the frutrum. */
      if (depth > zfar)
         break;

      /* Move along ray. */
      vec3 p = ro + rd * depth;

      /* Compute the nebula density. */
		float dens = density( p );

      /* March faster when no density. */
      if (dens <= 0.0) {
         depth += 1.5 * zstep;
         continue;
      }

      /* Computing the density integral so we have to normalize by sampling points. */
      float densn = dens / float(STEPS);

      /* Transmit ligth along, with some getting absorbed. */ 
      T *= 1.0 - (densn * ABSORPTION);
      /* Stop when mostly absorbed. */
      if (T <= 0.01)
         break;

      /* Very simply lighting model based on scattering with some additional texture noise.
       * Two colours used for inside vs outside. */
      float k = OPACITY * densn * T;
      float hue = mix( HUE_INNER, HUE_OUTTER, length(p) + 0.1*snoise(10.0*p) );
      color += k * vec4( hsv2rgb( vec3( hue, 0.8, 1.0 ) ), 1.0 );

      /* Continue the marching. */
      depth += zstep;
	}

   return color;
}
