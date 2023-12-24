/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include <time.h>

#define CONF_FILE       "conf.lua" /**< Configuration file by default. */

/**
 * CONFIGURATION DEFAULTS
 */
/* Gameplay options */
#define DOUBLETAP_SENSITIVITY_DEFAULT  250   /**< Default afterburner sensitivity. */
#define REDIRECT_FILE_DEFAULT          1     /**< Whether output should be redirected to a file. */
#define SAVE_COMPRESSION_DEFAULT       1     /**< Whether or not saved games should be compressed. */
#define MOUSE_HIDE_DEFAULT             3.    /**< Time (in seconds) to hide mouse when not moved. */
#define MOUSE_FLY_DEFAULT              1     /**< Whether or not middle clicking enables mouse flying. */
#define MOUSE_ACCEL_DEFAULT            1     /**< Whether or not to use mouse accel controls. */
#define MOUSE_DOUBLECLICK_TIME         0.5   /**< How long to consider double-clicks for. */
#define MANUAL_ZOOM_DEFAULT            0     /**< Whether or not to enable manual zoom controls. */
#define ZOOM_FAR_DEFAULT               0.5   /**< Far zoom distance (smaller is further) */
#define ZOOM_NEAR_DEFAULT              1.0   /**< Close zoom distance (bigger is larger) */
#define ZOOM_SPEED_DEFAULT             0.25 /**< Rate of change of zoom (bigger is faster). */
#define MAP_OVERLAY_OPACITY_DEFAULT    0.3   /**< Opacity fraction (0-1) for the overlay map. */
#define INPUT_MESSAGES_DEFAULT         5     /**< Amount of messages to display. */
#define DIFFICULTY_DEFAULT             NULL  /**< Default difficulty. */
/* Video options */
#define RESOLUTION_W_MIN               1280  /**< Minimum screen width (below which graphics are downscaled). */
#define RESOLUTION_H_MIN               720   /**< Minimum screen height (below which graphics are downscaled). */
#define RESOLUTION_W_DEFAULT           RESOLUTION_W_MIN /**< Default screen width. */
#define RESOLUTION_H_DEFAULT           RESOLUTION_H_MIN /**< Default screen height. */
#define FULLSCREEN_DEFAULT             0     /**< Whether to run in fullscreen mode. */
#define FULLSCREEN_MODESETTING         0     /**< Whether fullscreen uses video modesetting. */
#define FSAA_DEFAULT                   1     /**< Whether to use Full Screen Anti-Aliasing. */
#define VSYNC_DEFAULT                  0     /**< Whether to wait for vertical sync. */
#define SCALE_FACTOR_DEFAULT           1.    /**< Default scale factor. */
#define NEBULA_SCALE_FACTOR_DEFAULT    4.    /**< Default scale factor for nebula rendering. */
#define SHOW_FPS_DEFAULT               0     /**< Whether to display FPS on screen. */
#define FPS_MAX_DEFAULT                60    /**< Maximum FPS. */
#define SHOW_PAUSE_DEFAULT             1     /**< Whether to display pause status. */
#define MINIMIZE_DEFAULT               1     /**< Whether to minimize on focus loss. */
#define COLOURBLIND_SIM_DEFAULT        0     /**< Whether to enable colourblindness simulation. */
#define COLOURBLIND_TYPE_DEFAULT       0     /**< Type of colourblindness to simulate. */
#define COLOURBLIND_CORRECT_DEFAULT    0.    /**< Intensity of the colourblindness correction. */
#define HEALTHBARS_DEFAULT             1     /**< Whether or not to show pilot health bars. */
#define BG_BRIGHTNESS_DEFAULT          0.5   /**< How much to darken (or lighten) the backgrounds. */
#define NEBU_NONUNIFORMITY_DEFAULT     1.    /**< How much to darken (or lighten) the nebula stuff. */
#define GAMMA_CORRECTION_DEFAULT       1.    /**< How much gamma correction to do. */
#define BACKGROUND_FANCY_DEFAULT       0     /**< Default fancy background. */
#define JUMP_BRIGHTNESS_DEFAULT        1.    /**< Default jump brightness.*/
#define BIG_ICONS_DEFAULT              0     /**< Whether to display BIGGER icons. */
#define FONT_SIZE_CONSOLE_DEFAULT      10    /**< Default console font size. */
#define FONT_SIZE_INTRO_DEFAULT        18    /**< Default intro font size. */
#define FONT_SIZE_DEF_DEFAULT          12    /**< Default font size. */
#define FONT_SIZE_SMALL_DEFAULT        11    /**< Default small font size. */
/* Audio options */
#define USE_EFX_DEFAULT                1     /**< Whether or not to use EFX (if using OpenAL). */
#define MUTE_SOUND_DEFAULT             0     /**< Whether sound should be disabled. */
#define SOUND_VOLUME_DEFAULT           0.6   /**< Default sound volume. */
#define MUSIC_VOLUME_DEFAULT           0.8   /**< Default music volume. */
#define ENGINE_VOLUME_DEFAULT          0.8   /**< Default engine volume. */
/* Editor Options */
#define DEV_SAVE_SYSTEM_DEFAULT        "../dat/ssys/"
#define DEV_SAVE_SPOB_DEFAULT          "../dat/spob/"
#define DEV_SAVE_MAP_DEFAULT           "../dat/outfits/maps/"

/**
 * @brief Struct containing player options.
 *
 * @note Input is not handled here.
 */
typedef struct PlayerConf_s {
   int loaded; /**< Configuration file has been loaded (not an actual option). */

   /* ndata. */
   char *ndata; /**< Ndata path to use. */
   char *datapath; /**< Path for user data (saves, screenshots, etc.). */

   /* Language. */
   char *language; /**< Language to use. */

   /* OpenGL properties. */
   int fsaa; /**< Full Scene Anti-Aliasing to use. */
   int vsync; /**< Whether or not to use vsync. */

   /* Video options. */
   int width; /**< Width of the window to use. */
   int height; /**< Height of the window to use. */
   int explicit_dim; /**< Dimension is explicit. */
   double scalefactor; /**< Amount to reduce resolution by. */
   double nebu_scale; /**< Downscaling factor for the expensively rendered nebula. */
   int fullscreen; /**< Whether or not game is fullscreen. */
   int modesetting; /**< Whether to use modesetting for fullscreen. */
   int notresizable; /**< Whether or not the window is resizable. */
   int borderless; /**< Whether to disable window decorations. */
   int minimize; /**< Whether to minimize on focus loss. */
   int colourblind_sim; /**< Whether to enable colourblindness simulation. */
   int colourblind_type; /**< Type of colourblindness. */
   double colourblind_correct; /**< Whether to enable colourblindness simulation. */
   int healthbars; /**< Whether or not to show health bars next to pilots. */
   double bg_brightness; /**< How much to darken the background stuff. */
   double nebu_nonuniformity; /**< How much to darken the nebula stuff. */
   double jump_brightness; /**< Intensity to fade to/from when jumping. */
   double gamma_correction; /**< How much gamma correction to do. */
   int background_fancy; /**< High quality moving, but slow background. */

   /* Sound. */
   int al_efx; /**< Should EFX extension be used? (only applicable for OpenAL) */
   int nosound; /**< Whether or not sound is on. */
   double sound; /**< Sound level for sound effects. */
   double music; /**< Sound level for music. */
   double engine_vol; /**< Sound level for engines (relative). */

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
   double map_overlay_opacity; /**< Map overlay opacity. */
   int big_icons; /**< Use big icons or not. */
   int always_radar; /**< Radar is always visible. */

   /* Keyrepeat. */
   unsigned int repeat_delay; /**< Time in ms before start repeating. */
   unsigned int repeat_freq; /**< Time in ms between each repeat once started repeating. */

   /* Zoom. */
   int zoom_manual; /**< Zoom is under manual control. */
   double zoom_far; /**< Maximum in-game zoom to use should be less then zoom_near. */
   double zoom_near; /**< Minimum in-game zoom to use. */
   double zoom_speed; /**< Maximum zoom speed change. */

   /* Font sizes. */
   int font_size_console; /**< Console monospaced font size. */
   int font_size_intro;   /**< Intro text font size. */
   int font_size_def;     /**< Default large font size. */
   int font_size_small;   /**< Default small font size. */

   /* Misc. */
   char *difficulty; /**< Global difficulty setting. */
   double compression_velocity; /**< Velocity to compress to. */
   double compression_mult; /**< Maximum time multiplier. */
   int redirect_file; /**< Redirect output to files. */
   int save_compress; /**< Compress saved game. */
   unsigned int doubletap_sens; /**< Double tap key sensibility (used for afterburn and cooldown). */
   double mouse_hide; /**< Time to hide mouse. */
   int mouse_fly; /**< Whether middle clicking enables mouse flying or not. */
   int mouse_accel; /**< Whether mouse flying controls acceleration. */
   double mouse_doubleclick; /**< How long to consider double-clicks for. */
   double autonav_reset_dist; /**< Enemy distance condition for resetting autonav. */
   double autonav_reset_shield; /**< Shield condition for resetting autonav speed. */
   int devmode; /**< Developer mode. */
   int devautosave; /**< Developer mode autosave. */
   int lua_enet; /**< Enable the lua-enet library. */
   int lua_repl; /**< Enable the experimental CLI based on lua-repl. */
   int nosave; /**< Disables conf saving. */
   char *lastversion; /**< The last version the game was ran in. */
   int translation_warning_seen; /**< No need to warn about incomplete game translations again. */
   time_t last_played; /**< Date the game was last played. */

   /* Debugging. */
   int fpu_except; /**< Enable FPU exceptions? */

   /* Editor. */
   char *dev_save_sys; /**< Path to save systems to. */
   char *dev_save_map; /**< Path to save maps to. */
   char *dev_save_spob; /**< Path to save spobs to. */
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
void conf_parseCLI( int argc, char** argv );
void conf_cleanup (void);

/*
 * Some handling.
 */
void conf_copy( PlayerConf_t *dest, const PlayerConf_t *src );
void conf_free( PlayerConf_t *config );

/*
 * saving
 */
int conf_saveConfig( const char* file );
