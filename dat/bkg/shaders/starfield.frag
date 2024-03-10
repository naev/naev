/*
 * Based on http://casual-effects.blogspot.com/2013/08/starfield-shader.html by Morgan McGuire
 * which is based on Star Nest by Kali https://www.shadertoy.com/view/XlfGRj
 * Both under MIT license.
 * Adapted to the Naev engine by bobbens
 */

uniform vec4 u_camera = vec4(1.0); /* xy corresponds to screen space */

const vec3 R      = vec3( %f, %f, %f);
const vec3 UP     = vec3( 0.0, 1.0, 0.0 );
const vec3 AZ     = normalize( -R );
/* A user with Windows 10 and a "GeForce GT 610" GPU reported "error C1059: non constant expression in initialization" from
const vec3 AX     = normalize( cross( AZ, UP ) );
const vec3 AY     = normalize( cross( AX, AZ ) );
*/
const vec2 AZ_xz_n= normalize( AZ.xz );
const vec3 AX     = vec3( -AZ_xz_n.y, 0, AZ_xz_n.x );
const vec3 AY     = vec3( -AZ.y * AX.z, AX.z * AZ.x - AX.x * AZ.z, AZ.y * AX.x );
/* The above formulas are equivalent. */
const mat3 A      = mat3( AX, AY, AZ );
const float theta = %f;
const float cx = cos(theta);
const float sx = sin(theta);
const mat3 Rx = mat3(
   1.0, 0.0, 0.0,
   0.0,  cx,  sx,
   0.0, -sx,  cx
);
const float phi = %f;
const float cy = cos(phi);
const float sy = sin(phi);
const mat3 Ry = mat3(
    cy, 0.0, -sy,
   0.0, 1.0, 0.0,
    sy, 0.0,  cy
);
const float psi = %f;
const float cz = cos(psi);
const float sz = sin(psi);
const mat3 Rz = mat3(
    cz,  sz, 0.0,
   -sz,  cz, 0.0,
   0.0, 0.0, 1.0
);
const mat3 ROT = A * Rx * Ry * Rz;
const int ITERATIONS = 17;
const int VOLSTEPS   = 8;
const float SPARSITY = 0.7; /* 0.4 to 0.5 (sparse) */
const float STEPSIZE = 0.2;
const float FREQVAR  = 1.8; /* 0.5 to 2.0 */
const float BRIGHTNESS= 0.0010;
const float DISTFADING= 0.6800;

vec4 effect( vec4 colour_in, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = (texture_coords - 0.5) * love_ScreenSize.xy * u_camera.w;
   vec3 dir = ROT*vec3(uv, 1.0) + R;
   vec3 cam = vec3(1.0) + ROT*u_camera.xyz;

   float s = 0.1, fade = 0.01;
   vec4 colour = vec4( vec3(0.0), 1.0 );

   for (int r=0; r < VOLSTEPS; r++) {
      vec3 p = cam + dir * (s * 0.5);
      p = abs(vec3(FREQVAR) - mod(p, vec3(FREQVAR * 2.0)));

      float prevlen = 0.0, a = 0.0;
      for (int i=0; i < ITERATIONS; i++) {
         p = abs(p);
         p = p * (1.0 / dot(p, p)) + (-SPARSITY); // the magic formula
         float len = length(p);
         a += abs(len - prevlen); // absolute sum of average change
         prevlen = len;
      }

      a *= a * a; // add contrast

      /* Colouring based on distance. */
      colour.rgb += (vec3(s, s*s, s*s*s) * a * BRIGHTNESS + 1.0) * fade;
      fade *= DISTFADING; /* Distance fading. */
      s += STEPSIZE;
   }
   colour.rgb = min(colour.rgb, vec3(1.2));

   /* Some cheap antialiasing filtering. */
   float intensity = min(colour.r + colour.g + colour.b, 0.7);
   float w = fwidth(intensity);
   colour.rgb = mix( colour.rgb, vec3(0.0), smoothstep(0.0,1.0,w) );

   /* Colour conversion. */
   colour.rgb = clamp( pow( colour.rgb, vec3(2.0) ), 0.0, 1.0 );

   /* Darken it all a bit. */
   colour.rgb *= 0.6;
   return colour * colour_in;
}
