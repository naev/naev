const float MARGIN   = 30.0; /**< How much margin to leave when fading in/out. */

uniform vec4 color;
uniform vec2 dimensions;
//uniform float dt;
//uniform float r;

in vec2 pos;
out vec4 color_out;

void main(void) {
   vec2 pos_tex, pos_px;
   pos_tex.x = pos.x;
   pos_tex.y = 2.0 * pos.y - 1.0;
   pos_px = pos * dimensions;

   vec4 col = color;

   /* Make it wavy. */
   //pos_tex.y += 0.3 * snoise( 0.006*vec2( r, pos_px.x ) );
   //col.a *= step( abs(pos_tex.y), 0.5 );

   /* Fade in/out edges. */
   col.a *= smoothstep( 0.0, MARGIN, pos_px.x );
   col *= 1.0 - smoothstep( dimensions.x-MARGIN, dimensions.x, pos_px.x );

   color_out = col;
}

