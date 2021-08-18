uniform mat4 projection;

uniform sampler2D map_Kd, map_Bump;

uniform vec3 Ka, Kd;
uniform float d, bm;

in vec2 tex_coord;
in vec3 normal;
out vec4 color_out;

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
