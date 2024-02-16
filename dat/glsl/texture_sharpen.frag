uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 colour_out;

const float PARAM = 0.5; /**< Sharpening strength. */

/** mpv's unsharpen mask. */
void main()
{
   vec2 d = vec2(1.0) / textureSize( sampler, 0 );
   float st1 = 1.2;
   vec4 p = texture( sampler, tex_coord );
   vec4 sum1 = texture( sampler, st1 * vec2(+d.x, +d.yy) )
      + texture( sampler, st1 * vec2(+d.x, -d.y))
      + texture( sampler, st1 * vec2(-d.x, +d.y))
      + texture( sampler, st1 * vec2(-d.x, -d.y));
   float st2 = 1.5;
   vec4 sum2 = texture( sampler, st2 * vec2(+d.x,  0.0))
      + texture( sampler, st2 * vec2( 0.0, +d.y))
      + texture( sampler, st2 * vec2(-d.x,  0.0))
      + texture( sampler, st2 * vec2( 0.0, -d.y));
   vec4 t = p * 0.859375 + sum2 * -0.1171875 + sum1 * -0.09765625;
   colour_out = p + t * PARAM;
}
