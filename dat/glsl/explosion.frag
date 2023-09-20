#include "lib/math.glsl"
#include "lib/simplex.glsl"
#include "lib/gamma.glsl"

/* Common uniforms for special effects. */
uniform float u_time;   /**< Elapsed time. */
uniform float u_r;      /**< Random seed. */

/* Main constants. */
const float CAM_DIST = 2.0;         /**< Distance of the camera from the origin. Defaults to 2.0. */
const float FOV = 0.5;              /**< Determines the pseudo-field of view computation. Defaults to 0.5. */
const float RADIUS = 1.0;           /**< Radius of sphere enveloping explosion, used to determine what voxels to compute. Defaults to 1.0. */
const float DENSITY = 1.35;         /**< Determines the contrast between dark and bright colors. Low values make it more blurry and transparent. Defaults to 1.35. */
const float SPHERICALNESS = 1.0;    /**< Determines how spherical the explosion is. Higher values increase sphericalness. Defaults to 1.0. */
const float SPHERE_GROWTH = 2.2;    /**< Determines how fast the explosion spheres grow. Lower values increase growth speed. Defaults to 2.2. */
const float SMOOTHING = 0.4;        /**< Smooths the outter part of the spheres. Should be a value between 0.0 and 1.0 where 1.0 is no smoothing. Defaults to 0.4. */
const float COLOR_EVENNESS = 0.25;  /**< Controls the evenness of the colors in the explosion. Higher values correspond to more even explosions. Defaults to 0.25. */
const float BRIGHTNESS_OFFSET = 3.0;/**< Determines the brightness offset. Defaults to 3.0. */
const float BRIGHTNESS_VELOCITY = 2.2;/**< Determines the speed at which the brightness changes. Defaults to 2.2. */
const float BRIGHTNESS_RADIUS_OFFSET = 1.3;/**< Determines the radius-dependent brightness offset. Defaults to 1.3. */
const float CONTRAST = 1.0;         /**< Final colour contrast. Higher values correspond to less contrast. Defaults 1.0. */

/* Uniforms. */
uniform float u_smokiness;     /**< Lower value saturates the explosion, while higher values give it a smokey look. Default to 0.588. */
uniform float u_grain;           /**< Determines the details of the explosions. Increasing it likely requires increasing step size. Defaults to 0.5. */
uniform float u_speed;           /**< How fast the animation plays. Total play time is 1.0/u_speed. */
uniform vec4 u_colorbase; /**< Base colour of the explosion. Defaults to {1.2, 0.9, 0.5, 0.7}. */
uniform vec4 u_colorsmoke; /**< Colour to use for the smoke, most likely shouldn't be changed. Defaults to {0.15, 0.15, 0.15, 0.1}. */
uniform int u_steps;              /**< How many steps to march. Defaults to 16. */
uniform float u_roll_speed;      /**< How fast the sphere cloud effects roll. Defaults to 1.0. */
uniform float u_roll_dampening;  /**< How fast the sphere cloud effects roll gets dampened. Defaults to 0.7. */
uniform float u_smoke_fade;      /**< Determines how the smake fades. Larger values leave more smoke at the end. Defaults to 1.4. */

/* Noise function to use. Has to be in the [0,1] range. */
float noise( vec3 x )
{
   return snoise( x )*0.5 + 0.5;
}

/* Simple Fractal brownian motion with some rotation. */
float fbm( vec3 p, vec3 dir )
{
   const float SUM = 0.5 + 0.25 + 0.125;// + 0.0625 + 0.03125;
   float f;
   vec3 q = p - dir; f  = noise( q ) * 0.50000 / SUM;
   q = q*2.02 - dir; f += noise( q ) * 0.25000 / SUM;
   q = q*2.03 - dir; f += noise( q ) * 0.12500 / SUM;
   //q = q*2.01 - dir; f += noise( q ) * 0.06250 / SUM;
   //q = q*2.02 - dir; f += noise( q ) * 0.03125 / SUM;
   return f;
}

/* Determines the colour given density and other parameters. */
vec4 compute_colour( float density, float radius, float brightness )
{
   /* Base colour is defined by density, and gives the impression of occlusions
   within the media. */
   vec4 col = vec4( vec3(mix( 1.0, COLOR_EVENNESS, density )), density );
   /* We then mix colour in for the explosion. */
   col *= mix( u_colorbase, u_colorsmoke, min( (radius+0.5)*u_smokiness, 1.0 ) ) * brightness;
   return col;
}

/* Compute the density at a 3D position for a single sphere.
 The idea here is to compute two growing spheres. The first grows fast at the
 beginning and is solid, while the second one hollows out the first one and
 grows slow at first, but faster at the end.
 Noise is adding in the form of a rotating FBM in inverted space.
 */
float compute_density( vec3 p, float r, float t, vec3 dir )
{
   /* Base density is defined by the sphere itself. */
   float den = SPHERICALNESS + (SPHERE_GROWTH+SPHERICALNESS)*log(t)*r;
   /* Density gets hollowed out afterwards. */
   den -= (2.5+SPHERICALNESS) * pow(t, u_smoke_fade) / r;

   /* We skip small densities and make them seem flat. */
   if (den <= -3.0)
      return -1.0;

   /* We modify the pshere direction by rolling and dampening it. */
   float s  = 33.7*u_r - (u_roll_speed / (sin(min(t*3.0, 1.57)) + u_roll_dampening));
   dir     *= s;

   /* Invert the space. */
   p        = -u_grain*p / (dot(p,p));

   /* Calculate fractal brownian motion that is partially rotated. */
   float f  = fbm( p, dir );

   /* Scale the noise. */
   den     += 4.0*f;

   return den * DENSITY;
}

/* Sample on a sphere.
 * Based on http://mathworld.wolfram.com/SpherePointPicking.html
 * Assume rand in [0,1] range. */
vec3 sample_sphere( vec2 rand )
{
   float theta = rand.x * 2.0 * M_PI;
   float u = rand.y*2.0-1.0;
   float vmod = sqrt(1.0 - u*u);
   return vec3( vmod * cos(theta), vmod * sin(theta), u );
}

vec4 effect( vec4 color, sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   /* Normalized 0 centered coordinates. */
   vec2 uv = 2.0*texture_coords-1.0;

   /* Set up camera stuff, most is constant */
   const vec3 eye    = vec3( CAM_DIST, 0.0, 0.0 );
   const vec3 center = vec3( 0.0 ); /* Looking at center. */
   const vec3 at     = normalize( center - eye );
   /*
   const vec3 up  = vec3( 0.0, 1.0, 0.0 );
   const vec3 cx  = normalize( cross( at, up ) );
   vec3 ray = normalize( at + fov * uv.x * cx + fov * uv.y * up );
   */
   vec3 ray = normalize( at + FOV * vec3( 0.0, uv ) ); /* Simplified in case of fixed camera, but depends on p. */

   /* Sphere intersection for sphere at origin to see if pixel is in explosion.
   Sphere equation: |xyz| = r
   Position xyz is given by pos + t * ray
   Squaring it we get pos^2 + 2 * pos * ray * t + t^2 = r^2
   Solve for value t we get quadratic equation:
      t^2 + (2*pos*ray)*t + (pos^2 - r^2) = 0
   Also, express the pos^2 term as CAM_DIST*CAM_DIST since we've seen an old Radeon driver fail to recognize dot() as constexpr.
   */
   float b = dot( eye, ray ); /* We don't add the 2 here because
   it gets cancelled out by the 4 in the 4ac term in the sqrt. */
   const float c = CAM_DIST * CAM_DIST - RADIUS * RADIUS;
   float h = b*b - c; /* inside the square root (b^2-4ac) */
   /* Imaginary solution - no real intersection, we're done here. */
   if (h < 0.0)
      return vec4(0.0);
   /* Real solution. */
   h = sqrt(h); /* this term is multiplied by 2, but since a=2, we don't need to correct. */
   float inear = -b-h; /* near to camera intersection */
   float ifar  = -b+h; /* far from camera intersection */

   /* Fraction of total time run. */
   float progress = u_time * u_speed;

   /* Main sphere is centered on the origin. */
   vec3 roll_dir = sample_sphere( vec2(random(u_r), random(u_r+1.0)) );

   /* Base step is equally spread out through the sphere. */
   float step_base = RADIUS / float(u_steps);

   /* Normalize the progress to take into account explosion delays and represent the whole animation. */
   float smooth_t    = sin(progress*2.1);
   float brightness  = BRIGHTNESS_OFFSET + BRIGHTNESS_VELOCITY * pow( 1.0-progress, 2.0);

   /* Begin marching. */
   float depth = inear; /* We start marching at the first intersection. */
   vec4 sum = vec4( 0.0 );
   for (int i=0; i<u_steps; i++) {

      /* Stop when solid. */
      if (sum.a >= 1.0)
         break;

      /* Stop when we get to the other side. */
      if (depth > ifar)
         break;

      /* Compute the density and start marching through it. */
      vec3 pos       = eye + ray*depth; /* Move along ray. */

      /* See if out of range. */
      float radius = 1.0 / RADIUS * length(pos);
      /* Out of range, so just march ahead. */
      if (radius > 1.0) {
         depth    += step_base * 2.0;
         continue;
      }

      /* Compute density. */
      float density = compute_density( pos, radius, progress, roll_dir );
      /* No density, so just march ahead. */
      if (density <= 0.0) {
         depth    += step_base * 2.0;
         continue;
      }

      /* Smooth out the smoke at the far end so you don't see the clipping
      with the main sphere. Does not change the sign of the density.  */
      density *= 1.0 - smoothstep( SMOOTHING, 1.0, radius );

      /* Compute the colour based on the density. */
      float densityc = clamp( density, 0.0, 1.0 );
      vec4 col   = compute_colour( densityc, radius*(BRIGHTNESS_RADIUS_OFFSET+density), brightness );

      /* Uniform scale density. */
      col.a   *= (col.a + 0.4) * (1.0 - radius*radius);

      /* Alpha premultiply. */
      col.rgb *= col.a;

      /* Blend in the contribution of the sphere. */
      sum   += col*(1.0 - sum.a);
      sum.a += 0.15 * col.a;

      /* Heuristics to move faster through low density areas. */
      float step_mod = 1.0 + (1.0-max(density+col.a, 0.0));
      /* March along the ray. */
      depth  += step_base * step_mod;
   }

   vec4 col = clamp( sum, 0.0, 1.0 );
   col.rgb  = col.rgb*col.rgb*(1.0+CONTRAST-CONTRAST*col.rgb);
   col.rgb  = gammaToLinear( col.rgb );
   return col;
}

#ifndef _LOVE
in vec2 pos;
out vec4 color_out;
uniform sampler2D dummy;
void main (void)
{
   color_out = effect( vec4(0.0), dummy, pos, vec2(0.0) );
}
#endif
