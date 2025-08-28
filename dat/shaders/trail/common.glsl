// Libraries
#include "lib/math.glsl"

// For ideas: https://thebookofshaders.com/05/

uniform vec4 c1;  // Start colour
uniform vec4 c2;  // End colour
uniform vec2 t; // Start and end time [0,1]
uniform float dt; // Current time (in seconds)
uniform vec2 pos1;// Start position
uniform vec2 pos2;// End position
uniform float r;  // Unique value per trail [0,1]
uniform vec3 nebu_col; // Base colour of the nebula, only changes when entering new system

in vec2 pos;
out vec4 colour_out;

/* Has a peak at 1/k */
float impulse( float x, float k )
{
   float h = x*k;
   return h * exp( 1.0 - h );
}

float fastdropoff( float x, float k )
{
   return 1. - pow( max(0.0, abs(1.-x) * 2.0 - 1.0 ), k );
}

/* k is the sharpness, more k == smoother
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1. - pow( abs( sin( M_PI * x / 2. ) ), k );
}

/* Similar to smoothbeam, but more k == sharper. */
float sharpbeam( float x, float k )
{
   return pow( min( cos( M_PI * x / 2. ), 1.0 - abs(x) ), k );
}

/* Defined separately. */
vec4 trail_func( vec4 colour, vec2 pos_tex, vec2 dim );

void main(void) {
   vec2 pos_tex, pos_px;

   // Interpolate
   colour_out = mix( c1, c2, pos.x );
   pos_px    = mix( pos1, pos2, pos );
   pos_px.y *= pos.y;
   pos_tex.x = mix( t.x, t.y, pos.x );
   pos_tex.y = 2. * pos.y - 1.;

   colour_out = trail_func( colour_out, pos_tex, pos_px );
}
