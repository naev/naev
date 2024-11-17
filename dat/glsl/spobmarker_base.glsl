uniform vec4 colour;
uniform vec2 dimensions;

in vec2 pos;
out vec4 colour_out;

void main (void)
{
   float m = 1.0 / dimensions.x;
   float rad = 1.0 - 2.0*m;

   float d = marker_func( pos, rad );

   float alpha = smoothstep(    -m, 0.0, -d);
   float beta  = smoothstep(-2.0*m,  -m, -d);
   colour_out   = colour * vec4( vec3(alpha), beta );
}
