
uniform vec3 ground_color        = vec3(0.44,0.36,0.25);
uniform vec3 ground_color_bright = vec3( 0.7,0.65,0.6 );
uniform vec3 ground_color_dark   = vec3(0.1, 0.08,0.07);
uniform vec3 lava_color          = vec3( 0.8, 0.1,0.05);
uniform float lava_height = -1.0;
uniform float mountain_height = 0.5;

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

float voronoi_rigded2( in vec3 p )
{
   float t = 0.0;
   float amp = 2.0;
   for (int i=0; i<5; i++) {
      float noise = voronoi( p );
      t    += (1.0 - 2.0*noise) * amp;
      p    *= 2.0;
      amp  *= 0.75 - 0.5*noise;
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
   float n = voronoi_rigded( 4.0*pos + hash(vec3(21*u_seed, -17*u_seed, 9*u_seed)) );
   float h = 1.0 - voronoi_rigded2( vec3(0.75,0.75,0.3)*(1.0+0.2*n)*pos + 0.5*hash(vec3(3*u_seed, 5*u_seed, 7*u_seed)) );
   h = (0.5 + 0.5*mountain_height)*(min(h, 0.0)*min(h, 0.0) + 2.0*max(-h, 0.0) + n)*(h + 0.25*mountain_height*n);
   return h;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = vec4(1.0);
   // Scaled UV coordinates.
   vec2 uv = 2.0*(screen_coords / dimensions - 0.5);
   // Sphere coordinates.
   vec3 pos = sphere_coords( M_PI * uv*vec2(1.0,0.5) );
   // Render the height map.
   float h = heigth( pos );
   vec3 lava_glow = lava_color*4.0*max(lava_height - h - 0.5, 0.0);

   color_out.rgb = mix(mix(ground_color, ground_color_bright, clamp(1.0-abs(h - 0.85), 0.0, 1.0)), ground_color_dark, clamp(1.0-abs(h + 0.5), 0.0, 1.0));
   color_out.rgb += lava_glow;

   return color_out;
}
