#include "math.h"
const int ITERATIONS = 4;
const float SCALAR = 2.;

uniform vec4 color;
uniform vec2 center;
uniform float radius;
uniform float time;
out vec4 color_out;

// This shader is based on:
// --- Flow noise 2 by Fabrice NEYRET - 2016   https://www.shadertoy.com/view/MstXWn
// which partially implements
// cd publi http://evasion.imag.fr/~Fabrice.Neyret/flownoise/index.gb.html
//          http://mrl.nyu.edu/~perlin/flownoise-talk/
// on top of this MIT-licensed shader
// --- Perlin noise by inigo quilez - iq/2013   https://www.shadertoy.com/view/XdXGW8
// (plus an MIT-licensed hash by the same author).
// MIT License:
// Copyright © 2013 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

vec2 hash( vec2 p )
{
   p = vec2( dot(p,vec2(127.1,311.7)),
             dot(p,vec2(269.5,183.3)) );
   return -1. + 2.*fract( sin( p )*43758.5453123 );
}

// Compatible interface with srnoise from https://github.com/stegu/webgl-noise/blob/master/src/psrdnoise2D.glsl
float srnoise( in vec2 p, in float rot )
{
   vec2 i = floor( p );
   vec2 f = fract( p );
   vec2 u = smoothstep( 0, 1, fract( p ) );
   float t = 2*M_PI * (rot + i.x - i.y);
   mat2 R = mat2( cos(t), -sin(t),
                  sin(t),  cos(t) );

   return mix( mix( dot( hash( i + vec2(0,0) ),  (f - vec2(0,0))*R ),
                    dot( hash( i + vec2(1,0) ), -(f - vec2(1,0))*R ), u.x),
               mix( dot( hash( i + vec2(0,1) ), -(f - vec2(0,1))*R ),
                    dot( hash( i + vec2(1,1) ),  (f - vec2(1,1))*R ), u.x), u.y);
}

void main(void) {
   float f = 0.0;
   vec2 uv = (gl_FragCoord.xy-center)*4./radius;
   for (int i=0; i<ITERATIONS; i++) {
      float scale = pow(SCALAR, i);
      f += abs( srnoise( uv*scale, .5*time/scale ) ) / scale;
   }
   color_out = mix( vec4( 0, 0, 0, 1 ), color, .1 + .7*f );

#include "colorblind.glsl"
}
