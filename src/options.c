/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file options.c
 *
 * @brief Options menu
 */


#include "options.h"

#include <string.h>

#include "SDL.h"

#include "log.h"
#include "naev.h"
#include "input.h"
#include "toolkit.h"
#include "sound.h"
#include "music.h"


#define KEYBINDS_WIDTH  440 /**< Options menu width. */
#define KEYBINDS_HEIGHT 300 /**< Options menu height. */
#define AUDIO_WIDTH  340 /**< Options menu width. */
#define AUDIO_HEIGHT 200 /**< Options menu height. */

#define BUTTON_WIDTH    90 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */


/*
 * External stuff.
 */
extern const char *keybindNames[]; /**< from input.c */


/*
 * prototypes
 */
static int opt_isalpha( SDLKey k );
static const char* modToText( SDLMod mod );
static void menuKeybinds_update( unsigned int wid, char *name );
static void opt_setSFXLevel( unsigned int wid, char *str );
static void opt_setMusicLevel( unsigned int wid, char *str );


/**
 * @brief Checks to see if a key is alpha.
 *
 *    @param k Key to check.
 *    @return 1 if is alpha.
 */
static int opt_isalpha( SDLKey k )
{
   int ret;

   ret = 0;

   /* Alpha. */
   if ((k >= SDLK_a) && (k <= SDLK_z))
      ret = 1;

   return ret;
}


/**
 * @brief Opens the keybindings menu.
 */
void opt_menuKeybinds (void)
{
   int i, j;
   unsigned int wid;
   char **str;
   SDLKey key;
   KeybindType type;
   SDLMod mod;
   int reverse;

   /* Create the window. */
   wid = window_create( "Keybindings", -1, -1, KEYBINDS_WIDTH, KEYBINDS_HEIGHT );
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   /* Text stuff. */
   window_addText( wid, 240, -40, KEYBINDS_WIDTH-260, 30, 1, "txtName",
         NULL, &cDConsole, NULL );
   window_addText( wid, 240, -90,
         KEYBINDS_WIDTH-260, KEYBINDS_HEIGHT-70-60-BUTTON_HEIGHT,
         0, "txtDesc", &gl_smallFont, NULL, NULL );

   /* Create the list. */
   for (i=0; strcmp(keybindNames[i],"end"); i++);
   str = malloc(sizeof(char*) * i);
   for (j=0; j < i; j++) {
      str[j] = malloc(sizeof(char) * 64);
      key = input_getKeybind( keybindNames[j], &type, &mod, &reverse );
      switch (type) {
         case KEYBIND_KEYBOARD:
            /* SDL_GetKeyName returns lowercase which is ugly. */
            if (opt_isalpha(key))
               snprintf(str[j], 64, "%s <%c>", keybindNames[j], toupper(key) );
            else
               snprintf(str[j], 64, "%s <%s>", keybindNames[j], SDL_GetKeyName(key) );
            break;
         case KEYBIND_JAXIS:
            snprintf(str[j], 64, "%s <jb%d>", keybindNames[j], key);
            break;
         case KEYBIND_JBUTTON:
            snprintf(str[j], 64, "%s <ja%d>", keybindNames[j], key);
            break;
         default:
            snprintf(str[j], 64, "%s", keybindNames[j]);
            break;
      }
   }
   window_addList( wid, 20, -40, 200, KEYBINDS_HEIGHT-60, "lstKeybinds",
         str, i, 0, menuKeybinds_update );

   /* Update the list. */
   menuKeybinds_update( wid, NULL );
}


/**
 * @brief Gets the human readable version of mod.
 *
 *    @brief mod Mod to get human readable version from.
 *    @return Human readable version of mod.
 */
static const char* modToText( SDLMod mod )
{
   switch (mod) {
      case KMOD_LCTRL:  return "lctrl";
      case KMOD_RCTRL:  return "rctrl";
      case KMOD_LSHIFT: return "lshift";
      case KMOD_RSHIFT: return "rshift";
      case KMOD_LALT:   return "lalt";
      case KMOD_RALT:   return "ralt";
      case KMOD_LMETA:  return "lmeta";
      case KMOD_RMETA:  return "rmeta";
      case KMOD_ALL:    return "any";
      default:          return "unknown";
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
   int i;
   char *selected, keybind[32];
   const char *desc;
   SDLKey key;
   KeybindType type;
   SDLMod mod;
   int reverse;
   char buf[1024];
   char bind[32];

   /* Get the keybind. */
   selected = toolkit_getList( wid, "lstKeybinds" );

   /* Remove the excess. */
   for (i=0; (selected[i] != '\0') && (selected[i] != ' '); i++)
      keybind[i] = selected[i];
   keybind[i] = '\0';
   window_modifyText( wid, "txtName", keybind );

   /* Get information. */
   desc = input_getKeybindDescription( keybind );
   key = input_getKeybind( keybind, &type, &mod, &reverse );

   /* Create the text. */
   switch (type) {
      case KEYBIND_NULL:
         snprintf(bind, 64, "Not bound");
         break;
      case KEYBIND_KEYBOARD:
         /* SDL_GetKeyName returns lowercase which is ugly. */
         if (opt_isalpha(key))
            snprintf(bind, 32, "keyboard:   %s%s%c",
                  (mod != KMOD_NONE) ? modToText(mod) : "",
                  (mod != KMOD_NONE) ? " + " : "",
                  toupper(key));
         else
            snprintf(bind, 32, "keyboard:   %s%s%s",
                  (mod != KMOD_NONE) ? modToText(mod) : "",
                  (mod != KMOD_NONE) ? " + " : "",
                  SDL_GetKeyName(key));
         break;
      case KEYBIND_JAXIS:
         snprintf(bind, 64, "joy axis:   <%d>%s", key, (reverse) ? " rev" : "");
         break;
      case KEYBIND_JBUTTON:
         snprintf(bind, 64, "joy button:   <%d>", key);
         break;
   }

   /* Update text. */
   snprintf(buf, 1024, "%s\n\n%s\n", desc, bind);
   window_modifyText( wid, "txtDesc", buf );
}


/**
 * @brief Callback to set the sound level.
 */
static void opt_setSFXLevel( unsigned int wid, char *str )
{
	double vol;
   vol = window_getFaderValue(wid, str);
	sound_volume(vol);
}


/**
 * @brief Callback to set the music level.
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
            sound_defVolume, opt_setSFXLevel );
      window_addText( wid, 200, -40, AUDIO_WIDTH-220, 20, 1, "txtSound",
            NULL, NULL, "Sound Volume" );
   }
   else
      window_addText( wid, 200, -40, AUDIO_WIDTH-220, 20, 1, "txtSound",
            NULL, NULL, "Sound Disabled" );

   /* Music fader. */
   if (!music_disabled) {
      window_addFader( wid, 20, -80, 160, 20, "fadMusic", 0., 1.,
            music_defVolume, opt_setMusicLevel );
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


