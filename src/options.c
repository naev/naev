/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file options.c
 *
 * @brief Options menu
 */
/** @cond */
#include "physfs.h"
#include <ctype.h>
#include <libgen.h>

#include "naev.h"
/** @endcond */

#include "options.h"

#include "array.h"
#include "background.h"
#include "colour.h"
#include "conf.h"
#include "dialogue.h"
#include "difficulty.h"
#include "input.h"
#include "log.h"
#include "music.h"
#include "ndata.h"
#include "nebula.h"
#include "nfile.h"
#include "nstring.h"
#include "pause.h"
#include "player.h"
#include "plugin.h"
#include "render.h"
#include "sound.h"
#include "toolkit.h"

#define BUTTON_WIDTH 120 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT 30 /**< Button height, standard across menus. */

#define OPT_WIN_GAMEPLAY 0
#define OPT_WIN_ACCESSIBILITY 1
#define OPT_WIN_VIDEO 2
#define OPT_WIN_AUDIO 3
#define OPT_WIN_INPUT 4
#define OPT_WIN_PLUGINS 5
#define OPT_WINDOWS 6

#define AUTONAV_RESET_DIST_MAX 10e3
#define LANG_CODE_START                                                        \
   7 /**< Length of a language-list item's prefix, like the "[ 81%] " in "[    \
        81%] de". */

static unsigned int  opt_wid = 0;
static unsigned int *opt_windows;
static const char   *opt_names[] = {
   N_( "Gameplay" ), N_( "Accessibility" ), N_( "Video" ),
   N_( "Audio" ),    N_( "Input" ),         N_( "Plugins" ),
};
static_assert( ( sizeof( opt_names ) / sizeof( opt_names[0] ) ) == OPT_WINDOWS,
               "Options windows mismatch!" );
static const glColour *cHeader = &cFontGrey;

static int          opt_restart = 0;
static PlayerConf_t local_conf;

/*
 * External stuff.
 */
static KeySemanticType opt_selectedKeybind;  /**< Selected keybinding. */
static int             opt_lastKeyPress = 0; /**< Last keypress. */

/*
 * prototypes
 */
/* Misc. */
static void opt_close( unsigned int wid, const char *name );
static void opt_needRestart( void );
/* Gameplay. */
static char **lang_list( int *n );
static void   opt_gameplay( unsigned int wid );
static void   opt_setMapOverlayOpacity( unsigned int wid, const char *str );
static void   opt_OK( unsigned int wid, const char *str );
static int    opt_gameplaySave( unsigned int wid, const char *str );
static void   opt_gameplayDefaults( unsigned int wid, const char *str );
static void   opt_gameplayUpdate( unsigned int wid, const char *str );
/* Accessibility. */
static void opt_accessibility( unsigned int wid );
static int  opt_accessibilitySave( unsigned int wid, const char *str );
static void opt_accessibilityDefaults( unsigned int wid, const char *str );
static void opt_setBGBrightness( unsigned int wid, const char *str );
static void opt_setNebuNonuniformity( unsigned int wid, const char *str );
static void opt_setSaturation( unsigned int wid, const char *str );
static void opt_setJumpBrightness( unsigned int wid, const char *str );
static void opt_setColourblindCorrect( unsigned int wid, const char *str );
static void opt_setColourblindSimulate( unsigned int wid, const char *str );
static void opt_listColourblind( unsigned int wid, const char *str );
static void opt_setGameSpeed( unsigned int wid, const char *str );
/* Video. */
static void opt_video( unsigned int wid );
static int  opt_videoSave( unsigned int wid, const char *str );
static void opt_videoDefaults( unsigned int wid, const char *str );
static void opt_toggleFullscreen( unsigned int wid, const char *str );
static void opt_setGammaCorrection( unsigned int wid, const char *str );
static void opt_setScalefactor( unsigned int wid, const char *str );
static void opt_setZoomFar( unsigned int wid, const char *str );
static void opt_setZoomNear( unsigned int wid, const char *str );
static void opt_checkHealth( unsigned int wid, const char *str );
static void opt_checkViewport( unsigned int wid, const char *str );
static void opt_checkRestart( unsigned int wid, const char *str );
/* Audio. */
static void opt_audio( unsigned int wid );
static int  opt_audioSave( unsigned int wid, const char *str );
static void opt_audioDefaults( unsigned int wid, const char *str );
static void opt_audioUpdate( unsigned int wid );
static void opt_audioLevelStr( char *buf, int max, int type, double pos );
static void opt_setAudioLevel( unsigned int wid, const char *str );
static void opt_setEngineLevel( unsigned int wid, const char *str );
static void opt_beep( unsigned int wid, const char *str );
/* Keybind menu. */
static void opt_keybinds( unsigned int wid );
static void menuKeybinds_getDim( unsigned int wid, int *w, int *h, int *lw,
                                 int *lh, int *bw, int *bh );
static void menuKeybinds_genList( unsigned int wid );
static void menuKeybinds_update( unsigned int wid, const char *name );
static void opt_keyDefaults( unsigned int wid, const char *str );
/* Setting keybindings. */
static int  opt_setKeyEvent( unsigned int wid, SDL_Event *event );
static void opt_setKey( unsigned int wid, const char *str );
static void opt_unsetKey( unsigned int wid, const char *str );
/* Plugins menu. */
static void opt_plugins( unsigned int wid );
static void opt_plugins_regenList( unsigned int wid );
static void opt_plugins_add( unsigned int wid, const char *name );
static void opt_plugins_update( unsigned int wid, const char *name );

/**
 * @brief Creates the options menu thingy.
 */
void opt_menu( void )
{
   int          w, h;
   const char **names;

   /* Save current configuration over. */
   conf_copy( &local_conf, &conf );

   /* Dimensions. */
   w = 680;
   h = 525;

   /* Create window and tabs. */
   opt_wid = window_create( "wdwOptions", _( "Options" ), -1, -1, w, h );
   window_setCancel( opt_wid, opt_close );

   /* Create tabbed window. */
   names = calloc( 1, sizeof( opt_names ) );
   for ( size_t i = 0; i < sizeof( opt_names ) / sizeof( opt_names[0] ); i++ )
      names[i] = _( opt_names[i] );
   opt_windows = window_addTabbedWindow( opt_wid, -1, -1, -1, -1, "tabOpt",
                                         OPT_WINDOWS, (const char **)names, 0 );
   free( names );

   /* Common stuff. */
   for ( int i = 0; i < OPT_WINDOWS; i++ ) {
      unsigned int wid = opt_windows[i];
      window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "btnClose",
                        _( "OK" ), opt_OK );
      window_addText( opt_windows[i], 20, 10,
                      w - 20 - 3 * ( BUTTON_WIDTH + 20 ), BUTTON_HEIGHT + 10, 0,
                      "txtRestart", NULL, NULL, NULL );
   }

   /* Load tabs. */
   opt_gameplay( opt_windows[OPT_WIN_GAMEPLAY] );
   opt_accessibility( opt_windows[OPT_WIN_ACCESSIBILITY] );
   opt_video( opt_windows[OPT_WIN_VIDEO] );
   opt_audio( opt_windows[OPT_WIN_AUDIO] );
   opt_keybinds( opt_windows[OPT_WIN_INPUT] );
   opt_plugins( opt_windows[OPT_WIN_PLUGINS] );

   /* Set as need restart if needed. */
   if ( opt_restart )
      opt_needRestart();
}

/**
 * @brief Saves all options and closes the options screen.
 */
static void opt_OK( unsigned int wid, const char *str )
{
   int ret, prompted_restart;

   prompted_restart = opt_restart;
   ret              = 0;
   ret |= opt_gameplaySave( opt_windows[OPT_WIN_GAMEPLAY], str );
   ret |= opt_accessibilitySave( opt_windows[OPT_WIN_ACCESSIBILITY], str );
   ret |= opt_audioSave( opt_windows[OPT_WIN_AUDIO], str );
   ret |= opt_videoSave( opt_windows[OPT_WIN_VIDEO], str );

   if ( opt_restart && !prompted_restart )
      dialogue_msg( _( "Warning" ), "#r%s#0",
                    _( "Restart Naev for changes to take effect." ) );

   /* Close window if no errors occurred. */
   if ( !ret ) {
      /* Save current configuration over. */
      conf_copy( &local_conf, &conf );
      opt_close( wid, str );
   }
}

/**
 * @brief Closes the options screen without saving.
 */
static void opt_close( unsigned int wid, const char *name )
{
   (void)wid;
   (void)name;

   /* Load old config again. */
   conf_copy( &conf, &local_conf );

   /* At this point, set sound levels as defined in the config file.
    * This ensures that sound volumes are reset on "Cancel". */
   sound_volume( conf.sound );
   music_volume( conf.music );
   render_setGamma( conf.gamma_correction );

   window_destroy( opt_wid );
   opt_wid = 0;

   /* Free config. */
   conf_free( &local_conf );
}

/*
 * Gets the list of languages available. Options look like "[ 81%] de".
 */
static char **lang_list( int *n )
{
   char          **ls;
   LanguageOption *opts        = gettext_languageOptions();
   const char     *syslang     = gettext_getSystemLanguage();
   double          syscoverage = gettext_languageCoverage( syslang );

   /* Default English only. */
   ls = malloc( sizeof( char * ) * 128 );
   SDL_asprintf( &ls[0], _( "system (%s[%3.0f%%] %s#0)" ),
                 ( syscoverage < 0.8 ) ? "#r" : "", 100. * syscoverage,
                 syslang );
   *n = 1;

   /* Try to open the available languages. */
   for ( int i = 0; i < array_size( opts ); i++ ) {
      char **item = &ls[( *n )++];
      SDL_asprintf( item, "%s[%3.0f%%] %s",
                    ( opts[i].coverage < 0.8 ) ? "#r" : "",
                    100. * opts[i].coverage, opts[i].language );
      free( opts[i].language );
   }
   array_free( opts );

   qsort( &ls[1], *n - 1, sizeof( char * ), strsort_reverse );
   return ls;
}

/**
 * @brief Opens the gameplay menu.
 */
static void opt_gameplay( unsigned int wid )
{
   (void)wid;
   char              buf[STRMAX];
   int               cw;
   int               w, h, y, x, by, l, n, i, p;
   const char       *s;
   char            **ls, **diff_text, **diff_alt;
   const Difficulty *difficulty, *cur_difficulty;

   /* Get size. */
   window_dimWindow( wid, &w, &h );

   /* Close button */
   window_addButton( wid, -20 - 1 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnCancel", _( "Cancel" ), opt_close );
   window_addButton( wid, -20 - 2 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnDefaults", _( "Defaults" ),
                     opt_gameplayDefaults );

   /* Information. */
   cw = ( w - 40 );
   x  = 20;
   y  = -35;
   window_addText( wid, x, y, cw, 20, 1, "txtVersion", NULL, NULL,
                   naev_version( 1 ) );
   y -= 20;

   snprintf( buf, sizeof( buf ), "#n%s#0%s (%s)", _( "Platform Info: " ),
             SDL_GetPlatform(), HOST );
   window_addText( wid, x, y, cw, 20, 1, "txtPlatInfo", NULL, NULL, buf );
   y -= 20;

   snprintf( buf, sizeof( buf ), "#n%s#0%s" CONF_FILE, _( "Config Path: " ),
             nfile_configPath() );
   window_addText( wid, x, y, cw, 20, 1, "txtConfPath", NULL, NULL, buf );
   y -= 40;
   by = y;

   /* Language support. */
   cw = ( w - 60 ) / 2 - 40;
   y  = by;
   x  = 20;
   s  = _( "Language:" );
   l  = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 0, "txtLanguage", NULL, NULL, s );
   ls = lang_list( &n );
   i  = 0;
   if ( conf.language != NULL ) {
      for ( i = 1; i < n; i++ )
         if ( strcmp( conf.language, &ls[i][LANG_CODE_START] ) == 0 )
            break;
      if ( i >= n )
         i = 0;
   }
   window_addList( wid, x + l + 20, y, cw - l - 50, 100, "lstLanguage", ls, n,
                   i, NULL, NULL );
   y -= 110;

   /* Game difficulty. */
   difficulty = difficulty_getAll();
   n          = array_size( difficulty );
   diff_text  = malloc( sizeof( char * ) * n );
   diff_alt   = malloc( sizeof( char * ) * n );
   p          = 0;
   if ( player.p == NULL )
      difficulty_setLocal( NULL );
   cur_difficulty = difficulty_cur();
   for ( i = 0; i < n; i++ ) {
      const Difficulty *d = &difficulty[i];
      diff_text[i]        = strdup( _( d->name ) );
      diff_alt[i]         = difficulty_display( d );
      if ( strcmp( d->name, cur_difficulty->name ) == 0 )
         p = i;
   }
   if ( player.p != NULL )
      s = _( "Difficulty (this save):" );
   else
      s = _( "Difficulty (global):" );
   window_addText( wid, x, y, cw, 20, 0, "txtDifficulty", NULL, NULL, s );
   y -= 20;
   window_addList( wid, x, y, cw, 100, "lstDifficulty", diff_text, n, p, NULL,
                   NULL );
   toolkit_setListAltText( wid, "lstDifficulty", diff_alt );
   y -= 110;

   /* Compilation flags. */
   window_addText( wid, x, y, cw, 20, 0, "txtCompile", NULL, cHeader,
                   _( "Compilation Flags:" ) );
   y -= 20;
   window_addText( wid, x, y, cw, h + y - 20, 0, "txtFlags", NULL, &cFontOrange,
                   ""
#if DEBUGGING
#if DEBUG_PARANOID
                   "Debug Paranoid\n"
#else  /* DEBUG_PARANOID */
                   "Debug\n"
#endif /* DEBUG_PARANOID */
#endif /* DEBUGGING */
#if HAVE_LUAJIT
                   "Using LuaJIT\n"
#endif /* HAVE_LUAJIT */
   );

   /*y -= window_getTextHeight(wid, "txtFlags") + 10; */

   /* Options. */
   x = 20 + cw + 20;
   y = by;
   cw += 80;

   window_addText( wid, x, y, cw, 20, 0, "txtSettings", NULL, cHeader,
                   _( "Settings:" ) );
   y -= 25;

   window_addCheckbox( wid, x, y, cw, 20, "chkZoomManual",
                       _( "Enable manual zoom control" ), NULL,
                       conf.zoom_manual );
   y -= 25;
   window_addCheckbox( wid, x, y, cw, 20, "chkDoubletap",
                       _( "Enable double-tap afterburn/cooldown" ), NULL,
                       conf.doubletap_sens );
   y -= 25;
   window_addCheckbox( wid, x, y, cw, 20, "chkMouseFly",
                       _( "Enable mouse-flying (toggle with middle click)" ),
                       NULL, conf.mouse_fly );
   y -= 25;
   window_addCheckbox( wid, x, y, cw, 20, "chkMouseAccel",
                       _( "Enable mouse-flying accel control" ), NULL,
                       conf.mouse_accel );
   y -= 40;
   s = _( "Visible Messages" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtSMSG", NULL, NULL, s );
   window_addInput( wid, -50, y, 40, 20, "inpMSG", 4, 1, NULL );
   // y -= 30;

   /* Update. */
   opt_gameplayUpdate( wid, NULL );
}

/**
 * @brief Saves the gameplay options.
 */
static int opt_gameplaySave( unsigned int wid, const char *str )
{
   (void)str;
   int               f, p, newlang;
   const char       *vmsg, *s;
   const Difficulty *difficulty;

   /* List. */
   p       = toolkit_getListPos( wid, "lstLanguage" );
   s       = ( p == 0 ) ? NULL : toolkit_getList( wid, "lstLanguage" );
   newlang = ( ( s != NULL ) != ( conf.language != NULL ) ) ||
             ( ( s != NULL ) &&
               ( strcmp( &s[LANG_CODE_START], conf.language ) != 0 ) );
   if ( newlang ) {
      free( conf.language );
      conf.language = ( s == NULL ) ? NULL : strdup( &s[LANG_CODE_START] );
      LOG( "conf.language set to %s", conf.language );
      /* Apply setting going forward; advise restart to regen other text. */
      gettext_setLanguage( conf.language );
      opt_needRestart();

      /* Probably have to reload some fonts or it'll hate us. */
      gl_freeFont( NULL );
      gl_freeFont( &gl_smallFont );
      gl_freeFont( &gl_defFontMono );
      gl_fontInit( &gl_defFont, _( FONT_DEFAULT_PATH ), conf.font_size_def,
                   FONT_PATH_PREFIX, 0 ); /* initializes default font to size */
      gl_fontInit( &gl_smallFont, _( FONT_DEFAULT_PATH ), conf.font_size_small,
                   FONT_PATH_PREFIX, 0 ); /* small font */
      gl_fontInit( &gl_defFontMono, _( FONT_MONOSPACE_PATH ),
                   conf.font_size_def, FONT_PATH_PREFIX, 0 );
   }

   /* Save the difficulty mode. */
   difficulty = difficulty_getAll();
   p          = toolkit_getListPos( wid, "lstDifficulty" );
   difficulty = &difficulty[p];
   if ( player.p == NULL ) { /* Setting global difficulty. */
      free( conf.difficulty );
      if ( difficulty->def ) {
         conf.difficulty = NULL; /* Don't save default. */
         difficulty_setGlobal( NULL );
      } else {
         conf.difficulty = strdup( difficulty->name );
         difficulty_setGlobal( difficulty );
      }
   } else { /* Local difficulty. */
      free( player.difficulty );
      if ( difficulty == difficulty_get( NULL ) ) {
         player.difficulty = NULL;
         difficulty_setLocal( NULL );
      } else {
         player.difficulty = strdup( difficulty->name );
         difficulty_setLocal( difficulty );
      }
   }
   /* Apply difficulty to player ship. */
   if ( player.p != NULL )
      pilot_calcStats( player.p ); /* TODO apply to all player's ships. */

   /* Checkboxes. */
   f = window_checkboxState( wid, "chkDoubletap" );
   if ( ( conf.doubletap_sens != 0 ) != f )
      conf.doubletap_sens = ( f != 0 ) * 250;

   conf.zoom_manual = window_checkboxState( wid, "chkZoomManual" );
   conf.mouse_accel = window_checkboxState( wid, "chkMouseAccel" );
   conf.mouse_fly   = window_checkboxState( wid, "chkMouseFly" );

   /* Get rid of mouse if disabled. */
   if ( !conf.mouse_fly )
      player_rmFlag( PLAYER_MFLY );

   /* Input boxes. */
   vmsg              = window_getInput( wid, "inpMSG" );
   conf.mesg_visible = atoi( vmsg );
   if ( conf.mesg_visible == 0 )
      conf.mesg_visible = INPUT_MESSAGES_DEFAULT;

   return 0;
}

/**
 * @brief Sets the default gameplay options.
 */
static void opt_gameplayDefaults( unsigned int wid, const char *str )
{
   (void)str;
   char vmsg[16];

   /* Restore. */
   /* Checkboxes. */
   window_checkboxSet( wid, "chkZoomManual", MANUAL_ZOOM_DEFAULT );
   window_checkboxSet( wid, "chkDoubletap", DOUBLETAP_SENSITIVITY_DEFAULT );
   window_checkboxSet( wid, "chkMouseFly", MOUSE_FLY_DEFAULT );
   window_checkboxSet( wid, "chkMouseAccel", MOUSE_ACCEL_DEFAULT );

   /* Input boxes. */
   snprintf( vmsg, sizeof( vmsg ), "%d", INPUT_MESSAGES_DEFAULT );
   window_setInput( wid, "inpMSG", vmsg );
}

/**
 * @brief Updates the gameplay options.
 */
static void opt_gameplayUpdate( unsigned int wid, const char *str )
{
   (void)str;
   char vmsg[16];

   /* Checkboxes. */
   window_checkboxSet( wid, "chkZoomManual", conf.zoom_manual );
   window_checkboxSet( wid, "chkDoubletap", conf.doubletap_sens );
   window_checkboxSet( wid, "chkMouseFly", conf.mouse_fly );
   window_checkboxSet( wid, "chkMouseAccel", conf.mouse_accel );

   /* Input boxes. */
   snprintf( vmsg, sizeof( vmsg ), "%d", conf.mesg_visible );
   window_setInput( wid, "inpMSG", vmsg );
}

/**
 * @brief Gets the key binding menu dimensions.
 */
static void menuKeybinds_getDim( unsigned int wid, int *w, int *h, int *lw,
                                 int *lh, int *bw, int *bh )
{
   /* Get window dimensions. */
   window_dimWindow( wid, w, h );

   /* Get button dimensions. */
   if ( bw != NULL )
      *bw = BUTTON_WIDTH;
   if ( bh != NULL )
      *bh = BUTTON_HEIGHT;

   /* Get list dimensions. */
   if ( lw != NULL )
      *lw = *w - 40 - 2 * ( BUTTON_WIDTH + 20 );
   if ( lh != NULL )
      *lh = *h - 90;
}

/**
 * @brief Opens the keybindings menu.
 */
static void opt_keybinds( unsigned int wid )
{
   int w, h, lw, bw, bh;

   /* Get dimensions. */
   menuKeybinds_getDim( wid, &w, &h, &lw, NULL, &bw, &bh );

   /* Restore defaults button. */
   window_addButton( wid, -20 - 1 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnDefaults", _( "Defaults" ),
                     opt_keyDefaults );
   /* Set button. */
   window_addButton( wid, -20, 20 + 1 * ( BUTTON_HEIGHT + 20 ), bw, bh,
                     "btnSet", _( "Set Key" ), opt_setKey );

   /* Text stuff. */
   window_addText( wid, -20, -40, w - ( 20 + lw + 20 + 20 ), 30, 1, "txtName",
                   NULL, cHeader, NULL );
   window_addText( wid, -20, -90, w - ( 20 + lw + 20 + 20 ), h - 170 - 3 * bh,
                   0, "txtDesc", NULL, NULL, NULL );

   /* Generate the list. */
   menuKeybinds_genList( wid );
}

/**
 * @brief Generates the keybindings list.
 *
 *    @param wid Window to update.
 */
static void menuKeybinds_genList( unsigned int wid )
{
   int         p;
   char      **str, mod_text[64];
   KeybindType type;
   SDL_Keymod  mod;
   int         w, h;
   int         lw, lh;
   int         regen, pos, off;

   /* Get dimensions. */
   menuKeybinds_getDim( wid, &w, &h, &lw, &lh, NULL, NULL );

   /* Create the list. */
   str = malloc( sizeof( char * ) * KST_END );
   for ( int j = 0; j < KST_END; j++ ) {
      SDL_Keycode key;
      const char *short_desc = input_getKeybindName( j );
      int l = 128; /* GCC deduces 68 because we have a format string "%s <%s%c>"
                    * where "char mod_text[64]" is one of the "%s" args.
                    * (that plus brackets plus %c + null gets to 68.
                    * Just set to 128 as it's a power of two. */
      str[j] = malloc( l );
      key    = input_getKeybind( j, &type, &mod );
      switch ( type ) {
      case KEYBIND_KEYBOARD:
         /* Generate mod text. */
         if ( mod == NMOD_ANY )
            snprintf( mod_text, sizeof( mod_text ), _( "any+" ) );
         else {
            p           = 0;
            mod_text[0] = '\0';
            if ( mod & NMOD_SHIFT )
               p += scnprintf( &mod_text[p], sizeof( mod_text ) - p,
                               _( "shift+" ) );
            if ( mod & NMOD_CTRL )
               p += scnprintf( &mod_text[p], sizeof( mod_text ) - p,
                               _( "ctrl+" ) );
            if ( mod & NMOD_ALT )
               p += scnprintf( &mod_text[p], sizeof( mod_text ) - p,
                               _( "alt+" ) );
            if ( mod & NMOD_META )
               p += scnprintf( &mod_text[p], sizeof( mod_text ) - p,
                               _( "meta+" ) );
            (void)p;
         }

         /* Print key. Special-case ASCII letters (use uppercase, unlike
          * SDL_GetKeyName.). */
         if ( key < 0x100 && isalpha( key ) )
            snprintf( str[j], l, "%s <%s%c>", short_desc, mod_text,
                      toupper( key ) );
         else
            snprintf( str[j], l, "%s <%s%s>", short_desc, mod_text,
                      pgettext_var( "keyname", input_keyToStr( key ) ) );
         break;
      case KEYBIND_JAXISPOS:
         snprintf( str[j], l, "%s <ja+%d>", short_desc, key );
         break;
      case KEYBIND_JAXISNEG:
         snprintf( str[j], l, "%s <ja-%d>", short_desc, key );
         break;
      case KEYBIND_JBUTTON:
         snprintf( str[j], l, "%s <jb%d>", short_desc, key );
         break;
      case KEYBIND_JHAT_UP:
         snprintf( str[j], l, "%s <jh%d-up>", short_desc, key );
         break;
      case KEYBIND_JHAT_DOWN:
         snprintf( str[j], l, "%s <jh%d-down>", short_desc, key );
         break;
      case KEYBIND_JHAT_LEFT:
         snprintf( str[j], l, "%s <jh%d-left>", short_desc, key );
         break;
      case KEYBIND_JHAT_RIGHT:
         snprintf( str[j], l, "%s <jh%d-right>", short_desc, key );
         break;
      default:
         snprintf( str[j], l, "%s", short_desc );
         break;
      }
   }

   regen = widget_exists( wid, "lstKeybinds" );
   if ( regen ) {
      pos = toolkit_getListPos( wid, "lstKeybinds" );
      off = toolkit_getListOffset( wid, "lstKeybinds" );
      window_destroyWidget( wid, "lstKeybinds" );
   }

   window_addList( wid, 20, -40, lw, lh, "lstKeybinds", str, KST_END, 0,
                   menuKeybinds_update, opt_setKey );

   if ( regen ) {
      toolkit_setListPos( wid, "lstKeybinds", pos );
      toolkit_setListOffset( wid, "lstKeybinds", off );
   }
}

/**
 * @brief Updates the keybindings menu.
 *
 *    @param wid Window to update.
 *    @param name Unused.
 */
static void menuKeybinds_update( unsigned int wid, const char *name )
{
   (void)name;
   int             selected;
   KeySemanticType keybind;
   const char     *desc;
   SDL_Keycode     key;
   KeybindType     type;
   SDL_Keymod      mod;
   char            buf[STRMAX_SHORT];
   char            binding[64];

   /* Get the keybind. */
   selected = toolkit_getListPos( wid, "lstKeybinds" );

   /* Remove the excess. */
   keybind             = selected;
   opt_selectedKeybind = keybind;
   window_modifyText( wid, "txtName", input_getKeybindName( keybind ) );

   /* Get information. */
   desc = input_getKeybindDescription( keybind );
   key  = input_getKeybind( keybind, &type, &mod );

   /* Create the text. */
   switch ( type ) {
   case KEYBIND_NULL:
      snprintf( binding, sizeof( binding ), _( "Not bound" ) );
      break;
   case KEYBIND_KEYBOARD:
      /* Print key. Special-case ASCII letters (use uppercase, unlike
       * SDL_GetKeyName.). */
      if ( key < 0x100 && isalpha( key ) )
         snprintf( binding, sizeof( binding ), _( "keyboard:   %s%s%c" ),
                   ( mod != SDL_KMOD_NONE ) ? input_modToText( mod ) : "",
                   ( mod != SDL_KMOD_NONE ) ? " + " : "", toupper( key ) );
      else
         snprintf( binding, sizeof( binding ), _( "keyboard:   %s%s%s" ),
                   ( mod != SDL_KMOD_NONE ) ? input_modToText( mod ) : "",
                   ( mod != SDL_KMOD_NONE ) ? " + " : "",
                   pgettext_var( "keyname", input_keyToStr( key ) ) );
      break;
   case KEYBIND_JAXISPOS:
      snprintf( binding, sizeof( binding ), _( "joy axis pos:   <%d>" ), key );
      break;
   case KEYBIND_JAXISNEG:
      snprintf( binding, sizeof( binding ), _( "joy axis neg:   <%d>" ), key );
      break;
   case KEYBIND_JBUTTON:
      snprintf( binding, sizeof( binding ), _( "joy button:   <%d>" ), key );
      break;
   case KEYBIND_JHAT_UP:
      snprintf( binding, sizeof( binding ), _( "joy hat up:   <%d>" ), key );
      break;
   case KEYBIND_JHAT_DOWN:
      snprintf( binding, sizeof( binding ), _( "joy hat down: <%d>" ), key );
      break;
   case KEYBIND_JHAT_LEFT:
      snprintf( binding, sizeof( binding ), _( "joy hat left: <%d>" ), key );
      break;
   case KEYBIND_JHAT_RIGHT:
      snprintf( binding, sizeof( binding ), _( "joy hat right:<%d>" ), key );
      break;
   }

   /* Update text. */
   snprintf( buf, sizeof( buf ), "%s\n\n%s\n", desc, binding );
   window_modifyText( wid, "txtDesc", buf );
}

/**
 * @brief Restores the key defaults.
 */
static void opt_keyDefaults( unsigned int wid, const char *str )
{
   (void)str;
   const char *title, *caption;
   char       *ret;
   int         ind;

   const int   n      = 3;
   const char *opts[] = { _( "WASD" ), _( "Arrow Keys" ), _( "Cancel" ) };

   title   = _( "Restore Defaults" );
   caption = _( "Which layout do you want to use?" );

   dialogue_makeChoice( title, caption, n );

   for ( int i = 0; i < n; i++ )
      dialogue_addChoice( title, caption, opts[i] );

   ret = dialogue_runChoice();
   if ( ret == NULL )
      return;

   /* Find the index of the matched option. */
   ind = 0;
   for ( int i = 0; i < n; i++ )
      if ( strcmp( ret, opts[i] ) == 0 ) {
         ind = i;
         break;
      }
   free( ret );

   if ( ind == 2 )
      return;

   /* Restore defaults. */
   input_setDefault( ( ind == 0 ) ? 1 : 0 );

   /* Regenerate list widget. */
   menuKeybinds_genList( wid );

   /* Alert user it worked. */
   dialogue_msgRaw( _( "Defaults Restored" ),
                    _( "Keybindings restored to defaults." ) );
}

static void opt_setEngineLevel( unsigned int wid, const char *str )
{
   char        buf[32];
   double      vol    = window_getFaderValue( wid, str );
   const char *label  = _( "Engine Volume" );
   double      logvol = 1. / pow( 2., ( 1. - vol ) * 8. );
   conf.engine_vol    = vol;
   if ( sound_disabled )
      snprintf( buf, sizeof( buf ), _( "%s: %s" ), label, _( "Muted" ) );
   else {
      const double magic =
         -48. / log( 0.00390625 ); /* -48 dB minimum divided by logarithm of
                                      volume floor. */
      snprintf( buf, sizeof( buf ), _( "%s: %.2f (%.0f dB)" ), label, vol,
                log( logvol ) * magic );
   }
   window_modifyText( wid, "txtEngine", buf );
}

/**
 * @brief Callback to set the sound or music level.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setAudioLevel( unsigned int wid, const char *str )
{
   char   buf[32], *widget;
   double vol = window_getFaderValue( wid, str );
   if ( strcmp( str, "fadSound" ) == 0 ) {
      sound_volume( vol );
      widget = "txtSound";
      opt_audioLevelStr( buf, sizeof( buf ), 0, vol );
   } else {
      music_volume( vol );
      widget = "txtMusic";
      opt_audioLevelStr( buf, sizeof( buf ), 1, vol );
   }

   window_modifyText( wid, widget, buf );
}

/**
 * @brief Sets the sound or music volume string based on level.
 *
 *    @param[out] buf Buffer to use.
 *    @param max Maximum length of the buffer.
 *    @param type 0 for sound, 1 for audio.
 *    @param pos Position of the fader calling the function.
 */
static void opt_audioLevelStr( char *buf, int max, int type, double pos )
{
   const char *str = type ? _( "Music Volume" ) : _( "Sound Volume" );
   double      vol = type ? music_getVolumeLog() : sound_getVolumeLog();

   if ( vol == 0. )
      snprintf( buf, max, _( "%s: %s" ), str, _( "Muted" ) );
   else {
      const double magic =
         -48. / log( 0.00390625 ); /* -48 dB minimum divided by logarithm of
                                      volume floor. */
      snprintf( buf, max, _( "%s: %.2f (%.0f dB)" ), str, pos,
                log( vol ) * magic );
   }
}

/**
 * @brief Opens the audio settings menu.
 */
static void opt_audio( unsigned int wid )
{
   (void)wid;
   int cw, w, h, y, x;

   /* Get size. */
   window_dimWindow( wid, &w, &h );

   /* Close button */
   window_addButton( wid, -20 - 1 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnCancel", _( "Cancel" ), opt_close );
   window_addButton( wid, -20 - 2 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnDefaults", _( "Defaults" ),
                     opt_audioDefaults );

   cw = ( w - 60 ) / 2;
   x  = 20;
   y  = -60;
   window_addCheckbox( wid, x, y, cw, 20, "chkNosound",
                       _( "Disable all sound/music" ), NULL, conf.nosound );
   y -= 30;

   window_addCheckbox( wid, x, y, cw, 20, "chkEFX", _( "EFX (More CPU)" ), NULL,
                       conf.al_efx );

   /* Sound levels. */
   x = 20 + cw + 20;
   y = -60;
   window_addText( wid, x, y, cw - 40, 20, 0, "txtSVolume", NULL, cHeader,
                   _( "Volume Levels:" ) );
   y -= 30;

   /* Sound fader. */
   window_addText( wid, x, y, cw, 20, 1, "txtSound", NULL, NULL, NULL );
   y -= 20;
   window_addFader( wid, x, y, cw, 20, "fadSound", 0., 1., sound_getVolume(),
                    opt_setAudioLevel );
   window_faderScrollDone( wid, "fadSound", opt_beep );
   y -= 30;

   /* Music fader. */
   window_addText( wid, x, y, cw, 20, 1, "txtMusic", NULL, NULL, NULL );
   y -= 20;
   window_addFader( wid, x, y, cw, 20, "fadMusic", 0., 1., music_getVolume(),
                    opt_setAudioLevel );
   y -= 30;

   /* Engine fader. */
   window_addText( wid, x, y, cw, 20, 1, "txtEngine", NULL, NULL, NULL );
   y -= 20;
   window_addFader( wid, x, y, cw, 20, "fadEngine", 0., 1., conf.engine_vol,
                    opt_setEngineLevel );
   opt_setEngineLevel( wid, "fadEngine" );

   opt_audioUpdate( wid );
}

static void opt_beep( unsigned int wid, const char *str )
{
   (void)wid;
   (void)str;
   player_soundPlayGUI( snd_target, 1 );
}

/**
 * @brief Saves the audio stuff.
 */
static int opt_audioSave( unsigned int wid, const char *str )
{
   (void)str;
   int f;

   f = window_checkboxState( wid, "chkNosound" );
   if ( conf.nosound != f ) {
      conf.nosound = f;
      opt_needRestart();
   }

   f = window_checkboxState( wid, "chkEFX" );
   if ( conf.al_efx != f ) {
      conf.al_efx = f;
      opt_needRestart();
   }

   /* Faders. */
   conf.sound      = window_getFaderValue( wid, "fadSound" );
   conf.music      = window_getFaderValue( wid, "fadMusic" );
   conf.engine_vol = window_getFaderValue( wid, "fadEngine" );

   return 0;
}

/**
 * @brief Sets the audio defaults.
 */
static void opt_audioDefaults( unsigned int wid, const char *str )
{
   (void)str;

   /* Set defaults. */
   /* Faders. */
   window_faderValue( wid, "fadSound", SOUND_VOLUME_DEFAULT );
   window_faderValue( wid, "fadMusic", MUSIC_VOLUME_DEFAULT );
   window_faderValue( wid, "fadEngine", ENGINE_VOLUME_DEFAULT );

   /* Checkboxes. */
   window_checkboxSet( wid, "chkNosound", MUTE_SOUND_DEFAULT );
   window_checkboxSet( wid, "chkEFX", USE_EFX_DEFAULT );
}

/**
 * @brief Updates the gameplay options.
 */
static void opt_audioUpdate( unsigned int wid )
{
   /* Checkboxes. */
   window_checkboxSet( wid, "chkNosound", conf.nosound );
   window_checkboxSet( wid, "chkEFX", conf.al_efx );

   /* Faders. */
   window_faderValue( wid, "fadSound", conf.sound );
   window_faderValue( wid, "fadMusic", conf.music );
   window_faderValue( wid, "fadEngine", conf.engine_vol );
}

/**
 * @brief Tries to set the key from an event.
 */
static int opt_setKeyEvent( unsigned int wid, SDL_Event *event )
{
   unsigned int    parent;
   KeybindType     type;
   int             key, test_key_event;
   SDL_Keymod      mod;
   KeySemanticType boundkey;

   /* See how to handle it. */
   switch ( event->type ) {
   case SDL_EVENT_KEY_DOWN:
      key = event->key.key;
      /* If control key make player hit twice. */
      test_key_event = ( key == SDLK_NUMLOCKCLEAR ) ||
                       ( key == SDLK_CAPSLOCK ) || ( key == SDLK_SCROLLLOCK ) ||
                       ( key == SDLK_RSHIFT ) || ( key == SDLK_LSHIFT ) ||
                       ( key == SDLK_RCTRL ) || ( key == SDLK_LCTRL ) ||
                       ( key == SDLK_RALT ) || ( key == SDLK_LALT ) ||
                       ( key == SDLK_RGUI ) || ( key == SDLK_LGUI );
      if ( test_key_event && ( opt_lastKeyPress != key ) ) {
         opt_lastKeyPress = key;
         return 0;
      }
      type = KEYBIND_KEYBOARD;
      if ( window_checkboxState( wid, "chkAny" ) )
         mod = NMOD_ANY;
      else {
         SDL_Keymod ev_mod = event->key.mod;
         mod               = 0;
         if ( ev_mod & ( SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT ) )
            mod |= NMOD_SHIFT;
         if ( ev_mod & ( SDL_KMOD_LCTRL | SDL_KMOD_RCTRL ) )
            mod |= NMOD_CTRL;
         if ( ev_mod & ( SDL_KMOD_LALT | SDL_KMOD_RALT ) )
            mod |= NMOD_ALT;
         if ( ev_mod & ( SDL_KMOD_LGUI | SDL_KMOD_RGUI ) )
            mod |= NMOD_META;
      }
      /* Set key. */
      opt_lastKeyPress = key;
      break;

   case SDL_EVENT_JOYSTICK_AXIS_MOTION:
      if ( event->jaxis.value > 0 )
         type = KEYBIND_JAXISPOS;
      else if ( event->jaxis.value < 0 )
         type = KEYBIND_JAXISNEG;
      else
         return 0; /* Not handled. */
      key = event->jaxis.axis;
      mod = NMOD_ANY;
      break;

   case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
      type = KEYBIND_JBUTTON;
      key  = event->jbutton.button;
      mod  = NMOD_ANY;
      break;

   case SDL_EVENT_JOYSTICK_HAT_MOTION:
      switch ( event->jhat.value ) {
      case SDL_HAT_UP:
         type = KEYBIND_JHAT_UP;
         break;
      case SDL_HAT_DOWN:
         type = KEYBIND_JHAT_DOWN;
         break;
      case SDL_HAT_LEFT:
         type = KEYBIND_JHAT_LEFT;
         break;
      case SDL_HAT_RIGHT:
         type = KEYBIND_JHAT_RIGHT;
         break;
      default:
         return 0; /* Not handled. */
      }
      key = event->jhat.hat;
      mod = NMOD_ANY;
      break;

      /* Not handled. */
   default:
      return 0;
   }

   /* Warn if already bound. */
   boundkey = input_keyAlreadyBound( type, key, mod );
   if ( ( boundkey >= 0 ) && ( boundkey < KST_END ) &&
        ( boundkey != opt_selectedKeybind ) )
      dialogue_alert(
         _( "Key '#b%s#0' overlaps with key '#b%s#0' that was just set. "
            "You may want to correct this." ),
         input_getKeybindName( boundkey ),
         input_getKeybindName( opt_selectedKeybind ) );

   /* Set keybinding. */
   input_setKeybind( opt_selectedKeybind, type, key, mod );

   /* Close window. */
   window_close( wid, NULL );

   /* Update parent window. */
   parent = window_getParent( wid );
   menuKeybinds_genList( parent );

   return 0;
}

/**
 * @brief Rebinds a key.
 */
static void opt_setKey( unsigned int wid, const char *str )
{
   (void)wid;
   (void)str;
   unsigned int new_wid;
   int          w, h;

   /* Reset key. */
   opt_lastKeyPress = 0;

   /* Create new window. */
   w       = 20 + 2 * ( BUTTON_WIDTH + 20 );
   h       = 20 + BUTTON_HEIGHT + 20 + 20 + 80 + 40;
   new_wid = window_create( "wdwSetKey", _( "Set Keybinding" ), -1, -1, w, h );
   window_handleEvents( new_wid, opt_setKeyEvent );
   window_setParent( new_wid, wid );

   /* Set text. */
   window_addText(
      new_wid, 20, -40, w - 40, 60, 0, "txtInfo", NULL, NULL,
      _( "To use a modifier key hit that key twice in a row, otherwise it "
         "will register as a modifier. To set with any modifier click the "
         "checkbox." ) );

   /* Create button to cancel. */
   window_addButton( new_wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "btnCancel",
                     _( "Cancel" ), window_close );

   /* Button to unset. */
   window_addButton( new_wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "btnUnset",
                     _( "Unset" ), opt_unsetKey );

   /* Checkbox to set any modifier. */
   window_addCheckbox( new_wid, -20, 20 + BUTTON_HEIGHT + 20, w - 40, 20,
                       "chkAny", _( "Set any modifier" ), NULL, 0 );
}

/**
 * @brief Unsets the key.
 */
static void opt_unsetKey( unsigned int wid, const char *str )
{
   (void)str;
   unsigned int parent;

   /* Unsets the keybind. */
   input_setKeybind( opt_selectedKeybind, KEYBIND_NULL, 0, 0 );

   /* Close window. */
   window_close( wid, NULL );

   /* Update parent window. */
   parent = window_getParent( wid );
   menuKeybinds_genList( parent );
}

/**
 * @brief Initializes the accessibility window.
 */
static void opt_accessibility( unsigned int wid )
{
   int         cw, w, h, y, x;
   const char *colourblind_types[] = {
      _( "Protanopia" ),       _( "Deuteranopia" ),      _( "Tritanopia" ),
      _( "Rod Monochromacy" ), _( "Cone Monochromacy" ),
   };
   int    ntypes = sizeof( colourblind_types ) / sizeof( colourblind_types[0] );
   char **types  = malloc( ntypes * sizeof( char * ) );
   for ( int i = 0; i < ntypes; i++ )
      types[i] = strdup( colourblind_types[i] );

   /* Get size. */
   window_dimWindow( wid, &w, &h );

   /* Close button */
   window_addButton( wid, -20 - 1 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnCancel", _( "Cancel" ), opt_close );
   window_addButton( wid, -20 - 2 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnDefaults", _( "Defaults" ),
                     opt_accessibilityDefaults );

   /* Resolution bits. */
   cw = ( w - 60 ) / 2;
   x  = 20;
   y  = -40;

   /* Video. */
   window_addText( wid, x, y, 100, 20, 0, "txtSVideo", NULL, cHeader,
                   _( "Video:" ) );
   y -= 20;
   window_addText( wid, x, y - 3, cw - 20, 20, 0, "txtSaturation", NULL, NULL,
                   NULL );
   y -= 20;
   window_addFader( wid, x + 20, y, cw - 60, 20, "fadSaturation", 0., 1.,
                    conf.nebu_saturation, opt_setSaturation );
   opt_setSaturation( wid, "fadSaturation" );
   y -= 30;
   window_addText( wid, x, y - 3, cw - 20, 20, 0, "txtNebuNonuniformity", NULL,
                   NULL, NULL );
   y -= 20;
   window_addFader( wid, x + 20, y, cw - 60, 20, "fadNebuNonuniformity", 0., 1.,
                    conf.nebu_nonuniformity, opt_setNebuNonuniformity );
   opt_setNebuNonuniformity( wid, "fadNebuNonuniformity" );
   y -= 30;
   window_addText( wid, x, y - 3, cw - 20, 20, 0, "txtBGBrightness", NULL, NULL,
                   NULL );
   y -= 20;
   window_addFader( wid, x + 20, y, cw - 60, 20, "fadBGBrightness", 0., 1.,
                    conf.bg_brightness, opt_setBGBrightness );
   opt_setBGBrightness( wid, "fadBGBrightness" );
   y -= 30;
   window_addText( wid, x, y - 3, cw - 20, 20, 0, "txtJumpBrightness", NULL,
                   NULL, NULL );
   y -= 20;
   window_addFader( wid, x + 20, y, cw - 60, 20, "fadJumpBrightness", 0., 1.,
                    conf.jump_brightness, opt_setJumpBrightness );
   opt_setJumpBrightness( wid, "fadJumpBrightness" );
   y -= 30;

   /* Second column. */
   x = 20 + cw + 20;
   y = -40;

   window_addText( wid, x, y - 3, cw - 20, 20, 0, "txtColourblind", NULL, NULL,
                   _( "Colourblind type:" ) );
   y -= 25;
   window_addList( wid, x, y, cw, 100, "lstColourblind", (char **)types, ntypes,
                   conf.colourblind_type, opt_listColourblind, NULL );
   y -= 115;
   window_addText( wid, x, y - 3, cw - 20, 20, 0, "txtColourblindCorrect", NULL,
                   NULL, NULL );
   y -= 20;
   window_addFader( wid, x + 20, y, cw - 60, 20, "fadColourblindCorrect", 0.,
                    1., conf.colourblind_correct, opt_setColourblindCorrect );
   opt_setColourblindCorrect( wid, "fadColourblindCorrect" );
   y -= 30;
   window_addText( wid, x, y - 3, cw - 20, 20, 0, "txtColourblindSimulate",
                   NULL, NULL, NULL );
   y -= 20;
   window_addFader( wid, x + 20, y, cw - 60, 20, "fadColourblindSimulate", 0.,
                    1., conf.colourblind_sim, opt_setColourblindSimulate );
   opt_setColourblindSimulate( wid, "fadColourblindSimulate" );
   y -= 50;

   window_addText( wid, x, y, cw - 20, 20, 0, "txtSGameplay", NULL, cHeader,
                   _( "Gamplay:" ) );
   y -= 20;
   window_addCheckbox( wid, x, y, cw - 20, 20, "chkPuzzleSkip",
                       _( "Allow skipping puzzles" ), NULL, conf.puzzle_skip );
   y -= 30;
   window_addText( wid, x, y - 3, cw - 20, 20, 0, "txtGameSpeed", NULL, NULL,
                   NULL );
   y -= 20;
   window_addFader( wid, x + 20, y, cw - 60, 20, "fadGameSpeed", 0.1, 1.,
                    conf.game_speed, opt_setGameSpeed );
   opt_setGameSpeed( wid, "fadGameSpeed" );
}

static int opt_accessibilitySave( unsigned int wid, const char *str )
{
   (void)wid;
   (void)str;

   /* Checkboxes need saving. */
   conf.puzzle_skip = window_checkboxState( wid, "chkPuzzleSkip" );

   /* Colourblind and faders are handled in their respective functions. */
   return 0;
}

/**
 * @brief Sets video defaults.
 */
static void opt_accessibilityDefaults( unsigned int wid, const char *str )
{
   (void)str;

   /* Faders. */
   window_faderSetBoundedValue( wid, "fadNebuNonuniformity",
                                NEBU_NONUNIFORMITY_DEFAULT );
   window_faderSetBoundedValue( wid, "fadBGBrightness", BG_BRIGHTNESS_DEFAULT );
   window_faderSetBoundedValue( wid, "fadJumpBrightness",
                                JUMP_BRIGHTNESS_DEFAULT );
   window_faderSetBoundedValue( wid, "fadColourblindCorrect",
                                COLOURBLIND_CORRECT_DEFAULT );
   window_faderSetBoundedValue( wid, "fadColourblindSimulate",
                                COLOURBLIND_CORRECT_DEFAULT );

   /* Checkboxes. */
   window_checkboxSet( wid, "chkPuzzleSkip", PUZZLE_SKIP_DEFAULT );

   /* Reset colorblind if needed. */
   gl_colourblind();
}

/**
 * @brief Initializes the video window.
 */
static void opt_video( unsigned int wid )
{
   char        buf[16];
   int         cw, w, h, y, x, l;
   const char *s;

   /* Get size. */
   window_dimWindow( wid, &w, &h );

   /* Close button */
   window_addButton( wid, -20 - 1 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnCancel", _( "Cancel" ), opt_close );
   window_addButton( wid, -20 - 2 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnDefaults", _( "Defaults" ),
                     opt_videoDefaults );

   /* Resolution bits. */
   cw = ( w - 60 ) / 2;
   x  = 20;
   y  = -40;
   window_addText( wid, x, y, 100, 20, 0, "txtSRes", NULL, cHeader,
                   _( "Window:" ) );
   y -= 30;
   window_addCheckbox( wid, x, y, 100, 20, "chkFullscreen",
                       _( "Fullscreen Window" ), opt_toggleFullscreen,
                       conf.fullscreen );
   y -= 30;
   window_addText( wid, x, y - 3, 130, 20, 0, "txtScale", NULL, NULL, NULL );
   window_addFader( wid, x + 140, y, cw - 160, 20, "fadScale", log( 1. ),
                    log( 3. ), log( conf.scalefactor ), opt_setScalefactor );
   opt_setScalefactor( wid, "fadScale" );
   y -= 30;
   window_addText( wid, x, y - 3, 130, 20, 0, "txtZoomFar", NULL, NULL, NULL );
   window_addFader( wid, x + 140, y, cw - 160, 20, "fadZoomFar", log1p( 0.1 ),
                    log1p( 2.0 ), log1p( conf.zoom_far ), opt_setZoomFar );
   y -= 30;
   window_addText( wid, x, y - 3, 130, 20, 0, "txtZoomNear", NULL, NULL, NULL );
   window_addFader( wid, x + 140, y, cw - 160, 20, "fadZoomNear", log1p( 0.1 ),
                    log1p( 2.0 ), log1p( conf.zoom_near ), opt_setZoomNear );
   opt_setZoomFar( wid, "fadZoomFar" );
   opt_setZoomNear( wid, "fadZoomNear" );
   y -= 30;
   window_addText( wid, x, y - 3, 130, 20, 0, "txtGammaCorrection", NULL, NULL,
                   NULL );
   window_addFader( wid, x + 140, y, cw - 160, 20, "fadGammaCorrection",
                    -log( 3. ), log( 3. ), log( conf.gamma_correction ),
                    opt_setGammaCorrection );
   opt_setGammaCorrection( wid, "fadGammaCorrection" );
   y -= 40;

   /* FPS stuff. */
   window_addText( wid, x, y, 100, 20, 0, "txtFPSTitle", NULL, cHeader,
                   _( "FPS Control:" ) );
   y -= 25;
   s = _( "FPS Limit" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtSFPS", NULL, NULL, s );
   window_addInput( wid, x + l + 20, y, 40, 20, "inpFPS", 4, 1, NULL );
   window_setInputFilter( wid, "inpFPS", INPUT_FILTER_NUMBER );
   snprintf( buf, sizeof( buf ), "%d", conf.fps_max );
   window_setInput( wid, "inpFPS", buf );
   window_addCheckbox( wid, x + l + 20 + 40 + 20, y, cw, 20, "chkFPS",
                       _( "Show FPS" ), NULL, conf.fps_show );

   /* OpenGL options. */
   x = 20 + cw + 20;
   y = -40;
   window_addText( wid, x, y, 100, 20, 0, "txtSGL", NULL, cHeader,
                   _( "OpenGL:" ) );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20, "chkLowMemory",
                       _( "Optimize for low memory systems" ), opt_checkRestart,
                       conf.low_memory );
   y -= 25;
   window_addCheckbox( wid, x, y, cw, 20, "chkVSync", _( "Vertical Sync" ),
                       opt_checkRestart, conf.vsync );
   y -= 40;

   /* Features. */
   window_addText( wid, x, y, 100, 20, 0, "txtSFeatures", NULL, cHeader,
                   _( "Features:" ) );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20, "chkMinimize",
                       _( "Minimize on focus loss" ), NULL, conf.minimize );
   y -= 25;
   window_addCheckbox( wid, x, y, cw, 20, "chkHealth",
                       _( "Health bars for pilots" ), opt_checkHealth,
                       conf.healthbars );
   y -= 30;
   window_addText( wid, x, y - 3, cw - 20, 20, 0, "txtMOpacity", NULL, NULL,
                   NULL );
   y -= 20;
   window_addFader( wid, x + 20, y, cw - 60, 20, "fadMapOverlayOpacity", 0., 1.,
                    conf.map_overlay_opacity, opt_setMapOverlayOpacity );
   opt_setMapOverlayOpacity( wid, "fadMapOverlayOpacity" );
   y -= 25;
   window_addCheckbox( wid, x, y, cw, 20, "chkViewport",
                       _( "Show viewport in radar/overlay" ), opt_checkViewport,
                       conf.show_viewport );
   y -= 40;

   /* GUI */
   window_addText( wid, x, y, 100, 20, 0, "txtSGUI", NULL, cHeader,
                   _( "GUI:" ) );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20, "chkBigIcons", _( "Bigger icons" ),
                       NULL, conf.big_icons );
}

/**
 * @brief Marks that needs restart.
 */
static void opt_needRestart( void )
{
   const char *s = _( "#rRestart Naev for changes to take effect.#0" );
   opt_restart   = 1;

   /* Modify widgets. */
   for ( int i = 0; i < OPT_WINDOWS; i++ )
      window_modifyText( opt_windows[i], "txtRestart", s );
}

/**
 * @brief Saves the video settings.
 */
static int opt_videoSave( unsigned int wid, const char *str )
{
   (void)str;
   const char *inp;
   int         f;

   /* Fullscreen. */
   conf.fullscreen = window_checkboxState( wid, "chkFullscreen" );

   /* FPS. */
   conf.fps_show = window_checkboxState( wid, "chkFPS" );
   inp           = window_getInput( wid, "inpFPS" );
   conf.fps_max  = atoi( inp );

   /* OpenGL. */
   f = window_checkboxState( wid, "chkLowMemory" );
   if ( conf.low_memory != f ) {
      conf.low_memory = f;
      opt_needRestart();
   }
   f = window_checkboxState( wid, "chkVSync" );
   if ( conf.vsync != f ) {
      conf.vsync = f;
      opt_needRestart();
   }

   /* Features. */
   f = window_checkboxState( wid, "chkMinimize" );
   if ( conf.minimize != f ) {
      conf.minimize = f;
      SDL_SetHint( SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS,
                   conf.minimize ? "1" : "0" );
   }

   /* GUI. */
   conf.big_icons = window_checkboxState( wid, "chkBigIcons" );

   /* Reload background. */
   background_load( cur_system->background );

   return 0;
}

/**
 * @brief Handles the colourblind correction.
 */
static void opt_setColourblindCorrect( unsigned int wid, const char *str )
{
   char buf[STRMAX_SHORT];
   conf.colourblind_correct = window_getFaderValue( wid, str );
   if ( conf.colourblind_correct > 0. )
      snprintf( buf, sizeof( buf ), _( "Colourblind correction: %.2f" ),
                conf.colourblind_correct );
   else
      snprintf( buf, sizeof( buf ), _( "Colourblind correction: off" ) );
   window_modifyText( wid, "txtColourblindCorrect", buf );
   gl_colourblind();
}

/**
 * @brief Handles the colourblind correction.
 */
static void opt_setColourblindSimulate( unsigned int wid, const char *str )
{
   char buf[STRMAX_SHORT];
   conf.colourblind_sim = window_getFaderValue( wid, str );
   if ( conf.colourblind_sim > 0. )
      snprintf( buf, sizeof( buf ), _( "Colourblind simulation: %.2f" ),
                conf.colourblind_sim );
   else
      snprintf( buf, sizeof( buf ), _( "Colourblind simulation: off" ) );
   window_modifyText( wid, "txtColourblindSimulate", buf );
   gl_colourblind();
}

/**
 * @brief Handles the colourblind mode change.
 */
static void opt_listColourblind( unsigned int wid, const char *str )
{
   conf.colourblind_type = toolkit_getListPos( wid, str );
   gl_colourblind();
}

/**
 * @brief Handles the health bar checkbox.
 */
static void opt_checkHealth( unsigned int wid, const char *str )
{
   int f           = window_checkboxState( wid, str );
   conf.healthbars = f;
}

/**
 * @brief Handles the viewport checkbox.
 */
static void opt_checkViewport( unsigned int wid, const char *str )
{
   int f              = window_checkboxState( wid, str );
   conf.show_viewport = f;
}

/**
 * @brief Basically flags for needing a restart.
 */
static void opt_checkRestart( unsigned int wid, const char *str )
{
   (void)wid;
   (void)str;
   opt_needRestart();
}

/**
 * @brief Sets video defaults.
 */
static void opt_videoDefaults( unsigned int wid, const char *str )
{
   (void)str;
   char buf[16];

   /* Restore settings. */
   /* Inputs. */
   snprintf( buf, sizeof( buf ), "%d", FPS_MAX_DEFAULT );
   window_setInput( wid, "inpFPS", buf );

   /* Checkboxes. */
   window_checkboxSet( wid, "chkFullscreen", FULLSCREEN_DEFAULT );
   window_checkboxSet( wid, "chkVSync", VSYNC_DEFAULT );
   window_checkboxSet( wid, "chkFPS", SHOW_FPS_DEFAULT );
   window_checkboxSet( wid, "chkMinimize", MINIMIZE_DEFAULT );
   window_checkboxSet( wid, "chkBigIcons", BIG_ICONS_DEFAULT );

   /* Faders. */
   window_faderSetBoundedValue( wid, "fadScale", log( SCALE_FACTOR_DEFAULT ) );
   window_faderSetBoundedValue( wid, "fadZoomFar", log1p( ZOOM_FAR_DEFAULT ) );
   window_faderSetBoundedValue( wid, "fadZoomNear",
                                log1p( ZOOM_NEAR_DEFAULT ) );
   window_faderSetBoundedValue(
      wid, "fadGammaCorrection",
      log( GAMMA_CORRECTION_DEFAULT ) /* a.k.a. 0. */ );
   window_faderSetBoundedValue( wid, "fadMapOverlayOpacity",
                                MAP_OVERLAY_OPACITY_DEFAULT );
}

static void opt_toggleFullscreen( unsigned int wid, const char *str )
{
   gl_setFullscreen( window_checkboxState( wid, str ) );
}

/**
 * @brief Callback to set the scaling factor.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setScalefactor( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double scale = window_getFaderValue( wid, str );
   // scale = round(scale * 10.) / 10.;
   conf.scalefactor = exp( scale );
   snprintf( buf, sizeof( buf ), _( "Scaling: %.1fx" ),
             round( 10. * conf.scalefactor ) / 10. );
   window_modifyText( wid, "txtScale", buf );
   if ( FABS( conf.scalefactor - local_conf.scalefactor ) > 1e-4 )
      opt_needRestart();
}

/**
 * @brief Callback to set the far zoom.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setZoomFar( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double scale = window_getFaderValue( wid, str );
   // scale = round(scale * 10.) / 10.;
   conf.zoom_far = expm1( scale );
   snprintf( buf, sizeof( buf ), _( "Far Zoom: %.1fx" ),
             round( conf.zoom_far * 10. ) / 10. );
   window_modifyText( wid, "txtZoomFar", buf );
   if ( conf.zoom_far > conf.zoom_near ) {
      window_faderSetBoundedValue( wid, "fadZoomNear", log1p( conf.zoom_far ) );
      opt_setZoomNear( wid, "fadZoomNear" );
   }
   if ( FABS( conf.zoom_far - local_conf.zoom_far ) > 1e-4 )
      opt_needRestart();
}

/**
 * @brief Callback to set the far zoom.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setZoomNear( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double scale = window_getFaderValue( wid, str );
   // scale = round(scale * 10.) / 10.;
   conf.zoom_near = expm1( scale );
   snprintf( buf, sizeof( buf ), _( "Near Zoom: %.1fx" ),
             round( conf.zoom_near * 10. ) / 10. );
   window_modifyText( wid, "txtZoomNear", buf );
   if ( conf.zoom_near < conf.zoom_far ) {
      window_faderSetBoundedValue( wid, "fadZoomFar", log1p( conf.zoom_near ) );
      opt_setZoomFar( wid, "fadZoomFar" );
   }
   if ( FABS( conf.zoom_near - local_conf.zoom_near ) > 1e-4 )
      opt_needRestart();
}

/**
 * @brief Callback to set the gamma correction value (reciprocal of exponent).
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setGammaCorrection( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double scale          = window_getFaderValue( wid, str );
   conf.gamma_correction = exp( scale );
   snprintf( buf, sizeof( buf ), _( "Gamma: %.1f" ),
             round( 10. * conf.gamma_correction ) / 10. );
   window_modifyText( wid, "txtGammaCorrection", buf );
   render_setGamma( conf.gamma_correction );
}

/**
 * @brief Callback to set the background brightness.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setBGBrightness( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double fad         = window_getFaderValue( wid, str );
   conf.bg_brightness = fad;
   snprintf( buf, sizeof( buf ), _( "BG (Stars, etc.) brightness: %.0f%%" ),
             round( 100. * fad ) );
   window_modifyText( wid, "txtBGBrightness", buf );
}

static void opt_setSaturation( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double fad           = window_getFaderValue( wid, str );
   conf.nebu_saturation = fad;

   nebu_updateColour();

   /* Update text. */
   snprintf( buf, sizeof( buf ), _( "Nebula saturation: %.0f%%" ),
             round( 100. * fad ) );
   window_modifyText( wid, "txtSaturation", buf );
}

/**
 * @brief Callback to set the nebula non-uniformity parameter.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setNebuNonuniformity( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double fad              = window_getFaderValue( wid, str );
   conf.nebu_nonuniformity = fad;
   snprintf( buf, sizeof( buf ), _( "Nebula non-uniformity: %.0f%%" ),
             round( 100. * fad ) );
   window_modifyText( wid, "txtNebuNonuniformity", buf );
}

/**
 * @brief Callback to set the background brightness.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setJumpBrightness( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double fad           = window_getFaderValue( wid, str );
   conf.jump_brightness = fad;
   snprintf( buf, sizeof( buf ), _( "Jump brightness: %.0f%%" ),
             round( 100. * fad ) );
   window_modifyText( wid, "txtJumpBrightness", buf );
}

static void opt_setGameSpeed( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double prevspeed = conf.game_speed;
   conf.game_speed  = window_getFaderValue( wid, str );
   player.speed *= conf.game_speed / prevspeed;
   pause_setSpeed( player.speed );
   snprintf( buf, sizeof( buf ), _( "Game speed: %.0f%%" ),
             round( 100. * conf.game_speed ) );
   window_modifyText( wid, "txtGameSpeed", buf );
}

/**
 * @brief Callback to set autonav abort threshold.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setMapOverlayOpacity( unsigned int wid, const char *str )
{
   char   buf[STRMAX_SHORT];
   double fad               = window_getFaderValue( wid, str );
   conf.map_overlay_opacity = fad;
   snprintf( buf, sizeof( buf ), _( "Map Overlay Opacity: %.0f%%" ),
             round( 100. * fad ) );
   window_modifyText( wid, "txtMOpacity", buf );
}

/**
 * @brief Opens the keybindings menu.
 */
static void opt_plugins( unsigned int wid )
{
   int  w, h, lw, bw;
   char buf[STRMAX_SHORT];

   /* Get dimensions. */
   bw = BUTTON_WIDTH;
   window_dimWindow( wid, &w, &h );
   lw = w - 40 - 2 * ( bw + 20 );

   /* Text stuff. */
   snprintf( buf, sizeof( buf ), "#n%s#0%s%s", _( "Plugins Directory: " ),
             PHYSFS_getRealDir( "plugins" ), "plugins" );
   window_addText( wid, 20, -30, w - 40, 30, 1, "txtPath", NULL, NULL, buf );
   window_addText( wid, -20, -70, w - ( 20 + lw + 20 + 20 ), h - 100, 0,
                   "txtDesc", NULL, NULL, NULL );

   opt_plugins_regenList( wid );

   /* Add buttons. */
   /*
      window_addButton( wid, -20 - 1 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
      BUTTON_HEIGHT, "btnManager", _( "Manager" ),
      opt_keyDefaults );
      window_addButton( wid, -20 - 1 * ( BUTTON_WIDTH + 20 ), 20 + 1 *
      (BUTTON_HEIGHT+20), BUTTON_WIDTH, BUTTON_HEIGHT, "btnDisable", _( "Disable
      Plugin" ), opt_setKey );
      */
   // window_addButton( wid, -20 - 1 * ( BUTTON_WIDTH + 20 ), 20 + 1 * (
   // BUTTON_HEIGHT + 20 ), BUTTON_WIDTH,
   window_addButton( wid, -20 - 1 * ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnPluginAdd", _( "Add Plugin" ),
                     opt_plugins_add );
}

static void opt_plugins_regenList( unsigned int wid )
{
   int             w, h, lw, lh, bw, n, p;
   char          **str;
   const plugin_t *plgs = plugin_list();

   /* Get dimensions. */
   bw = BUTTON_WIDTH;
   window_dimWindow( wid, &w, &h );
   lw = w - 40 - 2 * ( bw + 20 );
   lh = h - 130;

   p = 0;
   if ( widget_exists( wid, "lstPlugins" ) ) {
      p = toolkit_getListPos( wid, "lstPlugins" );
      window_destroyWidget( wid, "lstPlugins" );
   }

   n = array_size( plgs );
   if ( n <= 0 ) {
      str    = malloc( sizeof( char * ) * 1 );
      str[0] = strdup( _( "No Plugins Found" ) );
      n      = 1;
   } else {
      str = malloc( sizeof( char * ) * n );
      for ( int i = 0; i < n; i++ )
         str[i] = strdup( plugin_name( &plgs[i] ) );
   }
   window_addList( wid, 20, -70, lw, lh, "lstPlugins", str, n, p,
                   opt_plugins_update, NULL );
}

static void opt_plugins_add_callback( void              *userdata,
                                      const char *const *filelist, int filter )
{
   (void)filter;
   unsigned int    wid = *(unsigned int *)userdata;
   const plugin_t *plgs;
   char            buf[STRMAX], buf_susp[STRMAX], path[PATH_MAX], *fname;
   int             suspicious = 0;
   const plugin_t *plg_susp   = NULL;

   if ( filelist == NULL ) {
      WARN( _( "Error calling %s: %s" ), "SDL_ShowOpenFileDialog",
            SDL_GetError() );
      return;
   } else if ( filelist[0] == NULL ) {
      /* Cancelled by user.  */
      return;
   }

   /* Check to see if valid. */
   plugin_t *plg = plugin_test( filelist[0] );
   if ( plg == NULL ) {
      dialogue_alert( _( "'%s' is not a valid plugin!" ), filelist[0] );
      return;
   }
   /* See if overlaps. */
   plgs = plugin_list();
   for ( int i = 0; i < array_size( plgs ); i++ ) {
      if ( strcmp( plugin_name( &plgs[i] ), plugin_name( plg ) ) == 0 ) {
         suspicious = 1;
         plg_susp   = &plgs[i];
         snprintf( buf_susp, sizeof( buf_susp ),
                   _( "#nName:#0 %s\n"
                      "#nAuthor:#0 %s\n"
                      "#nVersion:#0 %s\n"
                      "#nDescription:#0 %s" ),
                   plugin_name( plg_susp ), plg_susp->author, plg_susp->version,
                   plg_susp->description );
         break;
      }
   }

   /* New plugin path. */
   fname = strdup( filelist[0] );
   nfile_concatPaths( path, sizeof( path ), plugin_dir(), basename( fname ) );
   free( fname );

   /* Get plugin details. */
   snprintf( buf, sizeof( buf ),
             _( "#nName:#0 %s\n"
                "#nAuthor:#0 %s\n"
                "#nVersion:#0 %s\n"
                "#nDescription:#0 %s" ),
             plugin_name( plg ), plg->author, plg->version, plg->description );

   /* Check to see if definately add. */
   if ( nfile_fileExists( path ) ) {
      if ( suspicious ) {
         if ( !dialogue_YesNo(
                 _( "Update plugin?" ),
                 _( "Are you sure you want to update the plugin '%s'? This "
                    "will require a restart to take full "
                    "effect.\n\n#nNew Plugin Details#0\n%s\n\n#nOld Plugin "
                    "Details#0\n%s" ),
                 plugin_name( plg ), buf, buf_susp ) ) {
            plugin_free( plg );
            free( plg );
            return;
         }
      } else {
         fname = strdup( filelist[0] );
         if ( !dialogue_YesNo(
                 _( "Overwrite plugin?" ),
                 _( "Are you sure you want to add '%s' to your list of active "
                    "plugins? This will overwrite a plugin with the same file "
                    "name (%s), but not same plug in name. This will require a "
                    "restart to take full "
                    "effect.\n\n#nNew Plugin Details#0\n%s" ),
                 plugin_name( plg ), basename( fname ), buf ) ) {
            plugin_free( plg );
            free( plg );
            free( fname );
            return;
         }
         free( fname );
      }
   } else if ( !suspicious ) {
      if ( !dialogue_YesNo(
              _( "Add plugin?" ),
              _( "Are you sure you want to add '%s' to your list of active "
                 "plugins? This will require a restart to take full "
                 "effect.\n\n#nNew Plugin Details#0\n%s" ),
              plugin_name( plg ), buf ) ) {
         plugin_free( plg );
         free( plg );
         return;
      }
   } else {
      if ( !dialogue_YesNo(
              _( "Add plugin?" ),
              _( "Are you sure you want to add '%s' to your list of active "
                 "plugins? #rThis plugin has the same name as one of your "
                 "existing plugins and make cause issues#0. This will require "
                 "a restart to take full effect.\n\n#nNew Plugin "
                 "Details#0\n%s\n\n#nSame name Plugin Details#0\n%s" ),
              plugin_name( plg ), buf, buf_susp ) ) {
         plugin_free( plg );
         free( plg );
         return;
      }
   }

   /* Copy file over. */
   if ( nfile_copyIfExists( filelist[0], path ) ) {
      dialogue_alert( _( "Failed to copy '%s' to '%s'!" ), filelist[0], path );
      plugin_free( plg );
      free( plg );
      return;
   }

   /* Insert plugin. */
   plugin_insert( plg );
   free( plg );
   opt_needRestart();
   opt_plugins_regenList( wid );
}
static void opt_plugins_add( unsigned int wid, const char *name )
{
   (void)name;
   const SDL_DialogFileFilter filter[] = {
      { .name = _( "Naev Plugin File" ), .pattern = "zip" },
      { NULL, NULL },
   };
   /* Open dialogue to load the diff. */
   SDL_ShowOpenFileDialog( opt_plugins_add_callback, &wid, gl_screen.window,
                           filter, 1, conf.dev_data_dir, 0 );
}

static void opt_plugins_update( unsigned int wid, const char *name )
{
   char            buf[STRMAX];
   const plugin_t *plg, *plgs;
   int             pos = toolkit_getListPos( wid, name );
   int             l   = 0;

   plgs = plugin_list();
   if ( array_size( plgs ) <= 0 )
      return;
   plg = &plgs[pos];
   l += scnprintf( &buf[l], sizeof( buf ) - l, "#n%s#0\n",
                   p_( "plugins", "Name:" ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "   %s\n",
                   _( plugin_name( plg ) ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "#n%s#0\n",
                   p_( "plugins", "Author(s):" ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "   %s\n", _( plg->author ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "#n%s#0\n",
                   p_( "plugins", "Version:" ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "   %s\n", plg->version );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "#n%s#0\n",
                   p_( "plugins", "Description:" ) );
   l +=
      scnprintf( &buf[l], sizeof( buf ) - l, "   %s\n", _( plg->description ) );
   if ( plg->total_conversion )
      /*l +=*/scnprintf( &buf[l], sizeof( buf ) - l, "#g%s#0",
                         _( "Total Conversion" ) );

   window_modifyText( wid, "txtDesc", buf );
}
