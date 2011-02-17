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

#include <string.h>

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


static char opt_selectedKeybind[32]; /**< Selected keybinding. */
static int opt_lastKeyPress = 0; /**< Last keypress. */


/*
 * prototypes
 */
/* Misc. */
static void opt_close( unsigned int wid, char *name );
static void opt_needRestart (void);
/* Gameplay. */
static void opt_gameplay( unsigned int wid );
static void opt_gameplaySave( unsigned int wid, char *str );
static void opt_gameplayDefaults( unsigned int wid, char *str );
static void opt_gameplayUpdate( unsigned int wid, char *str );
/* Video. */
static void opt_video( unsigned int wid );
static void opt_videoRes( unsigned int wid, char *str );
static void opt_videoSave( unsigned int wid, char *str );
static void opt_videoDefaults( unsigned int wid, char *str );
static void opt_videoUpdate( unsigned int wid, char *str );
/* Audio. */
static void opt_audio( unsigned int wid );
static void opt_audioSave( unsigned int wid, char *str );
static void opt_audioDefaults( unsigned int wid, char *str );
static void opt_audioUpdate( unsigned int wid, char *str );
static void opt_setSFXLevel( unsigned int wid, char *str );
static void opt_setMusicLevel( unsigned int wid, char *str );
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
   h = 500;

   /* Create window and tabs. */
   opt_wid = window_create( "Options", -1, -1, w, h );
   window_setCancel( opt_wid, opt_close );

   /* Create tabbed window. */
   opt_windows = window_addTabbedWindow( opt_wid, -1, -1, -1, -1, "tabOpt",
         OPT_WINDOWS, opt_names );

   /* Load tabs. */
   opt_gameplay(  opt_windows[ OPT_WIN_GAMEPLAY ] );
   opt_video(     opt_windows[ OPT_WIN_VIDEO ] );
   opt_audio(     opt_windows[ OPT_WIN_AUDIO ] );
   opt_keybinds(  opt_windows[ OPT_WIN_INPUT ] );

   /* Set as need restart if needed. */
   if (opt_restart)
      opt_needRestart();
}
static void opt_close( unsigned int wid, char *name )
{
   (void) wid;
   (void) name;
   window_destroy( opt_wid );
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
         "btnClose", "Close", opt_close );
   window_addButton( wid, -20 - 1*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnApply", "Apply", opt_gameplaySave );
   window_addButton( wid, -20 - 2*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDefaults", "Defaults", opt_gameplayDefaults );

   /* Information. */
   cw = (w-40);
   x = 20;
   y = -60;
   window_addText( wid, x, y, cw, 20, 1, "txtVersion",
         NULL, NULL, naev_version(1) );
   y -= 20;
#ifdef GIT_COMMIT
   window_addText( wid, x, y, cw, 20, 1, "txtCommit",
         NULL, NULL, "Commit: "GIT_COMMIT );
#endif /* GIT_COMMIT */
   y -= 20;
   path = ndata_getPath();
   if (path == NULL)
      snprintf( buf, sizeof(buf), "not using ndata" );
   else
      snprintf( buf, sizeof(buf), "ndata: %s", path);
   window_addText( wid, x, y, cw, 20, 1, "txtNdata",
         NULL, NULL, buf );
   y -= 50;
   by = y;


   /* Compiletime stuff. */
   cw = (w-60)/2;
   y  = by;
   x  = 20;
   window_addText( wid, x+20, y, cw, 20, 0, "txtCompile",
         NULL, &cDConsole, "Compilation Flags" );
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
#elif defined(MACOSX)
         "Mac OS X\n"
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
#ifdef NDATA_DEF
         "ndata: "NDATA_DEF"\n"
#endif /* NDATA_DEF */
#ifdef PREFSDIR_DEF
         "preference directory: "PREFSDIR_DEF"\n"
#endif /* PREFSDIR_DEF */
         );


   /* Options. */
   y  = by;
   x += cw;
   window_addText( wid, x+20, y, cw, 20, 0, "txtSettings",
         NULL, &cDConsole, "Settings" );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkZoomManual", "Enable manual zoom control", NULL, conf.zoom_manual );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkAfterburn", "Enable doubletap afterburn", NULL, conf.afterburn_sens );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkCompress", "Enable savegame compression", NULL, conf.save_compress );
   y -= 50;
   s = "Visible messages";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtSMSG",
         NULL, &cBlack, s );
   window_addInput( wid, x+l+20, y, 40, 20, "inpMSG", 4, 1, NULL );
   y -= 20;

   /* Restart text. */
   window_addText( wid, -20, 20+BUTTON_HEIGHT+20, 3*(BUTTON_WIDTH + 20),
         30, 1, "txtRestart", &gl_smallFont, &cBlack, NULL );

   /* Update. */
   opt_gameplayUpdate( wid, NULL );
}

/**
 * @brief Saves the gameplay options.
 */
static void opt_gameplaySave( unsigned int wid, char *str )
{
   (void) str;
   int f;
   char *inp;

   /* Checkboxes. */
   f = window_checkboxState( wid, "chkAfterburn" );
   if (!!conf.afterburn_sens != f) {
      conf.afterburn_sens = (!!f)*250;
   }
   conf.zoom_manual = window_checkboxState( wid, "chkZoomManual" );
   conf.save_compress = window_checkboxState( wid, "chkCompress" );

   /* Input boxes. */
   inp = window_getInput( wid, "inpMSG" );
   conf.mesg_visible = atoi(inp);
   if (conf.mesg_visible == 0)
      conf.mesg_visible = 5;
}

/**
 * @brief Sets the default gameplay options.
 */
static void opt_gameplayDefaults( unsigned int wid, char *str )
{
   (void) str;

   /* Ask user. */
   if (!dialogue_YesNoRaw( "Restore Defaults", "Are you sure you want to restore default gameplay settings?" ))
      return;

   /* Restore. */
   conf_setGameplayDefaults();
   opt_gameplayUpdate( wid, NULL );

   /* Alert user it worked. */
   dialogue_msgRaw( "Defaults Restored", "Gameplay settings restored to defaults.");
}

/**
 * @brief Updates the gameplay options.
 */
static void opt_gameplayUpdate( unsigned int wid, char *str )
{
   (void) str;
   char buf[16];

   /* Checkboxes. */
   window_checkboxSet( wid, "chkZoomManual", conf.zoom_manual );
   window_checkboxSet( wid, "chkAfterburn", conf.afterburn_sens );
   window_checkboxSet( wid, "chkCompress", conf.save_compress );

   /* Input boxes. */
   snprintf( buf, sizeof(buf), "%d", conf.mesg_visible );
   window_setInput( wid, "inpMSG", buf );
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
         "btnClose", "Close", opt_close );
   /* Set button. */
   window_addButton( wid, -20 - bw - 20, 20, bw, bh,
         "btnSet", "Set Key", opt_setKey );
   /* Restore deafaults button. */
   window_addButton( wid, -20, 20+bh+20, bw, bh,
         "btnDefaults", "Defaults", opt_keyDefaults );

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

   /* Get dimensions. */
   menuKeybinds_getDim( wid, &w, &h, &lw, &lh, NULL, NULL );

   /* Create the list. */
   for (i=0; strcmp(keybind_info[i][0],"end"); i++);
   str = malloc(sizeof(char*) * i);
   for (j=0; j < i; j++) {
      l = 64;
      str[j] = malloc(sizeof(char) * l);
      key = input_getKeybind( keybind_info[j][0], &type, &mod );
      switch (type) {
         case KEYBIND_KEYBOARD:
            /* Generate mod text. */
            if (mod == NMOD_ALL)
               snprintf( mod_text, sizeof(mod_text), "any+" );
            else {
               p = 0;
               mod_text[0] = '\0';
               if (mod & NMOD_SHIFT)
                  p += snprintf( &mod_text[p], sizeof(mod_text)-p, "shift+" );
               if (mod & NMOD_CTRL)
                  p += snprintf( &mod_text[p], sizeof(mod_text)-p, "ctrl+" );
               if (mod & NMOD_ALT)
                  p += snprintf( &mod_text[p], sizeof(mod_text)-p, "alt+" );
               if (mod & NMOD_META)
                  p += snprintf( &mod_text[p], sizeof(mod_text)-p, "meta+" );
            }

            /* SDL_GetKeyName returns lowercase which is ugly. */
            if (nstd_isalpha(key))
               snprintf(str[j], l, "%s <%s%c>", keybind_info[j][1], mod_text, nstd_toupper(key) );
            else
               snprintf(str[j], l, "%s <%s%s>", keybind_info[j][1], mod_text, SDL_GetKeyName(key) );
            break;
         case KEYBIND_JAXISPOS:
            snprintf(str[j], l, "%s <ja+%d>", keybind_info[j][1], key);
            break;
         case KEYBIND_JAXISNEG:
            snprintf(str[j], l, "%s <ja-%d>", keybind_info[j][1], key);
            break;
         case KEYBIND_JBUTTON:
            snprintf(str[j], l, "%s <jb%d>", keybind_info[j][1], key);
            break;
         default:
            snprintf(str[j], l, "%s", keybind_info[j][1]);
            break;
      }
   }
   window_addList( wid, 20, -40, lw, lh, "lstKeybinds",
         str, i, 0, menuKeybinds_update );

   /* Update the list. */
   menuKeybinds_update( wid, NULL );
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
   window_modifyText( wid, "txtName", keybind );

   /* Get information. */
   desc = input_getKeybindDescription( keybind );
   key = input_getKeybind( keybind, &type, &mod );

   /* Create the text. */
   switch (type) {
      case KEYBIND_NULL:
         snprintf(binding, sizeof(binding), "Not bound");
         break;
      case KEYBIND_KEYBOARD:
         /* SDL_GetKeyName returns lowercase which is ugly. */
         if (nstd_isalpha(key))
            snprintf(binding, sizeof(binding), "keyboard:   %s%s%c",
                  (mod != KMOD_NONE) ? input_modToText(mod) : "",
                  (mod != KMOD_NONE) ? " + " : "",
                  nstd_toupper(key));
         else
            snprintf(binding, sizeof(binding), "keyboard:   %s%s%s",
                  (mod != KMOD_NONE) ? input_modToText(mod) : "",
                  (mod != KMOD_NONE) ? " + " : "",
                  SDL_GetKeyName(key));
         break;
      case KEYBIND_JAXISPOS:
         snprintf(binding, sizeof(binding), "joy axis pos:   <%d>", key );
         break;
      case KEYBIND_JAXISNEG:
         snprintf(binding, sizeof(binding), "joy axis neg:   <%d>", key );
         break;
      case KEYBIND_JBUTTON:
         snprintf(binding, sizeof(binding), "joy button:   <%d>", key);
         break;
   }

   /* Update text. */
   snprintf(buf, 1024, "%s\n\n%s\n", desc, binding);
   window_modifyText( wid, "txtDesc", buf );
}


/**
 * @brief Restores the key defaults.
 */
static void opt_keyDefaults( unsigned int wid, char *str )
{
   (void) str;

   /* Ask user if he wants to. */
   if (!dialogue_YesNoRaw( "Restore Defaults", "Are you sure you want to restore default keybindings?" ))
      return;

   /* Restore defaults. */
   input_setDefault();

   /* Regenerate list widget. */
   window_destroyWidget( wid, "lstKeybinds" );
   menuKeybinds_genList( wid );

   /* Alert user it worked. */
   dialogue_msgRaw( "Defaults Restored", "Keybindings restored to defaults.");
}


/**
 * @brief Callback to set the sound level.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setSFXLevel( unsigned int wid, char *str )
{
   char buf[32];
	double vol;

   /* Set fader. */
   vol = window_getFaderValue(wid, str);
	sound_volume(vol);

   /* Update message. */
   snprintf( buf, sizeof(buf), "Sound Volume: %.2f", sound_getVolume() );
   window_modifyText( wid, "txtSound", buf );
}


/**
 * @brief Callback to set the music level.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setMusicLevel( unsigned int wid, char *str )
{
   char buf[32];
   double vol;

   /* Update fader. */
   vol = window_getFaderValue(wid, str);
	music_volume(vol);

   /* Update message. */
   snprintf( buf, sizeof(buf), "Music Volume: %.2f", music_getVolume() );
   window_modifyText( wid, "txtMusic", buf );
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
   char buf[32], **s;
   const char *str;

   /* Get size. */
   window_dimWindow( wid, &w, &h );

   /* Close button */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", opt_close );
   window_addButton( wid, -20 - 1*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnApply", "Apply", opt_audioSave );
   window_addButton( wid, -20 - 2*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDefaults", "Defaults", opt_audioDefaults );

   /* General options. */
   cw = (w-60)/2;
   x = 20;
   y = -60;
   window_addText( wid, x+20, y, cw, 20, 0, "txtSGeneral",
         NULL, &cDConsole, "General" );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkNosound", "Disable all sound/music", NULL, conf.nosound );
   y -= 30;
   str = "Backends";
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
   s[i++] = strdup("openal");
#endif /* USE_PONAL */
#if USE_SDLMIX
   if (strcmp(conf.sound_backend,"sdlmix")==0)
      j = i;
   s[i++] = strdup("sdlmix");
#endif /* USE_SDLMIX */
   if (i==0)
      s[i++] = strdup("none");
   window_addList( wid, x+l, y, cw-(x+l), 40, "lstSound", s, i, j, NULL );
   y -= 50;

   /* OpenAL options. */
   window_addText( wid, x+20, y, cw, 20, 0, "txtSOpenal",
         NULL, &cDConsole, "OpenAL" );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkEFX", "EFX (More CPU)", NULL, conf.al_efx );
   y -= 20;


   /* Sound levels. */
   x = 20 + cw + 20;
   y = -60;
   window_addText( wid, x+20, y, 100, 20, 0, "txtSVolume",
         NULL, &cDConsole, "Volume Levels" );
   y -= 30;

   /* Sound fader. */
   snprintf( buf, sizeof(buf), "Sound Volume: %.2f", sound_getVolume() );
   window_addText( wid, x, y, cw, 20, 1, "txtSound",
         NULL, NULL, buf );
   y -= 20;
   window_addFader( wid, x, y, cw, 20, "fadSound", 0., 1.,
         sound_getVolume(), opt_setSFXLevel );
   y -= 40;

   /* Music fader. */
   snprintf( buf, sizeof(buf), "Music Volume: %.2f", music_getVolume() );
   window_addText( wid, x, y, cw, 20, 1, "txtMusic",
         NULL, NULL, buf );
   y -= 20;
   window_addFader( wid, x, y, cw, 20, "fadMusic", 0., 1.,
         music_getVolume(), opt_setMusicLevel );
   y -= 20;


   /* Restart text. */
   window_addText( wid, -20, 20+BUTTON_HEIGHT+20, 3*(BUTTON_WIDTH + 20),
         30, 1, "txtRestart", &gl_smallFont, &cBlack, NULL );
}

/**
 * @brief Saves the audio stuff.
 */
static void opt_audioSave( unsigned int wid, char *str )
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
}

/**
 * @brief Sets the audio defaults.
 */
static void opt_audioDefaults( unsigned int wid, char *str )
{
   (void) str;

   /* Ask user. */
   if (!dialogue_YesNoRaw( "Restore Defaults", "Are you sure you want to restore default audio settings?" ))
      return;

   /* Set defaults. */
   conf_setAudioDefaults();

   /* Have sound levels affect. */
   sound_volume(conf.sound);
	music_volume(conf.music);

   /* Update widgets. */
   opt_audioUpdate( wid, NULL );

   /* Alert user it worked. */
   dialogue_msgRaw( "Defaults Restored", "Audio settings restored to defaults.");
}

/**
 * @brief Updates the audio widgets.
 */
static void opt_audioUpdate( unsigned int wid, char *str )
{
   (void) str;

   /* Faders. */
   window_faderValue( wid, "fadSound", sound_getVolume() );
   window_faderValue( wid, "fadMusic", music_getVolume() );

   /* Checkboxkes. */
   window_checkboxSet( wid, "chkNosound", conf.nosound );
   window_checkboxSet( wid, "chkEngineGlow", conf.al_efx );

   /* List. */
   toolkit_setList( wid, "lstSound",
         (conf.sound_backend==NULL) ? "none" : conf.sound_backend );
}


/**
 * @brief Tries to set the key from an event.
 */
static int opt_setKeyEvent( unsigned int wid, SDL_Event *event )
{
   unsigned int parent;
   KeybindType type;
   int key;
   SDLMod mod, ev_mod;
   const char *str;
   int pos, off;

   /* See how to handle it. */
   switch (event->type) {
      case SDL_KEYDOWN:
         key  = event->key.keysym.sym;
         /* If control key make player hit twice. */
         if (((key == SDLK_NUMLOCK) ||
                  (key == SDLK_CAPSLOCK) ||
                  (key == SDLK_SCROLLOCK) ||
                  (key == SDLK_RSHIFT) ||
                  (key == SDLK_LSHIFT) ||
                  (key == SDLK_RCTRL) ||
                  (key == SDLK_LCTRL) ||
                  (key == SDLK_RALT) ||
                  (key == SDLK_LALT) ||
                  (key == SDLK_RMETA) ||
                  (key == SDLK_LMETA) ||
                  (key == SDLK_LSUPER) ||
                  (key == SDLK_RSUPER))
                  && (opt_lastKeyPress != key)) {
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
      dialogue_alert( "Key '%s' overlaps with key '%s' that was just set. "
            "You may want to correct this.",
            str, opt_selectedKeybind );

   /* Set keybinding. */
   input_setKeybind( opt_selectedKeybind, type, key, mod );

   /* Close window. */
   window_close( wid, NULL );

   /* Update parent window. */
   parent = window_getParent( wid );
   pos = toolkit_getListPos( parent, "lstKeybinds" );
   off = toolkit_getListOffset( parent, "lstKeybinds" );
   window_destroyWidget( parent, "lstKeybinds" );
   menuKeybinds_genList( parent );
   toolkit_setListPos( parent, "lstKeybinds", pos );
   toolkit_setListOffset( parent, "lstKeybinds", off );

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
         "To use a modifier key hit that key twice in a row, otherwise it "
         "will register as a modifier. To set with any modifier click the checkbox." );

   /* Create button to cancel. */
   window_addButton( new_wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnCancel", "Cancel", window_close );

   /* Button to unset. */
   window_addButton( new_wid,  20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnUnset",  "Unset", opt_unsetKey );

   /* Checkbox to set any modifier. */
   window_addCheckbox( new_wid, -20, 20 + BUTTON_HEIGHT + 20, w-40, 20,
         "chkAny", "Set any modifier", NULL, 0 );
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
   window_destroyWidget( parent, "lstKeybinds" );
   menuKeybinds_genList( parent );
}


/**
 * @brief Initializes the video window.
 */
static void opt_video( unsigned int wid )
{
   (void) wid;
   int i, j;
   char buf[16];
   int cw;
   int w, h, y, x, l;
   SDL_Rect** modes;
   char **res;
   const char *s;

   /* Get size. */
   window_dimWindow( wid, &w, &h );

   /* Close button */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", opt_close );
   window_addButton( wid, -20 - 1*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnApply", "Apply", opt_videoSave );
   window_addButton( wid, -20 - 2*(BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDefaults", "Defaults", opt_videoDefaults );

   /* Resolution bits. */
   cw = (w-60)/2;
   x = 20;
   y = -60;
   window_addText( wid, x+20, y, 100, 20, 0, "txtSRes",
         NULL, &cDConsole, "Resolution" );
   y -= 40;
   window_addInput( wid, x, y, 100, 20, "inpRes", 16, 1, NULL );
   snprintf( buf, sizeof(buf), "%dx%d", conf.width, conf.height );
   window_setInput( wid, "inpRes", buf );
   window_setInputFilter( wid, "inpRes",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   window_addCheckbox( wid, x+20+100, y, 100, 20,
         "chkFullscreen", "Fullscreen", NULL, conf.fullscreen );
   y -= 30;
   modes = SDL_ListModes( NULL, SDL_OPENGL | SDL_FULLSCREEN );
   j = 1;
   for (i=0; modes[i]; i++) {
      if ((modes[i]->w == conf.width) && (modes[i]->h == conf.height))
         j = 0;
   }
   res = malloc( sizeof(char*) * (i+j) );
   if (j) {
      res[0] = malloc(16);
      snprintf( res[0], 16, "%dx%d", conf.width, conf.height );
      j = 0;
   }
   for (i=0; modes[i]; i++) {
      res[i] = malloc(16);
      snprintf( res[i], 16, "%dx%d", modes[i]->w, modes[i]->h );
      if ((modes[i]->w == conf.width) && (modes[i]->h == conf.height))
         j = i;
   }
   window_addList( wid, x, y, 140, 100, "lstRes", res, i, j, opt_videoRes );
   y -= 150;


   /* FPS stuff. */
   window_addText( wid, x+20, y, 100, 20, 0, "txtFPSTitle",
         NULL, &cDConsole, "FPS Control" );
   y -= 30;
   s = "FPS Limit";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtSFPS",
         NULL, &cBlack, s );
   window_addInput( wid, x+l+20, y, 40, 20, "inpFPS", 4, 1, NULL );
   snprintf( buf, sizeof(buf), "%d", conf.fps_max );
   window_setInput( wid, "inpFPS", buf );
   window_setInputFilter( wid, "inpFPS",
         "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkFPS", "Show FPS", NULL, conf.fps_show );
   y -= 40;


   /* OpenGL options. */
   x = 20+cw+20;
   y = -60;
   window_addText( wid, x+20, y, 100, 20, 0, "txtSGL",
         NULL, &cDConsole, "OpenGL" );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkVSync", "Vertical Sync", NULL, conf.vsync );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkVBO", "Vertex Buffer Objects*", NULL, conf.vbo );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkMipmaps", "Mipmaps*", NULL, conf.mipmaps );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkInterpolate", "Interpolation*", NULL, conf.interpolate );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkNPOT", "NPOT Textures*", NULL, conf.npot );
   y -= 30;
   window_addText( wid, x, y, cw, 20, 1,
         "txtSCompat", NULL, &cBlack, "*Disable for compatibility." );
   y -= 40;
   /* Features. */
   window_addText( wid, x+20, y, 100, 20, 0, "txtSFeatures",
         NULL, &cDConsole, "Features" );
   y -= 30;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkEngineGlow", "Engine Glow (More RAM)", NULL, conf.engineglow );
   y -= 20;


   /* Restart text. */
   window_addText( wid, -20, 20+BUTTON_HEIGHT+20, 3*(BUTTON_WIDTH + 20),
         30, 1, "txtRestart", &gl_smallFont, &cBlack, NULL );
}

/**
 * @brief Marks that needs restart.
 */
static void opt_needRestart (void)
{
   const char *s;

   /* Values. */
   opt_restart = 1;
   s           = "Restart Naev for changes to take effect";

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
static void opt_videoSave( unsigned int wid, char *str )
{
   (void) str;
   int i, j, s;
   char *inp, buf[16], width[16], height[16];
   int w, h, f;

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
      dialogue_alert( "Height/Width invalid. Should be formatted like 1024x768." );
      return;
   }
   if ((w != conf.width) || (h != conf.height)) {
      conf.explicit_dim = 1;
      conf.width  = w;
      conf.height = h;
      opt_needRestart();
      snprintf( buf, sizeof(buf), "%dx%d", conf.width, conf.height );
      window_setInput( wid, "inpRes", buf );
   }

   /* Fullscreen. */
   f = window_checkboxState( wid, "chkFullscreen" );
   if (conf.fullscreen != f) {
      conf.fullscreen = f;
      opt_needRestart();
   }

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
}

/**
 * @brief Sets video defaults.
 */
static void opt_videoDefaults( unsigned int wid, char *str )
{
   (void) str;

   /* Ask user. */
   if (!dialogue_YesNoRaw( "Restore Defaults", "Are you sure you want to restore default video settings?" ))
      return;

   /* Restore settings. */
   conf_setVideoDefaults();
   opt_videoUpdate( wid, NULL );

   /* Alert user it worked. */
   dialogue_msgRaw( "Defaults Restored", "Video settings restored to defaults.");
}

static void opt_videoUpdate( unsigned int wid, char *str )
{
   (void) str;
   char buf[16];

   /* Inputs. */
   snprintf( buf, sizeof(buf), "%dx%d", conf.width, conf.height );
   window_setInput( wid, "inpRes", buf );
   snprintf( buf, sizeof(buf), "%d", conf.fps_max );
   window_setInput( wid, "inpFPS", buf );

   /* Checkboxkes. */
   window_checkboxSet( wid, "chkFullscreen", conf.fullscreen );
   window_checkboxSet( wid, "chkVSync", conf.vsync );
   window_checkboxSet( wid, "chkVBO", conf.vbo );
   window_checkboxSet( wid, "chkMipmaps", conf.mipmaps );
   window_checkboxSet( wid, "chkInterpolate", conf.interpolate );
   window_checkboxSet( wid, "chkNPOT", conf.npot );
   window_checkboxSet( wid, "chkFPS", conf.fps_show );
   window_checkboxSet( wid, "chkEngineGlow", conf.engineglow );

   /* Just in case - lazy. */
   opt_needRestart();
}


