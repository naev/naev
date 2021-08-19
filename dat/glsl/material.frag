/*
 * Phong illumination for materials
 */

uniform mat4 projection;

/* Textures. */
uniform sampler2D map_Kd;  /* Diffuse map. */
uniform sampler2D map_Bump;/* Bump map. */

/* Phong model parameters. */
uniform float Ns; /* Specular shininess. */
uniform vec3 Ka;  /* Ambient colour. */
uniform vec3 Kd;  /* Diffuse colour. */
uniform vec3 Ks;  /* Specular colour. */
uniform vec3 Ke;  /* Emissive colour. */
uniform float Ni; /* Optical density. */
uniform float d;  /* Dissolve (opacity). */

uniform float bm;

in vec2 tex_coord;
in vec3 normal;
out vec4 color_out;

/* Illumination. */
const vec3 lightDir = normalize( vec3(0.0, 0.0, -1.0) );

void main(void) {
   vec3 norm = normal;
   if (bm > 0.01)
      norm += bm * texture(map_Bump, tex_coord).xyz * 2.0 - 1.0;
   norm = normalize((projection * vec4(norm, 0.0)).xyz);

   vec3 ambient= Ka;

   vec3 diffuse= Kd * max(dot(norm, lightDir), 0.0);

   color_out   = texture(map_Kd, tex_coord);
   color_out.rgb *= 0.4 * ambient + 0.7 * diffuse;
   color_out.a = d;
}
