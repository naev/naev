const float RADIUS = 0.8;

uniform float mountain_height    = 0.25;  // Controls height scale of mountains.
uniform float rotation_speed     = 0.05;  // Rotation speed of the sphere.
uniform float normal_scale = 0.2;         // Strength of the normal displacement.
uniform float crater_depth = 0.1;         // How deep the craters look.
uniform vec3 light_color = vec3(1.0,0.975,0.9);
uniform vec3 light_dir   = normalize(vec3(0.6, 0.2, -0.75));
uniform float ambient = 0.02;       // Ambient light.
uniform vec3 atmosphere_color_light = vec3(0.9, 0.7, 0.6); // Color of the atmosphere gradient.
uniform vec3 atmosphere_color_dark  = vec3(0.3,0.25, 0.2);
uniform float atmosphere_density = 0.25;  // Opacity of the atmosphere.
uniform float atmosphere_radius  = 0.05;  // Size of the atmopshere.
uniform int depth_min_layers = 8;         // Min and max number of steps for parallax mapping.
uniform int depth_max_layers = 32;
uniform float depth_scale = 0.1;          // Amount of parallax mapping.

uniform sampler2D diffuse;
uniform sampler2D height;
uniform sampler2D normal;

uniform float u_time;
uniform vec2 dimensions;
uniform int u_seed = 0;


vec3 hash( vec3 x )
{
   x = vec3(   dot(x,vec3(127.1,311.7, 74.7)),
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


/* Compute coordinates on a sphere with radius RADIUS. */
vec3 sphere_coords( vec2 uv, float phase )
{
   float phi = asin(uv.x/RADIUS);
   float theta = asin(uv.y/RADIUS);
   vec3 pos = RADIUS*vec3(sin(phi+phase)*cos(theta), cos(phi+phase)*cos(theta), sin(theta));
   return pos;
}

float get_depth( vec2 uv )
{
   return texture( height, uv ).g;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 color_out = vec4(0.0);
   // Scaled UV coordinates.
   vec2 uv = 2.0*(texture_coords - 0.5)*vec2(dimensions/min(dimensions.x, dimensions.y));
   // Scaled radius.
   float radius = length(uv) / RADIUS;
   vec2 norm2 = uv / RADIUS;
   // Normal vector orthogonal to sphere.
   vec3 norm = normalize( vec3( norm2, 1.0-length( norm2 ) ) );

   if (radius>0.75) {
      // Render the outer atmosphere gradient.
      // Weight for atmosphere gradient.
      float a = max( norm2.x*norm2.x + norm2.y*norm2.y - RADIUS*RADIUS + atmosphere_radius*atmosphere_radius, 0.0 ) / (RADIUS*RADIUS);
      vec3 norm = normalize(vec3(uv/(RADIUS+atmosphere_radius), 1.0-length(uv/(RADIUS+atmosphere_radius))));
      float light = clamp(dot(norm, -light_dir), 0.0, 1.0);
      vec3 atmosphere_color;
      a = max(0.75-a/(RADIUS+1.5*0.25+atmosphere_radius), 0.0);
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
      // Rescaled and rotated UV coordinates.
      vec2 p = fract(vec2(asin(uv.x/RADIUS/sqrt(1.0-uv.y*uv.y/RADIUS*RADIUS))/M_PI/4.0, 0.5 + asin(uv.y/RADIUS)/M_PI) + 0.25*vec2(rotation_speed/M_PI*u_time, 0.0));

      {
         // Parallax mapping.
         vec3 view_dir = vec3(uv.x*uv.x*sign(uv.x)/RADIUS/RADIUS, uv.y*uv.y*sign(uv.y)/RADIUS/RADIUS, 1.0-(uv.x*uv.x + uv.y*uv.y)/RADIUS/RADIUS);
         // Max number of steps.
         float depth_layers = mix(float(depth_max_layers), float(depth_min_layers), abs(dot(vec3(0,0,1), view_dir)));
         float layer_depth = 1.0 / depth_layers;
         float current_layer_depth = 0.0;
         // Offset direction.
         vec2 dir = view_dir.xy * depth_scale * vec2(0.25, 1.0);
         // Step size.
         vec2 delta = dir / depth_layers;
         vec2 ofs = p;
         float depth = get_depth(ofs);
         float current_depth = 0.0;
         while (current_depth < depth) {
            // Iterate until depth is reached.
            ofs -= delta;
            depth = get_depth(fract(ofs));
            current_depth += layer_depth;
         }
         vec2 prev_ofs = ofs + delta;
         float after_depth  = depth - current_depth;
         float before_depth = get_depth(prev_ofs) - current_depth + layer_depth;
         float weight = after_depth / (after_depth - before_depth);
         ofs = mix(ofs, prev_ofs, weight);
         p = fract(ofs);
      }

      float a = max( norm2.x*norm2.x + norm2.y*norm2.y - RADIUS*RADIUS + 8.0*atmosphere_radius*atmosphere_radius, 0.0 ) / (RADIUS*RADIUS + 8.0*atmosphere_radius*atmosphere_radius);
      // Smooth the edges.
      float alpha = clamp(100.0*(1.0-radius-0.005), 0.0, 1.0);
      float h = get_depth( p );
      vec3 surface_color = texture(diffuse, p).rgb;

      // Add terrain normals to normal vector.
      norm2 = mix(norm2, texture( normal, p ).xy, normal_scale);
      norm = normalize(vec3(norm2, 1.0-length(norm2)));
      // Compute light strength for terrain.
      light = clamp(dot(norm, -light_dir), 0.0, 1.0);
      a *= 2.0*a;
      atmosphere_color = mix(atmosphere_color_dark, atmosphere_color_light, a);
      // Blend off lowered terrain on the edges.
      alpha *= clamp(50.0*(1.01 - (radius+crater_depth*(0.9-h))), 0.0, 1.0);
      // Specular blob.
      spec = 0.5*max(1.0 + min(h, 0.0), 0.0);
      light += spec*exp(20.0*spec*(light-0.95));
      // Apply light.
      surface_color *= min(ambient + (1.0-ambient)*light_color*light, 4.0);
      // Add surface color.
      color_out.rgb = min(mix(color_out.rgb, surface_color, alpha), 1.0);
      color_out.a = max(color_out.a, alpha);
      // Add a gradient for the atmosphere.
      color_out.rgb = clamp(color_out.rgb + a*atmosphere_density*alpha*(ambient + (1.0-ambient)*light)*atmosphere_color, 0.0, 1.0);
   }

   return color_out;
}
