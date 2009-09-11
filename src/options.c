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


#define BUTTON_WIDTH    90 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */

#define OPT_WINDOWS     3

static unsigned int opt_wid = 0;
static unsigned int *opt_windows;
static const char *opt_names[] = {
   "Video",
   "Audio",
   "Input"
};


static int opt_restart = 0;


/*
 * External stuff.
 */
extern const char *keybindNames[]; /**< from input.c */


static char opt_selectedKeybind[32]; /**< Selected keybinding. */
static int opt_lastKeyPress = 0; /**< Last keypress. */


/*
 * prototypes
 */
/* Misc. */
static void opt_close( unsigned int wid, char *name );
static void opt_needRestart (void);
/* Keybind menu. */
static void opt_keybinds( unsigned int wid );
static void menuKeybinds_getDim( unsigned int wid, int *w, int *h,
      int *lw, int *lh, int *bw, int *bh );
static void menuKeybinds_genList( unsigned int wid );
static void menuKeybinds_update( unsigned int wid, char *name );
/* Music. */
static void opt_audio( unsigned int wid );
static void opt_audioSave( unsigned int wid, char *str );
static void opt_audioDefaults( unsigned int wid, char *str );
static void opt_audioUpdate( unsigned int wid, char *str );
static void opt_setSFXLevel( unsigned int wid, char *str );
static void opt_setMusicLevel( unsigned int wid, char *str );
/* Setting keybindings. */
static int opt_setKeyEvent( unsigned int wid, SDL_Event *event );
static void opt_setKey( unsigned int wid, char *str );
static void opt_unsetKey( unsigned int wid, char *str );
/* Video stuff. */
static void opt_video( unsigned int wid );
static void opt_videoRes( unsigned int wid, char *str );
static void opt_videoSave( unsigned int wid, char *str );
static void opt_videoDefaults( unsigned int wid, char *str );
static void opt_videoUpdate( unsigned int wid, char *str );


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
   opt_windows = window_addTabbedWindow( opt_wid, -1, -1, -1, -1, "tabOpt",
         OPT_WINDOWS, opt_names );

   /* Load tabs. */
   opt_video(     opt_windows[0] );
   opt_audio(     opt_windows[1] );
   opt_keybinds(  opt_windows[2] );

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

   /* Text stuff. */
   window_addText( wid, 20+lw+20, -40, w-(20+lw+20), 30, 1, "txtName",
         NULL, &cDConsole, NULL );
   window_addText( wid, 20+lw+20, -90, w-(20+lw+20), h-70-60-bh,
         0, "txtDesc", &gl_smallFont, NULL, NULL );

   /* Generate the list. */
   menuKeybinds_genList( wid );
}

static void menuKeybinds_genList( unsigned int wid )
{
   int i, j;
   char **str;
   SDLKey key;
   KeybindType type;
   SDLMod mod;
   int w, h;
   int lw, lh;

   /* Get dimensions. */
   menuKeybinds_getDim( wid, &w, &h, &lw, &lh, NULL, NULL );

   /* Create the list. */
   for (i=0; strcmp(keybindNames[i],"end"); i++);
   str = malloc(sizeof(char*) * i);
   for (j=0; j < i; j++) {
      str[j] = malloc(sizeof(char) * 64);
      key = input_getKeybind( keybindNames[j], &type, &mod );
      switch (type) {
         case KEYBIND_KEYBOARD:
            /* SDL_GetKeyName returns lowercase which is ugly. */
            if (nstd_isalpha(key))
               snprintf(str[j], 64, "%s <%c>", keybindNames[j], nstd_toupper(key) );
            else
               snprintf(str[j], 64, "%s <%s>", keybindNames[j], SDL_GetKeyName(key) );
            break;
         case KEYBIND_JAXISPOS:
            snprintf(str[j], 64, "%s <ja+%d>", keybindNames[j], key);
            break;
         case KEYBIND_JAXISNEG:
            snprintf(str[j], 64, "%s <ja-%d>", keybindNames[j], key);
            break;
         case KEYBIND_JBUTTON:
            snprintf(str[j], 64, "%s <jb%d>", keybindNames[j], key);
            break;
         default:
            snprintf(str[j], 64, "%s", keybindNames[j]);
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
   int i;
   char *selected, *keybind;
   const char *desc;
   SDLKey key;
   KeybindType type;
   SDLMod mod;
   char buf[1024];
   char binding[32];

   /* Get the keybind. */
   selected = toolkit_getList( wid, "lstKeybinds" );

   /* Remove the excess. */
   for (i=0; (selected[i] != '\0') && (selected[i] != ' '); i++)
      opt_selectedKeybind[i] = selected[i];
   opt_selectedKeybind[i] = '\0';
   keybind                = opt_selectedKeybind;
   window_modifyText( wid, "txtName", keybind );

   /* Get information. */
   desc = input_getKeybindDescription( keybind );
   key = input_getKeybind( keybind, &type, &mod );

   /* Create the text. */
   switch (type) {
      case KEYBIND_NULL:
         snprintf(binding, 64, "Not bound");
         break;
      case KEYBIND_KEYBOARD:
         /* SDL_GetKeyName returns lowercase which is ugly. */
         if (nstd_isalpha(key))
            snprintf(binding, 32, "keyboard:   %s%s%c",
                  (mod != KMOD_NONE) ? input_modToText(mod) : "",
                  (mod != KMOD_NONE) ? " + " : "",
                  nstd_toupper(key));
         else
            snprintf(binding, 32, "keyboard:   %s%s%s",
                  (mod != KMOD_NONE) ? input_modToText(mod) : "",
                  (mod != KMOD_NONE) ? " + " : "",
                  SDL_GetKeyName(key));
         break;
      case KEYBIND_JAXISPOS:
         snprintf(binding, 64, "joy axis pos:   <%d>", key );
         break;
      case KEYBIND_JAXISNEG:
         snprintf(binding, 64, "joy axis neg:   <%d>", key );
         break;
      case KEYBIND_JBUTTON:
         snprintf(binding, 64, "joy button:   <%d>", key);
         break;
   }

   /* Update text. */
   snprintf(buf, 1024, "%s\n\n%s\n", desc, binding);
   window_modifyText( wid, "txtDesc", buf );
}


/**
 * @brief Callback to set the sound level.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setSFXLevel( unsigned int wid, char *str )
{
	double vol;

   vol = window_getFaderValue(wid, str);
	sound_volume(vol);
}


/**
 * @brief Callback to set the music level.
 *
 *    @param wid Window calling the callback.
 *    @param str Name of the widget calling the callback.
 */
static void opt_setMusicLevel( unsigned int wid, char *str )
{
   double vol;

   vol = window_getFaderValue(wid, str);
	music_volume(vol);
}


/**
 * @brief Opens the audio settings menu.
 */
static void opt_audio( unsigned int wid )
{
   (void) wid;
   int cw;
   int w, h, y, x;
   char buf[64];

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

   /* Sound levels. */
   cw = (w-60)/2;
   x = 20;
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
   (void) wid;
}

/**
 * @brief Sets the audio defaults.
 */
static void opt_audioDefaults( unsigned int wid, char *str )
{
   (void) str;

   conf_setAudioDefaults();
   opt_audioUpdate( wid, NULL );
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
}


/**
 * @brief Tries to set the key from an event.
 */
static int opt_setKeyEvent( unsigned int wid, SDL_Event *event )
{
   unsigned int parent;
   KeybindType type;
   int key;
   SDLMod mod;
   const char *str;

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
            mod = KMOD_ALL;
         else
            mod  = event->key.keysym.mod & ~(KMOD_CAPS | KMOD_NUM | KMOD_MODE);
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
         mod  = KMOD_ALL;
         break;

      case SDL_JOYBUTTONDOWN:
         type = KEYBIND_JBUTTON;
         key  = event->jbutton.button;
         mod  = KMOD_ALL;
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
   parent = window_get("Keybindings");
   window_destroyWidget( parent, "lstKeybinds" );
   menuKeybinds_genList( parent );
   menuKeybinds_update( parent, NULL );

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
   parent = window_get("Keybindings");
   window_destroyWidget( parent, "lstKeybinds" );
   menuKeybinds_genList( parent );
   menuKeybinds_update( parent, NULL );
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
   window_addInput( wid, x, y, 100, 20, "inpRes", 16, 1 );
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
   window_addInput( wid, x+l+20, y, 40, 20, "inpFPS", 4, 1 );
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
         "chkVBO", "VBOs (Disable for compatibility)", NULL, conf.vbo );
   y -= 20;
   window_addCheckbox( wid, x, y, cw, 20,
         "chkMipmaps", "Mipmaps (Disable for compatibility)", NULL, conf.mipmaps );
   y -= 50;


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
   s           = "Restart NAEV for changes to take effect";

   /* Modify widgets. */
   window_modifyText( opt_windows[0], "txtRestart", s );
   window_modifyText( opt_windows[1], "txtRestart", s );
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

   conf_setVideoDefaults();
   opt_videoUpdate( wid, NULL );
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
   window_checkboxSet( wid, "chkFPS", conf.fps_show );
   window_checkboxSet( wid, "chkEngineGlow", conf.engineglow );

   /* Just in case - lazy. */
   opt_needRestart();
}


