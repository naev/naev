/*
 * Phong illumination for materials
 */

uniform mat4 projection;

/* Textures. */
uniform sampler2D map_Kd;  /* Diffuse map. */
uniform sampler2D map_Ks;  /* Specular map. */
uniform sampler2D map_Ke;  /* Emission map. */
uniform sampler2D map_Bump;/* Bump map. */

/* Phong model parameters. */
uniform float Ns; /* Specular shininess. */
uniform vec3 Ka;  /* Ambient colour. */
uniform vec3 Kd;  /* Diffuse colour. */
uniform vec3 Ks;  /* Specular colour. */
uniform vec3 Ke;  /* Emissive colour. */
uniform float Ni; /* Optical density. */
uniform float d;  /* Dissolve (opacity). */

uniform float bm; /* Bump mapping parameter. */

in vec2 tex_coord;
in vec3 normal;
out vec4 color_out;

/* Illumination. */
const vec3 lightDir = normalize( vec3(0.0, 0.0, -1.0) );

void main(void) {
   /* Compute normal taking into account the bump map. */
   vec3 norm = normal;
   if (bm > 0.01)
      norm += bm * texture(map_Bump, tex_coord).xyz * 2.0 - 1.0;
   /* http://www.lighthouse3d.com/tutorials/glsl-tutorial/the-normal-matrix/ */
   mat3 projection_normal = transpose(inverse(mat3(projection)));
   norm = normalize(projection_normal * norm);

   /* Compute lighting. */
   vec3 La = vec3(1.0) * max(dot(norm, lightDir), 0.0) * 1.0;
   vec3 Ld = vec3(1.0) * 1.0;
   vec3 Ls = vec3(0.0);

   /* Set up textures. */
   vec4 tex_Kd = texture(map_Kd, tex_coord);
   vec4 tex_Ks = texture(map_Ks, tex_coord);
   vec4 tex_Ke = texture(map_Ke, tex_coord);

   /* We do the model here. */
   color_out = vec4(
         tex_Kd.rgb * ( Ke * tex_Ke.rgb + Ka * La + Kd * Ld + Ks * tex_Ks.rgb * pow( Ls, vec3(Ns) ) ),
         d );
}
