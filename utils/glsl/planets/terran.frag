const float RADIUS = 0.8;
const float NORMAL_OFFSET = 0.005;

uniform vec3 ground_color     = vec3(0.44,0.36,0.25);
uniform vec3 sand_color       = vec3(0.68,0.58,0.40);
uniform vec3 mud_color        = vec3(0.22,0.15,0.08);
uniform vec3 mountain_color   = vec3(0.75,0.70,0.65);
uniform vec3 vegetation_dark_color   = vec3(0.10,0.14,0.06);
uniform vec3 vegetation_bright_color = vec3(0.19,0.26,0.12);
uniform vec3 bright_water_color      = vec3(0.15,0.25,0.40);
uniform vec3 deep_water_color = vec3(0.02,0.03,0.04);
uniform vec3 snow_color       = vec3(0.9,0.9,0.9);
uniform vec3 cloud_color      = vec3(0.75,0.75,0.75);
uniform float water_height = 0.65;
uniform float snow_width = 0.15;
uniform float vegetation_width = 0.3;
uniform float cloud_density = 0.5;
uniform float mountain_height = 0.5;
uniform float rotation_speed     = 0.05; // Rotation speed of the sphere.
uniform float cloud_scroll_speed = 0.01; // Rotation speed of cloud layer.
uniform float cloud_height = 0.025; // Distance of the cloud layer's shadow.
uniform float normal_scale = 0.01;  // Strength of the normal displacement.
uniform vec3 light_color = vec3(1.0,0.975,0.9);
uniform vec3 light_dir   = normalize(vec3(0.6, 0.2, -0.75));
uniform float ambient = 0.02;       // Ambient light.
uniform vec3 atmosphere_color_light = vec3(0.5, 0.6, 0.9); // Color of the atmosphere gradient.
uniform vec3 atmosphere_color_dark  = vec3(0.2, 0.4, 0.7);
uniform float atmosphere_density = 1.5;  // Opacity of the atmosphere.
uniform float atmosphere_radius  = 0.05; // Size of the atmopshere.

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
   for (int i=0; i<6; i++) {
      float noise = voronoi( p );
      t    += (1.0 - 2.0*noise) * amp;
      p    *= 2.0;
      amp  *= 0.75 - 0.25*noise;
   }
   return t / 2.0;
}


/* Compute coordinates on a sphere with radius RADIUS. */
vec3 sphere_coords( vec2 uv, float phase )
{
   float phi = asin(uv.x/RADIUS);
   float theta = asin(uv.y/RADIUS);
   vec3 pos = RADIUS*vec3(sin(phi+phase)*cos(theta), cos(phi+phase)*cos(theta), sin(theta));
   return pos;
}

/* Compute height map. */
float heigth( vec3 pos )
{
   float h = 0.5+0.25*snoise( 2.0*pos - vec3(u_seed) )+0.25*snoise( -pos + vec3(u_seed) );
   h += 0.1*(0.5+0.5*cnoise( 8.0*pos + vec3(u_seed) ));
   h += (0.05 + 0.5*mountain_height*(h-water_height/2.0)*h)*voronoi_rigded( 4.0*pos + hash(vec3(21*u_seed, -17*u_seed, 9*u_seed)) ) - 0.1;
   return h;
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
   float w = 0.25 - 0.75*voronoi_fbm( 15.0*pos ) + 0.5*voronoi_rigded( 17.0*pos );
   w = clamp(1.5*(w-0.5) + 0.5, 0.0, 1.0);
   vec3 vegetation_color = mix(vegetation_dark_color, vegetation_bright_color, w);
   return vegetation_color;
}

float cloud_texture( vec3 pos )
{
   float w = (0.7-0.7*voronoi( 4.0*pos + u_seed + 1.5*snoise( 0.5*pos + u_seed ) ))*(0.75 + 0.25*snoise( 1.5*pos ));
   float t = 2.0*abs(1.0 - 2.0*fract(cloud_scroll_speed*u_time)) - 1.0;
   float amp = 4.0*cnoise( 9.0*pos*cloud_scroll_speed*t ) + cos(12.0*cloud_scroll_speed*u_time);
   w = clamp(w*w, 0.0, 1.0);
   vec3 p = pos + 2.0*t*w*w*(clamp(2.0*w - 1.0, 0.0, 1.0)*cross(pos, vec3(snoise( amp*pos + u_seed ), snoise( amp*pos - u_seed ), 1.0)) + clamp(2.0-2.0*w, 0.0, 1.0)*cross(pos, vec3(0.707,0.707/2.0,snoise( 1.1*amp*pos + u_seed/2.0 ))));
   vec3 seed_offset = hash(vec3(9*u_seed, 17*u_seed, 21*u_seed));
   float h = (1.0-voronoi_rigded(vec3(0.4,0.4,0.6)*p + seed_offset)) + voronoi_fbm(vec3(1.8,1.8,2.2)*p - seed_offset) - 0.5 + cloud_density - 0.5;
   h = clamp((1.5*(h-0.5) + 0.5) * (0.5 + 0.5*cloud_density), 0.0, 1.0);
   return h;
}


/* Compute normal map */
vec2 normal( vec2 uv )
{
   vec2 uv1 = uv + vec2(  NORMAL_OFFSET, 0.0 );
   vec2 uv2 = uv + vec2( -NORMAL_OFFSET, 0.0 );
   vec2 uv3 = uv + vec2( 0.0,  NORMAL_OFFSET );
   vec2 uv4 = uv + vec2( 0.0, -NORMAL_OFFSET );
   float min_level = 0.9*water_height;
   float h1 = max(heigth( sphere_coords( uv1, rotation_speed*u_time ) ) - min_level, 0.0) / (1.0 - min_level);
   float h2 = max(heigth( sphere_coords( uv2, rotation_speed*u_time ) ) - min_level, 0.0) / (1.0 - min_level);
   float h3 = max(heigth( sphere_coords( uv3, rotation_speed*u_time ) ) - min_level, 0.0) / (1.0 - min_level);
   float h4 = max(heigth( sphere_coords( uv4, rotation_speed*u_time ) ) - min_level, 0.0) / (1.0 - min_level);
   vec2 norm = vec2( (h2*h2 - h1*h1)/(2.0*NORMAL_OFFSET), (h4*h4 - h3*h3)/(2.0*NORMAL_OFFSET));
   return norm;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = vec4(0.0);
   // Scaled UV coordinates.
   vec2 uv = 2.0*(screen_coords / dimensions - 0.5);
   // Scaled radius.
   float radius = length(uv) / RADIUS;
   vec2 norm2 = uv / RADIUS;
   // Normal vector orthogonal to sphere.
   vec3 norm = normalize( vec3( norm2, 1.0-length( norm2 ) ) );
   // Weight for atmosphere gradient.

   if (radius>0.9875) {
      // Render the outer atmosphere gradient.
      float a = max( norm2.x*norm2.x + norm2.y*norm2.y - RADIUS*RADIUS + atmosphere_radius*atmosphere_radius, 0.0 ) / (RADIUS*RADIUS);
      vec3 norm = normalize(vec3(uv/(RADIUS+atmosphere_radius), 1.0-length(uv/(RADIUS+atmosphere_radius))));
      float light = clamp(dot(norm, -light_dir), 0.0, 1.0);
      vec3 atmosphere_color;
      a = max(1.0-a/(RADIUS+atmosphere_radius), 0.0);
      a = min(4.0*a*a*a, 1.0);
      atmosphere_color = mix(atmosphere_color_dark, atmosphere_color_light, a);
      color_out.rgb = atmosphere_density*atmosphere_color*min(ambient + (1.0-ambient)*light_color*light, 1.0);
      color_out.a = clamp(4.0*a*atmosphere_density, 0.0, 1.0);
   }

   if (radius<=1.0) {
      // Render the surface.
      float light;
      float spec;
      vec3 atmosphere_color;
      float a = max( norm2.x*norm2.x + norm2.y*norm2.y - RADIUS*RADIUS + 8.0*atmosphere_radius*atmosphere_radius, 0.0 ) / (RADIUS*RADIUS + 8.0*atmosphere_radius*atmosphere_radius);
      // Smooth the edges.
      float alpha = clamp(100.0*(1.0-radius-0.25/100.0), 0.0, 1.0);
      vec3 pos = sphere_coords( uv, rotation_speed*u_time );
      vec3 cloud_pos = sphere_coords( uv*vec2(0.75,1.0), (rotation_speed+cloud_scroll_speed)*u_time );
      float h = heigth( pos );
      vec3 surface_color;
      vec3 terrain_color = ground_color;
      float water_weight = clamp(25.0*(water_height-h), 0.0, 1.0);
      vec3 water_color;
      float snow_weight = clamp(4.0*(max((snow_width-uv.y/RADIUS-1.0)/snow_width, (uv.y/RADIUS-snow_width-1.0+2.0*snow_width)/snow_width) + (h - water_height/2.0)/(1.0-water_height/2.0)), 0.0, 1.0);
      float cloud_weight = cloud_texture( cloud_pos );
      float cloud_shadow = clamp(cloud_texture( cloud_pos + cloud_height*light_dir ) - 0.8*cloud_weight*cloud_weight, 0.0, 1.0);
      float add_noise = 0.5 + 0.5*snoise( 32.0*pos );
      float ns = normal_scale*max(1.0 - cloud_weight, 0.0);

      if (water_weight < 1.0) {
         float sand_weight = sand_texture( pos );
         float mud_weight = mud_texture( pos );
         float vegetation_weight = clamp((2.0 - (0.25-abs(uv.y)/RADIUS)/0.125 + snoise( -6.0*pos + vec3(u_seed) ) + snoise( pos - vec3(u_seed) ))*vegetation_width, 0.0, 0.8 + 0.2*(0.5 + 0.5*snoise( 4.0*pos + vec3(u_seed) )));
         vec3 vegetation_color = vegetation_texture( pos );
         float height_weight = (h-water_height)/(1.0-water_height);
         height_weight = clamp((height_weight-0.1)*(height_weight+0.1), 0.0, 1.0);
         vegetation_weight *= vegetation_weight;
         terrain_color = mix(terrain_color, mud_color, mud_weight);
         terrain_color = mix(terrain_color, sand_color, sand_weight);
         terrain_color = mix(terrain_color, mountain_color, height_weight);
         terrain_color = mix(terrain_color, vegetation_color, vegetation_weight);
      }

      // Add terrain normals to normal vector.
      norm2 = (1.0-ns)*norm2 + ns*normal( uv );
      norm = normalize(vec3(norm2, 1.0-length(norm2)));
      // Compute light strength.
      light = clamp(dot(norm, -light_dir), 0.0, 1.0);
      a *= 2.0*a;
      atmosphere_color = mix(atmosphere_color_dark, atmosphere_color_light, a);

      spec = clamp(0.2 + 0.9*water_weight - 0.8*snow_weight, 0.0, 1.0);
      light += max(2.0 - water_weight - 1.5*cloud_weight, 0.0)*spec*exp(20.0*spec*(light-0.95));

      water_color = mix(deep_water_color, bright_water_color, min(0.5 + 0.5*clamp(1.0-(water_height-h)/h, 0.0, 1.0) + 0.2*light, 1.0));

      surface_color = mix(terrain_color*( 0.8 + 0.2*add_noise ), water_color*( 0.95 - 0.05*add_noise ), water_weight);
      surface_color = mix(surface_color, snow_color*( 0.9 + 0.1*add_noise ), snow_weight);
      // Add clouds.
      surface_color = mix(surface_color, cloud_color*min(0.75 + 0.5*cloud_weight, 1.0), cloud_weight);
      light *= max(1.0 - cloud_shadow, 0.5);
      // Adjust brightness and contrast.
      surface_color = clamp(0.9*(0.55*surface_color-0.6) + 0.55, 0.0, 1.0);
      // Add a gradient for the atmosphere.
      surface_color = clamp(surface_color + a*atmosphere_density*atmosphere_color, 0.0, 1.0);
      // Apply light.
      surface_color *= min(ambient + (1.0-ambient)*light_color*light, 4.0);
      color_out.rgb = min(mix(color_out.rgb, surface_color, (alpha+1.0-color_out.a)/2.0), 1.0);
      color_out.a = max(color_out.a, alpha);
   }

   return color_out;
}
