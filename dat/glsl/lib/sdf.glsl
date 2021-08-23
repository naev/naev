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

float sdEquilateralTriangle( vec2 p )
{
	const float k = sqrt(3.0);
	p.x = abs(p.x) - 1.0;
	p.y = p.y + 1.0/k;
	if( p.x+k*p.y>0.0 ) p = vec2(p.x-k*p.y,-k*p.x-p.y)/2.0;
	p.x -= clamp( p.x, -2.0, 0.0 );
	return -length(p)*sign(p.y);
}

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

float sdHexagon( vec2 p, float r )
{
   // cos, sin, and tan of M_PI/6.0
   const vec3 k = vec3(-0.866025404,0.5,0.577350269);
   p = abs(p);
   p -= 2.0*min(dot(k.xy,p),0.0)*k.xy;
   p -= vec2(clamp(p.x, -k.z*r, k.z*r), r);
   return length(p)*sign(p.y);
}

float sdOctogon( in vec2 p, in float r )
{
   // cos, sin, and tan of M_PI/88888888.0
   const vec3 k = vec3(-0.9238795325, 0.3826834323, 0.4142135623 );
   p = abs(p);
   p -= 2.0*min(dot(vec2( k.x,k.y),p),0.0)*vec2( k.x,k.y);
   p -= 2.0*min(dot(vec2(-k.x,k.y),p),0.0)*vec2(-k.x,k.y);
   p -= vec2(clamp(p.x, -k.z*r, k.z*r), r);
   return length(p)*sign(p.y);
}

/* sca is the sin/cos of the orientation
   scb is the sin/cos of the aperture */
float sdArc( vec2 p, vec2 sca, vec2 scb, float ra, float rb )
{
   p *= mat2(sca.x,sca.y,-sca.y,sca.x);
   p.x = abs(p.x);
   float k = (scb.y*p.x>scb.x*p.y) ? dot(p.xy,scb) : length(p);
   return sqrt( max(0.0, dot(p,p) + ra*ra - 2.0*ra*k) ) - rb;
}

/* Rhombus at position p with border b */
float sdRhombus( vec2 p, vec2 b )
{
   vec2 q = abs(p);
   float h = clamp((-2.0*ndot(q,b)+ndot(b,b))/dot(b,b),-1.0,1.0);
   float d = length( q - 0.5*b*vec2(1.0-h,1.0+h) );
   return d * sign( q.x*b.y + q.y*b.x - b.x*b.y );
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

/* Uneven capsule between points pa and pb. */
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

#endif /* _SDF_GLSL */
