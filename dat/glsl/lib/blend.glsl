#ifndef _BLEND_GLSL
#define _BLEND_GLSL

/*
The MIT License (MIT) Copyright (c) 2015 Jamie Owen

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
// https://github.com/jamieowen/glsl-blend

/* Overlay. */
float blendOverlay(float base, float blend) {
   return base<0.5?(2.0*base*blend):(1.0-2.0*(1.0-base)*(1.0-blend));
}
vec3 blendOverlay(vec3 base, vec3 blend) {
   return vec3(blendOverlay(base.r,blend.r),blendOverlay(base.g,blend.g),blendOverlay(base.b,blend.b));
}
vec3 blendOverlay(vec3 base, vec3 blend, float opacity) {
   return (blendOverlay(base, blend) * opacity + base * (1.0 - opacity));
}

/* Color Burn. */
float blendColorBurn(float base, float blend) {
   return (blend==0.0)?blend:max((1.0-((1.0-base)/blend)),0.0);
}
vec3 blendColorBurn(vec3 base, vec3 blend) {
   return vec3(blendColorBurn(base.r,blend.r),blendColorBurn(base.g,blend.g),blendColorBurn(base.b,blend.b));
}
vec3 blendColorBurn(vec3 base, vec3 blend, float opacity) {
   return (blendColorBurn(base, blend) * opacity + base * (1.0 - opacity));
}

/* Color Dodge. */
float blendColorDodge(float base, float blend) {
   return (blend==1.0)?blend:min(base/(1.0-blend),1.0);
}
vec3 blendColorDodge(vec3 base, vec3 blend) {
   return vec3(blendColorDodge(base.r,blend.r),blendColorDodge(base.g,blend.g),blendColorDodge(base.b,blend.b));
}
vec3 blendColorDodge(vec3 base, vec3 blend, float opacity) {
   return (blendColorDodge(base, blend) * opacity + base * (1.0 - opacity));
}

/* Soft Light. */
float blendSoftLight(float base, float blend) {
   return (blend<0.5)?(2.0*base*blend+base*base*(1.0-2.0*blend)):(sqrt(base)*(2.0*blend-1.0)+2.0*base*(1.0-blend));
}
vec3 blendSoftLight(vec3 base, vec3 blend) {
   return vec3(blendSoftLight(base.r,blend.r),blendSoftLight(base.g,blend.g),blendSoftLight(base.b,blend.b));
}
vec3 blendSoftLight(vec3 base, vec3 blend, float opacity) {
   return (blendSoftLight(base, blend) * opacity + base * (1.0 - opacity));
}

/* Hard Light. */
vec3 blendHardLight(vec3 base, vec3 blend) {
   return blendOverlay(blend,base);
}
vec3 blendHardLight(vec3 base, vec3 blend, float opacity) {
   return (blendHardLight(base, blend) * opacity + base * (1.0 - opacity));
}

/* Vivid Light. */
float blendVividLight(float base, float blend) {
   return (blend<0.5)?blendColorBurn(base,(2.0*blend)):blendColorDodge(base,(2.0*(blend-0.5)));
}
vec3 blendVividLight(vec3 base, vec3 blend) {
   return vec3(blendVividLight(base.r,blend.r),blendVividLight(base.g,blend.g),blendVividLight(base.b,blend.b));
}
vec3 blendVividLight(vec3 base, vec3 blend, float opacity) {
   return (blendVividLight(base, blend) * opacity + base * (1.0 - opacity));
}


/* Screen. */
float blendScreen(float base, float blend) {
   return 1.0-((1.0-base)*(1.0-blend));
}
vec3 blendScreen(vec3 base, vec3 blend) {
   return vec3(blendScreen(base.r,blend.r),blendScreen(base.g,blend.g),blendScreen(base.b,blend.b));
}
vec3 blendScreen(vec3 base, vec3 blend, float opacity) {
   return (blendScreen(base, blend) * opacity + base * (1.0 - opacity));
}

/* Reflect. */
float blendReflect(float base, float blend) {
   return (blend==1.0)?blend:min(base*base/(1.0-blend),1.0);
}
vec3 blendReflect(vec3 base, vec3 blend) {
   return vec3(blendReflect(base.r,blend.r),blendReflect(base.g,blend.g),blendReflect(base.b,blend.b));
}
vec3 blendReflect(vec3 base, vec3 blend, float opacity) {
   return (blendReflect(base, blend) * opacity + base * (1.0 - opacity));
}

/* Glow. */
vec3 blendGlow(vec3 base, vec3 blend) {
   return blendReflect(blend,base);
}
vec3 blendGlow(vec3 base, vec3 blend, float opacity) {
   return (blendGlow(base, blend) * opacity + base * (1.0 - opacity));
}

#endif /* _BLEND_GLSL */
