uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 colour_out;

vec4 sharpen( in sampler2D tex, in vec2 coords, in vec2 renderSize )
{
   float dx = 1.0 / renderSize.x;
   float dy = 1.0 / renderSize.y;
   vec4 sum = vec4(0.0);
   sum += -1.0 * texture(tex, coords + vec2( -1.0 * dx , 0.0 * dy));
   sum += -1.0 * texture(tex, coords + vec2( 0.0 * dx , -1.0 * dy));
   sum +=  5.0 * texture(tex, coords + vec2( 0.0 * dx , 0.0 * dy));
   sum += -1.0 * texture(tex, coords + vec2( 0.0 * dx , 1.0 * dy));
   sum += -1.0 * texture(tex, coords + vec2( 1.0 * dx , 0.0 * dy));
   return sum;
}

void main()
{
   colour_out = vec4( sharpen( sampler, tex_coord, textureSize(sampler,0) ) );
}
