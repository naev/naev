#ifndef _BLUR_GLSL
#define _BLUR_GLSL

#include "lib/math.glsl"

/*
   Blur originally taken from https://github.com/Jam3/glsl-fast-gaussian-blur
   Heavily modified to suit Naev.
*/
/*
The MIT License (MIT) Copyright (c) 2015 Jam3

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
vec4 blur5(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
   vec4 color = texture(image, uv) * 0.29411764705882354;
   vec2 off1 = vec2(1.3333333333333333) * direction;
   color += texture(image, uv + (off1 / resolution)) * 0.35294117647058826;
   color += texture(image, uv - (off1 / resolution)) * 0.35294117647058826;
   return color;
}
vec4 blur5( sampler2D image, vec2 uv, vec2 resolution, float strength ) {
   vec4 color = texture(image, uv) * 0.2941176470588234;
   vec2 off1 = vec2(1.3333333333333333,0.0) * strength;
   /* Horizontal. */
   vec2 off1xy = off1.xy / resolution;
   color += texture(image, uv + off1xy) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1xy) * 0.35294117647058826 * 0.25;
   /* Vertical. */
   vec2 off1yx = off1.yx / resolution;
   color += texture(image, uv + off1yx) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1yx) * 0.35294117647058826 * 0.25;
   /* BL-TR Diagonal. */
   vec2 off1d1 = off1.xx * M_SQRT2 / resolution;
   color += texture(image, uv + off1d1) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1d1) * 0.35294117647058826 * 0.25;
   /* TL-BR Diagonal. */
   vec2 off1d2 = vec2(off1d1.x, -off1d1.y);
   color += texture(image, uv + off1d2) * 0.35294117647058826 * 0.25;
   color += texture(image, uv - off1d2) * 0.35294117647058826 * 0.25;
   return color;
}
vec4 blur9( sampler2D image, vec2 uv, vec2 resolution, vec2 direction ) {
   vec4 color = texture(image, uv) * 0.2270270270;
   vec2 off1 = vec2(1.3846153846) * direction;
   vec2 off2 = vec2(3.2307692308) * direction;
   color += texture(image, uv + (off1 / resolution)) * 0.3162162162;
   color += texture(image, uv - (off1 / resolution)) * 0.3162162162;
   color += texture(image, uv + (off2 / resolution)) * 0.0702702703;
   color += texture(image, uv - (off2 / resolution)) * 0.0702702703;
   return color;
}
vec4 blur9( sampler2D image, vec2 uv, vec2 resolution, float strength ) {
   vec4 color = texture(image, uv) * 0.2270270270;
   vec2 off1 = vec2(1.3846153846,0.0) * strength;
   vec2 off2 = vec2(3.2307692308,0.0) * strength;
   /* Horizontal. */
   vec2 off1xy = off1.xy / resolution;
   vec2 off2xy = off2.xy / resolution;
   color += texture(image, uv + off1xy) * 0.3162162162 * 0.25;
   color += texture(image, uv - off1xy) * 0.3162162162 * 0.25;
   color += texture(image, uv + off2xy) * 0.0702702703 * 0.25;
   color += texture(image, uv - off2xy) * 0.0702702703 * 0.25;
   /* Vertical. */
   vec2 off1yx = off1.yx / resolution;
   vec2 off2yx = off2.yx / resolution;
   color += texture(image, uv + off1yx) * 0.3162162162 * 0.25;
   color += texture(image, uv - off1yx) * 0.3162162162 * 0.25;
   color += texture(image, uv + off2yx) * 0.0702702703 * 0.25;
   color += texture(image, uv - off2yx) * 0.0702702703 * 0.25;
   /* BL-TR Diagonal. */
   vec2 off1d1 = off1.xx * M_SQRT2 / resolution;
   vec2 off2d1 = off2.xx * M_SQRT2 / resolution;
   color += texture(image, uv + off1d1) * 0.3162162162 * 0.25;
   color += texture(image, uv - off1d1) * 0.3162162162 * 0.25;
   color += texture(image, uv + off2d1) * 0.0702702703 * 0.25;
   color += texture(image, uv - off2d1) * 0.0702702703 * 0.25;
   /* TL-BR Diagonal. */
   vec2 off1d2 = vec2(off1d1.x, -off1d1.y);
   vec2 off2d2 = vec2(off2d1.x, -off2d1.y);
   color += texture(image, uv + off1d2) * 0.3162162162 * 0.25;
   color += texture(image, uv - off1d2) * 0.3162162162 * 0.25;
   color += texture(image, uv + off2d2) * 0.0702702703 * 0.25;
   color += texture(image, uv - off2d2) * 0.0702702703 * 0.25;
   return color;
}
vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
   vec4 color = texture(image, uv) * 0.1964825501511404;
   vec2 off1 = vec2(1.411764705882353) * direction;
   vec2 off2 = vec2(3.2941176470588234) * direction;
   vec2 off3 = vec2(5.176470588235294) * direction;
   color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
   color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
   color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
   color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
   color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
   color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
   return color;
}
vec4 blur13( sampler2D image, vec2 uv, vec2 resolution, float strength ) {
   vec4 color = texture(image, uv) * 0.1964825501511404;
   vec2 off1 = vec2(1.411764705882353) * strength;
   vec2 off2 = vec2(3.2941176470588234) * strength;
   vec2 off3 = vec2(5.176470588235294) * strength;
   /* Horizontal. */
   vec2 off1xy = off1.xy / resolution;
   vec2 off2xy = off2.xy / resolution;
   vec2 off3xy = off3.xy / resolution;
   color += texture(image, uv + off1xy) * 0.2969069646728344 * 0.25;
   color += texture(image, uv - off1xy) * 0.2969069646728344 * 0.25;
   color += texture(image, uv + off2xy) * 0.09447039785044732 * 0.25;
   color += texture(image, uv - off2xy) * 0.09447039785044732 * 0.25;
   color += texture(image, uv + off3xy) * 0.010381362401148057 * 0.25;
   color += texture(image, uv - off3xy) * 0.010381362401148057 * 0.25;
   /* Vertical. */
   vec2 off1yx = off1.yx / resolution;
   vec2 off2yx = off2.yx / resolution;
   vec2 off3yx = off3.yx / resolution;
   color += texture(image, uv + off1yx) * 0.2969069646728344 * 0.25;
   color += texture(image, uv - off1yx) * 0.2969069646728344 * 0.25;
   color += texture(image, uv + off2yx) * 0.09447039785044732 * 0.25;
   color += texture(image, uv - off2yx) * 0.09447039785044732 * 0.25;
   color += texture(image, uv + off3yx) * 0.010381362401148057 * 0.25;
   color += texture(image, uv - off3yx) * 0.010381362401148057 * 0.25;
   /* BL-TR Diagonal. */
   vec2 off1d1 = off1.xx * M_SQRT2 / resolution;
   vec2 off2d1 = off2.xx * M_SQRT2 / resolution;
   vec2 off3d1 = off3.xx * M_SQRT2 / resolution;
   color += texture(image, uv + off1d1) * 0.2969069646728344 * 0.25;
   color += texture(image, uv - off1d1) * 0.2969069646728344 * 0.25;
   color += texture(image, uv + off2d1) * 0.09447039785044732 * 0.25;
   color += texture(image, uv - off2d1) * 0.09447039785044732 * 0.25;
   color += texture(image, uv + off3d1) * 0.010381362401148057 * 0.25;
   color += texture(image, uv - off3d1) * 0.010381362401148057 * 0.25;
   /* TL-BR Diagonal. */
   vec2 off1d2 = vec2(off1d1.x, -off1d1.y);
   vec2 off2d2 = vec2(off2d1.x, -off2d1.y);
   vec2 off3d2 = vec2(off3d1.x, -off3d1.y);
   color += texture(image, uv + off1d2) * 0.2969069646728344 * 0.25;
   color += texture(image, uv - off1d2) * 0.2969069646728344 * 0.25;
   color += texture(image, uv + off2d2) * 0.09447039785044732 * 0.25;
   color += texture(image, uv - off2d2) * 0.09447039785044732 * 0.25;
   color += texture(image, uv + off3d2) * 0.010381362401148057 * 0.25;
   color += texture(image, uv - off3d2) * 0.010381362401148057 * 0.25;
   return color;
}

/*
 * These two implementations below are computed per pixel and much slower.
 * Meant for offline use (e.g., to create canvases)
 */
vec4 blurlinear( sampler2D image, vec2 uv, vec2 resolution, vec2 direction, float strength ) {
   vec4 color = texture( image, uv );
   float dstep = 1.0 / dot( resolution, direction );
   float sum = 1.0;
   for (float i = 1.0; i<=strength; i++ ) {
      vec2 off = direction*i*dstep;
      color += texture( image, uv+off );
      color += texture( image, uv-off );
      sum += 2.0;
   }
   color /= sum;
   return color;
}
vec4 blurgaussian( sampler2D image, vec2 uv, vec2 resolution, vec2 direction, float sigma ) {
   const float threshold = 0.01; // Threshold to ignore pixels at
   vec4 color = texture( image, uv );
   float dstep = 1.0 / dot( resolution, direction );
   float g = 1.0; // exp( 0.0 )
   float sum = g;
   float den = 1.0 / (2.0 * pow(sigma, 2.0));
   for (float i = 1.0; (g=exp( -pow(i,2.0)*den )) > threshold; i++ ) {
      vec2 off = direction*i*dstep;
      color += texture( image, uv+off ) * g;
      color += texture( image, uv-off ) * g;
      sum += 2.0 * g;
   }
   color /= sum;
   return color;
}

#endif /* _BLUR_GLSL */
