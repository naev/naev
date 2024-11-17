const float RADIUS = 0.8;

uniform vec3 atmosphere_color_light = vec3(0.5, 0.6, 0.9); // Color of the atmosphere gradient.
uniform vec3 atmosphere_color_dark  = vec3(0.6, 0.2, 0.4);
uniform vec3 glow_color_bright      = vec3(0.8, 0.5, 0.3);
uniform vec3 glow_color_dark        = vec3(0.3, 0.2, 0.6);
uniform float rotation_speed     = 0.05;  // Rotation speed of the sphere.
uniform float normal_scale = 0.2;         // Strength of the normal displacement.
uniform vec3 light_color = vec3(1.0,0.975,0.9);
uniform vec3 light_dir   = normalize(vec3(0.7, 0.2, -0.3));
uniform float ambient = 0.02;       // Ambient light.
uniform float atmosphere_density = 1.0;   // Opacity of the atmosphere.
uniform float atmosphere_radius  = 0.05;  // Size of the atmopshere.
uniform float glow_speed = 0.025;

uniform sampler2D diffuse;
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

float glow_strength( vec3 pos, float h )
{
   float m = clamp(-0.25 + snoise( 4.0*pos - vec3(u_seed) + vec3(0.1*glow_speed*u_time, glow_speed*u_time, -0.05*glow_speed*u_time) ) + snoise( 3.0*pos + vec3(u_seed) + vec3(-0.1*glow_speed*u_time, -glow_speed*u_time, 0.05*glow_speed*u_time) ), 0.0, 1.0);
   m *= clamp(4.0*( 0.1 - 0.3*min(min(abs(h-0.75 + 0.1*abs(1.0-2.0*fract(1.3*glow_speed*u_time)) ), 2.0*abs(h-0.25 + 0.1*abs(1.0-2.0*fract(0.9*glow_speed*u_time)))), 1.5*abs(h-0.5 - 0.1*abs(1.0-2.0*fract(0.7*glow_speed*u_time)))) ), 0.0, 1.0);
   m = pow(m, 0.5 + 1.5*abs(1.0-2.0*fract(pos.x + glow_speed*u_time + 2.0*glow_speed*m)));
   return m;
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

   if (radius>0.85) {
      // Render the outer atmosphere gradient.
      // Weight for atmosphere gradient.
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
      // Rescaled and rotated UV coordinates.
      vec2 p = fract( vec2( (asin(uv.x/RADIUS/sqrt(1.0-uv.y*uv.y/RADIUS*RADIUS))/M_PI + rotation_speed/M_PI*u_time)/4.0, 0.5 + asin(uv.y/RADIUS)/M_PI ) );
      float a = max( norm2.x*norm2.x + norm2.y*norm2.y - RADIUS*RADIUS + 8.0*atmosphere_radius*atmosphere_radius, 0.0 ) / (RADIUS*RADIUS + 8.0*atmosphere_radius*atmosphere_radius);
      // Smooth the edges.
      float alpha = clamp(100.0*(1.0-radius-0.005), 0.0, 1.0);
      float h = get_depth( p );
      vec3 surface_color = texture(diffuse, p).rgb;
      vec3 pos = sphere_coords( uv, rotation_speed*u_time );
      float glow = glow_strength( pos, h );

      // Add terrain normals to normal vector.
      norm2 = (1.0-normal_scale)*norm2 + normal_scale*texture( normal, p ).xy;
      norm = normalize(vec3(norm2, 1.0-length(norm2)));
      // Compute light strength.
      light = clamp(dot(norm, -light_dir), 0.0, 1.0);
      a *= 2.0*a;
      atmosphere_color = mix(atmosphere_color_dark, atmosphere_color_light, a);
      // Specular blob.
      spec = 0.5*max(1.0 + min(h, 0.0), 0.0);
      light += spec*exp(20.0*spec*(light-0.95));
      // Apply light.
      surface_color *= min(ambient + (1.0-ambient)*light_color*light, 4.0);
      // Add surface color.
      color_out.rgb = min(mix(color_out.rgb, surface_color, alpha), 1.0);
      color_out.rgb += 0.5*(1.0+5.0*light)*glow*mix(glow_color_dark, glow_color_bright, clamp(4.0*glow - 0.25, 0.0, 1.0));
      color_out.a = max(color_out.a, alpha);
      // Add a gradient for the atmosphere.
      color_out.rgb = clamp(color_out.rgb + a*atmosphere_density*alpha*(ambient + (1.0-ambient)*light)*atmosphere_color, 0.0, 1.0);
   }

   return color_out;
}
