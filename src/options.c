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


#define AUDIO_WIDTH  340 /**< Options menu width. */
#define AUDIO_HEIGHT 200 /**< Options menu height. */

#define BUTTON_WIDTH    90 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */


/*
 * External stuff.
 */
extern const char *keybindNames[]; /**< from input.c */


static char opt_selectedKeybind[32]; /**< Selected keybinding. */
static int opt_lastKeyPress = 0; /**< Last keypress. */


/*
 * prototypes
 */
/* Keybind menu. */
static void menuKeybinds_getDim( unsigned int wid, int *w, int *h,
      int *lw, int *lh, int *bw, int *bh );
static void menuKeybinds_genList( unsigned int wid );
static void menuKeybinds_update( unsigned int wid, char *name );
/* Music. */
static void opt_setSFXLevel( unsigned int wid, char *str );
static void opt_setMusicLevel( unsigned int wid, char *str );
/* Setting keybindings. */
static int opt_setKeyEvent( unsigned int wid, SDL_Event *event );
static void opt_setKey( unsigned int wid, char *str );
static void opt_unsetKey( unsigned int wid, char *str );


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
void opt_menuKeybinds (void)
{
   unsigned int wid;
   int w, h;
   int bw, bh;
   int lw;

   /* Dimensions. */
   w = 500;
   h = 300;

   /* Create the window. */
   wid = window_create( "Keybindings", -1, -1, w, h );

   menuKeybinds_getDim( wid, &w, &h, &lw, NULL, &bw, &bh );

   /* Close button. */
   window_addButton( wid, -20, 20, bw, bh,
         "btnClose", "Close", window_close );
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
void opt_menuAudio (void)
{
   unsigned int wid;

   /* Create the window. */
   wid = window_create( "Audio", -1, -1, AUDIO_WIDTH, AUDIO_HEIGHT );

   /* Sound fader. */
   if (!sound_disabled) {
      window_addFader( wid, 20, -40, 160, 20, "fadSound", 0., 1.,
            sound_getVolume(), opt_setSFXLevel );
      window_addText( wid, 200, -40, AUDIO_WIDTH-220, 20, 1, "txtSound",
            NULL, NULL, "Sound Volume" );
   }
   else
      window_addText( wid, 200, -40, AUDIO_WIDTH-220, 20, 1, "txtSound",
            NULL, NULL, "Sound Disabled" );

   /* Music fader. */
   if (!music_disabled) {
      window_addFader( wid, 20, -80, 160, 20, "fadMusic", 0., 1.,
            music_getVolume(), opt_setMusicLevel );
      window_addText( wid, 200, -80, AUDIO_WIDTH-220, 20, 1, "txtMusic",
            NULL, NULL, "Music Volume" );
   }
   else
      window_addText( wid, 200, -80, AUDIO_WIDTH-220, 20, 1, "txtMusic",
            NULL, NULL, "Music Disabled" );

   /* Close button */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );
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


