
uniform float mountain_height = 0.1;

uniform float u_time;
uniform vec2 dimensions;
uniform int u_seed = 0;


vec3 hash( vec3 x )
{
   x = vec3( dot(x,vec3(127.1,311.7, 74.7)),
             dot(x,vec3(269.5,183.3,246.1)),
             dot(x,vec3(113.5,271.9,124.6)));
   return fract(sin(x)*43758.5453123);
}

/* 3D Voronoi- Inigo Qquilez (MIT) */
float voronoi( vec3 p )
{
   vec3 b, r, g = floor(p);
   p = fract(p);
   float d = 1.0;
   for(int j = -1; j <= 1; j++) {
      for(int i = -1; i <= 1; i++) {
         b = vec3(i, j, -1);
         r = b - p + hash(g+b);
         d = min(d, dot(r,r));
         b.z = 0.0;
         r = b - p + hash(g+b);
         d = min(d, dot(r,r));
         b.z = 1.0;
         r = b - p + hash(g+b);
         d = min(d, dot(r,r));
      }
   }
   return d;
}

/* Fractal brownian motion with voronoi! */
float voronoi_fbm( in vec3 p )
{
   float t = 0.0;
   float amp = 1.0;
   for (int i=0; i<5; i++) {
      t    += voronoi( p ) * amp;
      p    *= 2.0;
      amp  *= 0.5;
   }
   return t / 2.0;
}

float voronoi_rigded( in vec3 p )
{
   float t = 0.0;
   float amp = 1.0;
   for (int i=0; i<5; i++) {
      float noise = voronoi( p );
      t    += (1.0 - 2.0*noise) * amp;
      p    *= 2.0;
      amp  *= 0.75 - 0.25*noise;
   }
   return t / 2.0;
}


/* Compute coordinates on a sphere with radius 1. */
vec3 sphere_coords( vec2 uv )
{
   vec3 pos = vec3(sin(uv.x)*cos(uv.y), cos(uv.x)*cos(uv.y), sin(uv.y));
   return pos;
}

/* Compute height map. */
float heigth( vec3 pos )
{
   vec3 ofs = hash(vec3(21*u_seed, -17*u_seed, 9*u_seed));
   float n = voronoi_rigded( 4.0*pos + ofs );
   float h = 1.25*voronoi( -0.25 * voronoi( 5.0*pos + ofs ) + 2.0*pos - ofs );
   float s = clamp(2.0*(0.5 - voronoi_fbm( 2.0*pos + pos*ofs )) + 0.25, 0.0, 1.0);
   h = mix(1.0 * (0.7 + 0.2*n) * h - 0.1, 1.0, 8.0*(0.75 + 0.25*n)*max(0.2 - h + 0.1*n, 0.0));
   h = mix(h, 1.2 - 0.1*n, s*s*s);
   h = clamp(h, 0.0, 1.0) * (1.0-mountain_height + mountain_height*(0.5 + 0.5*cnoise( 32.0 * (pos + ofs) )));
   return h;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = vec4(0.0);
   // Scaled UV coordinates.
   vec2 uv = 2.0*(screen_coords / dimensions - 0.5);
   // Sphere coordinates.
   vec3 pos = sphere_coords( M_PI * uv*vec2(1.0,0.5) );
   // Render the height map.
   float h = heigth( pos );

   color_out = vec4(vec3(h),1.0);

   return color_out;
}
