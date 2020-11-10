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
#define REDIRECT_FILE_DEFAULT                1     /**< Whether output should be redirected to a file. */
#define SAVE_COMPRESSION_DEFAULT             1     /**< Whether or not saved games should be compressed. */
#define MOUSE_THRUST_DEFAULT                 1     /**< Whether or not to use mouse thrust controls. */
#define MOUSE_DOUBLECLICK_TIME               0.5   /**< How long to consider double-clicks for. */
#define AUTONAV_RESET_SPEED_DEFAULT          1.    /**< Shield level (0-1) to reset autonav speed at. 1 means at enemy presence, 0 means at armour damage. */
#define MANUAL_ZOOM_DEFAULT                  0     /**< Whether or not to enable manual zoom controls. */
#define INPUT_MESSAGES_DEFAULT               5     /**< Amount of messages to display. */
/* Video options */
#define RESOLUTION_W_DEFAULT                 1024  /**< Default screen width. */
#define RESOLUTION_H_DEFAULT                 768   /**< Default screen height. */
#define FULLSCREEN_DEFAULT                   0     /**< Whether to run in fullscreen mode. */
#define FULLSCREEN_MODESETTING               0     /**< Whether fullscreen uses video modesetting. */
#define FSAA_DEFAULT                         1     /**< Whether to use Full Screen Anti-Aliasing. */
#define VSYNC_DEFAULT                        0     /**< Whether to wait for vertical sync. */
#define MIPMAP_DEFAULT                       0     /**< Whether to use Mip Mapping. */
#define TEXTURE_COMPRESSION_DEFAULT          0     /**< Whether to use texture compression. */
#define INTERPOLATION_DEFAULT                1     /**< Whether to use interpolation. */
#define NPOT_TEXTURES_DEFAULT                0     /**< Whether to allow non-power-of-two textures. */
#define SCALE_FACTOR_DEFAULT                 1.    /**< Default scale factor. */
#define SHOW_FPS_DEFAULT                     0     /**< Whether to display FPS on screen. */
#define FPS_MAX_DEFAULT                      60    /**< Maximum FPS. */
#define SHOW_PAUSE_DEFAULT                   1     /**< Whether to display pause status. */
#define ENGINE_GLOWS_DEFAULT                 1     /**< Whether to display engine glows. */
#define MINIMIZE_DEFAULT                     1     /**< Whether to minimize on focus loss. */
/* Audio options */
#define VOICES_DEFAULT                       128   /**< Amount of voices to use. */
#define VOICES_MIN                           16    /**< Minimum amount of voices to use. */
#define PILOT_RELATIVE_DEFAULT               1     /**< Whether the sound is relative to the pilot (as opposed to the camera). */
#define USE_EFX_DEFAULT                      1     /**< Whether or not to use EFX (if using OpenAL). */
#define BUFFER_SIZE_DEFAULT                  128   /**< Default buffer size (if using OpenAL). */
#define MUTE_SOUND_DEFAULT                   0     /**< Whether sound should be disabled. */
#define SOUND_VOLUME_DEFAULT                 0.6   /**< Default sound volume. */
#define MUSIC_VOLUME_DEFAULT                 0.8   /**< Default music volume. */
#if USE_OPENAL
#define BACKEND_DEFAULT                      "openal"
#else /* USE_OPENAL */
#define BACKEND_DEFAULT                      "sdlmix"
#endif /* USE_OPENAL */
/* Editor Options */
#define DEV_SAVE_SYSTEM_DEFAULT           "ssys/"
#define DEV_SAVE_ASSET_DEFAULT            "assets/"
#define DEV_SAVE_MAP_DEFAULT              "outfits/maps/"


/**
 * @brief Struct containing player options.
 *
 * @note Input is not handled here.
 */
typedef struct PlayerConf_s {

   /* ndata. */
   char *ndata; /**< Ndata path to use. */
   char *datapath; /**< Path for user data (saves, screenshots, etc.). */

   /* Language. */
   char *language; /**< Language to use. */

   /* OpenGL properties. */
   int fsaa; /**< Full Scene Anti-Aliasing to use. */
   int vsync; /**< Whether or not to use vsync. */
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
   int modesetting; /**< Whether to use modesetting for fullscreen. */
   int minimize; /**< Whether to minimize on focus loss. */

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
   int fps_show; /**< Whether or not FPS should be shown */
   int fps_max; /**< Maximum FPS to limit to. */

   /* Pause. */
   int pause_show; /**< Whether pause status should be shown. */

   /* Joystick. */
   int joystick_ind; /**< Index of joystick to use. */
   char *joystick_nam; /**< Name of joystick to use. */

   /* GUI. */
   int mesg_visible; /**< Amount of visible messages. */

   /* Keyrepeat. */
   unsigned int repeat_delay; /**< Time in ms before start repeating. */
   unsigned int repeat_freq; /**< Time in ms between each repeat once started repeating. */

   /* Zoom. */
   int zoom_manual; /**< Zoom is under manual control. */
   double zoom_far; /**< Maximum in-game zoom to use should be less then zoom_near. */
   double zoom_near; /**< Minimum in-game zoom to use. */
   double zoom_speed; /**< Maximum zoom speed change. */
   double zoom_stars; /**< How much stars can zoom (modulates zoom_[mix|max]). */

   /* Font sizes. */
   int font_size_console; /**< Console monospaced font size. */
   int font_size_intro;   /**< Intro text font size. */
   int font_size_def;     /**< Default large font size. */
   int font_size_small;   /**< Default small font size. */
   char *font_name_default; /**< Default font filename. */
   char *font_name_monospace; /**< Monospace font filename. */

   /* Misc. */
   double compression_velocity; /**< Velocity to compress to. */
   double compression_mult; /**< Maximum time multiplier. */
   int redirect_file; /**< Redirect output to files. */
   int save_compress; /**< Compress savegame. */
   unsigned int afterburn_sens; /**< Afterburn sensibility. */
   int mouse_thrust; /**< Whether mouse flying controls thrust. */
   double mouse_doubleclick; /**< How long to consider double-clicks for. */
   double autonav_reset_speed; /**< Condition for resetting autonav speed. */
   int nosave; /**< Disables conf saving. */
   int devmode; /**< Developer mode. */
   int devautosave; /**< Developer mode autosave. */
   int devcsv; /**< Output CSV data. */
   char *lastversion; /**< The last version the game was ran in. */

   /* Debugging. */
   int fpu_except; /**< Enable FPU exceptions? */

   /* Editor. */
   char *dev_save_sys; /**< Path to save systems to. */
   char *dev_save_map; /**< Path to save maps to. */
   char *dev_save_asset; /**< Path to save assets to. */

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
