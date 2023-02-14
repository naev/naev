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

// Modulo 289 without a division (only multiplications)
vec4 mod289(vec4 x) {
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec3 mod289(vec3 x) {
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec2 mod289(vec2 x) {
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}

// Modulo 7 without a division
vec4 mod7(vec4 x) {
   return x - floor(x * (1.0 / 7.0)) * 7.0;
}
vec3 mod7(vec3 x) {
   return x - floor(x * (1.0 / 7.0)) * 7.0;
}

// Permutation polynomial: (34x^2 + x) mod 289
vec4 permute(vec4 x) {
   return mod289((34.0 * x + 10.0) * x);
}
vec3 permute(vec3 x) {
   return mod289((34.0 * x + 10.0) * x);
}

vec4 taylorInvSqrt(vec4 r) {
   return 1.79284291400159 - 0.85373472095314 * r;
}

vec3 fade(vec3 t) {
   return t*t*t*(t*(t*6.0-15.0)+10.0);
}
vec2 fade(vec2 t) {
   return t*t*t*(t*(t*6.0-15.0)+10.0);
}

/* Returns a value in the [0,1] range. */
float random(float n)
{
   return fract(sin(n) * 43758.5453123);
}

/* Returns a value in the [0,1] range.
 http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
 */
float random(vec2 co)
{
   const highp float seed = 12.9898;
   highp float a = seed;
   highp float b = 78.233;
   highp float c = 43758.5453;
   highp float dt= dot(co.xy ,vec2(a,b));
   highp float sn= mod(dt,M_PI);
   return fract(sin(sn) * c);
}

float snoise(vec2 v)
{
   const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
         0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
         -0.577350269189626,  // -1.0 + 2.0 * C.x
         0.024390243902439); // 1.0 / 41.0
   // First corner
   vec2 i  = floor(v + dot(v, C.yy) );
   vec2 x0 = v -   i + dot(i, C.xx);

   // Other corners
   vec2 i1;
   //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
   //i1.y = 1.0 - i1.x;
   i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
   // x0 = x0 - 0.0 + 0.0 * C.xx ;
   // x1 = x0 - i1 + 1.0 * C.xx ;
   // x2 = x0 - 1.0 + 2.0 * C.xx ;
   vec4 x12 = x0.xyxy + C.xxzz;
   x12.xy -= i1;

   // Permutations
   i = mod289(i); // Avoid truncation effects in permutation
   vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
         + i.x + vec3(0.0, i1.x, 1.0 ));

   vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
   m = m*m ;
   m = m*m ;

   // Gradients: 41 points uniformly over a line, mapped onto a diamond.
   // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

   vec3 x = 2.0 * fract(p * C.www) - 1.0;
   vec3 h = abs(x) - 0.5;
   vec3 ox = floor(x + 0.5);
   vec3 a0 = x - ox;

   // Normalise gradients implicitly by scaling m
   // Approximation of: m *= inversesqrt( a0*a0 + h*h );
   m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

   // Compute final noise value at P
   vec3 g;
   g.x  = a0.x  * x0.x  + h.x  * x0.y;
   g.yz = a0.yz * x12.xz + h.yz * x12.yw;
   return 130.0 * dot(m, g);
}

float snoise(vec3 v)
{
   const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
   const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

   // First corner
   vec3 i  = floor(v + dot(v, C.yyy) );
   vec3 x0 =   v - i + dot(i, C.xxx) ;

   // Other corners
   vec3 g = step(x0.yzx, x0.xyz);
   vec3 l = 1.0 - g;
   vec3 i1 = min( g.xyz, l.zxy );
   vec3 i2 = max( g.xyz, l.zxy );

   //   x0 = x0 - 0.0 + 0.0 * C.xxx;
   //   x1 = x0 - i1  + 1.0 * C.xxx;
   //   x2 = x0 - i2  + 2.0 * C.xxx;
   //   x3 = x0 - 1.0 + 3.0 * C.xxx;
   vec3 x1 = x0 - i1 + C.xxx;
   vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
   vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

   // Permutations
   i = mod289(i);
   vec4 p = permute( permute( permute(
               i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
         + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

   // Gradients: 7x7 points over a square, mapped onto an octahedron.
   // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
   float n_ = 0.142857142857; // 1.0/7.0
   vec3  ns = n_ * D.wyz - D.xzx;

   vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

   vec4 x_ = floor(j * ns.z);
   vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

   vec4 x = x_ *ns.x + ns.yyyy;
   vec4 y = y_ *ns.x + ns.yyyy;
   vec4 h = 1.0 - abs(x) - abs(y);

   vec4 b0 = vec4( x.xy, y.xy );
   vec4 b1 = vec4( x.zw, y.zw );

   //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
   //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
   vec4 s0 = floor(b0)*2.0 + 1.0;
   vec4 s1 = floor(b1)*2.0 + 1.0;
   vec4 sh = -step(h, vec4(0.0));

   vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
   vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

   vec3 p0 = vec3(a0.xy,h.x);
   vec3 p1 = vec3(a0.zw,h.y);
   vec3 p2 = vec3(a1.xy,h.z);
   vec3 p3 = vec3(a1.zw,h.w);

   //Normalise gradients
   vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
   p0 *= norm.x;
   p1 *= norm.y;
   p2 *= norm.z;
   p3 *= norm.w;

   // Mix final noise value
   vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
   m = m * m;
   return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
            dot(p2,x2), dot(p3,x3) ) );
}

float psrdnoise(vec2 x, vec2 period, float alpha, out vec2 gradient) {

	// Transform to simplex space (axis-aligned hexagonal grid)
	vec2 uv = vec2(x.x + x.y*0.5, x.y);

	// Determine which simplex we're in, with i0 being the "base"
	vec2 i0 = floor(uv);
	vec2 f0 = fract(uv);
	// o1 is the offset in simplex space to the second corner
	float cmp = step(f0.y, f0.x);
	vec2 o1 = vec2(cmp, 1.0-cmp);

	// Enumerate the remaining simplex corners
	vec2 i1 = i0 + o1;
	vec2 i2 = i0 + vec2(1.0, 1.0);

	// Transform corners back to texture space
	vec2 v0 = vec2(i0.x - i0.y * 0.5, i0.y);
	vec2 v1 = vec2(v0.x + o1.x - o1.y * 0.5, v0.y + o1.y);
	vec2 v2 = vec2(v0.x + 0.5, v0.y + 1.0);

	// Compute vectors from v to each of the simplex corners
	vec2 x0 = x - v0;
	vec2 x1 = x - v1;
	vec2 x2 = x - v2;

	vec3 iu, iv;
	vec3 xw, yw;

	// Wrap to periods, if desired
	if(any(greaterThan(period, vec2(0.0)))) {
		xw = vec3(v0.x, v1.x, v2.x);
		yw = vec3(v0.y, v1.y, v2.y);
		if(period.x > 0.0)
			xw = mod(vec3(v0.x, v1.x, v2.x), period.x);
		if(period.y > 0.0)
			yw = mod(vec3(v0.y, v1.y, v2.y), period.y);
		// Transform back to simplex space and fix rounding errors
		iu = floor(xw + 0.5*yw + 0.5);
		iv = floor(yw + 0.5);
	} else { // Shortcut if neither x nor y periods are specified
		iu = vec3(i0.x, i1.x, i2.x);
		iv = vec3(i0.y, i1.y, i2.y);
	}

	// Compute one pseudo-random hash value for each corner
	vec3 hash = mod(iu, 289.0);
	hash = mod((hash*51.0 + 2.0)*hash + iv, 289.0);
	hash = mod((hash*34.0 + 10.0)*hash, 289.0);

	// Pick a pseudo-random angle and add the desired rotation
	vec3 psi = hash * 0.07482 + alpha;
	vec3 gx = cos(psi);
	vec3 gy = sin(psi);

	// Reorganize for dot products below
	vec2 g0 = vec2(gx.x,gy.x);
	vec2 g1 = vec2(gx.y,gy.y);
	vec2 g2 = vec2(gx.z,gy.z);

	// Radial decay with distance from each simplex corner
	vec3 w = 0.8 - vec3(dot(x0, x0), dot(x1, x1), dot(x2, x2));
	w = max(w, 0.0);
	vec3 w2 = w * w;
	vec3 w4 = w2 * w2;

	// The value of the linear ramp from each of the corners
	vec3 gdotx = vec3(dot(g0, x0), dot(g1, x1), dot(g2, x2));

	// Multiply by the radial decay and sum up the noise value
	float n = dot(w4, gdotx);

	// Compute the first order partial derivatives
	vec3 w3 = w2 * w;
	vec3 dw = -8.0 * w3 * gdotx;
	vec2 dn0 = w4.x * g0 + dw.x * x0;
	vec2 dn1 = w4.y * g1 + dw.y * x1;
	vec2 dn2 = w4.z * g2 + dw.z * x2;
	gradient = 10.9 * (dn0 + dn1 + dn2);

	// Scale the return value to fit nicely into the range [-1,1]
	return 10.9 * n;
}

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

float sdArc2( in vec2 p, in vec2 sc, in float ra, float rb )
{
    // sc is the sin/cos of the arc's aperture
    p.x = abs(p.x);
    return ((sc.y*p.x>sc.x*p.y) ? length(p-sc*ra) :
                                  abs(length(p)-ra)) - rb;
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

vec4 sdf_vnarrow( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;
   uv = vec2(uv.y,-uv.x);

   float d1 = sdTriangleIsosceles( uv+vec2(0.0,-0.4), vec2(0.6,0.4) );
   float d2 = sdTriangleIsosceles( uv+vec2(0.0,0.2),  vec2(0.6,0.4) );
   float d3 = sdTriangleIsosceles( uv+vec2(0.0,0.8),  vec2(0.6,0.4) );
   //d1 = max(d1, -sdTriangleIsosceles( uv+vec2(0.0,-0.8), vec2(3.0,0.35) ) );

   float d = min(min(d1, d2), d3);

   d = abs(d)-0.01;

   colour.a *= smoothstep( -m, 0.0, -d );
   colour.a += pow( 1.0-d, 20.0 );
   return colour;
}

vec4 sdf_vncont( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;
   //uv = vec2(uv.y,-uv.x);

   vec2 auv = vec2(uv.x, abs(uv.y));
   float d = sdSegment( auv, vec2(0.0,0.8), vec2(0.8,0.0) );

   float ds = sdCircle( uv+vec2(-0.15,0.0), 0.2 );
   ds = min( ds, sdCircle( uv+vec2(0.35,0.0), 0.15 ) );
   ds = min( ds, sdCircle( uv+vec2(0.75,0.0), 0.1 ) );

   d = min( d, ds );

   d = abs(d)-0.01;
   colour.a *= smoothstep( -m, 0.0, -d );
   colour.a += pow( 1.0-d, 20.0 );
   return colour;
}

vec4 sdf_vndone( vec4 colour, vec2 uv )
{
   float m = 1.0 / dimensions.x;
   //uv = vec2(uv.y,-uv.x);

   float d = sdBox( uv, vec2(0.7) );
   d = max( d, -sdBox( uv, vec2(0.2,1.0) ) );
   d = max( d, -sdBox( uv, vec2(1.0,0.2) ) );

   d = abs(d)-0.01;
   colour.a *= smoothstep( -m, 0.0, -d );
   colour.a += pow( 1.0-d, 20.0 );
   return colour;
}

vec4 sdf_notemarker( vec4 colour, vec2 uv )
{
   vec2 pos = uv.yx;
   const vec2 b = vec2( 0.8, 0.25 );
   const vec2 c = vec2( -0.15, 0.0);
   const mat2 R = mat2(
      M_SQRT1_2, M_SQRT1_2,
     -M_SQRT1_2, M_SQRT1_2
   );
   float m = 1.0 / dimensions.x;

   float d = sdEgg( pos+vec2(0.2,0.0), b-2.0*m );
   vec2 cpos = R*(pos+c)+vec2(m,0.0);
   d = min( d, sdBox( cpos, vec2(0.4) )-0.16 );
   d = max( d, -sdSegment( abs(cpos), vec2(-0.38,0.33), vec2(0.38,0.33) )+0.08 );
   d = max( d, -sdSegment( cpos, vec2(-0.38,0.0), vec2(0.38,0.0) )+0.08 );
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

const float PERIOD   = 2.0;
vec4 sm_highlight( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float progress = mod( u_time, PERIOD ) / PERIOD;
   vec2 uv = texture_coords*2.0-1.0;
   float r = progress;
   float l = length(uv);
   if (l > r)
      discard;
   float d = abs(length(uv)-r);
   colour.a *= smoothstep( -0.1, 0.0, -d );
   colour.a *= min( 1.0,  10.0*(1.0-progress) );
   return colour;
}

vec4 sm( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   colour.a *= clamp( 10.0*sin( u_time * 1.5 ), 0.0, 1.0 );
   if (colour.a <= 0.0)
      discard;
   vec2 uv = texture_coords*2.0-1.0;
   float d = length(uv)-0.3;
   colour.a *= smoothstep( -0.7, 0.0, -d );
   return colour;
}

vec4 electric( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float m = 1.0 / dimensions.x;
   float progress = fract( u_time*1.3 );
   vec2 uv = texture_coords*2.0-1.0;
   float u_r = floor(u_time*1.3);//floor(2.0*u_time);

   colour = vec4( 0.1, 0.7, 0.9, 1.0 );

   float angle = 2.0*M_PI*random(u_r);
   float shear = 0.5*random(10.0*u_r+10.0);
   float c = cos(angle);
   float s = sin(angle);

   mat2 R = mat2(c,-s,s,c);
   mat2 S = mat2(1.0,shear,0.0,1.0);
   uv = S * R * uv;

   float r = length(uv);

   vec3 nuv = vec3( 3.0*uv, u_time );
   float n = abs(snoise( nuv ));
   n += 0.5*abs(snoise(2.0*nuv));
   n += 0.25*abs(snoise(4.0*nuv));
   n = pow(1.0-n,2.0);
   colour.rgb += 0.3*n;

   const float UP = 0.7;
   const float MAX = 0.65;

   float t = 0.5*smoothstep( 0.0, 1.0, progress / UP );
   float mixr = smoothstep( 0.0, 1.0, (progress-UP) / (1.0-UP) );
   float w = 0.3;
   float d, d2;
   if (mixr <= 0.0)
      d = abs(sdCircle( uv, t ))-w;
   else
      d = abs(sdCircle( uv, MAX - 2.0*mixr ))-w-mixr;
   colour.a *= smoothstep( 0.0, w-0.1, -d ) * n;
   //colour.a *= smoothstep( 0.0, w-0.1, -d );

   w = 0.1;
   if (mixr <= 0.0)
      d = abs(sdCircle( uv, t ))-w;
   else
      d = abs(sdCircle( uv, MAX - 2.0*mixr ))-w-mixr;
   colour.a += smoothstep( 0.0, w, -d );

   colour.a = pow( colour.a, 2.0 );

   return colour;
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
   //col_out = sdf_jumpmarker( colour, uv_rel );
   //col_out = sdf_pilotmarker( colour, uv_rel );
   //col_out = sdf_playermarker( colour, uv_rel );
   //col_out = sdf_stealth( colour, uv_rel );
   //col_out = sdf_jumplane( colour, uv_rel );
   //col_out = sdf_gear( colour, uv_rel );
   //col_out = sdf_sysrhombus( colour, uv_rel );
   //col_out = sdf_sysmarker( colour, uv_rel );
   //col_out = sdf_missilelockon( colour, uv_rel );
   //col_out = sdf_selectposition( colour, uv_rel );
   //col_out = sdf_vnarrow( colour, uv_rel );
   //col_out = sdf_vncont( colour, uv_rel );
   //col_out = sdf_vndone( colour, uv_rel );
   //col_out = sdf_notemarker( colour, uv_rel );
   //col_out = sm_highlight( colour, tex, uv, px );
   //col_out = sm( colour, tex, uv, px );
   col_out = electric( colour, tex, uv, px );

   return mix( bg(uv), col_out, col_out.a );
}
