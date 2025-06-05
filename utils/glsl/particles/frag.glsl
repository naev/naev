#pragma language glsl3
uniform float u_time = 0.0;
uniform float u_fade = 1.0;
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

// Cellular noise, returning F1 and F2 in a vec2.
// 3x3x3 search region for good F2 everywhere, but a lot
// slower than the 2x2x2 version.
// The code below is a bit scary even to its author,
// but it has at least half decent performance on a
// modern GPU. In any case, it beats any software
// implementation of Worley noise hands down.
vec2 cellular(vec3 P) {
#define K 0.142857142857 // 1/7
#define Ko 0.428571428571 // 1/2-K/2
#define K2 0.020408163265306 // 1/(7*7)
#define Kz 0.166666666667 // 1/6
#define Kzo 0.416666666667 // 1/2-1/6*2
#define jitter 1.0 // smaller jitter gives more regular pattern

	vec3 Pi = mod289(floor(P));
 	vec3 Pf = fract(P) - 0.5;

	vec3 Pfx = Pf.x + vec3(1.0, 0.0, -1.0);
	vec3 Pfy = Pf.y + vec3(1.0, 0.0, -1.0);
	vec3 Pfz = Pf.z + vec3(1.0, 0.0, -1.0);

	vec3 p = permute(Pi.x + vec3(-1.0, 0.0, 1.0));
	vec3 p1 = permute(p + Pi.y - 1.0);
	vec3 p2 = permute(p + Pi.y);
	vec3 p3 = permute(p + Pi.y + 1.0);

	vec3 p11 = permute(p1 + Pi.z - 1.0);
	vec3 p12 = permute(p1 + Pi.z);
	vec3 p13 = permute(p1 + Pi.z + 1.0);

	vec3 p21 = permute(p2 + Pi.z - 1.0);
	vec3 p22 = permute(p2 + Pi.z);
	vec3 p23 = permute(p2 + Pi.z + 1.0);

	vec3 p31 = permute(p3 + Pi.z - 1.0);
	vec3 p32 = permute(p3 + Pi.z);
	vec3 p33 = permute(p3 + Pi.z + 1.0);

	vec3 ox11 = fract(p11*K) - Ko;
	vec3 oy11 = mod7(floor(p11*K))*K - Ko;
	vec3 oz11 = floor(p11*K2)*Kz - Kzo; // p11 < 289 guaranteed

	vec3 ox12 = fract(p12*K) - Ko;
	vec3 oy12 = mod7(floor(p12*K))*K - Ko;
	vec3 oz12 = floor(p12*K2)*Kz - Kzo;

	vec3 ox13 = fract(p13*K) - Ko;
	vec3 oy13 = mod7(floor(p13*K))*K - Ko;
	vec3 oz13 = floor(p13*K2)*Kz - Kzo;

	vec3 ox21 = fract(p21*K) - Ko;
	vec3 oy21 = mod7(floor(p21*K))*K - Ko;
	vec3 oz21 = floor(p21*K2)*Kz - Kzo;

	vec3 ox22 = fract(p22*K) - Ko;
	vec3 oy22 = mod7(floor(p22*K))*K - Ko;
	vec3 oz22 = floor(p22*K2)*Kz - Kzo;

	vec3 ox23 = fract(p23*K) - Ko;
	vec3 oy23 = mod7(floor(p23*K))*K - Ko;
	vec3 oz23 = floor(p23*K2)*Kz - Kzo;

	vec3 ox31 = fract(p31*K) - Ko;
	vec3 oy31 = mod7(floor(p31*K))*K - Ko;
	vec3 oz31 = floor(p31*K2)*Kz - Kzo;

	vec3 ox32 = fract(p32*K) - Ko;
	vec3 oy32 = mod7(floor(p32*K))*K - Ko;
	vec3 oz32 = floor(p32*K2)*Kz - Kzo;

	vec3 ox33 = fract(p33*K) - Ko;
	vec3 oy33 = mod7(floor(p33*K))*K - Ko;
	vec3 oz33 = floor(p33*K2)*Kz - Kzo;

	vec3 dx11 = Pfx + jitter*ox11;
	vec3 dy11 = Pfy.x + jitter*oy11;
	vec3 dz11 = Pfz.x + jitter*oz11;

	vec3 dx12 = Pfx + jitter*ox12;
	vec3 dy12 = Pfy.x + jitter*oy12;
	vec3 dz12 = Pfz.y + jitter*oz12;

	vec3 dx13 = Pfx + jitter*ox13;
	vec3 dy13 = Pfy.x + jitter*oy13;
	vec3 dz13 = Pfz.z + jitter*oz13;

	vec3 dx21 = Pfx + jitter*ox21;
	vec3 dy21 = Pfy.y + jitter*oy21;
	vec3 dz21 = Pfz.x + jitter*oz21;

	vec3 dx22 = Pfx + jitter*ox22;
	vec3 dy22 = Pfy.y + jitter*oy22;
	vec3 dz22 = Pfz.y + jitter*oz22;

	vec3 dx23 = Pfx + jitter*ox23;
	vec3 dy23 = Pfy.y + jitter*oy23;
	vec3 dz23 = Pfz.z + jitter*oz23;

	vec3 dx31 = Pfx + jitter*ox31;
	vec3 dy31 = Pfy.z + jitter*oy31;
	vec3 dz31 = Pfz.x + jitter*oz31;

	vec3 dx32 = Pfx + jitter*ox32;
	vec3 dy32 = Pfy.z + jitter*oy32;
	vec3 dz32 = Pfz.y + jitter*oz32;

	vec3 dx33 = Pfx + jitter*ox33;
	vec3 dy33 = Pfy.z + jitter*oy33;
	vec3 dz33 = Pfz.z + jitter*oz33;

	vec3 d11 = dx11 * dx11 + dy11 * dy11 + dz11 * dz11;
	vec3 d12 = dx12 * dx12 + dy12 * dy12 + dz12 * dz12;
	vec3 d13 = dx13 * dx13 + dy13 * dy13 + dz13 * dz13;
	vec3 d21 = dx21 * dx21 + dy21 * dy21 + dz21 * dz21;
	vec3 d22 = dx22 * dx22 + dy22 * dy22 + dz22 * dz22;
	vec3 d23 = dx23 * dx23 + dy23 * dy23 + dz23 * dz23;
	vec3 d31 = dx31 * dx31 + dy31 * dy31 + dz31 * dz31;
	vec3 d32 = dx32 * dx32 + dy32 * dy32 + dz32 * dz32;
	vec3 d33 = dx33 * dx33 + dy33 * dy33 + dz33 * dz33;

	// Sort out the two smallest distances (F1, F2)
#ifndef CELLULAR_NOISE_ACCURATE
	// Cheat and sort out only F1
	vec3 d1 = min(min(d11,d12), d13);
	vec3 d2 = min(min(d21,d22), d23);
	vec3 d3 = min(min(d31,d32), d33);
	vec3 d = min(min(d1,d2), d3);
	d.x = min(min(d.x,d.y),d.z);
	return vec2(sqrt(d.x)); // F1 duplicated, no F2 computed
#else
	// Do it right and sort out both F1 and F2
	vec3 d1a = min(d11, d12);
	d12 = max(d11, d12);
	d11 = min(d1a, d13); // Smallest now not in d12 or d13
	d13 = max(d1a, d13);
	d12 = min(d12, d13); // 2nd smallest now not in d13
	vec3 d2a = min(d21, d22);
	d22 = max(d21, d22);
	d21 = min(d2a, d23); // Smallest now not in d22 or d23
	d23 = max(d2a, d23);
	d22 = min(d22, d23); // 2nd smallest now not in d23
	vec3 d3a = min(d31, d32);
	d32 = max(d31, d32);
	d31 = min(d3a, d33); // Smallest now not in d32 or d33
	d33 = max(d3a, d33);
	d32 = min(d32, d33); // 2nd smallest now not in d33
	vec3 da = min(d11, d21);
	d21 = max(d11, d21);
	d11 = min(da, d31); // Smallest now in d11
	d31 = max(da, d31); // 2nd smallest now not in d31
	d11.xy = (d11.x < d11.y) ? d11.xy : d11.yx;
	d11.xz = (d11.x < d11.z) ? d11.xz : d11.zx; // d11.x now smallest
	d12 = min(d12, d21); // 2nd smallest now not in d21
	d12 = min(d12, d22); // nor in d22
	d12 = min(d12, d31); // nor in d31
	d12 = min(d12, d32); // nor in d32
	d11.yz = min(d11.yz,d12.xy); // nor in d12.yz
	d11.y = min(d11.y,d12.z); // Only two more to go
	d11.y = min(d11.y,d11.z); // Done! (Phew!)
	return sqrt(d11.xy); // F1, F2
#endif
}
#undef K
#undef Ko
#undef K2
#undef Kz
#undef Kzo
#undef jitter

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

float dot2( in vec2 v ) { return dot(v,v); }
float sdRoundedCross( in vec2 p, in float h )
{
   float k = 0.5*(h+1.0/h);
   p = abs(p);
   return ( p.x<1.0 && p.y<p.x*(k-h)+h ) ?
      k-sqrt(dot2(p-vec2(1,k)))  :
      sqrt(min(dot2(p-vec2(0,h)),
               dot2(p-vec2(1,0))));
}

/*  p is center
 *  r is length
 *  d is circleness (0.0 is circle, 1.0 is empty) */
float sdVesica( vec2 p, float r, float d )
{
   p = abs(p);
   float b = sqrt(r*r-d*d);
   return ((p.y-b)*d>p.x*b) ? length(p-vec2(0.0,b))
      : length(p-vec2(-d,0.0))-r;
}

float blendOverlay(float base, float blend) {
   return base<0.5?(2.0*base*blend):(1.0-2.0*(1.0-base)*(1.0-blend));
}
vec3 blendOverlay(vec3 base, vec3 blend) {
   return vec3(blendOverlay(base.r,blend.r),blendOverlay(base.g,blend.g),blendOverlay(base.b,blend.b));
}
vec3 blendOverlay(vec3 base, vec3 blend, float opacity) {
   return (blendOverlay(base, blend) * opacity + base * (1.0 - opacity));
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

const float u_r = 0.0;

vec4 chakra( vec2 uv )
{
   const vec2 b = vec2( 0.8, 0.5 );
   vec4 colour = vec4( 1.0, 0.8, 0.0, 1.0 );

   float d = sdEgg( uv, b );
   vec2 nuv = vec2(2.0,4.0) * uv * vec2( exp(uv.x), pow(uv.y,0.5) );
   float n = 0.3*snoise( nuv + 3.0*vec2(u_time,u_r) );
   colour.a *= smoothstep( -0.1, 0.8, -d ) * (n+0.6);
   colour += smoothstep( -0.4, 0.7, -d );

   return colour;
}

vec4 chakra_explosion( vec2 uv )
{
   vec4 colour = vec4( 1.0, 0.8, 0.0, 1.0 );
   float progress = fract( u_time*0.5 );

   float angle = 2.0*M_PI*random(u_r);
   float shear = 0.3*random(10.0*u_r+10.0);
   float c = cos(angle);
   float s = sin(angle);

   mat2 R = mat2(c,-s,s,c);
   mat2 S = mat2(1.0,shear,0.0,1.0);
   uv = S * R * uv;

   float d = sdCircle( uv, 0.8*progress );
   d = max( d, -sdCircle( uv, 2.8*(progress-0.5) ) );
   vec2 nuv = 3.0 * uv;
   float n = 0.3*snoise( nuv + vec2(u_r) );
   colour.a *= smoothstep( -0.2, 0.2, -d ) * (n+0.6);

   colour   += 0.6 * smoothstep( -0.1, 0.1, -d );

   return colour;
}

vec4 cleansing_flames( vec2 uv )
{
   vec4 colour = vec4( 1.0, 0.8, 0.0, 1.0 );
   float progress = fract( u_time*0.5 );

   float d = sdCircle( uv, 1.0*progress-0.1 );
   d = max( d, -sdCircle( uv, 1.5*progress-0.4 ) );
   vec2 nuv = 4.0 * uv;
   float n = 0.675*snoise( nuv + vec2(u_r) );
   n += 0.325*snoise( nuv*2.0 - vec2(u_r) );
   colour.a *= smoothstep( -0.1, 0.1, -d ) * (n+0.8);

   colour.a += 0.9 * smoothstep( -0.1, 0.1, -d );

   return colour;
}

vec4 emp_exp( vec2 uv )
{
   vec4 colour = vec4( 1.0, 1.0, 1.0, 1.0 );
   float progress = fract( u_time*0.4 );

   uv.y *= 1.4; /// approximate affine

   /*
    * Back part.
    */
   if (1==1) {
      /* Base shape. */
      float d = sdCircle( uv, 1.0*progress-0.1 );
      d = max( d, -sdCircle( uv, 0.6*progress-0.4 ) );
      colour.rgb -= vec3( 1.0, 0.6, 0.1 )*progress - 0.15;
      colour.a *= smoothstep( -0.1, 0.3, -d ) * max(0.0, 2.0-progress);
      colour.a *= min(1.0, 6.0-6.0*progress);
      /* Noise texture. */
      float r = sqrt(dot(uv, uv)) - 0.5*progress;
      float a = atan(uv.y, uv.x);
      vec2 nuv = vec2(1.0*r, 8.0*a) * 0.5;
      float n = 0.65*snoise( nuv );
      n += 0.35*snoise( 2.0*nuv );
      colour.a *= min(1.0, 0.8+0.3*n*progress);
   }
   else
      colour = vec4( vec3(1.0), 0.0 );

   /* On top part. */
   if (1==1) {
      float c, s, r;
      r = M_PI*(progress+u_r);
      s = sin(r);
      c = cos(r);
      mat2 R = mat2( c, s, -s, c );
      vec2 nuv = R*uv;
      nuv *= 13.0/min(10.0,20.0*progress) * pow(length(nuv),-0.5);

      float d = sdRoundedCross( nuv, 1.0 );
      float a = smoothstep( 0.0, 0.2, -d ) * clamp( (2.0*progress) * (4.0-4.0*progress), 0.0, 1.0 );
      colour.a = max(colour.a,a);
      colour.rgb = a*vec3(1.0)+(1.0-a)*colour.rgb;
   }

   return colour;
}

vec4 spittle( vec2 uv )
{
   vec4 colour = vec4( 0.2, 0.8, 0.3, 1.0 );
   float progress = fract( u_time*0.4 );
   float dout = sdCircle( uv, 0.98 );
   float din = sdCircle( uv, 0.95 );

   vec3 uvt = 2.0*vec3( uv, length(uv)-0.4*u_time );
   float f = (1.0-cellular( uvt ).x);

   colour.a *= 1.0-smoothstep( -0.05, 1.0, -din );
   colour.a *= 1.0-smoothstep( 0.2, 0.0, -dout-0.08*f+0.1 );
   colour.a *= 0.7+0.4*f;

   return colour;
}

vec4 energy_harpoon( vec2 uv )
{
   vec4 colour = vec4( 0.95, 0.1, 0.3, 1.0 );

   float fade = min(u_time*4.0,u_fade+0.25)-0.25;
   float n = snoise( uv+vec2(6.0*u_time,u_r) );

   float d = sdRoundedCross( 1.25*(uv+vec2(0.03*n,0.0)), min(0.25,fade) );
   float din = sdVesica( uv.yx+vec2(0.05*n,0.0), 1.0, 0.8 );

   colour.rgb += pow( smoothstep( 0.0, 0.2, -din ), 2.0 );
   colour.a *= smoothstep( 0.0, 0.2, -d+0.25 );

   return colour;
}

vec4 razor( vec2 uv )
{
   vec4 colour = vec4( 0.95, 0.7, 0.3, 1.0 ); // S3
   //vec4 colour = vec4( 0.8, 0.65, 0.7, 1.0 ); // S2
   //vec4 colour = vec4( 0.6, 0.6, 0.9, 1.0 ); // S1

   vec2 uvoff = vec2(2.0*u_time,u_r);
   vec2 nuv = uv+uvoff;
   float n = snoise( nuv );
   //n += 0.5*snoise( 2.0*nuv ); // At the sizes we render, probably don't need two octaves...

   float d = sdCircle( uv, 1.0 );

   colour.a *= smoothstep( 0.0, 0.5, -d );
   //colour.a *= mix( 0.5 + 0.5*snoise( normalize(uv)*0.7+uvoff ), 1.0, colour.a );
   colour.rgb += smoothstep( 0.3, 1.0, -d ) * (n*0.7+0.3);
   //colour.rgb += smoothstep( 0.3, 1.0, -d ) * (n*0.5+0.2); // With two octaves

   return colour;
}

vec4 disruptor( vec2 uv )
{
   vec4 colour = vec4( 0.95, 0.7, 0.5, 1.0 ); // S3
   //vec4 colour = vec4( 0.8, 0.6, 0.75, 1.0 ); // S2
   //vec4 colour = vec4( 0.4, 0.6, 0.95, 1.0 ); // S1

   vec2 uvoff = vec2(1.4*u_time,u_r);
   vec2 nuv = uv+uvoff;
   nuv *= 1.5;
   float n = snoise( nuv );
   n += 0.5*snoise( 2.0*nuv );

   float d = sdCircle( uv, 1.0 );

   colour.a *= smoothstep( 0.0, 0.5, -d );
   colour.a *= mix( 0.5 + 0.5*snoise( normalize(uv)*0.7+uvoff ), 1.0, colour.a );
   colour.rgb += smoothstep( 0.3, 1.0, -d ) * (n*0.7+0.3); // With two octaves

   return colour;
}

vec4 quantum( vec2 uv )
{
   vec4 colour = vec4( 0.4, 0.6, 0.95, 1.0 ); // S1
   colour = vec4(1.0);

   vec2 nuv = dimensions*uv/8.0 + vec2(u_time,u_r);
   vec2 duv = abs(nuv-round(nuv));
   float din = min(duv.x,duv.y);

   float d = sdEgg( uv, vec2( 1.0, 0.7 ) );

   float n = snoise( nuv );

   colour.a *= smoothstep( 0.0, 0.3, -d ) * smoothstep( -0.5, 0.0, -din );

   return colour;
}

vec4 laser_square( vec2 uv )
{
   const vec4 COLOUR = vec4( 0.95, 0.1, 0.3, 1.0 );
   const vec4 COLOUR_FADE = vec4( 0.75, 0.75, 0.1, 1.0 );

   float fade = min(u_time*4.0,u_fade+0.3)-0.3;
   float n = snoise( 1.5*uv+vec2(2.0*u_time,u_r) );

   float d = sdBox( 1.25*(uv+vec2(0.015*n,0.0)), vec2(0.9,min(0.1,fade)) );
   float din = sdBox( uv+vec2(0.02*n,0.0), vec2(0.6, -0.04) );

   vec4 colour = mix( COLOUR_FADE, COLOUR, u_fade );
   colour.rgb += pow( smoothstep( 0.0, 0.2, -din+0.2 ), 1.5 ) + vec3(0.2)*n;
   colour.a *= smoothstep( 0.0, 0.2, -d+0.3 );

   return colour;
}

vec4 laser( vec2 uv )
{
   //const vec4 COLOUR = vec4( 0.95, 0.1, 0.3, 1.0 ); // MK1
   const vec4 COLOUR = vec4( 0.1, 0.95, 0.1, 1.0 ); // MK2
   //const vec4 COLOUR = vec4( 0.1, 0.95, 0.8, 1.0 ); // Heavy Laser
   //const vec4 COLOUR = vec4( 0.6, 0.1, 0.95, 1.0 ); // Turbolaser
   const vec4 COLOUR_FADE = vec4( 0.75, 0.75, 0.1, 1.0 );

   float fade = min(u_time*4.0,u_fade+0.3)-0.3;
   float n = snoise( 1.5*uv+vec2(2.0*u_time,u_r) );

   float d = sdVesica( uv.yx+vec2(0.0,0.015*n), 2.3, 2.1 );

   vec4 colour = mix( COLOUR_FADE, COLOUR, u_fade );
   colour.rgb += pow( smoothstep( 0.0, 0.2, -d-0.025 ), 1.5 ) + vec3(0.2)*n;
   colour.a *= smoothstep( 0.0, 0.2, -d+0.1 );

   return colour;
}

vec4 ripper_square( vec2 uv )
{
   const vec4 COLOUR = vec4( 0.1, 0.95, 0.3, 1.0 );
   const vec4 COLOUR_FADE = vec4( 0.75, 0.75, 0.1, 1.0 );

   float fade = min(u_time*4.0,u_fade+0.3)-0.3;
   float n = snoise( uv+vec2(2.0*u_time,u_r) );
   vec2 offset = vec2( 0.0, -0.3 );

   uv.y = abs(uv.y);

   float d = sdBox( 1.25*(uv+vec2(0.015*n,0.0)+offset), vec2(0.9,min(0.05,fade)) );
   float din = sdBox( uv+vec2(0.02*n,0.0)+offset, vec2(0.8, 0.15) );

   vec4 colour = mix( COLOUR_FADE, COLOUR, u_fade );
   colour.rgb += pow( smoothstep( 0.0, 0.2, -din ), 2.0 ) + vec3(0.2)*n;
   colour.a *= smoothstep( 0.0, 0.2, -d+0.3 );

   return colour;
}

vec4 ripper( vec2 uv )
{
   const vec4 COLOUR = vec4( 0.1, 0.95, 0.3, 1.0 );
   const vec4 COLOUR_FADE = vec4( 0.75, 0.75, 0.1, 1.0 );

   float fade = min(u_time*4.0,u_fade+0.3)-0.3;
   float n = snoise( uv+vec2(2.0*u_time,u_r) );
   vec2 offset = vec2( -0.3, 0.0 );

   uv.y = abs(uv.y);

   float d = sdVesica( uv.yx+vec2(0.0,0.015*n)+offset, 2.3, 2.1 );

   vec4 colour = mix( COLOUR_FADE, COLOUR, u_fade );
   colour.rgb += pow( smoothstep( 0.0, 0.2, -d-0.01 ), 1.5 ) + vec3(0.2)*n;
   colour.a *= smoothstep( 0.0, 0.2, -d+0.1 );

   return colour;
}

vec4 blend( vec4 b, vec4 a )
{
   vec4 result;
   result.a   = a.a + b.a * (1.0-a.a);
   result.rgb = (a.rgb * a.a + b.rgb * b.a * (1.0-a.a)) / result.a;
   return result;
}
vec4 reaver_square( vec2 uv )
{
   const vec4 COLOUR = vec4( 0.1, 0.95, 0.3, 1.0 );
   const vec4 COLOUR_FADE = vec4( 0.75, 0.75, 0.1, 1.0 );

   float fade = min(u_time*4.0,u_fade+0.3)-0.3;
   float n1 = snoise( uv+vec2(2.0*u_time,u_r) );
   float n2 = snoise( uv+vec2(2.0*u_time,u_r+100.0) );
   float spin = u_time * (2.5 + 1.5*sin(2.0/7.0*u_time + u_r)) + 7.0*u_r;
   float st = sin(spin);
   vec2 offset1 = vec2( 0.0, -0.3 ) * st;
   vec2 offset2 = vec2( 0.0,  0.3 ) * st;

   float d1 = sdBox( 1.25*(uv+vec2(0.015*n1,0.0)+offset1), vec2(0.9,min(0.03,fade)) );
   float din1 = sdBox( uv+vec2(0.02*n1,0.0)+offset1, vec2(0.8, 0.15) );

   float d2 = sdBox( 1.25*(uv+vec2(0.015*n2,0.0)+offset2), vec2(0.9,min(0.03,fade)) );
   float din2 = sdBox( uv+vec2(0.02*n2,0.0)+offset2, vec2(0.8, 0.15) );

   vec4 col1 = mix( COLOUR_FADE, COLOUR, u_fade );
   col1.rgb += pow( smoothstep( 0.0, 0.2, -din1 ), 2.0 ) + vec3(0.2)*n2;
   col1.a *= smoothstep( 0.0, 0.2, -d1+0.3 );

   vec4 col2 = mix( COLOUR_FADE, COLOUR, u_fade );
   col2.rgb += pow( smoothstep( 0.0, 0.2, -din2 ), 2.0 ) + vec3(0.2)*n1;
   col2.a *= smoothstep( 0.0, 0.2, -d2+0.3 );

   vec4 colour;
   if (cos(spin) < 0.0)
      colour = blend( col1, col2 );
   else
      colour = blend( col2, col1 );
   return colour;
}
vec4 reaver( vec2 uv )
{
   const vec4 COLOUR = vec4( 0.1, 0.95, 0.3, 1.0 );
   const vec4 COLOUR_FADE = vec4( 0.75, 0.75, 0.1, 1.0 );

   float fade = min(u_time*4.0,u_fade+0.3)-0.3;
   float n1 = snoise( uv+vec2(2.0*u_time,u_r) );
   float n2 = snoise( uv+vec2(2.0*u_time,u_r+100.0) );
   float spin = u_time * (2.5 + 1.5*sin(2.0/7.0*u_time + u_r)) + 7.0*u_r;
   float st = sin(spin);
   vec2 offset1 = vec2( -0.4, 0.0 ) * st;
   vec2 offset2 = vec2(  0.4, 0.0 ) * st;

   float d1 = sdVesica( uv.yx+vec2(0.0,0.015*n1)+offset1, 2.3, 2.125 );
   float d2 = sdVesica( uv.yx+vec2(0.0,0.015*n2)+offset2, 2.3, 2.125 );

   vec4 col1 = mix( COLOUR_FADE, COLOUR, u_fade );
   col1.rgb += pow( smoothstep( 0.0, 0.2, -d1-0.01 ), 1.5 ) + vec3(0.2)*n1;
   col1.a *= smoothstep( 0.0, 0.2, -d1+0.1 );

   vec4 col2 = mix( COLOUR_FADE, COLOUR, u_fade );
   col2.rgb += pow( smoothstep( 0.0, 0.2, -d2-0.01 ), 1.5 ) + vec3(0.2)*n2;
   col2.a *= smoothstep( 0.0, 0.2, -d2+0.1 );

   vec4 colour;
   if (cos(spin) < 0.0)
      colour = blend( col1, col2 );
   else
      colour = blend( col2, col1 );
   return colour;
}

vec4 eruptor( vec2 uv )
{
   const vec3 COLOUR_IN = vec3( 1.2, 0.1, 0.4 );
   const vec3 COLOUR_OUT = vec3( 1.95, 0.8, 1.95 );

   vec2 r = vec2(-u_time*0.25, u_r);

   float n = snoise( 0.7*(uv + r) )*0.5+0.25;
   n += snoise( 2.0*(uv + r) )*0.25;

   vec4 colour;
   colour.a = n * max( 0.0, (min(1.0,u_time)-pow(length(uv),1.5)) );
   colour.rgb = mix( COLOUR_OUT, COLOUR_IN, colour.a*2.0-0.5 );
   return colour;
}

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 col_out;
   vec2 uv_rel = uv*2.0-1.0;
   uv_rel.y = -uv_rel.y;

   //col_out = chakra( uv_rel );
   //col_out = chakra_explosion( uv_rel );
   //col_out = cleansing_flames( uv_rel );
   //col_out = emp_exp( uv_rel );
   //col_out = spittle( uv_rel );
   //col_out = energy_harpoon( uv_rel );
   //col_out = razor( uv_rel );
   //col_out = disruptor( uv_rel );
   //col_out = quantum( uv_rel );
   //col_out = laser_square( uv_rel );
   col_out = laser( uv_rel );
   //col_out = ripper_square( uv_rel );
   //col_out = ripper( uv_rel );
   //col_out = reaver_square( uv_rel );
   //col_out = reaver( uv_rel );
   //col_out = eruptor( uv_rel );

   return mix( bg(uv), col_out, clamp(col_out.a, 0.0, 1.0) );
}
