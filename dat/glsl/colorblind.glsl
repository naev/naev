//#define COLORBLIND_MODE 5

#ifdef COLORBLIND_MODE
// Convert to LMS
float L = (0.31399022f * color_out.r) + (0.63951294f * color_out.g) + (0.04649755f * color_out.b);
float M = (0.15537241f * color_out.r) + (0.75789446f * color_out.g) + (0.08670142f * color_out.b);
float S = (0.01775239f * color_out.r) + (0.10944209f * color_out.g) + (0.87256922f * color_out.b);

// Simulate color blindness
#if COLORBLIND_MODE == 1 // Protanope - reds are greatly reduced (1% men)
float l = 0.0f * L + 1.05118294f * M + -0.05116099 * S;
float m = 0.0f * L + 1.0f * M + 0.0f * S;
float s = 0.0f * L + 0.0f * M + 1.0f * S;
#elif COLORBLIND_MODE == 2 // Deuteranope - greens are greatly reduced (1% men)
float l = 1.0f * L + 0.0f * M + 0.0f * S;
float m = 0.9513092 * L + 0.0f * M + 0.04866992 * S;
float s = 0.0f * L + 0.0f * M + 1.0f * S;
#elif COLORBLIND_MODE == 3 // Tritanope - blues are greatly reduced (0.003% population)
float l = 1.0f * L + 0.0f * M + 0.0f * S;
float m = 0.0f * L + 1.0f * M + 0.0f * S;
float s = -0.86744736 * L + 1.86727089f * M + 0.0f * S;
#elif COLORBLIND_MODE == 4 // Blue Cone Monochromat (high light conditions)
float l = 0.01775f * L + 0.10945f * M + 0.87262f * S;
float m = 0.01775f * L + 0.10945f * M + 0.87262f * S;
float s = 0.01775f * L + 0.10945f * M + 0.87262f * S;
#elif COLORBLIND_MODE == 5 // Rod Monochromat (Achromatopsia)
float l = 0.212656f * L + 0.715158f * M + 0.072186f * S;
float m = 0.212656f * L + 0.715158f * M + 0.072186f * S;
float s = 0.212656f * L + 0.715158f * M + 0.072186f * S;
#endif /* COLORBLIND_MODE */

// Convert to RGB
color_out.r = (5.47221206f * l) + (-4.6419601f * m) + (0.16963708f * s);
color_out.g = (-1.1252419f * l) + (2.29317094f * m) + (-0.1678952f * s);
color_out.b = (0.02980165f * l) + (-0.19318073f * m) + (1.16364789f * s);
#endif /* MONOCHROME */
