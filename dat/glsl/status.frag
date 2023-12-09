uniform float paramf;

const vec3 outline_colour = vec3( 0.0, 0.0, 0.0 );
const vec3 colour_0 = vec3( 0.8, 0.2, 0.2 );
const vec3 colour_1 = vec3( 0.2, 0.8, 0.2 );
const float r2 = sqrt( 2.0 );

in vec2 pos;
out vec4 colour_out;

// Colour cutoffs, corresponding to "dist" below.
const float glyph_center     = 0.5;
const float outline_center   = 0.2;
const float glyph_stepsize   = 0.1;
const float outline_stepsize = 0.125;

void main(void) {
   vec2 uv = pos*2.0-1.0;
   float len = length(uv);
   float dcirc = abs( len - 0.64 );
   float dline = abs( uv.x + uv.y ) / r2 + step( 0.64, len ) + paramf;
   // dist is a value between 0 and 1 with 0.5 on the edge and 1 inside it.
   float dist = 1.0 - 5.0 * min( dcirc, dline );
   // TODO: maybe drop off faster if <.5
   // smoothstep maps values below 0.5 to 0 and above 0.5 to 1, with a smooth transition at 0.5.
   float alpha = smoothstep(glyph_center-glyph_stepsize, glyph_center+glyph_stepsize, dist);
   float beta = smoothstep(outline_center-outline_stepsize, outline_center+outline_stepsize, dist);
   vec3 fg_c = alpha * mix( colour_0, colour_1, paramf);
   colour_out = vec4( fg_c, beta );
}
