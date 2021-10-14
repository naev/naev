#pragma language glsl3
uniform float u_time = 0.0;
uniform float dt = 0.0;
uniform float u_size = 100.0;
uniform int parami = 0;
uniform vec2 dimensions;

const float M_PI        = 3.14159265358979323846;  /* pi */
const float M_SQRT1_2   = 0.70710678118654752440;  /* 1/sqrt(2) */

float cro(in vec2 a, in vec2 b ) { return a.x*b.y - a.y*b.x; }
float ndot( vec2 a, vec2 b ) { return a.x*b.x - a.y*b.y; }

float smin( float a, float b, float k )
{
   float h = max( k-abs(a-b), 0.0 )/k;
   return min( a, b ) - h*h*k*(1.0/4.0);
}
float sdSmoothUnion( float d1, float d2, float k )
{
   return smin( d1, d2, k );
}

/* Equilateral triangle centered at p facing "up" */
float sdTriangleEquilateral( vec2 p )
{
	const float k = sqrt(3.0);
	p.x = abs(p.x) - 1.0;
	p.y = p.y + 1.0/k;
	if( p.x+k*p.y>0.0 ) p = vec2(p.x-k*p.y,-k*p.x-p.y)/2.0;
	p.x -= clamp( p.x, -2.0, 0.0 );
	return -length(p)*sign(p.y);
}

/* Isosceles triangle centered at p facing "up".
 * q indicates (width, height) */
float sdTriangleIsosceles( vec2 p, vec2 q )
{
	p.x = abs(p.x);
	vec2 a = p - q*clamp( dot(p,q)/dot(q,q), 0.0, 1.0 );
	vec2 b = p - q*vec2( clamp( p.x/q.x, 0.0, 1.0 ), 1.0 );
	float s = -sign( q.y );
	vec2 d = min( vec2( dot(a,a), s*(p.x*q.y-p.y*q.x) ),
			vec2( dot(b,b), s*(p.y-q.y)  ));
	return -sqrt(d.x)*sign(d.y);
}

float sdBox( vec2 p, vec2 b )
{
   vec2 d = abs(p)-b;
   return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

float sdRhombus( vec2 p, vec2 b )
{
   vec2 q = abs(p);
   float h = clamp((-2.0*ndot(q,b)+ndot(b,b))/dot(b,b),-1.0,1.0);
   float d = length( q - 0.5*b*vec2(1.0-h,1.0+h) );
   return d * sign( q.x*b.y + q.y*b.x - b.x*b.y );
}

/* Egg shape (semicircle glued half a vesica) at position p with size b. The cusp is at vec2(-b.x, 0). */
float sdEgg( vec2 p, vec2 b )
{
    /* Transform to Inigo's code */
    const float k           = 1.73205080756887729353;  /* sqrt(3) */
    float ra = b.y;
    float rb = ra + 2.0*b.x - 2.0*b.x*b.x/ra;
    p = vec2(abs(p.y), b.x - ra - p.x);
    /* The rest of the calculation matches the web page cited above. */
    float r = ra - rb;
    return ((p.y<0.0)       ? length(vec2(p.x,  p.y    )) - r :
            (k*(p.x+r)<p.y) ? length(vec2(p.x,  p.y-k*r)) :
                              length(vec2(p.x+r,p.y    )) - 2.0*r) - rb;
}

float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
   vec2 pa = p-a, ba = b-a;
   float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
   return length( pa - ba*h );
}

// c=sin/cos of aperture
float sdPie( vec2 p, vec2 c, float r )
{
   p.x = abs(p.x);
   float l = length(p) - r;
   float m = length(p-c*clamp(dot(p,c),0.0,r));
   return max(l,m*sign(c.y*p.x-c.x*p.y));
}

float sdUnevenCapsuleY( in vec2 p, in float ra, in float rb, in float h )
{
   p.x = abs(p.x);

   float b = (ra-rb)/h;
   vec2  c = vec2(sqrt(1.0-b*b),b);
   float k = cro(c,p);
   float m = dot(c,p);
   float n = dot(p,p);

        if( k < 0.0   ) return sqrt(n)               - ra;
   else if( k > c.x*h ) return sqrt(n+h*h-2.0*h*p.y) - rb;
                        return m                     - ra;
}

float sdUnevenCapsule( in vec2 p, in vec2 pa, in vec2 pb, in float ra, in float rb )
{
   p  -= pa;
   pb -= pa;
   float h = dot(pb,pb);
   vec2  q = vec2( dot(p,vec2(pb.y,-pb.x)), dot(p,pb) )/h;

   //-----------

   q.x = abs(q.x);

   float b = ra-rb;
   vec2  c = vec2(sqrt(h-b*b),b);

   float k = cro(c,q);
   float m = dot(c,q);
   float n = dot(q,q);

   if( k < 0.0 ) return sqrt(h*(n            )) - ra;
   else if( k > c.x ) return sqrt(h*(n+1.0-2.0*q.y)) - rb;
   return m                       - ra;
}


// sca is the sin/cos of the orientation
// scb is the sin/cos of the aperture
float sdArc( in vec2 p, in vec2 sca, in vec2 scb, in float ra, in float rb )
{
   p *= mat2(sca.x,sca.y,-sca.y,sca.x);
   p.x = abs(p.x);
   float k = (scb.y*p.x>scb.x*p.y) ? dot(p.xy,scb) : length(p);
   return sqrt( max(0.0, dot(p,p) + ra*ra - 2.0*ra*k) ) - rb;
}

float sdCircle( in vec2 p, in float r )
{
   return length(p)-r;
}

vec4 sdf_alarm( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   colour.a *= sin(u_time*20.0) * 0.1 + 0.9;

   /* Base Alpha */
   float a = step( sin((px.x + px.y) * 0.3), 0.0);

   /* Signed Distance Function Exclamation Point */
   vec2 p = 2.0*uv-1.0;
   p.y *= -1.0;
   float dc = sdCircle( p, 1.0 );
   p *= 1.2;
   float d = min( sdCircle( p+vec2(0.0,0.65), 0.15), sdUnevenCapsuleY( p+vec2(0,0.15), 0.1, 0.25, 0.7 ));

   a *= step( 0.0, d-20.0/u_size );
   a += step( d, 0.0 );

   /* Second border. */
   a *= step( dc+15.0/u_size, 0.0 );
   a += step( -(dc+15.0/u_size), 0.0 );
   a *= step( dc, 0.0 );

   colour.a *= a;
   return colour;
}

#define CS(A)  vec2(sin(A),cos(A))
vec4 sdf_pilot( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;
   uv = abs(uv);

   float d = sdArc( uv, CS(M_PI*0.75), CS(M_PI/10.0), 1.0, 0.02 );

   d = min( d, sdUnevenCapsule( uv, vec2(M_SQRT1_2), vec2(0.8), 0.07, 0.02) );
   d -= (1.0+sin(3.0*dt)) * 0.007;
   d = max( -sdCircle( uv-vec2(M_SQRT1_2), 0.04 ), d );

   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_pilot2( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;

   const float arclen1 = M_PI/4.0;
   const float arclen2 = M_PI/7.0;

   float w = 2.0 * m;
   float inner = 1.0-w-m;
   float d = sdArc( uv, CS(0.0), CS(arclen1), inner, w );

   vec2 yuv = vec2( uv.x, abs(uv.y) );

   d = min( d, sdCircle( yuv-CS(M_PI*3.0/2.0+arclen1)*inner, 7.0 * m) );
   d = max( -sdCircle(   yuv-CS(M_PI*3.0/2.0+arclen1)*inner, 3.5 * m), d );

   d = min( d, sdArc( uv, CS(M_PI), CS(arclen2), inner, w ) );

   d = min( d, sdCircle( yuv-CS(M_PI/2.0-arclen2)*inner, 7.0 * m) );
   d = max( -sdCircle(   yuv-CS(M_PI/2.0-arclen2)*inner, 3.5 * m), d );

   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_planet( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;

   /* Outer stuff. */
   float w = 1.0 * m;
   float inner = 1.0-w-m;
   float d = sdArc( uv, CS(-M_PI/4.0), CS(M_PI/22.0*32.0), inner, w );

   /* Rotation matrix. */
   float dts = 0.1 * max( 0.5, 100.0 * m );
   float c, s;
   s = sin(dt*dts);
   c = cos(dt*dts);
   mat2 R = mat2( c, s, -s, c );

   vec2 auv = abs(uv);
   if (auv.y < auv.x)
      auv.xy = vec2( auv.y, auv.x );
   if (dimensions.x > 100.0) {
      const float arcseg = M_PI/64.0;
      const vec2 shortarc = CS(arcseg);
      for (int i=2; i<16; i+=4)
         d = min( d, sdArc( auv, CS(M_PI/2.0+float(i)*arcseg),  shortarc, inner, w ) );

      /* Moving inner stuff. */
      uv = uv*R;
      const vec2 arclen = CS(M_PI/9.0);
      w = 2.0 * m;
      inner -= 2.0*(w+m);
      for (int i=0; i<5; i++)
         d = min( d, sdArc( uv, CS( float(i)*M_PI*2.0/5.0), arclen, inner, w ) );
   }
   else {
      const float arcseg = M_PI/32.0;
      const vec2 shortarc = CS(arcseg);
      for (int i=2; i<8; i+=4)
         d = min( d, sdArc( auv, CS(M_PI/2.0+float(i)*arcseg),  shortarc, inner, w ) );

      /* Moving inner stuff. */
      uv = uv*R;
      const vec2 arclen = CS(M_PI/6.0);
      w = 2.0 * m;
      inner -= 2.0*(w+m);
      for (int i=0; i<3; i++)
         d = min( d, sdArc( uv, CS( float(i)*M_PI*2.0/3.0), arclen, inner, w ) );
   }

   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

float sdf_emptyCircle( vec2 uv, float d, float m )
{
   d = min( d, sdCircle( uv, 7.0 * m) );
   d = max( -sdCircle(   uv, 3.5 * m), d );
   return d;
}
vec4 sdf_planet2( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;

   /* Outter stuff. */
   float w = 2.0 * m;
   float inner = 1.0-w-m;
   float d = sdArc( uv, CS(-M_PI/4.0), CS(M_PI/22.0*32.0), inner, w );

   /* Inner steps */
   float dts = 0.05 * max( 0.5, 100.0 * m );
   float c, s;
   s = sin(dt*dts);
   c = cos(dt*dts);
   mat2 R = mat2( c, s, -s, c );
   vec2 auv = abs(uv*R);
   if (auv.y < auv.x)
      auv.xy = vec2( auv.y, auv.x );
   const int nmax = 9; // only works well with odd numbers
   for (int i=0; i<nmax; i++)
      d = min( d, sdSegment( auv,
            CS((float(i)+0.5)*0.5*M_PI/float(nmax)*0.5)*0.91,
            CS((float(i)+0.5)*0.5*M_PI/float(nmax)*0.5)*0.93 )-m );
   d = min( d, sdSegment( auv,
         CS((float(nmax/2)+0.5)*0.5*M_PI/float(nmax)*0.5)*0.89,
         CS((float(nmax/2)+0.5)*0.5*M_PI/float(nmax)*0.5)*0.93 )-1.5*m );

   /* Circles on main. */
   if (uv.y < uv.x)
      uv.xy = vec2(uv.y, uv.x);
   d = sdf_emptyCircle( uv - CS(M_PI/4.0)*inner, d, m );
   d = sdf_emptyCircle( uv - CS(M_PI/4.0+M_PI/22.0*32.0)*inner, d, m );
   //d = sdf_emptyCircle( uv - CS(M_PI/4.0-M_PI/22.0*32.0)*inner, d, m );

   /* Circles off main. */
   //d = sdf_emptyCircle( uv - CS(M_PI/4.0 + M_PI*0.9)*inner, d, m );
   d = sdf_emptyCircle( uv - CS(M_PI/4.0 + M_PI*1.0)*inner, d, m );
   d = sdf_emptyCircle( uv - CS(M_PI/4.0 + M_PI*1.1)*inner, d, m );

   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_blinkmarker( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;

   const float w = 0.20;
   const float h = 0.05;

   uv = abs(uv);
   const float s = sin(M_PI/4.0);
   const float c = cos(M_PI/4.0);
   const mat2 R = mat2( c, s, -s, c );
   uv = uv - (vec2(1.0-w*M_SQRT1_2)-m);
   uv = R * uv;

   float d = sdRhombus( uv, vec2(h,w) );

   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_jumpmarker( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;

   uv = vec2( uv.y, uv.x );

   float db = sdBox( uv+vec2(0.0,0.6), vec2(0.2,0.6) );
   float dt = 2.0*sdTriangleIsosceles( 0.5*uv+vec2(0.0,0.2), vec2(0.5-m, 0.5) );
   float d = sdSmoothUnion( db, dt, 0.5 );
   d = max( d, abs(uv.y)-1.0+2.0*m );
   d = abs(d);

   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_pilotmarker( vec4 colour, vec2 uv )
{
   uv = vec2( uv.y, uv.x );
   float m = 1.0 / dimensions.x;
   float d = sdTriangleEquilateral( uv*1.15  ) / 1.15;
   d = abs(d+2.0*m);
   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_playermarker( vec4 colour, vec2 uv )
{
   uv = vec2( uv.y, -uv.x );
   float m = 1.0 / dimensions.x;
   float d = 2.0*sdTriangleIsosceles( uv*0.5+vec2(0.0,0.5), vec2(0.2,0.7) );
   d = abs(d+2.0*m);
   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_stealth( vec4 colour, vec2 uv )
{
   float r = fract(dt*0.1);
   float a = r * M_PI;
   float c = cos(a);
   float s = sin(a);
   uv.y = -uv.y;
   uv = mat2(c,-s,s,c) * uv;
   float d = sdPie( uv*dimensions, vec2(s,c), dimensions.x-1.0 );
   float alpha = smoothstep(-1.0, 0.0, -d);
   colour.a *= smoothstep( -1.0, 0.0, -d );
   return colour;
}

vec4 sdf_jumplane( vec4 colour, vec2 uv )
{
   uv   *= dimensions;
   float d = sdBox( uv, dimensions-vec2(1.0) );

   uv.y  = abs(uv.y);
   uv.x -= u_time*dimensions.y*0.5;
   uv.x  = mod(-uv.x,dimensions.y)-0.25*dimensions.y;
   float ds = -0.2*abs(uv.x-0.5*uv.y) + 2.0/3.0;
   d = max( d, ds );

   float alpha = smoothstep(-1.0, 0.0, -d);
   colour.a *= smoothstep( -1.0, 0.0, -d );
   return colour;
}

vec4 sdf_gear( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;
   float d = sdCircle( uv, 0.75 );

   vec2 auv = abs(uv);
   if (auv.y < auv.x)
      auv.xy = vec2( auv.y, auv.x );
   const float s = sin( -M_PI/8.0 );
   const float c = cos( -M_PI/8.0 );
   const mat2 R = mat2( c, s, -s, c );
   d = sdSmoothUnion( d, sdBox( auv*R, vec2(0.1,1.0-m) )-0.025, 0.2 );

   d = max( d, -sdCircle( uv, 0.6 ) );
   d = min( d, sdCircle( uv, 0.25 ) );

   colour.a = smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_sysrhombus( vec4 colour, vec2 uv)
{
   vec2 pos = vec2( uv.y, uv.x );
   const vec2 b1 = vec2( 0.9, 0.6 );
   const vec2 b2 = vec2( 0.9, 0.4 );
   float m = 1.0 / dimensions.x;

   float d;
   if (parami==1) {
      const vec2 b = b2;
      d = sdRhombus( pos, b );
      d = max( -sdRhombus( pos*2.0, b ), d );
      d = min(  d, sdRhombus( pos*4.0, b ) );
   }
   else {
      const vec2 b = b1;
      d = sdRhombus( pos, b );
      d = max( -sdRhombus( pos*2.0, b ), d );
      d = min(  d, sdRhombus( pos*4.0, b ) );
   }
   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_sysmarker( vec4 colour, vec2 uv )
{
   vec2 pos = vec2( uv.y, uv.x );
   const vec2 b = vec2( 1.0, 0.65 );
   const vec2 c = vec2(-0.35,0.0);
   float m = 1.0 / dimensions.x;

   float d = sdEgg( pos, b-2.0*m );
   vec2 cpos = pos+c;
   if (parami==1)
      d = max( -sdSegment( cpos, vec2(0.0), vec2(1.0,0.0) )+0.15, d );
   d = max( -sdCircle( cpos, 0.5 ), d );
   d = min( sdCircle( cpos, 0.2 ), d );
   colour.a *= smoothstep( -m, 0.0, -d );

   return colour;
}

vec4 sdf_missilelockon( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;

   /* Outter stuff. */
   float d = 1e1000;

   /* Inner steps */
   float dts = 0.05 * max( 0.5, 100.0 * m );
   vec2 auv = abs(uv);
   if (auv.y < auv.x)
      auv.xy = vec2( auv.y, auv.x );
   const int nmax = 1; // only works well with odd numbers
   for (int i=0; i<nmax; i++)
      d = min( d, sdSegment( auv,
            CS((float(i)+0.5)*0.5*M_PI/float(nmax)*0.5)*0.8,
            CS((float(i)+0.5)*0.5*M_PI/float(nmax)*0.5)*1.0 )-m );
   
   float r = fract(dt*0.1);
   float a = r * M_PI;
   float c = cos(a);
   float s = sin(a);
   uv.y = -uv.y;
   uv = mat2(c,-s,s,c) * uv;
   float dp = sdPie( uv*dimensions, vec2(s,c), 1.5*dimensions.x );

   d = max(d,-dp);

   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 sdf_selectposition( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;
   uv = abs( uv );
   if (uv.y < uv.x)
      uv.xy = uv.yx;
   float d = sdSegment( uv, vec2(0.2+m,1.0-2.0*m), vec2(1.0,1.0)-2.0*m );
   colour.a *= smoothstep( -m, 0.0, -d );
   return colour;
}

vec4 bg( vec2 uv )
{
   vec3 c;
   uv *= 10.0;
   if (mod( floor(uv.x)+floor(uv.y), 2.0 ) == 0.0)
      c = vec3( 0.2 );
   else
      c = vec3( 0.0 );
   c = gammaToLinear( c );
   return vec4( c, 1.0 );
}

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 col_out;
   vec2 uv_rel = uv*2.0-1.0;
   uv_rel.y = - uv_rel.y;

   //col_out = sdf_alarm( colour, tex, uv, px );
   //col_out = sdf_pilot( colour, uv_rel );
   //col_out = sdf_pilot2( colour, uv_rel );
   //col_out = sdf_planet( colour, uv_rel );
   //col_out = sdf_planet2( colour, uv_rel );
   //col_out = sdf_blinkmarker( colour, uv_rel );
   col_out = sdf_jumpmarker( colour, uv_rel );
   //col_out = sdf_pilotmarker( colour, uv_rel );
   //col_out = sdf_playermarker( colour, uv_rel );
   //col_out = sdf_stealth( colour, uv_rel );
   //col_out = sdf_jumplane( colour, uv_rel );
   //col_out = sdf_gear( colour, uv_rel );
   //col_out = sdf_sysrhombus( colour, uv_rel );
   //col_out = sdf_sysmarker( colour, uv_rel );
   //col_out = sdf_missilelockon( colour, uv_rel );
   //col_out = sdf_selectposition( colour, uv_rel );

   return mix( bg(uv), col_out, col_out.a );
}

