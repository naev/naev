layout(std140) uniform TextureData {
   mat3 tex_mat;
   mat3 transform;
   vec4 colour;
};

layout(std140) uniform SDFData {
   float m;
   float outline;
};

#if defined(VERT)
layout(location = 0) in vec2 vertex;
out vec2 tex_coord;

void main(void) {
   vec3 pos = vec3( vertex, 1.0 );
   tex_coord = (tex_mat * pos).st;
   gl_Position = vec4( (transform * pos).xy, 0.0, 1.0 );
}
#elif defined(FRAG)
#include "lib/math.glsl"

uniform sampler2D sampler;

in vec2 tex_coord;
layout(location = 0) out vec4 colour_out;

const vec4 OUTLINE_COLOUR = vec4( vec3(0.0), 1.0 );

void main(void)
{
   float d  = (texture(sampler, tex_coord).r-0.5)*m;
   float alpha = smoothstep(-0.5    , +0.5, d);
   float beta  = smoothstep(-M_SQRT2, -1.0, d);
   vec4 fg_c   = mix( OUTLINE_COLOUR, colour, alpha );
   colour_out   = vec4( fg_c.rgb, beta*fg_c.a );
}
#endif
