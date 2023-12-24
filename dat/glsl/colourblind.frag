/*
 * Modified by Bytez under GPLv3
 * Original: https://godotshaders.com/shader/colorblindness-correction-shader/
 *
 * Colorblindness correction shader with adjustable intensity. Can correct for:
 * 1. Protanopia (Greatly reduced reds)
 * 2. Deuteranopia (Greatly reduced greens)
 * 3. Tritanopia (Greatly reduced blues)
 *
 *   The correction algorithm is taken from http://www.daltonize.org/search/label/Daltonize
 */

uniform sampler2D MainTex;
in vec4 VaryingTexCoord;
out vec4 colour_out;

/* Color correction types */
#define PROTANOPIA         0
#define DEUTERANOPIA       1
#define TRITANOPIA         2
uniform int type;
uniform float intensity;

void main (void)
{
   float l, m, s;
   float L, M, S;
   vec4 tex = texture( MainTex, VaryingTexCoord.st );

   L = (17.8824 * tex.r) + (43.5161 * tex.g) + (4.11935 * tex.b);
   M = (3.45565 * tex.r) + (27.1554 * tex.g) + (3.86714 * tex.b);
   S = (0.0299566 * tex.r) + (0.184309 * tex.g) + (1.46709 * tex.b);

   if (type == PROTANOPIA) {
      l = 0.0 * L + 2.02344 * M + -2.52581 * S;
      m = 0.0 * L + 1.0 * M + 0.0 * S;
      s = 0.0 * L + 0.0 * M + 1.0 * S;
   }
   else if (type == DEUTERANOPIA) {
      l = 1.0 * L + 0.0 * M + 0.0 * S;
      m = 0.494207 * L + 0.0 * M + 1.24827 * S;
      s = 0.0 * L + 0.0 * M + 1.0 * S;
   }
   else if (type == TRITANOPIA) {
      l = 1.0 * L + 0.0 * M + 0.0 * S;
      m = 0.0 * L + 1.0 * M + 0.0 * S;
      s = -0.395913 * L + 0.801109 * M + 0.0 * S;
   }
   else {
      /* Unhandled cases here. */
      colour_out = tex;
      return;
   }

   vec4 error;
   error.r = (0.0809444479 * l) + (-0.130504409 * m) + (0.116721066 * s);
   error.g = (-0.0102485335 * l) + (0.0540193266 * m) + (-0.113614708 * s);
   error.b = (-0.000365296938 * l) + (-0.00412161469 * m) + (0.693511405 * s);
   error.a = 1.0;
   vec4 diff = tex - error;
   vec4 correction;
   correction.r = 0.0;
   correction.g = (diff.r * 0.7) + (diff.g * 1.0);
   correction.b = (diff.r * 0.7) + (diff.b * 1.0);
   correction = tex + correction;
   //correction.a = tex.a * intensity;

   /* Alpha compositing. */
   colour_out.rgb = intensity * correction.rgb + (1.0-intensity) * tex.rgb;
   colour_out.a = tex.a;
}
