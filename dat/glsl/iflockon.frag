#include "lib/sdf.glsl"
#include "lib/math.glsl"

uniform vec4 colour;
uniform vec2 dimensions;
uniform float paramf;

in vec2 pos;
out vec4 colour_out;

#define CS(A)  vec2(sin(A),cos(A))
void main(void) {
   float m = 1.0 / dimensions.x;
   vec2 uv = pos;

   float dts = 0.05 * max( 0.5, 100.0 * m );
   vec2 auv = abs(uv);
   if (auv.y < auv.x)
      auv.xy = vec2( auv.y, auv.x );
   /*
   const int nmax = 1; // only works well with odd numbers
   float d = 1e1000;
   for (int i=0; i<nmax; i++)
      d = min( d, sdSegment( auv,
            CS((float(i)+0.5)*0.5*M_PI/float(nmax)*0.5)*0.8,
            CS((float(i)+0.5)*0.5*M_PI/float(nmax)*0.5)*1.0 )-m );
   */
   float d = sdSegment( auv,
         CS(0.5*0.5*M_PI*0.5)*0.8,
         CS(0.5*0.5*M_PI*0.5)*1.0 )-m;

   float a = paramf * M_PI;
   float c = cos(a);
   float s = sin(a);
   uv.y = -uv.y;
   uv = mat2(c,-s,s,c) * uv;
   float dp = sdPie( uv*dimensions, vec2(s,c), 1.5*dimensions.x );

   d = max(d,-dp);

   float alpha = smoothstep(     -m, 0.0, -d );
   //float beta  = smoothstep( -2.0*m,  -m, -d );
   //colour_out   = colour * vec4( vec3(alpha), beta );
   colour_out   = colour;
   colour_out.a*= alpha;
}
