/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef CONF_H
#  define CONF_H


/**
 * @brief Struct containing player options.
 *
 * @note Input is not handled here.
 */
typedef struct PlayerConf_s {
   
   /* ndata. */
   char *ndata; /**< Ndata path to use. */

   /* OpenGL properties. */
   int fsaa; /**< Full Scene Anti-Aliasing to use. */
   int vsync; /**< Whether or not to use vsync. */
   int vbo; /**< Use vbo. */
   int mipmaps; /**< Use mipmaps. */
   int compress; /**< Use texture compression. */
   int interpolate; /**< Use texture interpolation. */
   int npot; /**< Use NPOT textures if available. */

   /* Memory usage. */
   int engineglow; /**< Sets engine glow. */

   /* Window dimensions. */
   int width; /**< Width of the window to use. */
   int height; /**< Height of the window to use. */
   int explicit_dim; /**< Dimension is explicit. */
   double scalefactor; /**< Amount to reduce resolution by. */
   int fullscreen; /**< Whether or not game is fullscreen. */

   /* Sound. */
   char *sound_backend; /**< Sound backend to use. */
   int al_efx; /**< Should EFX extension be used? (only applicable for OpenAL) */
   int al_bufsize; /**< Size of the buffer (in kilobytes) to use for music. */
   int nosound; /**< Whether or not sound is on. */
   double sound; /**< Sound level for sound effects. */
   double music; /**< Sound level for music. */

   /* FPS. */
   int fps_show; /**< Whether or not should show FPS. */
   int fps_max; /**< Maximum FPS to limit to. */

   /* Joystick. */
   int joystick_ind; /**< Index of joystick to use. */
   char *joystick_nam; /**< Name of joystick to use. */

   /* Land. */
   int autorefuel; /**< Whether or not to autorefuel when landing. */

   /* GUI. */
   int mesg_visible; /**< Amount of visible messages. */

   /* Keyrepeat. */
   unsigned int repeat_delay; /**< Time in ms before start repeating. */
   unsigned int repeat_freq; /**< Time in ms between each repeat once started repeating. */

   /* Zoom. */
   double zoom_far; /**< Maximum ingame zoom to use should be less then zoom_near. */
   double zoom_near; /**< Minimum ingame zoom to use. */
   double zoom_speed; /**< Maximum zoom speed change. */
   double zoom_stars; /**< How much stars can zoom (modulates zoom_[mix|max]). */

   /* Misc. */
   int save_compress; /**< Compress savegame. */
   unsigned int afterburn_sens; /**< Afterburn sensibility. */
   int nosave; /**< Disables conf saving. */
   int devmode; /**< Developer mode. */
   int devcsv; /**< Output CSV data. */

   /* Debugging. */
   int fpu_except; /**< Enable FPU exceptions? */

} PlayerConf_t;
extern PlayerConf_t conf; /**< Player configuration. */


/*
 * loading
 */
void conf_setDefaults (void);
void conf_setGameplayDefaults (void);
void conf_setAudioDefaults (void);
void conf_setVideoDefaults (void);
int conf_loadConfig( const char* file );
void conf_parseCLI( int argc, char** argv );
void conf_cleanup (void);

/*
 * saving
 */
int conf_saveConfig( const char* file );


#endif /* CONF_H */
