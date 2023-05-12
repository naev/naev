
uniform vec3 terrain_color  = vec3( 0.1, 0.12, 0.15);
uniform vec3 building_color = vec3(0.65,  0.7,  0.6);
uniform vec3 detail_color   = vec3( 0.4,  0.2,  0.1);

uniform sampler2D height;
uniform sampler2D normal;

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

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = vec4(1.0);
   // Scaled UV coordinates.
   vec2 uv = screen_coords / dimensions;
   // Render the height map.
   float h = 1.0 - texture( height, uv*vec2(0.5,1.0) ).g;
   vec2 normal = texture( normal, uv*vec2(0.5,1.0)).rg;
   vec2 tangent = normal.yx * vec2(1.0, -1.0);
   vec3 pos = sphere_coords( M_PI * uv*vec2(1.0,0.5) );
   float add_noise = 0.5 + 0.5*snoise( 43.0*pos );

   color_out.rgb = mix( terrain_color, building_color, h );
   color_out.rgb = mix( color_out.rgb, detail_color, clamp( ((0.5 - 0.5*cos(M_PI*40.0*dot(tangent, uv*mat2(vec2(cos(2.0*M_PI*h),sin(2.0*M_PI*h)),vec2(-sin(2.0*M_PI*h),cos(2.0*M_PI*h))))))*(h-0.125)*6.0 - 0.5), 0.0, 1.0) );
   color_out.rgb = mix(vec3(add_noise), color_out.rgb, clamp(1.0 - 0.75*h*h, 0.0, 1.0));

   return color_out;
}
