uniform float ok;
const vec3 outline_color = vec3( 0., 0., 0. );
const vec3 color_0 = vec3( .8, .2, .2 );
const vec3 color_1 = vec3( .2, .8, .2 );
const float r2 = sqrt( 2. );

in vec2 pos;
out vec4 color_out;

// Colour cutoffs, corresponding to "dist" below.
const float glyph_center     = 0.5;
const float outline_center   = 0.2;
const float glyph_stepsize   = 0.1;
const float outline_stepsize = 0.125;

void main(void) {
   float r = length(pos);
   float dcirc = abs( r - .64 );
   float dline = abs( pos.x + pos.y ) / r2 + step( .64, r ) + ok;
   // dist is a value between 0 and 1 with 0.5 on the edge and 1 inside it.
   float dist = 1 - 5 * min( dcirc, dline );
   // TODO: maybe drop off faster if <.5
   // smoothstep maps values below 0.5 to 0 and above 0.5 to 1, with a smooth transition at 0.5.
   float alpha = smoothstep(glyph_center-glyph_stepsize, glyph_center+glyph_stepsize, dist);
   float beta = smoothstep(outline_center-outline_stepsize, outline_center+outline_stepsize, dist);
   vec3 fg_c = alpha * mix( color_0, color_1, ok );
   color_out = vec4( fg_c, beta );
}
