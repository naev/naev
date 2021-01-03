//#define COLORBLIND_MODE 1

#ifdef COLORBLIND_MODE
// RGB to LMS matrix conversion
float L = (17.8824f * color_out.r) + (43.5161f * color_out.g) + (4.11935f * color_out.b);
float M = (3.45565f * color_out.r) + (27.1554f * color_out.g) + (3.86714f * color_out.b);
float S = (0.0299566f * color_out.r) + (0.184309f * color_out.g) + (1.46709f * color_out.b);
   
// Simulate color blindness
#if COLORBLIND_MODE == 1 // Protanope - reds are greatly reduced (1% men)
float l = 0.0f * L + 2.02344f * M + -2.52581f * S;
float m = 0.0f * L + 1.0f * M + 0.0f * S;
float s = 0.0f * L + 0.0f * M + 1.0f * S;
#elif COLORBLIND_MODE == 2 // Deuteranope - greens are greatly reduced (1% men)
float l = 1.0f * L + 0.0f * M + 0.0f * S;
float m = 0.494207f * L + 0.0f * M + 1.24827f * S;
float s = 0.0f * L + 0.0f * M + 1.0f * S;
#elif COLORBLIND_MODE == 3 // Tritanope - blues are greatly reduced (0.003% population)
float l = 1.0f * L + 0.0f * M + 0.0f * S;
float m = 0.0f * L + 1.0f * M + 0.0f * S;
float s = -0.395913f * L + 0.801109f * M + 0.0f * S;
#endif /* COLORBLIND_MODE */
   
// LMS to RGB matrix conversion
color_out.r = (0.0809444479f * l) + (-0.130504409f * m) + (0.116721066f * s);
color_out.g = (-0.0102485335f * l) + (0.0540193266f * m) + (-0.113614708f * s);
color_out.b = (-0.000365296938f * l) + (-0.00412161469f * m) + (0.693511405f * s);
#endif /* COLORBLIND_MODE */
