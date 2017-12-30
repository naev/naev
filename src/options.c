/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file options.c
 *
 * @brief Options menu
 */


#include "options.h"

#include "naev.h"

#include "nstring.h"

#include "SDL.h"

#include "log.h"
#include "input.h"
#include "toolkit.h"
#include "sound.h"
#include "music.h"
#include "nstd.h"
#include "dialogue.h"
#include "conf.h"
#include "ndata.h"
#include "player.h"


#define BUTTON_WIDTH    90 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */

#define OPT_WIN_GAMEPLAY   0
#define OPT_WIN_VIDEO      1
#define OPT_WIN_AUDIO      2
#define OPT_WIN_INPUT      3
#define OPT_WINDOWS        4

static unsigned int opt_wid = 0;
static unsigned int *opt_windows;
static const char *opt_names[] = {
   "Gameplay",
   "Video",
   "Audio",
   "Input"
};


static int opt_restart = 0;


/*
 * External stuff.
 */
extern const char *keybind_info[][3]; /**< from input.c */


static const char *opt_selectedKeybind; /**< Selected keybinding. */
static int opt_lastKeyPress = 0; /**< Last keypress. */


/*
 * prototypes
 */
/* Misc. */
static void opt_close( unsigned int wid, char *name );
static void opt_needRestart (void);
/* Gameplay. */
static void opt_gameplay( unsigned int wid );
static void opt_setAutonavResetSpeed( unsigned int wid, char *str );
static void opt_OK( unsigned int wid, char *str );
static int opt_gameplaySave( unsigned int wid, char *str );
static void opt_gameplayDefaults( unsigned int wid, char *str );
static void opt_gameplayUpdate( unsigned int wid, char *str );
/* Video. */
static void opt_video( unsigned int wid );
static void opt_videoRes( unsigned int wid, char *str );
static int opt_videoSave( unsigned int wid, char *str );
static void opt_videoDefaults( unsigned int wid, char *str );
static void opt_setScalefactor( unsigned int wid, char *str );
/* Audio. */
static void opt_audio( unsigned int wid );
static int opt_audioSave( unsigned int wid, char *str );
static void opt_audioDefaults( unsigned int wid, char *str );
static void opt_audioUpdate( unsigned int wid );
static void opt_audioLevelStr( char *buf, int max, int type, double pos );
static void opt_setAudioLevel( unsigned int wid, char *str );
static void opt_beep( unsigned int wid, char *str );
/* Keybind menu. */
static void opt_keybinds( unsigned int wid );
static void menuKeybinds_getDim( unsigned int wid, int *w, int *h,
      int *lw, int *lh, int *bw, int *bh );
static void menuKeybinds_genList( unsigned int wid );
static void menuKeybinds_update( unsigned int wid, char *name );
static void opt_keyDefaults( unsigned int wid, char *str );
/* Setting keybindings. */
static int opt_setKeyEvent( unsigned int wid, SDL_Event *event );
static void opt_setKey( unsigned int wid, char *str );
static void opt_unsetKey( unsigned int wid, char *str );


/**
 * @brief Creates the options menu thingy.
 */
void opt_menu (void)
{
   int w, h;

   /* Dimensions. */
   w = 600;
   h = 525;

   /* Create window and tabs. */
   opt_wid = window_create( "Options", -1, -1, w, h );
   window_setCancel( opt_wid, opt_close );

   /* Create tabbed window. */
   opt_windows = window_addTabbedWindow( opt_wid, -1, -1, -1, -1, "tabOpt",
         OPT_WINDOWS, opt_names, 0 );

   /* Load tabs. */
   opt_gameplay(  opt_windows[ OPT_WIN_GAMEPLAY ] );
   opt_video(     opt_windows[ OPT_WIN_VIDEO ] );
   opt_audio(     opt_windows[ OPT_WIN_AUDIO ] );
   opt_keybinds(  opt_windows[ OPT_WIN_INPUT ] );

   /* Set as need restart if needed. */
   if (opt_restart)
      opt_needRestart();
}


/**
 * @brief Saves all options and closes the options screen.
 */
static void opt_OK( unsigned int wid, char *str )
{
   int ret;

   ret = 0;
   ret |= opt_gameplaySave( opt_windows[ OPT_WIN_GAMEPLAY ], str);
   ret |= opt_audioSave(    opt_windows[ OPT_WIN_AUDIO ], str);
   ret |= opt_videoSave(    opt_windows[ OPT_WIN_VIDEO ], str);

   /* Close window if no errors occurred. */
   if (!ret)
      opt_close(wid, str);
}

/**
 * @brief Closes the options screen without saving.
 */
static void opt_close( unsigned int wid, char *name )
{
   (void) wid;
   (void) name;

   /* At this point, set sound levels as defined in the config file.
    * This ensures that sound volumes are reset on "Cancel". */
   sound_volume(conf.sound);
   music_volume(conf.music);

   window_destroy( opt_wid );
   opt_wid = 0;
}


/**
 * @brief Handles resize events for the options menu.
 */
void opt_resize (void)
{
   char buf[16];

   /* Nothing to do if not open. */
   if (!opt_wid)
      return;

   /* Update the resolution input widget. */
   nsnprintf( buf, sizeof(buf), "%dx%d", gl_screen.rw, gl_screen.rh );
   window_setInput( opt_windows[OPT_WIN_VIDEO], "inpRes", buf );
}


/**
 * @brief Opens the gameplay menu.
 */
static void opt_gameplay( unsigned int wid )
{
   (void) wid;
   char buf[PATH_MAX];
   const char *path;
   int cw;
   int w, h, y, x, by, l;
   char *s;

   /* Get size. */
   window_dimWindow( wid, &w, &h );

   /* Close button */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", _("OK"), opt_OK );
   window_addButton( wid, -20 - 1*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnCancel", _("Cancel"), opt_close );
   window_addButton( wid, -20 - 2*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDefaults", _("Defaults"), opt_gameplayDefaults );

   /* Information. */
   cw = (w-40);
   x = 20;
   y = -60;
   window_addText( wid, x, y, cw, 20, 1, "txtVersion",
         NULL, NULL, naev_version(1) );
   y -= 20;
#ifdef GIT_COMMIT
   nsnprintf( buf, sizeof(buf), _("Commit: %s"), GIT_COMMIT );
   window_addText( wid, x, y, cw, 20, 1, "txtCommit",
         NULL, NULL, buf );
#endif /* GIT_COMMIT */
   y -= 20;
   path = ndata_getPath();
   if (path == NULL)
      nsnprintf( buf, sizeof(buf), _("not using ndata") );
   else
      nsnprintf( buf, sizeof(buf), _("ndata: %s"), path);
   window_addText( wid, x, y, cw, 20, 1, "txtNdata",
         NULL, NULL, buf );
   y -= 40;
   by = y;


   /* Compiletime stuff. */
   cw = (w-60)/2;
   y  = by;
   x  = 20;
   window_addText( wid, x+20, y, cw, 20, 0, "txtCompile",
         NULL, &cDConsole, _("Compilation Flags") );
   y -= 30;
   window_addText( wid, x, y, cw, h+y-20, 0, "txtFlags",
         NULL, NULL,
         ""
#ifdef DEBUGGING
#ifdef DEBUG_PARANOID
         "Debug Paranoid\n"
#else /* DEBUG_PARANOID */
         "Debug\n"
#endif /* DEBUG_PARANOID */
#endif /* DEBUGGING */
#if defined(LINUX)
         "Linux\n"
#elif defined(FREEBSD)
         "FreeBSD\n"
#elif defined(MACOS)
         "macOS\n"
#elif defined(WIN32)
         "Windows\n"
#else
         "Unknown OS\n"
#endif
#ifdef USE_OPENAL
         "With OpenAL\n"
#endif /* USE_OPENAL */
#ifdef USE_SDLMIX
         "With SDL_mixer\n"
#endif
#ifdef HAVE_LUAJIT
         "Using LuaJIT\n"
#endif
#ifdef NDATA_DEF
         "ndata: "NDATA_DEF"\n"
#endif /* NDATA_DEF */
         );


   /* Options. */
   y  = by;

   /* Autonav abort. */
   x = 20 + cw + 20;
   window_addText( wid, x+65, y, 150, 150, 0, "txtAAutonav",
         NULL, &cDConsole, _("Stop Speedup At:") );
   y -= 20;

   /* Autonav abort fader. */
   window_addText( wid, x, y, cw, 20, 1, "txtAutonav",
         NULL, NULL, NULL );
   y -= 20;
   window_addFader( wid, x, y, cw, 20, "fadAutonav", 0., 1.,
         conf.autonav_reset_speed, opt_setAutonavResetSpeed );
   y -= 40;

   window_addText( wid, x+20, y, cw, 20, 0, "txtSettings",
         NULL, &cDConsole, _("Settings") );
   y -= 25;

   window_addCheckbox( wid, x, y, cw, 20,
         "chkZoomManual", _("Enable manual zoom control"), NULL, conf.zoom_manual );
   y -= 25;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkAfterburn", _("Enable double-tap afterburn"), NULL, conf.afterburn_sens );
   y -= 25;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkMouseThrust", _("Enable mouse-flying thrust control"), NULL, conf.mouse_thrust );
   y -= 25;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkCompress", _("Enable savegame compression"), NULL, conf.save_compress );
   y -= 30;
   s = _("Visible Messages");
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, -100, y, l, 20, 1, "txtSMSG",
         NULL, &cBlack, s );
   window_addInput( wid, -50, y, 40, 20, "inpMSG", 4, 1, NULL );
   y -= 30;
   s = _("Max Time Compression Factor");
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, -100, y, l, 20, 1, "txtTMax",
         NULL, &cBlack, s );
   window_addInput( wid, -50, y, 40, 20, "inpTMax", 4, 1, NULL );

   /* Restart text. */
   window_addText( wid, 20, 10, 3*(BUTTON_WIDTH + 20),
         30, 0, "txtRestart", &gl_smallFont, &cBlack, NULL );

   /* Update. */
   opt_gameplayUpdate( wid, NULL );
}

/**
 * @brief Saves the gameplay options.
 */
static int opt_gameplaySave( unsigned int wid, char *str )
{
   (void) str;
   int f;
   char *vmsg, *tmax;

   /* Checkboxes. */
   f = window_checkboxState( wid, "chkAfterburn" );
   if (!!conf.afterburn_sens != f)
      conf.afterburn_sens = (!!f)*250;

   conf.zoom_manual = window_checkboxState( wid, "chkZoomManual" );
   conf.mouse_thrust = window_checkboxState(wid, "chkMouseThrust" );
   conf.save_compress = window_checkboxState( wid, "chkCompress" );

   /* Faders. */
   conf.autonav_reset_speed = window_getFaderValue(wid, "fadAutonav");

   /* Input boxes. */
   vmsg = window_getInput( wid, "inpMSG" );
   tmax = window_getInput( wid, "inpTMax" );
   conf.mesg_visible = atoi(vmsg);
   conf.compression_mult = atoi(tmax);
   if (conf.mesg_visible == 0)
      conf.mesg_visible = INPUT_MESSAGES_DEFAULT;

   return 0;
}

/**
 * @brief Sets the default gameplay options.
 */
static void opt_gameplayDefaults( unsigned int wid, char *str )
{
   (void) str;
   char vmsg[16], tmax[16];

   /* Restore. */
   /* Checkboxes. */
   window_checkboxSet( wid, "chkZoomManual", MANUAL_ZOOM_DEFAULT );
   window_checkboxSet( wid, "chkAfterburn", AFTERBURNER_SENSITIVITY_DEFAULT );
   window_checkboxSet( wid, "chkMouseThrust", MOUSE_THRUST_DEFAULT );
   window_checkboxSet( wid, "chkCompress", SAVE_COMPRESSION_DEFAULT );

   /* Faders. */
   window_faderValue( wid, "fadAutonav", AUTONAV_RESET_SPEED_DEFAULT );

   /* Input boxes. */
   nsnprintf( vmsg, sizeof(vmsg), "%d", INPUT_MESSAGES_DEFAULT );
   window_setInput( wid, "inpMSG", vmsg );
   nsnprintf( tmax, sizeof(tmax), "%d", TIME_COMPRESSION_DEFAULT_MULT );
   window_setInput( wid, "inpTMax", tmax );
}

/**
 * @brief Updates the gameplay options.
 */
static void opt_gameplayUpdate( unsigned int wid, char *str )
{
   (void) str;
   char vmsg[16], tmax[16];

   /* Checkboxes. */
   window_checkboxSet( wid, "chkZoomManual", conf.zoom_manual );
   window_checkboxSet( wid, "chkAfterburn", conf.afterburn_sens );
   window_checkboxSet( wid, "chkMouseThrust", conf.mouse_thrust );
   window_checkboxSet( wid, "chkCompress", conf.save_compress );

   /* Faders. */
   window_faderValue( wid, "fadAutonav", conf.autonav_reset_speed );

   /* Input boxes. */
   nsnprintf( vmsg, sizeof(vmsg), "%d", conf.mesg_visible );
   window_setInput( wid, "inpMSG", vmsg );
   nsnprintf( tmax, sizeof(tmax), "%g", conf.compression_mult );
   window_setInput( wid, "inpTMax", tmax );
}


/**
 * @brief Callback to set autonav abort threshold.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setAutonavResetSpeed( unsigned int wid, char *str )
{
   char buf[PATH_MAX];
   double autonav_reset_speed;

   /* Set fader. */
   autonav_reset_speed = window_getFaderValue(wid, str);

   /* Generate message. */
   if (autonav_reset_speed >= 1.)
      nsnprintf( buf, sizeof(buf), _("Enemy Presence") );
   else if (autonav_reset_speed > 0.)
      nsnprintf( buf, sizeof(buf), _("%.0f%% Shield"), autonav_reset_speed * 100 );
   else
      nsnprintf( buf, sizeof(buf), _("Armour Damage") );

   window_modifyText( wid, "txtAutonav", buf );
}


/**
 * @brief Gets the keybind menu dimensions.
 */
static void menuKeybinds_getDim( unsigned int wid, int *w, int *h,
      int *lw, int *lh, int *bw, int *bh )
{
   /* Get window dimensions. */
   window_dimWindow( wid, w, h );

   /* Get button dimensions. */
   if (bw != NULL)
      *bw = BUTTON_WIDTH;
   if (bh != NULL)
      *bh = BUTTON_HEIGHT;

   /* Get list dimensions. */
   if (lw != NULL)
      *lw = *w - 2*BUTTON_WIDTH - 80;
   if (lh != NULL)
      *lh = *h - 60;
}


/**
 * @brief Opens the keybindings menu.
 */
static void opt_keybinds( unsigned int wid )
{
   int w, h, lw, bw, bh;

   /* Get dimensions. */
   menuKeybinds_getDim( wid, &w, &h, &lw, NULL, &bw, &bh );

   /* Close button. */
   window_addButton( wid, -20, 20, bw, bh,
         "btnClose", _("OK"), opt_OK );
   /* Set button. */
   window_addButton( wid, -20 - bw - 20, 20, bw, bh,
         "btnSet", _("Set Key"), opt_setKey );
   /* Restore deafaults button. */
   window_addButton( wid, -20, 20+bh+20, bw, bh,
         "btnDefaults", _("Defaults"), opt_keyDefaults );

   /* Text stuff. */
   window_addText( wid, 20+lw+20, -40, w-(20+lw+20), 30, 1, "txtName",
         NULL, &cDConsole, NULL );
   window_addText( wid, 20+lw+20, -90, w-(20+lw+20), h-70-60-bh,
         0, "txtDesc", &gl_smallFont, NULL, NULL );

   /* Generate the list. */
   menuKeybinds_genList( wid );
}


/**
 * @brief Generates the keybindings list.
 *
 *    @param wid Window to update.
 *    @param regen Whether to destroy and recreate the widget.
 */
static void menuKeybinds_genList( unsigned int wid )
{
   int i, j, l, p;
   char **str, mod_text[64];
   SDLKey key;
   KeybindType type;
   SDLMod mod;
   int w, h;
   int lw, lh;
   int regen, pos, off;

   /* Get dimensions. */
   menuKeybinds_getDim( wid, &w, &h, &lw, &lh, NULL, NULL );

   /* Create the list. */
   for (i=0; strcmp(keybind_info[i][0],"end"); i++);
   str = malloc(sizeof(char*) * i);
   for (j=0; j < i; j++) {
      l = 64;
      str[j] = malloc(l);
      key = input_getKeybind( keybind_info[j][0], &type, &mod );
      switch (type) {
         case KEYBIND_KEYBOARD:
            /* Generate mod text. */
            if (mod == NMOD_ALL)
               nsnprintf( mod_text, sizeof(mod_text), "any+" );
            else {
               p = 0;
               mod_text[0] = '\0';
               if (mod & NMOD_SHIFT)
                  p += nsnprintf( &mod_text[p], sizeof(mod_text)-p, "shift+" );
               if (mod & NMOD_CTRL)
                  p += nsnprintf( &mod_text[p], sizeof(mod_text)-p, "ctrl+" );
               if (mod & NMOD_ALT)
                  p += nsnprintf( &mod_text[p], sizeof(mod_text)-p, "alt+" );
               if (mod & NMOD_META)
                  p += nsnprintf( &mod_text[p], sizeof(mod_text)-p, "meta+" );
            }

            /* SDL_GetKeyName returns lowercase which is ugly. */
            if (nstd_isalpha(key))
               nsnprintf(str[j], l, "%s <%s%c>", keybind_info[j][1], mod_text, nstd_toupper(key) );
            else
               nsnprintf(str[j], l, "%s <%s%s>", keybind_info[j][1], mod_text, SDL_GetKeyName(key) );
            break;
         case KEYBIND_JAXISPOS:
            nsnprintf(str[j], l, "%s <ja+%d>", keybind_info[j][1], key);
            break;
         case KEYBIND_JAXISNEG:
            nsnprintf(str[j], l, "%s <ja-%d>", keybind_info[j][1], key);
            break;
         case KEYBIND_JBUTTON:
            nsnprintf(str[j], l, "%s <jb%d>", keybind_info[j][1], key);
            break;
         default:
            nsnprintf(str[j], l, "%s", keybind_info[j][1]);
            break;
      }
   }

   regen = widget_exists( wid, "lstKeybinds" );
   if (regen) {
      pos = toolkit_getListPos( wid, "lstKeybinds" );
      off = toolkit_getListOffset( wid, "lstKeybinds" );
      window_destroyWidget( wid, "lstKeybinds" );
   }

   window_addList( wid, 20, -40, lw, lh, "lstKeybinds",
         str, i, 0, menuKeybinds_update );

   if (regen) {
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
static void menuKeybinds_update( unsigned int wid, char *name )
{
   (void) name;
   int selected;
   const char *keybind;
   const char *desc;
   SDLKey key;
   KeybindType type;
   SDLMod mod;
   char buf[1024];
   char binding[64];

   /* Get the keybind. */
   selected = toolkit_getListPos( wid, "lstKeybinds" );

   /* Remove the excess. */
   keybind = keybind_info[selected][0];
   opt_selectedKeybind = keybind;
   window_modifyText( wid, "txtName", keybind );

   /* Get information. */
   desc = input_getKeybindDescription( keybind );
   key = input_getKeybind( keybind, &type, &mod );

   /* Create the text. */
   switch (type) {
      case KEYBIND_NULL:
         nsnprintf(binding, sizeof(binding), _("Not bound"));
         break;
      case KEYBIND_KEYBOARD:
         /* SDL_GetKeyName returns lowercase which is ugly. */
         if (nstd_isalpha(key))
            nsnprintf(binding, sizeof(binding), _("keyboard:   %s%s%c"),
                  (mod != KMOD_NONE) ? input_modToText(mod) : "",
                  (mod != KMOD_NONE) ? " + " : "",
                  nstd_toupper(key));
         else
            nsnprintf(binding, sizeof(binding), _("keyboard:   %s%s%s"),
                  (mod != KMOD_NONE) ? input_modToText(mod) : "",
                  (mod != KMOD_NONE) ? " + " : "",
                  SDL_GetKeyName(key));
         break;
      case KEYBIND_JAXISPOS:
         nsnprintf(binding, sizeof(binding), _("joy axis pos:   <%d>"), key );
         break;
      case KEYBIND_JAXISNEG:
         nsnprintf(binding, sizeof(binding), _("joy axis neg:   <%d>"), key );
         break;
      case KEYBIND_JBUTTON:
         nsnprintf(binding, sizeof(binding), _("joy button:   <%d>"), key);
         break;
   }

   /* Update text. */
   nsnprintf(buf, 1024, "%s\n\n%s\n", desc, binding);
   window_modifyText( wid, "txtDesc", buf );
}


/**
 * @brief Restores the key defaults.
 */
static void opt_keyDefaults( unsigned int wid, char *str )
{
   (void) str;
   char *title, *caption, *ret;
   int i, ind;

   const int n = 3;
   const char *opts[] = {
      _("WASD"),
      _("Arrow Keys"),
      _("Cancel")
   };

   title   = _("Restore Defaults");
   caption = _("Which layout do you want to use?");

   dialogue_makeChoice( title, caption, 3 );

   for (i=0; i<n; i++)
      dialogue_addChoice( title, caption, opts[i] );

   ret = dialogue_runChoice();
   if (ret == NULL)
      return;

   /* Find the index of the matched option. */
   ind = 0;
   for (i=0; i<n; i++)
      if (strcmp(ret, opts[i]) == 0) {
         ind = i;
         break;
      }

   if (ind == 2)
      return;

   /* Restore defaults. */
   input_setDefault( (ind == 0) ? 1 : 0 );

   /* Regenerate list widget. */
   menuKeybinds_genList( wid );

   /* Alert user it worked. */
   dialogue_msgRaw( _("Defaults Restored"), _("Keybindings restored to defaults."));
}


/**
 * @brief Callback to set the sound or music level.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 *    @param type 0 for sound, 1 for audio.
 */
static void opt_setAudioLevel( unsigned int wid, char *str )
{
   char buf[32], *widget;
   double vol;

   vol = window_getFaderValue(wid, str);
   if (strcmp(str,"fadSound")==0) {
      sound_volume(vol);
      widget = "txtSound";
      opt_audioLevelStr( buf, sizeof(buf), 0, vol );
   }
   else {
      music_volume(vol);
      widget = "txtMusic";
      opt_audioLevelStr( buf, sizeof(buf), 1, vol );
   }

   window_modifyText( wid, widget, buf );
}


/**
 * @brief Sets the sound or music volume string based on level and sound backend.
 *
 *    @param[out] buf Buffer to use.
 *    @param max Maximum length of the buffer.
 *    @param type 0 for sound, 1 for audio.
 *    @param pos Position of the fader calling the function.
 */
static void opt_audioLevelStr( char *buf, int max, int type, double pos )
{
   double vol, magic;
   char *str;

   str = type ? _("Music") : _("Sound");
   vol = type ? music_getVolumeLog() : sound_getVolumeLog();

   if (vol == 0.)
      nsnprintf( buf, max, _("%s Volume: Muted"), str );
   else {
      magic = -48. / log(0.00390625); /* -48 dB minimum divided by logarithm of volume floor. */
      nsnprintf( buf, max, _("%s Volume: %.2f (%.0f dB)"), str, pos, log(vol) * magic );
   }
}


/**
 * @brief Opens the audio settings menu.
 */
static void opt_audio( unsigned int wid )
{
   (void) wid;
   int i, j;
   int cw;
   int w, h, y, x, l;
   char **s;
   const char *str;

   /* Get size. */
   window_dimWindow( wid, &w, &h );

   /* Close button */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", _("OK"), opt_OK );
   window_addButton( wid, -20 - 1*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnCancel", _("Cancel"), opt_close );
   window_addButton( wid, -20 - 2*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDefaults", _("Defaults"), opt_audioDefaults );

   /* General options. */
   cw = (w-60)/2;
   x = 20;
   y = -60;
   window_addText( wid, x+20, y, cw, 20, 0, "txtSGeneral",
         NULL, &cDConsole, _("General") );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkNosound", _("Disable all sound/music"), NULL, conf.nosound );
   y -= 30;
   str = _("Backends");
   l = gl_printWidthRaw( NULL, str );
   window_addText( wid, x, y, l, 40, 0, "txtSBackends",
         NULL, NULL, str );
   l += 10;
   i = 0;
   j = 0;
   s = malloc(sizeof(char*)*2);
#if USE_OPENAL
   if (strcmp(conf.sound_backend,"openal")==0)
      j = i;
   s[i++] = strdup(_("openal"));
#endif /* USE_OPENAL */
#if USE_SDLMIX
   if (strcmp(conf.sound_backend,"sdlmix")==0)
      j = i;
   s[i++] = strdup(_("sdlmix"));
#endif /* USE_SDLMIX */
   if (i==0)
      s[i++] = strdup(_("none"));
   window_addList( wid, x+l, y, cw-(x+l), 40, "lstSound", s, i, j, NULL );
   y -= 50;

   /* OpenAL options. */
   window_addText( wid, x+20, y, cw, 20, 0, "txtSOpenal",
         NULL, &cDConsole, _("OpenAL") );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkEFX", _("EFX (More CPU)"), NULL, conf.al_efx );


   /* Sound levels. */
   x = 20 + cw + 20;
   y = -60;
   window_addText( wid, x+20, y, 100, 20, 0, "txtSVolume",
         NULL, &cDConsole, _("Volume Levels") );
   y -= 30;

   /* Sound fader. */
   window_addText( wid, x, y, cw, 20, 1, "txtSound",
         NULL, NULL, NULL );
   y -= 20;
   window_addFader( wid, x, y, cw, 20, "fadSound", 0., 1.,
         sound_getVolume(), opt_setAudioLevel );
   window_faderScrollDone( wid, "fadSound", opt_beep );
   y -= 40;

   /* Music fader. */
   window_addText( wid, x, y, cw, 20, 1, "txtMusic",
         NULL, NULL, NULL );
   y -= 20;
   window_addFader( wid, x, y, cw, 20, "fadMusic", 0., 1.,
         music_getVolume(), opt_setAudioLevel );

   /* Restart text. */
   window_addText( wid, 20, 10, 3*(BUTTON_WIDTH + 20),
         30, 0, "txtRestart", &gl_smallFont, &cBlack, NULL );

   opt_audioUpdate(wid);
}


static void opt_beep( unsigned int wid, char *str )
{
   (void) wid;
   (void) str;
   player_soundPlayGUI( snd_target, 1 );
}


/**
 * @brief Saves the audio stuff.
 */
static int opt_audioSave( unsigned int wid, char *str )
{
   (void) str;
   int f;
   char *s;

   /* General. */
   f = window_checkboxState( wid, "chkNosound" );
   if (conf.nosound != f) {
      conf.nosound = f;
      opt_needRestart();
   }

   /* Backend. */
   s = toolkit_getList( wid, "lstSound" );
   if (conf.sound_backend != NULL) {
      if (strcmp(s,conf.sound_backend)!=0) {
         free(conf.sound_backend);
         conf.sound_backend = strdup(s);
         opt_needRestart();
      }
   }
   else {
      conf.sound_backend = strdup(s);
      opt_needRestart();
   }

   /* OpenAL. */
   f = window_checkboxState( wid, "chkEFX" );
   if (conf.al_efx != f) {
      conf.al_efx = f;
      opt_needRestart();
   }

   /* Faders. */
   conf.sound = window_getFaderValue(wid, "fadSound");
   conf.music = window_getFaderValue(wid, "fadMusic");

   return 0;
}


/**
 * @brief Sets the audio defaults.
 */
static void opt_audioDefaults( unsigned int wid, char *str )
{
   (void) str;

   /* Set defaults. */
   /* Faders. */
   window_faderValue( wid, "fadSound", SOUND_VOLUME_DEFAULT );
   window_faderValue( wid, "fadMusic", MUSIC_VOLUME_DEFAULT );

   /* Checkboxes. */
   window_checkboxSet( wid, "chkNosound", MUTE_SOUND_DEFAULT );
   window_checkboxSet( wid, "chkEFX", USE_EFX_DEFAULT );

   /* List. */
   toolkit_setList( wid, "lstSound",
         (conf.sound_backend==NULL) ? _("none") : BACKEND_DEFAULT );
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

   /* Backend box */
   /* TODO */
}


/**
 * @brief Tries to set the key from an event.
 */
static int opt_setKeyEvent( unsigned int wid, SDL_Event *event )
{
   unsigned int parent;
   KeybindType type;
   int key, test_key_event;
   SDLMod mod, ev_mod;
   const char *str;

   /* See how to handle it. */
   switch (event->type) {
      case SDL_KEYDOWN:
         key  = event->key.keysym.sym;
         /* If control key make player hit twice. */
         test_key_event = (key == SDLK_NUMLOCK) ||
                          (key == SDLK_CAPSLOCK) ||
                          (key == SDLK_SCROLLOCK) ||
                          (key == SDLK_RSHIFT) ||
                          (key == SDLK_LSHIFT) ||
                          (key == SDLK_RCTRL) ||
                          (key == SDLK_LCTRL) ||
                          (key == SDLK_RALT) ||
                          (key == SDLK_LALT) ||
                          (key == SDLK_RMETA) ||
                          (key == SDLK_LMETA);
#if !SDL_VERSION_ATLEAST(2,0,0) /* SUPER don't exist in 2.0.0 */
         test_key_event = test_key_event ||
                          (key == SDLK_LSUPER) ||
                          (key == SDLK_RSUPER);
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */                 
         if (test_key_event  && (opt_lastKeyPress != key)) {
            opt_lastKeyPress = key;
            return 0;
         }
         type = KEYBIND_KEYBOARD;
         if (window_checkboxState( wid, "chkAny" ))
            mod = NMOD_ALL;
         else {
            ev_mod = event->key.keysym.mod;
            mod    = 0;
            if (ev_mod & (KMOD_LSHIFT | KMOD_RSHIFT))
               mod |= NMOD_SHIFT;
            if (ev_mod & (KMOD_LCTRL | KMOD_RCTRL))
               mod |= NMOD_CTRL;
            if (ev_mod & (KMOD_LALT | KMOD_RALT))
               mod |= NMOD_ALT;
            if (ev_mod & (KMOD_LMETA | KMOD_RMETA))
               mod |= NMOD_META;
         }
         /* Set key. */
         opt_lastKeyPress = key;
         break;

      case SDL_JOYAXISMOTION:
         if (event->jaxis.value > 0)
            type = KEYBIND_JAXISPOS;
         else if (event->jaxis.value < 0)
            type = KEYBIND_JAXISNEG;
         else
            return 0; /* Not handled. */
         key  = event->jaxis.axis;
         mod  = NMOD_ALL;
         break;

      case SDL_JOYBUTTONDOWN:
         type = KEYBIND_JBUTTON;
         key  = event->jbutton.button;
         mod  = NMOD_ALL;
         break;

      /* Not handled. */
      default:
         return 0;
   }

   /* Warn if already bound. */
   str = input_keyAlreadyBound( type, key, mod );
   if ((str != NULL) && strcmp(str, opt_selectedKeybind))
      dialogue_alert( _("Key '%s' overlaps with key '%s' that was just set. "
            "You may want to correct this."),
            str, opt_selectedKeybind );

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
static void opt_setKey( unsigned int wid, char *str )
{
   (void) wid;
   (void) str;
   unsigned int new_wid;
   int w, h;

   /* Reset key. */
   opt_lastKeyPress = 0;

   /* Create new window. */
   w = 20 + 2*(BUTTON_WIDTH + 20);
   h = 20 + BUTTON_HEIGHT + 20 + 20 + 80 + 40;
   new_wid = window_create( "Set Keybinding", -1, -1, w, h );
   window_handleEvents( new_wid, opt_setKeyEvent );
   window_setParent( new_wid, wid );

   /* Set text. */
   window_addText( new_wid, 20, -40, w-40, 60, 0, "txtInfo",
         &gl_smallFont, &cBlack,
         _("To use a modifier key hit that key twice in a row, otherwise it "
         "will register as a modifier. To set with any modifier click the checkbox.") );

   /* Create button to cancel. */
   window_addButton( new_wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnCancel", _("Cancel"), window_close );

   /* Button to unset. */
   window_addButton( new_wid,  20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnUnset",  _("Unset"), opt_unsetKey );

   /* Checkbox to set any modifier. */
   window_addCheckbox( new_wid, -20, 20 + BUTTON_HEIGHT + 20, w-40, 20,
         "chkAny", _("Set any modifier"), NULL, 0 );
}


/**
 * @brief Unsets the key.
 */
static void opt_unsetKey( unsigned int wid, char *str )
{
   (void) str;
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
 * @brief Initializes the video window.
 */
static void opt_video( unsigned int wid )
{
   (void) wid;
   int i, j, nres, res_def;
   char buf[16];
   int cw;
   int w, h, y, x, l;
   char **res;
   const char *s;

   /* Get size. */
   window_dimWindow( wid, &w, &h );

   /* Close button */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", _("OK"), opt_OK );
   window_addButton( wid, -20 - 1*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnCancel", _("Cancel"), opt_close );
   window_addButton( wid, -20 - 2*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDefaults", _("Defaults"), opt_videoDefaults );

   /* Resolution bits. */
   cw = (w-60)/2;
   x = 20;
   y = -60;
   window_addText( wid, x+20, y, 100, 20, 0, "txtSRes",
         NULL, &cDConsole, _("Resolution") );
   y -= 40;
   window_addInput( wid, x, y, 100, 20, "inpRes", 16, 1, NULL );
   window_setInputFilter( wid, "inpRes",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   window_addCheckbox( wid, x+20+100, y, 100, 20,
         "chkFullscreen", _("Fullscreen"), NULL, conf.fullscreen );
   y -= 30;
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_DisplayMode mode;
   int k;
   int n = SDL_GetNumDisplayModes( 0 );
   j = 1;
   for (i=0; i<n; i++) {
      SDL_GetDisplayMode( 0, i, &mode  );
      if ((mode.w == conf.width) && (mode.h == conf.height))
         j = 0;
   }
   res   = malloc( sizeof(char*) * (i+j) );
   nres  = 0;
   res_def = 0;
   if (j) {
      res[0]   = malloc(16);
      nsnprintf( res[0], 16, "%dx%d", conf.width, conf.height );
      nres     = 1;
   }
   for (i=0; i<n; i++) {
      SDL_GetDisplayMode( 0, i, &mode  );
      res[ nres ] = malloc(16);
      nsnprintf( res[ nres ], 16, "%dx%d", mode.w, mode.h );

      /* Make sure doesn't already exist. */
      for (k=0; k<nres; k++)
         if (strcmp( res[k], res[nres] )==0)
            break;
      if (k<nres)
         continue;

      /* Add as default if necessary and increment. */
      if ((mode.w == conf.width) && (mode.h == conf.height))
         res_def = i;
      nres++;
   }
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   SDL_Rect** modes = SDL_ListModes( NULL, SDL_OPENGL | SDL_FULLSCREEN );
   j = 1;
   for (i=0; modes[i]; i++) {
      if ((modes[i]->w == conf.width) && (modes[i]->h == conf.height))
         j = 0;
   }
   res   = malloc( sizeof(char*) * (i+j) );
   nres  = 0;
   res_def = 0;
   if (j) {
      res[0]   = malloc(16);
      nsnprintf( res[0], 16, "%dx%d", conf.width, conf.height );
      nres     = 1;
   }
   for (i=0; modes[i]; i++) {
      res[ nres ] = malloc(16);
      nsnprintf( res[ nres ], 16, "%dx%d", modes[i]->w, modes[i]->h );
      if ((modes[i]->w == conf.width) && (modes[i]->h == conf.height))
         res_def = i;
      nres++;
   }
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
   window_addList( wid, x, y, 140, 100, "lstRes", res, nres, -1, opt_videoRes );
   y -= 120;
   window_addText( wid, x, y-3, 110, 20, 0, "txtScale",
         NULL, &cBlack, NULL );
   window_addFader( wid, x+120, y, cw-140, 20, "fadScale", 1., 3.,
         conf.scalefactor, opt_setScalefactor );
   opt_setScalefactor( wid, "fadScale" );
   y -= 60;

   /* FPS stuff. */
   window_addText( wid, x+20, y, 100, 20, 0, "txtFPSTitle",
         NULL, &cDConsole, _("FPS Control") );
   y -= 30;
   s = _("FPS Limit");
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtSFPS",
         NULL, &cBlack, s );
   window_addInput( wid, x+l+20, y, 40, 20, "inpFPS", 4, 1, NULL );
   toolkit_setListPos( wid, "lstRes", res_def);
   window_setInputFilter( wid, "inpFPS",
         "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   nsnprintf( buf, sizeof(buf), "%d", conf.fps_max );
   window_setInput( wid, "inpFPS", buf );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkFPS", _("Show FPS"), NULL, conf.fps_show );

   /* Sets inpRes to current resolution, must be after lstRes is added. */
   opt_resize();

   /* OpenGL options. */
   x = 20+cw+20;
   y = -60;
   window_addText( wid, x+20, y, 100, 20, 0, "txtSGL",
         NULL, &cDConsole, _("OpenGL") );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkVSync", _("Vertical Sync"), NULL, conf.vsync );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkVBO", _("Vertex Buffer Objects*"), NULL, conf.vbo );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkMipmaps", _("Mipmaps*"), NULL, conf.mipmaps );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkInterpolate", _("Interpolation*"), NULL, conf.interpolate );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkNPOT", _("NPOT Textures*"), NULL, conf.npot );
   y -= 30;
   window_addText( wid, x, y, cw, 20, 1,
         "txtSCompat", NULL, &cBlack, _("*Disable for compatibility.") );
   y -= 40;

   /* Features. */
   window_addText( wid, x+20, y, 100, 20, 0, "txtSFeatures",
         NULL, &cDConsole, _("Features") );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkEngineGlow", _("Engine Glow (More RAM)"), NULL, conf.engineglow );

#if SDL_VERSION_ATLEAST(2,0,0)
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkMinimize", _("Minimize on focus loss"), NULL, conf.minimize );
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

   /* Restart text. */
   window_addText( wid, 20, 10, 3*(BUTTON_WIDTH + 20),
         30, 0, "txtRestart", &gl_smallFont, &cBlack, NULL );
}

/**
 * @brief Marks that needs restart.
 */
static void opt_needRestart (void)
{
   const char *s;

   /* Values. */
   opt_restart = 1;
   s           = _("Restart Naev for changes to take effect.");

   /* Modify widgets. */
   window_modifyText( opt_windows[ OPT_WIN_GAMEPLAY ], "txtRestart", s );
   window_modifyText( opt_windows[ OPT_WIN_VIDEO ], "txtRestart", s );
   window_modifyText( opt_windows[ OPT_WIN_AUDIO ], "txtRestart", s );
}


/**
 * @brief Callback when resolution changes.
 */
static void opt_videoRes( unsigned int wid, char *str )
{
   char *buf;
   buf = toolkit_getList( wid, str );
   window_setInput( wid, "inpRes", buf );
}


/**
 * @brief Saves the video settings.
 */
static int opt_videoSave( unsigned int wid, char *str )
{
   (void) str;
   int i, j, s;
   char *inp, buf[16], width[16], height[16];
   int w, h, f, fullscreen;

   /* Handle resolution. */
   inp = window_getInput( wid, "inpRes" );
   memset( width, '\0', sizeof(width) );
   memset( height, '\0', sizeof(height) );
   j = 0;
   s = 0;
   for (i=0; i<16; i++) {
      if (isdigit(inp[i])) {
         if (j==0)
            width[s++] = inp[i];
         else
            height[s++] = inp[i];
      }
      else {
         j++;
         s = 0;
      }
   }
   w = atoi(width);
   h = atoi(height);
   if ((w==0) || (h==0)) {
      dialogue_alert( _("Height/Width invalid. Should be formatted like 1024x768.") );
      return 1;
   }

   /* Fullscreen. */
   fullscreen = window_checkboxState( wid, "chkFullscreen" );

#if SDL_VERSION_ATLEAST(2,0,0)
   int origw, origh, origf, mode, changed;
   int rw, rh, nw, nh; /* Real width and height. */
   SDL_DisplayMode current;

   changed = 0;
   SDL_GetWindowSize( gl_screen.window, &rw, &rh );
   SDL_GetWindowDisplayMode( gl_screen.window, &current );
   mode = (conf.modesetting) ?
         SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;

   origw = conf.width;
   origh = conf.height;
   origf = conf.fullscreen;

   if ((w != conf.width) || (h != conf.height)) {
      conf.explicit_dim = 1;
      conf.width  = w;
      conf.height = h;
   }

   /* Enable or disable fullscreen. */
   if (fullscreen != conf.fullscreen) {
      conf.fullscreen = fullscreen;
      changed = 1;

      if (fullscreen) {
         if (conf.modesetting) {
            current.w = w;
            current.h = h;

            SDL_SetWindowDisplayMode( gl_screen.window, &current );
         }
         SDL_SetWindowFullscreen( gl_screen.window, mode );
      }
      else /* Restore windowed mode. */
         SDL_SetWindowFullscreen( gl_screen.window, 0 );
   }

   /* Attempt to detect maximized state (doesn't work on X11) */
   if (SDL_GetWindowFlags(gl_screen.window) & SDL_WINDOW_MAXIMIZED)
      dialogue_alert(_("Resolution can't be changed while maximized."));
   /* Set size. Done second, because it can't be set while fullscreen. */
   else if ((w != rw) || (h != rh)) {
      /* Can't change window size while fullscreen. */
      if (fullscreen && origf)
         opt_needRestart();
      else if (!fullscreen) {
         SDL_SetWindowSize( gl_screen.window, w, h );
         naev_resize( w, h );
         SDL_SetWindowPosition( gl_screen.window,
               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );

         changed = 1;
      }
   }

   /* Desktop fullscreen size must be determined dynamically. */
   if (fullscreen && !conf.modesetting)
      SDL_GetWindowSize( gl_screen.window, &nw, &nh );
   else {
      nw = conf.width;
      nh = conf.height;
   }

   /* Settings have changed, switch and offer to reset. */
   if (changed && !dialogue_YesNo(_("Keep Video Settings"),
         _("Do you want to keep running at %dx%d %s?"),
         nw, nh, fullscreen ? _("fullscreen") : _("windowed"))) {
      conf.width      = origw;
      conf.height     = origh;
      conf.fullscreen = origf;
      window_checkboxSet( wid, "chkFullscreen", conf.fullscreen );

      /* Restore previous resolution. */
      if ((w != rw) || (h != rw)) {
         SDL_SetWindowSize( gl_screen.window, rw, rh );
         naev_resize( rw, rh );
         SDL_SetWindowPosition( gl_screen.window,
               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
      }

      /* Restore windowed mode. */
      if (fullscreen && (fullscreen != conf.fullscreen))
         SDL_SetWindowFullscreen( gl_screen.window, 0 );
      else if (!fullscreen && (fullscreen != conf.fullscreen)) {
         if  (conf.modesetting) {
            current.w = origw;
            current.h = origh;

            SDL_SetWindowDisplayMode( gl_screen.window, &current );
         }
         SDL_SetWindowFullscreen( gl_screen.window, mode );
      }

      nsnprintf( buf, sizeof(buf), "%dx%d", conf.width, conf.height );
      window_setInput( wid, "inpRes", buf );

      dialogue_msg( _("Video Settings Restored"),
            _("Resolution reset to %dx%d %s."),
            rw, rh, conf.fullscreen ? _("fullscreen") : _("windowed") );

      return 1;
   }
   else if (changed) {
      nsnprintf( buf, sizeof(buf), "%dx%d", conf.width, conf.height );
      window_setInput( wid, "inpRes", buf );
   }
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   if ((w != conf.width) || (h != conf.height)) {
      conf.explicit_dim = 1;
      conf.width  = w;
      conf.height = h;
      opt_needRestart();
      nsnprintf( buf, sizeof(buf), "%dx%d", conf.width, conf.height );
      window_setInput( wid, "inpRes", buf );
   }

   if (conf.fullscreen != fullscreen) {
      conf.fullscreen = fullscreen;
      opt_needRestart();
   }
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

   /* FPS. */
   conf.fps_show = window_checkboxState( wid, "chkFPS" );
   inp = window_getInput( wid, "inpFPS" );
   conf.fps_max = atoi(inp);

   /* OpenGL. */
   f = window_checkboxState( wid, "chkVSync" );
   if (conf.vsync != f) {
      conf.vsync = f;
      opt_needRestart();
   }
   f = window_checkboxState( wid, "chkVBO" );
   if (conf.vbo != f) {
      conf.vbo = f;
      opt_needRestart();
   }
   f = window_checkboxState( wid, "chkMipmaps" );
   if (conf.mipmaps != f) {
      conf.mipmaps = f;
      opt_needRestart();
   }
   f = window_checkboxState( wid, "chkInterpolate" );
   if (conf.interpolate != f) {
      conf.interpolate = f;
      opt_needRestart();
   }
   f = window_checkboxState( wid, "chkNPOT" );
   if (conf.npot != f) {
      conf.npot = f;
      opt_needRestart();
   }

   /* Features. */
   f = window_checkboxState( wid, "chkEngineGlow" );
   if (conf.engineglow != f) {
      conf.engineglow = f;
      opt_needRestart();
   }
   f = window_checkboxState( wid, "chkMinimize" );
   if (conf.minimize != f) {
      conf.minimize = f;
#if SDL_VERSION_ATLEAST(2,0,0)
      SDL_SetHint( SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS,
            conf.minimize ? "1" : "0" );
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
   }

   return 0;
}

/**
 * @brief Sets video defaults.
 */
static void opt_videoDefaults( unsigned int wid, char *str )
{
   (void) str;
   char buf[16];

   /* Restore settings. */
   /* Inputs. */
   nsnprintf( buf, sizeof(buf), "%dx%d", RESOLUTION_W_DEFAULT, RESOLUTION_H_DEFAULT );
   window_setInput( wid, "inpRes", buf );
   nsnprintf( buf, sizeof(buf), "%d", FPS_MAX_DEFAULT );
   window_setInput( wid, "inpFPS", buf );

   /* Checkboxes. */
   window_checkboxSet( wid, "chkFullscreen", FULLSCREEN_DEFAULT );
   window_checkboxSet( wid, "chkVSync", VSYNC_DEFAULT );
   window_checkboxSet( wid, "chkVBO", VBO_DEFAULT );
   window_checkboxSet( wid, "chkMipmaps", MIPMAP_DEFAULT );
   window_checkboxSet( wid, "chkInterpolate", INTERPOLATION_DEFAULT );
   window_checkboxSet( wid, "chkNPOT", NPOT_TEXTURES_DEFAULT );
   window_checkboxSet( wid, "chkFPS", SHOW_FPS_DEFAULT );
   window_checkboxSet( wid, "chkEngineGlow", ENGINE_GLOWS_DEFAULT );
   window_checkboxSet( wid, "chkMinimize", MINIMIZE_DEFAULT );

   /* Faders. */
   window_faderValue(  wid, "fadScale", SCALE_FACTOR_DEFAULT );
}

/**
 * @brief Callback to set the scaling factor.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 *    @param type 0 for sound, 1 for audio.
 */
static void opt_setScalefactor( unsigned int wid, char *str )
{
   char buf[32];
   double scale = window_getFaderValue(wid, str);
   if (fabs(conf.scalefactor-scale) > 1e-4)
      opt_needRestart();
   conf.scalefactor = scale;
   nsnprintf( buf, sizeof(buf), _("Scaling: %.1fx"), conf.scalefactor );
   window_modifyText( wid, "txtScale", buf );
}
