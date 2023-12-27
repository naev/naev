uniform sampler2D MainTex;
in vec4 VaryingTexCoord;
out vec4 colour_out;

#define PROTANOPIA         0
#define DEUTERANOPIA       1
#define TRITANOPIA         2
#define ROD_MONOCHROMACY   3  /* Old default. */
#define CONE_MONOCHROMACY  4
uniform int type;
uniform float intensity;

void main (void)
{
   float l, m, s;
   float L, M, S;
   vec4 tex = texture( MainTex, VaryingTexCoord.st );

   // Convert to LMS
   L = (0.31399022f * tex.r) + (0.63951294f * tex.g) + (0.04649755f * tex.b);
   M = (0.15537241f * tex.r) + (0.75789446f * tex.g) + (0.08670142f * tex.b);
   S = (0.01775239f * tex.r) + (0.10944209f * tex.g) + (0.87256922f * tex.b);

   // Simulate colour blindness
   if (type==PROTANOPIA) {
      // Protanope - reds are greatly reduced (1% men)
      l = 0.0f * L + 1.05118294f * M + -0.05116099 * S;
      m = 0.0f * L + 1.0f * M + 0.0f * S;
      s = 0.0f * L + 0.0f * M + 1.0f * S;
   }
   else if (type==DEUTERANOPIA) {
      // Deuteranope - greens are greatly reduced (1% men)
      l = 1.0f * L + 0.0f * M + 0.0f * S;
      m = 0.9513092 * L + 0.0f * M + 0.04866992 * S;
      s = 0.0f * L + 0.0f * M + 1.0f * S;
   }
   else if (type==TRITANOPIA) {
      // Tritanope - blues are greatly reduced (0.003% population)
      l = 1.0f * L + 0.0f * M + 0.0f * S;
      m = 0.0f * L + 1.0f * M + 0.0f * S;
      s = -0.86744736 * L + 1.86727089f * M + 0.0f * S;
   }
   else if (type==CONE_MONOCHROMACY) {
      // Blue Cone Monochromat (high light conditions) - only brightness can
      // be detected, with blues greatly increased and reds nearly invisible
      // (0.001% population)
      // Note: This looks different from what many colourblindness simulators
      // show because this simulation assumes high light conditions. In low
      // light conditions, a blue cone monochromat can see a limited range of
      // colour because both rods and cones are active. However, as we expect
      // a player to be looking at a lit screen, this simulation of high
      // light conditions is more useful.
      l = m = s = 0.01775f * L + 0.10945f * M + 0.87262f * S;
   }
   else if (type==ROD_MONOCHROMACY) {
      // Rod Monochromat (Achromatopsia) - only brightness can be detected
      // (0.003% population)
      l = m = s = 0.212656f * L + 0.715158f * M + 0.072186f * S;
   }

   // Convert to RGB
   colour_out.r = (5.47221206f * l) + (-4.6419601f * m) + (0.16963708f * s);
   colour_out.g = (-1.1252419f * l) + (2.29317094f * m) + (-0.1678952f * s);
   colour_out.b = (0.02980165f * l) + (-0.19318073f * m) + (1.16364789f * s);
   colour_out.a = tex.a;
   colour_out.rgb = colour_out.rgb*intensity + (1.0-intensity)*tex.rgb;
}
