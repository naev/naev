
uniform vec3 ground_color_bright = vec3(0.6 ,0.7 , 0.8);
uniform vec3 ground_color_dark   = vec3(0.05,0.04,0.03);
uniform vec3 cyan_ice_color      = vec3(0.2, 0.3, 0.4 );
uniform vec3 bright_water_color  = vec3(0.15,0.25,0.40);
uniform vec3 deep_water_color    = vec3(0.02,0.03,0.04);
uniform float water_height = 0.3;
uniform float mountain_height = 0.1;
uniform float cap_width = 0.25;

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

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = vec4(1.0);
   // Scaled UV coordinates.
   vec2 uv = screen_coords / dimensions;
   // Render the height map.
   float h = texture( height, uv*vec2(0.5,1.0) ).g;
   vec3 pos = sphere_coords( M_PI * uv*vec2(1.0,0.5) );
   float n = 1.0-voronoi_rigded( 2.0*pos + vec3(0,0,u_seed) );
   float d = clamp(max(1.0 - max(uv.y, 1.0 - uv.y), 0.0) - cap_width/2.0*n - 0.2*(h - 1.0), 0.0, 1.0);

   color_out.rgb = mix(mix(ground_color_dark, ground_color_bright, clamp(0.5*((h + water_height)/(1.0 - water_height) - water_height), 0.0, 1.0)), cyan_ice_color, clamp(0.5*(1.0-20.0*(h-1.5*water_height)*(h-1.5*water_height)), 0.0, 1.0));
   color_out.rgb = mix(color_out.rgb, mix(deep_water_color, bright_water_color, h/water_height), clamp(10.0*(water_height - h)/water_height, 0.0, 1.0));
   color_out.rgb = mix(color_out.rgb, ground_color_bright, 10.0*max(cap_width - d, 0.0)/cap_width);

   return color_out;
}
