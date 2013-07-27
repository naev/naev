/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef CONF_H
#  define CONF_H

/**
 * CONFIGURATION DEFAULTS
 */
/* Gameplay options */
#define AFTERBURNER_SENSITIVITY_DEFAULT      250   /**< Default afterburner sensitivity. */
#define TIME_COMPRESSION_DEFAULT_MAX         5000. /**< Maximum default level of time compression (target speed to match). */
#define TIME_COMPRESSION_DEFAULT_MULT        200   /**< Default level of time compression multiplier. */
#define SAVE_COMPRESSION_DEFAULT             1     /**< Whether or not saved games should be compressed. */
#define MOUSE_THRUST_DEFAULT                 1     /**< Whether or not to use mouse thrust controls. */
#define AUTONAV_ABORT_DEFAULT                1.    /**< Shield level (0-1) to abort autonav at. 1 means at missile lock, 0 means at armour damage. */
#define AUTONAV_PAUSE_DEFAULT                0     /**< Whether or not the game should pause when autonav is aborted. */
#define MANUAL_ZOOM_DEFAULT                  0     /**< Whether or not to enable manual zoom controls. */
#define INPUT_MESSAGES_DEFAULT               5     /**< Amount of messages to display. */
/* Video options */
#define RESOLUTION_W_DEFAULT                 1024  /**< Default screen width. */
#define RESOLUTION_H_DEFAULT                 768   /**< Default screen height. */
#define FULLSCREEN_DEFAULT                   0     /**< Whether to run in fullscreen mode. */
#define FSAA_DEFAULT                         1     /**< Whether to use Full Screen Anti-Aliasing. */
#define VSYNC_DEFAULT                        0     /**< Whether to wait for vertical sync. */
#define VBO_DEFAULT                          0     /**< Whether to use Vertex Buffer Objects. */
#define MIPMAP_DEFAULT                       0     /**< Whether to use Mip Mapping. */
#define TEXTURE_COMPRESSION_DEFAULT          0     /**< Whether to use texture compression. */
#define INTERPOLATION_DEFAULT                1     /**< Whether to use interpolation. */
#define NPOT_TEXTURES_DEFAULT                0     /**< Whether to allow non-power-of-two textures. */
#define SCALE_FACTOR_DEFAULT                 1.    /**< Default scale factor. */
#define SHOW_FPS_DEFAULT                     0     /**< Whether to display FPS on screen. */
#define FPS_MAX_DEFAULT                      60    /**< Maximum FPS. */
#define ENGINE_GLOWS_DEFAULT                 1     /**< Whether to display engine glows. */
/* Audio options */
#define VOICES_DEFAULT                       128   /**< Amount of voices to use. */
#define PILOT_RELATIVE_DEFAULT               1     /**< Whether the sound is relative to the pilot (as opposed to the camera). */
#define USE_EFX_DEFAULT                      1     /**< Whether or not to use EFX (if using OpenAL). */
#define BUFFER_SIZE_DEFAULT                  128   /**< Default buffer size (if using OpenAL). */
#define MUTE_SOUND_DEFAULT                   0     /**< Whether sound should be disabled. */
#define SOUND_VOLUME_DEFAULT                 0.4   /**< Default sound volume. */
#define MUSIC_VOLUME_DEFAULT                 0.8   /**< Default music volume. */
#if USE_OPENAL
#define BACKEND_DEFAULT                      "openal"
#else /* USE_OPENAL */
#define BACKEND_DEFAULT                      "sdlmix"
#endif /* USE_OPENAL */


/**
 * @brief Struct containing player options.
 *
 * @note Input is not handled here.
 */
typedef struct PlayerConf_s {

   /* ndata. */
   char *ndata; /**< Ndata path to use. */
   char *datapath; /**< Path for user data (saves, screenshots, etc.). */

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
   int snd_voices; /**< Number of sound voices to use. */
   int snd_pilotrel; /**< Sound is relative to pilot when following. */
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
   int zoom_manual; /**< Zoom is under manual control. */
   double zoom_far; /**< Maximum ingame zoom to use should be less then zoom_near. */
   double zoom_near; /**< Minimum ingame zoom to use. */
   double zoom_speed; /**< Maximum zoom speed change. */
   double zoom_stars; /**< How much stars can zoom (modulates zoom_[mix|max]). */

   /* Font sizes. */
   int font_size_console; /**< Console monospaced font size. */
   int font_size_intro;   /**< Intro text font size. */
   int font_size_def;     /**< Default large font size. */
   int font_size_small;   /**< Default small font size. */

   /* Misc. */
   double compression_velocity; /**< Velocity to compress to. */
   double compression_mult; /**< Maximum time multiplier. */
   int save_compress; /**< Compress savegame. */
   unsigned int afterburn_sens; /**< Afterburn sensibility. */
   int mouse_thrust; /**< Whether mouse flying controls thrust. */
   double autonav_abort; /**< Condition for aborting autonav. */
   int autonav_pause;/**< Pauses game instead of aborting autonav. */
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
void conf_loadConfigPath( void );
int conf_loadConfig( const char* file );
void conf_parseCLIPath( int argc, char** argv );
void conf_parseCLI( int argc, char** argv );
void conf_cleanup (void);

/*
 * saving
 */
int conf_saveConfig( const char* file );


#endif /* CONF_H */
