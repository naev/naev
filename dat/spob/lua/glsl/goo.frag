
const float RADIUS = 0.8;
const float NORMAL_OFFSET = 0.01;
uniform vec3 base_color   = vec3(0.4,0.4,0.4); // Surface color.
uniform vec3 effect_color = vec3(0.4,0.3,0.5); // Color variation.
uniform float rotation_speed = 0.01;  // Rotation speed of the sphere.
uniform float speed = 0.04;           // Speed of the displacement effect.
uniform float effect_strength = 0.1;  // Strength of the color modulation.
uniform float normal_scale = 0.003;   // Strength of the normal displacement.
uniform float noise_scale = 2.0;      // Scales amount of "rings".
uniform float water_height = 0.45;    // Flatten the heightmap below this.
uniform vec3 light_color = vec3(1.0,0.975,0.9);
uniform vec3 light_dir = normalize(vec3(0.6, 0.15, -0.4));
uniform float ambient = 0.02;         // Ambient light.
uniform vec3 atmosphere_color_light = vec3(0.6, 0.5, 0.9); // Color of the atmosphere gradient.
uniform vec3 atmosphere_color_dark  = vec3(0.4, 0.3, 0.8);
uniform float atmosphere_density = 1.0; // Opacity of the atmosphere.
uniform float atmosphere_radius = 0.1;  // Size of the atmopshere.

uniform float u_time;

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


/* Compute coordinates on a sphere with radius RADIUS. */
vec3 sphere_coords( vec2 uv )
{
   float phi = asin(uv.x/RADIUS);
   float theta = asin(uv.y/(RADIUS));
   vec3 pos = RADIUS*vec3(sin(phi+rotation_speed*u_time)*cos(theta), cos(phi+rotation_speed*u_time)*cos(theta), sin(theta));
   return pos;
}

/* Compute height map. */
float heigth( vec3 pos )
{
   float radius = 3.5+1.0*sin(speed/4.0*u_time);
   float h = noise_scale*voronoi( radius*pos );
   h = 2.0*voronoi( 0.5 * (1.0+h) * vec3(cos(pos.x), sin(pos.x), 16.0+6.0*abs(1.0-2.0*fract(speed/20.0*u_time))) );
   h = clamp(3.0*(h-(1.0+radius/4.0)*water_height), 0.0, 1.0);
   return h;
}

/* Compute normal map */
vec2 normal( vec2 uv )
{
   vec2 uv1 = uv + vec2(  NORMAL_OFFSET, 0.0 );
   vec2 uv2 = uv + vec2( -NORMAL_OFFSET, 0.0 );
   vec2 uv3 = uv + vec2( 0.0,  NORMAL_OFFSET );
   vec2 uv4 = uv + vec2( 0.0, -NORMAL_OFFSET );
   float h1 = heigth( sphere_coords( uv1 ) );
   float h2 = heigth( sphere_coords( uv2 ) );
   float h3 = heigth( sphere_coords( uv3 ) );
   float h4 = heigth( sphere_coords( uv4 ) );
   vec2 norm = vec2( (h2 - h1)/(2.0*NORMAL_OFFSET), (h4 - h3)/(2.0*NORMAL_OFFSET));
   return norm;
}


vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = vec4(0.0);
   // Scaled UV coordinates.
   vec2 uv = 2.0*(texture_coords - 0.5);
   // Scaled radius.
   float radius = length(uv) / RADIUS;
   vec2 norm2 = uv / RADIUS;
   // Normal vector orthogonal to sphere.
   vec3 norm = normalize( vec3( norm2, 1.0-length( norm2 ) ) );
   // Weight for atmosphere gradient.
   float a = max( norm2.x*norm2.x + norm2.y*norm2.y - RADIUS*RADIUS + atmosphere_radius*atmosphere_radius, 0.0 ) / (RADIUS*RADIUS);

   if (radius>0.9875) {
      // Render the outer atmosphere gradient.
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
      vec3 atmosphere_color;
      // Smooth the edges.
      float alpha = clamp(100.0*(1.0-radius-0.25/100.0), 0.0, 1.0);
      float h = heigth( sphere_coords( uv ) );
      vec3 surface_color;
      // Add terrain normals to normal vector.
      norm2 = (1.0-normal_scale*effect_strength)*norm2 + normal_scale*effect_strength*normal( uv );
      norm = normalize(vec3(norm2, 1.0-length(norm2)));
      // Compute light strength.
      light = clamp(dot(norm, -light_dir), 0.0, 1.0);
      a *= 2.0*a;
      atmosphere_color = mix(atmosphere_color_dark, atmosphere_color_light, a);

      surface_color = base_color + effect_strength*vec3(h)*effect_color;
      // Add a gradient for the atmosphere.
      surface_color = clamp(surface_color + a*atmosphere_density*atmosphere_color, 0.0, 1.0);
      // Apply light.
      surface_color *= min(ambient + (1.0-ambient)*light_color*light, 1.0);
      color_out.rgb = mix(color_out.rgb, surface_color, (alpha+1.0-color_out.a)/2.0);
      color_out.a = max(color_out.a, alpha);
   }

   return color_out;
}
