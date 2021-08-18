uniform mat4 projection;

uniform sampler2D map_Kd, map_Bump;

uniform vec3 Ka, Kd;
uniform float d, bm;

in vec2 tex_coord;
in vec3 normal;
out vec4 color_out;

const vec3 lightDir = vec3(0.0, 0.0, -1.0);

void main(void) {
   float normal_ratio = step(0.01, bm);
   vec3 norm      = (1.0 - normal_ratio) * normal;
   norm          += normal_ratio * bm * texture(map_Bump, tex_coord).xyz;
   norm           = normalize((projection * vec4(norm, 1.0)).xyz);

   vec3 ambient   = Ka;

   vec3 diffuse   = Kd * max(dot(norm, lightDir), 0.0);

   color_out      = texture(map_Kd, tex_coord);
   color_out.rgb *= 0.4 * ambient + 0.7 * diffuse;
   color_out.a    = d;
}
