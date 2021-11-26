#ifndef _GAMMA_GLSL
#define _GAMMA_GLSL

/*
 * Gamma-correction tools. We try to follow love2D API here
 * https://love2d.org/wiki/love.graphics.isGammaCorrect
 */

/* Fast approximations taken from
 * http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
 */
float gammaToLinearFast( float c ) { return c * (c * (c * 0.305306011 + 0.682171111) + 0.012522878); }
vec3 gammaToLinearFast( vec3 c )   { return c * (c * (c * 0.305306011 + 0.682171111) + 0.012522878); }
vec4 gammaToLinearFast( vec4 c )   { return vec4(gammaToLinearFast(c.rgb), c.a); }
float linearToGammaFast( float c ) { return max(1.055 * pow(max(c, 0.0), 0.41666666) - 0.055, 0.0); }
vec3 linearToGammaFast( vec3 c )   { return max(1.055 * pow(max(c, vec3(0.0)), vec3(0.41666666)) - 0.055, vec3(0.0)); }
vec4 linearToGammaFast( vec4 c )   { return vec4(linearToGammaFast(c.rgb), c.a); }

/*
 * Gamma-corrected sRGB to linear sRGB.
 */
float gammaToLinearPrecise( float c ) {
   return c <= 0.04045 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}
vec3 gammaToLinearPrecise( vec3 c ) {
   return mix( pow((c.rgb+0.055)/1.055,vec3(2.4)),
               c.rgb/12.92,
               step(c.rgb, vec3(0.04045)));
}
vec4 gammaToLinearPrecise( vec4 c ) {
   return vec4( gammaToLinearPrecise(c.rgb), c.a );
}

/*
 * Linear sRGB to gamma-corrected sRGB
 */
float linearToGammaPrecise( float c ) {
   return c < 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1.0 / 2.4) - 0.055;
}
vec3 linearToGammaPrecise( vec3 c ) {
   return mix( 1.055*pow(c.rgb,vec3(1.0/2.4))-0.055,
               c.rgb*12.92,
               step(c.rgb, vec3(0.0031308)));
}
vec4 linearToGammaPrecise( vec4 c ) {
   return vec4( linearToGammaPrecise(c.rgb), c.a );
}

#define gammaToLinear               gammaToLinearFast
#define linearToGamma               linearToGammaFast

#define gammaCorrectColor           gammaToLinear
#define unGammaCorrectColor         linearToGamma
#define gammaCorrectColorPrecise    gammaToLinearPrecise
#define unGammaCorrectColorPrecise  linearToGammaPrecise
#define gammaCorrectColorFast       gammaToLinearFast
#define unGammaCorrectColorFast     linearToGammaFast

#endif /* _GAMMA_GLSL */
