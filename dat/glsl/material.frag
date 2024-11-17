/*
 * Blinn-Phong illumination for materials
 */
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
out vec4 colour_out;

/* Illumination. */
//const vec3 lightDir = normalize( vec3(0.0, 0.0, 1.0) );
const vec3 lightDir = normalize( vec3(1.0, 0.5, 0.1) );

const vec3 eye = normalize( vec3(0.0, 1.0, 1.0) );

void main(void) {
   /* Compute normal taking into account the bump map. */
   vec3 norm = normal;
   if (bm > 0.01)
      norm += bm * texture(map_Bump, tex_coord).xyz * 2.0 - 1.0;
   //norm = mix( norm, normal, 0.999 );
   norm = normalize(norm);

   /* Compute lighting. */
   vec3 La        = vec3(1.0) * max(dot(norm, lightDir), 0.0) * 1.0;
   const vec3 Ld  = vec3(1.0) * 1.0;
   const vec3 h   = normalize(lightDir + eye); /* Halfway vector. */
   vec3 Ls        = vec3(1.0) * max(dot(norm, h), 0.0) * 1.0;

   /* Set up textures. */
   vec3 Td = texture(map_Kd, tex_coord).rgb;
   vec3 Ta = Td; // Assume ambient is the same as dispersion
   vec3 Ts = texture(map_Ks, tex_coord).rgb;
   vec3 Te = texture(map_Ke, tex_coord).rgb;

   /* We do the model here. */
   colour_out = vec4(
         ( Ke * Te + Ka * La * Td + Kd * Ld * Td + Ks * Ts * pow( Ls, vec3(Ns) ) ),
         d );
   //colour_out.rgb = mix( colour_out.rgb, norm*0.5+0.5, 0.999 );
}
