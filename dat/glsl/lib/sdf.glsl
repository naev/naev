#ifndef _SDF_GLSL
#define _SDF_GLSL

/*
 * Largely taken or inspired by https://iquilezles.untergrund.net/www/articles/distfunctions2d/distfunctions2d.htm

MIT License. Inigo Quilez, Edgar Simo-Serra

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * All these are centered at p with parameters being some sort of radius from the center.
 */

/*
 * Helper Functions.
 */
float cro( vec2 a, vec2 b ) { return a.x*b.y - a.y*b.x; }
float ndot( vec2 a, vec2 b ) { return a.x*b.x - a.y*b.y; }
float dot2( vec2 v ) { return dot(v,v); }


/* Circle. */
float sdCircle( vec2 p, float r )
{
   return length(p)-r;
}

/* Box at position b with border b. */
float sdBox( vec2 p, vec2 b )
{
   vec2 d = abs(p)-b;
   return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

/* Segment going from point a to point b with 0 width. */
float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
   vec2 pa = p-a, ba = b-a;
   float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
   return length( pa - ba*h );
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

/* Pentagon centered at p with radius r. */
float sdPentagon( vec2 p, float r )
{
   // cos, sin, and tan of M_PI/5.0
   const vec3 k = vec3(0.809016994,0.587785252,0.726542528);
   p.x = abs(p.x);
   p -= 2.0*min(dot(vec2(-k.x,k.y),p),0.0)*vec2(-k.x,k.y);
   p -= 2.0*min(dot(vec2( k.x,k.y),p),0.0)*vec2( k.x,k.y);
   p -= vec2(clamp(p.x,-r*k.z,r*k.z),r);
   return length(p)*sign(p.y);
}

/* Hexagon centered at p with radius r. */
float sdHexagon( vec2 p, float r )
{
   // cos, sin, and tan of M_PI/6.0
   const vec3 k = vec3(-0.866025404,0.5,0.577350269);
   p = abs(p);
   p -= 2.0*min(dot(k.xy,p),0.0)*k.xy;
   p -= vec2(clamp(p.x, -k.z*r, k.z*r), r);
   return length(p)*sign(p.y);
}

/* Octogan centered at p with radius r. */
float sdOctogon( vec2 p, float r )
{
   // cos, sin, and tan of M_PI/88888888.0
   const vec3 k = vec3(-0.9238795325, 0.3826834323, 0.4142135623 );
   p = abs(p);
   p -= 2.0*min(dot(vec2( k.x,k.y),p),0.0)*vec2( k.x,k.y);
   p -= 2.0*min(dot(vec2(-k.x,k.y),p),0.0)*vec2(-k.x,k.y);
   p -= vec2(clamp(p.x, -k.z*r, k.z*r), r);
   return length(p)*sign(p.y);
}

/* Arc that is part of a circle centered at p.
 * sca is the sin/cos of the orientation
 * scb is the sin/cos of the aperture
 * ra is inner radius
 * rb is outter radius */
float sdArc( vec2 p, vec2 sca, vec2 scb, float ra, float rb )
{
   p *= mat2(sca.x,sca.y,-sca.y,sca.x);
   p.x = abs(p.x);
   float k = (scb.y*p.x>scb.x*p.y) ? dot(p.xy,scb) : length(p);
   return sqrt( max(0.0, dot(p,p) + ra*ra - 2.0*ra*k) ) - rb;
}

/* Pie that is part of a circle centered at p.
 * c is the sin/cos of aperture
 * r is the radius */
float sdPie( vec2 p, vec2 c, float r )
{
    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p-c*clamp(dot(p,c),0.0,r));
    return max(l,m*sign(c.y*p.x-c.x*p.y));
}

/* Rhombus at position p with size b. */
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

/* h is height */
float sdRoundedCross( vec2 p, float h )
{
   float k = 0.5*(h+1.0/h); // k should be const at modeling time
   p = abs(p);
   return ( p.x<1.0 && p.y<p.x*(k-h)+h ) ?
      k-sqrt(dot2(p-vec2(1.0,k)))  :
      sqrt(min(dot2(p-vec2(0.0,h)),
               dot2(p-vec2(1.0,0.0))));
}

/* Uneven capsule oriented on Y axis. */
float sdUnevenCapsuleY( vec2 p, float ra, float rb, float h )
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

/* Uneven capsule between points pa and pb with radius ra at point pa and rb at
 * point pb. */
float sdUnevenCapsule( vec2 p, vec2 pa, vec2 pb, float ra, float rb )
{
    p  -= pa;
    pb -= pa;
    float h = dot(pb,pb);
    vec2  q = vec2( dot(p,vec2(pb.y,-pb.x)), dot(p,pb) )/h;

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

/*  r1 is first width
 *  r2 is second width
 *  he is height
 */
float sdTrapezoid( vec2 p, float r1, float r2, float he )
{
    vec2 k1 = vec2(r2,he);
    vec2 k2 = vec2(r2-r1,2.0*he);
    p.x = abs(p.x);
    vec2 ca = vec2(p.x-min(p.x,(p.y<0.0)?r1:r2), abs(p.y)-he);
    vec2 cb = p - k1 + k2*clamp( dot(k1-p,k2)/dot2(k2), 0.0, 1.0 );
    float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(dot2(ca),dot2(cb)) );
}

float sdSmoothUnion( float a, float b, float k )
{
   float h = max( k-abs(a-b), 0.0 )/k;
   return min( a, b ) - h*h*k*(1.0/4.0);
}

float sdSmoothDifference( float a, float b, float k )
{
   float h = max(k-abs(-a-b),0.0);
   return max(-a, b) + h*h*0.25/k;
}

#endif /* _SDF_GLSL */
