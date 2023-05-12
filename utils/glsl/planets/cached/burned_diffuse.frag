
uniform vec3 ground_color       = vec3(0.44,0.36,0.25);
uniform vec3 sand_color         = vec3(0.61,0.54,0.40);
uniform vec3 mud_color          = vec3(0.22,0.08,0.15);
uniform vec3 mountain_color     = vec3(0.75,0.70,0.65);
uniform vec3 vegetation_dark_color   = vec3(0.15,0.04,0.13);
uniform vec3 vegetation_bright_color = vec3(0.19,0.26,0.12);
uniform vec3 bright_water_color      = vec3(0.20,0.25,0.40);
uniform vec3 deep_water_color   = vec3(0.09,0.01,0.03);
uniform vec3 snow_color         = vec3(0.9,0.9,0.9);
uniform float water_height = 0.55;
uniform float snow_width = 0.15;
uniform float vegetation_width = 0.2;
uniform float cloud_density = 0.5;
uniform float mountain_height = 0.5;

uniform sampler2D height;

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
   for (int i=0; i<6; i++) {
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

/* Compute textures. */
float sand_texture( vec3 pos )
{
   float h = clamp(2.5*(0.5*snoise( 8.0*pos + vec3(u_seed) )+0.25*snoise( -16.0*pos - vec3(u_seed) )-0.15) + 0.5, 0.0, 1.0);
   return h;
}

float mud_texture( vec3 pos )
{
   float h = clamp(1.25*(0.5*cnoise( 16.0*pos - vec3(u_seed) )+0.25*snoise( 12.0*pos + vec3(u_seed) )) + 0.5, 0.0, 1.0);
   return h;
}

vec3 vegetation_texture( vec3 pos )
{
//   float w = clamp(0.5*cnoise( 2.0*pos - vec3(u_seed) )+0.5*cnoise( 16.0*pos + vec3(u_seed) ) + 0.65, 0.0, 1.0);
   float w = 0.25 - 0.75*voronoi_fbm( 15.0*pos + vec3(0,0,1) ) + 0.5*voronoi_rigded( 17.0*pos - vec3(0,0,1) );
   w = clamp(1.5*(w-0.5) + 0.5, 0.0, 1.0);
   vec3 vegetation_color = mix(vegetation_dark_color, vegetation_bright_color, w);
   return vegetation_color;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = vec4(1.0);
   // Scaled UV coordinates.
   vec2 uv = screen_coords / dimensions;
   // Render the height map.
   float h = texture( height, uv*vec2(0.5,1.0) ).g;
   vec3 pos = sphere_coords( M_PI * uv*vec2(1.0,0.5) );
   float water_weight = clamp(15.0*(water_height-h), 0.0, 1.0);
   vec3 water_color = mix(deep_water_color, bright_water_color, min(0.5 + 0.5*clamp(1.0-(water_height-h)/h, 0.0, 1.0), 1.0));
   float add_noise = 0.5 + 0.5*snoise( 48.0*pos );
   vec3 surface_color;
   vec3 terrain_color;

   if (water_weight < 1.0) {
         float sand_weight = sand_texture( pos );
         float mud_weight = mud_texture( pos );
         float vegetation_weight = clamp((2.0 - (0.25-abs(uv.y))/0.125 + snoise( -6.0*pos + vec3(u_seed) ) + snoise( pos - vec3(u_seed) ))*vegetation_width, 0.0, 0.8 + 0.2*(0.5 + 0.5*snoise( 4.0*pos + vec3(u_seed) )));
         vec3 vegetation_color = vegetation_texture( pos );
         float height_weight = (h-water_height)/(1.0-water_height);
         height_weight = clamp((height_weight-0.1)*(height_weight+0.1), 0.0, 1.0);
         vegetation_weight *= vegetation_weight;
         terrain_color = mix(terrain_color, mud_color, mud_weight);
         terrain_color = mix(terrain_color, sand_color*(1.0 - 0.4*add_noise), sand_weight);
         terrain_color = mix(terrain_color, mountain_color, height_weight);
         terrain_color = mix(terrain_color, vegetation_color, vegetation_weight);
      }

   // Add water.
   surface_color = mix(terrain_color*( 0.8 + 0.2*add_noise ), water_color*( 0.95 - 0.05*add_noise ), water_weight);

   color_out.rgb = surface_color;

   return color_out;
}
