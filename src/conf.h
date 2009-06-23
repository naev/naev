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

   /* Window dimensions. */
   int width; /**< Width of the window to use. */
   int height; /**< Height of the window to use. */
   int explicit_dim; /**< Dimension is explicit. */
   double scalefactor; /**< Amount to reduce resolution by. */
   int fullscreen; /**< Whether or not game is fullscreen. */

   /* Sound. */
   char *sound_backend; /**< Sound backend to use. */
   int efx; /**< Should EFX extension be used? (only applicable for OpenAL) */
   int nosound; /**< Whether or not sound is on. */
   double sound; /**< Sound level for sound effects. */
   double music; /**< Sound level for music. */

   /* FPS. */
   int fps_show; /**< Whether or not should show FPS. */
   int fps_max; /**< Maximum FPS to limit to. */

   /* Joystick. */
   int joystick_ind; /**< Index of joystick to use. */
   char *joystick_nam; /**< Name of joystick to use. */

   /* Misc. */
   double zoom_max; /**< Maximum ingame zoom to use. */
   double zoom_min; /**< Minimum ingame zoom to use. */
   double zoom_speed; /**< Maximum zoom speed change. */
   unsigned int afterburn_sens; /**< Afterburn sensibility. */

} PlayerConf_t;
extern PlayerConf_t conf; /**< Player configuration. */


/*
 * loading
 */
void conf_setDefaults (void);
int conf_loadConfig( const char* file );
void conf_parseCLI( int argc, char** argv );
void conf_cleanup (void);

/*
 * saving
 */
int conf_saveConfig ( const char* file );


#endif /* CONF_H */
