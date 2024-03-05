uniform sampler2D u_tex;
uniform sampler2D u_img;

uniform float u_r;
uniform float u_elapsed;

in vec2 tex_coord;
in vec2 tex_scale;
out vec4 colour_out;

void main(void)
{
   vec2 uv = tex_coord;
   vec4 orig = texture( u_tex, uv );
   float e = u_r + u_elapsed * 0.4;
   float c = cos(e);
   float s = sin(e);
   mat2 R = mat2( c, -s, s, c );
   vec4 onion = texture( u_img, (R*(uv/tex_scale-0.5)+0.5) );
   if (orig.a+onion.a <= 0.0)
      discard;

   colour_out = mix( orig, onion, min(u_elapsed*2.0,1.0) * (0.9+0.1*cos(u_r+u_elapsed*2.0)) );
   if (colour_out.a <= 0.0)
      discard;
}
