/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file input.c
 *
 * @brief Handles all the keybindings and input.
 */


#include "input.h"

#include "naev.h"

#include "log.h"
#include "player.h"
#include "pause.h"
#include "toolkit.h"
#include "menu.h"
#include "board.h"
#include "map.h"
#include "escort.h"
#include "land.h"
#include "nstd.h"
#include "gui.h"
#include "weapon.h"
#include "console.h"
#include "conf.h"


#define KEY_PRESS    ( 1.) /**< Key is pressed. */
#define KEY_RELEASE  (-1.) /**< Key is released. */


/* keybinding structure */
/**
 * @brief NAEV Keybinding.
 */
typedef struct Keybind_ {
   const char *name; /**< keybinding name, taken from keybindNames */
   KeybindType type; /**< type, defined in playe.h */
   SDLKey key; /**< key/axis/button event number */
   SDLMod mod; /**< Key modifiers (where applicable). */
} Keybind;


static Keybind** input_keybinds; /**< contains the players keybindings */

/* name of each keybinding */
const char *keybindNames[] = {
   /* Movement. */
   "accel", "left", "right", "reverse", "afterburn",
  /* Targetting. */
   "target_next", "target_prev", "target_nearest",
   "target_nextHostile", "target_prevHostile", "target_hostile",
   /* Fighting. */
   "primary", "face", "board", "safety",
   /* Weapon selection. */
   "weap_all", "weap_turret", "weap_forward",
   /* Secondary weapons. */
   "secondary", "secondary_next", "secondary_prev",
   /* Escorts. */
   "e_targetNext", "e_targetPrev", "e_attack", "e_hold", "e_return", "e_clear",
   /* Space navigation. */
   "autonav", "target_planet", "land", "thyperspace", "starmap", "jump",
   /* Communication. */
   "hail",
   /* Misc. */
   "mapzoomin", "mapzoomout", "screenshot", "pause", "speed", "menu", "info",
   "console",
   /* Must terminate in "end". */
   "end"
}; /**< Names of possible keybindings. */
/*
 * Keybinding descriptions.  Should match in position the names.
 */
const char *keybindDescription[] = {
   /* Movement. */
   "Makes your ship accelerate forward.",
   "Makes your ship turn left.",
   "Makes your ship turn right.",
   "Makes your ship turn around and face the direction you're moving from.  Good for braking.",
   "Makes your ship afterburn if you have an afterburner installed.",
   /* Targetting. */
   "Cycles through ship targets.",
   "Cycles backwards through ship targets.",
   "Targets the nearest non-disabled ship.",
   "Cycles through hostile ship targets.",
   "Cycles backwards through hostile ship targets.",
   "Targets the nearest hostile ship.",
   /* Fighting. */
   "Fires your primary weapons.",
   "Faces your target (ship target if you have one, otherwise your planet target).",
   "Attempts to board your target ship.",
   "Toggles weapon safety (hitting of friendly ships).",
   /* Weapon selection. */
   "Sets fire mode to use all weapons available (both turret and forward mounts).",
   "Sets fire mode to only use turret-class primary weapons.",
   "Sets fire mode to only use forward-class primary weapons.",
   /* Secondary weapons. */
   "Fires your secondary weapon.",
   "Cycles through secondary weapons.",
   "Cycles backwards through secondary weapons.",
   /* Escorts. */
   "Cycles through your escorts.",
   "Cycles backwards through your escorts.",
   "Tells your escorts to attack your target.",
   "Tells your escorts to hold their positions.",
   "Tells your escorts to return to your ship hangars.",
   "Clears your escorts of commands.",
   /* Space navigation. */
   "Initializes the autonavigation system.",
   "Cycles through planet targets",
   "Attempts to land on your targetted planet or targets the nearest landable planet.  Requests for landing if you don't have permission yet.",
   "Cycles through hyperspace targets.",
   "Opens the Star Map.",
   "Attempts to jump to your hyperspace target.",
   /* Communication. */
   "Attempts to initialize communication with your targetted ship.",
   /* Misc. */
   "Zooms in on your radar.",
   "Zooms out on your radar.",
   "Takes a screenshot.",
   "Pauses the game.",
   "Toggles 2x speed modifier.",
   "Opens the small ingame menu.",
   "Opens the information menu.",
   "Opens the Lua console.",
   NULL /* To match sentinel. */
}; /**< Descriptions of the keybindings.  Should be in the same position as the
        matching keybinding name. */


/*
 * accel hacks
 */
static unsigned int input_accelLast = 0; /**< Used to see if double tap */


/*
 * from player.c
 */
extern double player_left; /**< player.c */
extern double player_right; /**< player.c */


/*
 * Key conversion table.
 */
#if SDL_VERSION_ATLEAST(1,3,0)
#  define INPUT_NUMKEYS     SDL_NUM_SCANCODES /**< Number of keys available. */
#else /* SDL_VERSION_ATLEAST(1,3,0) */
#  define INPUT_NUMKEYS     SDLK_LAST /**< Number of keys available. */
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
static char *keyconv[INPUT_NUMKEYS]; /**< Key conversion table. */


/*
 * Prototypes.
 */
static void input_keyConvGen (void);
static void input_keyConvDestroy (void);


/**
 * @brief Sets the default input keys.
 */
void input_setDefault (void)
{
   /* Movement. */
   input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_UP, KMOD_ALL );
   input_setKeybind( "afterburn", KEYBIND_NULL, SDLK_z, KMOD_ALL );
   input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_LEFT, KMOD_ALL );
   input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_RIGHT, KMOD_ALL );
   input_setKeybind( "reverse", KEYBIND_KEYBOARD, SDLK_DOWN, KMOD_ALL );
   /* Targetting. */
   input_setKeybind( "target_next", KEYBIND_KEYBOARD, SDLK_TAB, KMOD_NONE );
   input_setKeybind( "target_prev", KEYBIND_KEYBOARD, SDLK_TAB, KMOD_LCTRL );
   input_setKeybind( "target_nearest", KEYBIND_KEYBOARD, SDLK_t, KMOD_NONE );
   input_setKeybind( "target_nextHostile", KEYBIND_KEYBOARD, SDLK_r, KMOD_LCTRL );
   input_setKeybind( "target_prevHostile", KEYBIND_NULL, SDLK_UNKNOWN, KMOD_NONE );
   input_setKeybind( "target_hostile", KEYBIND_KEYBOARD, SDLK_r, KMOD_NONE );
   /* Combat. */
   input_setKeybind( "primary", KEYBIND_KEYBOARD, SDLK_SPACE, KMOD_ALL );
   input_setKeybind( "face", KEYBIND_KEYBOARD, SDLK_a, KMOD_ALL );
   input_setKeybind( "board", KEYBIND_KEYBOARD, SDLK_b, KMOD_NONE );
   input_setKeybind( "safety", KEYBIND_KEYBOARD, SDLK_s, KMOD_LCTRL );
   /* Weapon selection. */
   input_setKeybind( "weap_all", KEYBIND_KEYBOARD, SDLK_1, KMOD_NONE );
   input_setKeybind( "weap_turret", KEYBIND_KEYBOARD, SDLK_2, KMOD_NONE );
   input_setKeybind( "weap_forward", KEYBIND_KEYBOARD, SDLK_3, KMOD_NONE );
   /* Secondary weapons. */
   input_setKeybind( "secondary", KEYBIND_KEYBOARD, SDLK_LSHIFT, KMOD_ALL );
   input_setKeybind( "secondary_next", KEYBIND_KEYBOARD, SDLK_w, KMOD_NONE );
   input_setKeybind( "secondary_prev", KEYBIND_KEYBOARD, SDLK_w, KMOD_LCTRL );
   /* Escorts. */
   input_setKeybind( "e_targetNext", KEYBIND_KEYBOARD, SDLK_e, KMOD_NONE );
   input_setKeybind( "e_targetPrev", KEYBIND_KEYBOARD, SDLK_e, KMOD_LCTRL );
   input_setKeybind( "e_attack", KEYBIND_KEYBOARD, SDLK_f, KMOD_NONE );
   input_setKeybind( "e_hold", KEYBIND_KEYBOARD, SDLK_g, KMOD_NONE );
   input_setKeybind( "e_return", KEYBIND_KEYBOARD, SDLK_c, KMOD_LCTRL );
   input_setKeybind( "e_clear", KEYBIND_KEYBOARD, SDLK_c, KMOD_NONE );
   /* Space. */
   input_setKeybind( "autonav", KEYBIND_KEYBOARD, SDLK_j, KMOD_LCTRL );
   input_setKeybind( "target_planet", KEYBIND_KEYBOARD, SDLK_p, KMOD_NONE );
   input_setKeybind( "land", KEYBIND_KEYBOARD, SDLK_l, KMOD_NONE );
   input_setKeybind( "thyperspace", KEYBIND_KEYBOARD, SDLK_h, KMOD_NONE );
   input_setKeybind( "starmap", KEYBIND_KEYBOARD, SDLK_m, KMOD_NONE );
   input_setKeybind( "jump", KEYBIND_KEYBOARD, SDLK_j, KMOD_NONE );
   /* Communication. */
   input_setKeybind( "hail", KEYBIND_KEYBOARD, SDLK_y, KMOD_NONE );
   /* Misc. */
   input_setKeybind( "mapzoomin", KEYBIND_KEYBOARD, SDLK_KP_PLUS, KMOD_ALL );
   input_setKeybind( "mapzoomout", KEYBIND_KEYBOARD, SDLK_KP_MINUS, KMOD_ALL );
   input_setKeybind( "screenshot", KEYBIND_KEYBOARD, SDLK_KP_MULTIPLY, KMOD_ALL );
   input_setKeybind( "pause", KEYBIND_KEYBOARD, SDLK_PAUSE, KMOD_ALL );
   input_setKeybind( "speed", KEYBIND_KEYBOARD, SDLK_BACKQUOTE, KMOD_ALL );
   input_setKeybind( "menu", KEYBIND_KEYBOARD, SDLK_ESCAPE, KMOD_ALL );
   input_setKeybind( "info", KEYBIND_KEYBOARD, SDLK_i, KMOD_NONE );
   input_setKeybind( "console", KEYBIND_KEYBOARD, SDLK_F2, KMOD_ALL );
}


/**
 * @brief Initializes the input subsystem (does not set keys).
 */
void input_init (void)
{  
   Keybind *temp;
   int i;

   /* We need unicode for the input widget. */
   SDL_EnableUNICODE(1);

#ifdef DEBUGGING
   /* To avoid stupid segfaults like in the 0.3.6 release. */
   if (sizeof(keybindNames) != sizeof(keybindDescription)) {
      WARN("Keybind names and descriptions aren't of the same size!");
      WARN("   %u descriptions for %u names",
            (unsigned int) sizeof(keybindNames),
            (unsigned int) sizeof(keybindDescription));
   }
#endif /* DEBUGGING */

#if SDL_VERSION_ATLEAST(1,3,0)
   /* Window. */
   SDL_EventState( SDL_WINDOWEVENT,     SDL_DISABLE );
   SDL_EventState( SDL_SYSWMEVENT,      SDL_DISABLE );

   /* Keyboard. */
   SDL_EventState( SDL_KEYDOWN,         SDL_ENABLE );
   SDL_EventState( SDL_KEYUP,           SDL_ENABLE );
   SDL_EventState( SDL_TEXTINPUT,       SDL_DISABLE );

   /* Mice. */
   SDL_EventState( SDL_MOUSEMOTION,     SDL_ENABLE );
   SDL_EventState( SDL_MOUSEBUTTONDOWN, SDL_ENABLE );
   SDL_EventState( SDL_MOUSEBUTTONUP,   SDL_ENABLE );
   SDL_EventState( SDL_MOUSEWHEEL,      SDL_ENABLE );
   
   /* Joystick, enabled in joystick.c if needed. */
   SDL_EventState( SDL_JOYAXISMOTION,   SDL_DISABLE );
   SDL_EventState( SDL_JOYHATMOTION,    SDL_DISABLE );
   SDL_EventState( SDL_JOYBUTTONDOWN,   SDL_DISABLE );
   SDL_EventState( SDL_JOYBUTTONUP,     SDL_DISABLE );

   /* Quit. */
   SDL_EventState( SDL_QUIT,            SDL_ENABLE );

   /* Proximity. */
   SDL_EventState( SDL_PROXIMITYIN,     SDL_DISABLE );
   SDL_EventState( SDL_PROXIMITYOUT,    SDL_DISABLE );
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   /* Get the number of keybindings. */
   for (i=0; strcmp(keybindNames[i],"end"); i++);
      input_keybinds = malloc(i*sizeof(Keybind*));

   /* Create sane null keybinding for each. */
   for (i=0; strcmp(keybindNames[i],"end"); i++) {
      temp = malloc(sizeof(Keybind));
      memset( temp, 0, sizeof(Keybind) );
      temp->name        = keybindNames[i];
      temp->type        = KEYBIND_NULL;
      temp->key         = SDLK_UNKNOWN;
      temp->mod         = KMOD_NONE;
      input_keybinds[i] = temp;
   }

   /* Generate Key translation table. */
   input_keyConvGen();
}


/**
 * @brief Exits the input subsystem.
 */
void input_exit (void)
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      free(input_keybinds[i]);
   free(input_keybinds);

   input_keyConvDestroy();
}


/**
 * @brief Creates the key conversion table.
 */
static void input_keyConvGen (void)
{
   SDLKey k;

   for (k=0; k < INPUT_NUMKEYS; k++)
      keyconv[k] = strdup( SDL_GetKeyName(k) );
}


/**
 * @brief Destroys the key conversion table.
 */
static void input_keyConvDestroy (void)
{
   int i;

   for (i=0; i < INPUT_NUMKEYS; i++)
      free( keyconv[i] );
}


/**
 * @brief Gets the key id from it's name.
 *
 *    @param name Name of the key to get id from.
 *    @return ID of the key.
 */
SDLKey input_keyConv( const char *name )
{
   SDLKey k, m;
   size_t l;
   char buf;

   l = strlen(name);
   buf = tolower(name[0]);

   /* Compare for single character. */
   if (l == 1) {
      m = MIN(256, INPUT_NUMKEYS);
      for (k=0; k < m; k++) { /* Only valid for char range. */
         /* Must not be NULL. */
         if (keyconv[k] == NULL)
            continue;

         /* Check if is also a single char. */
         if ((buf == tolower(keyconv[k][0])) && (keyconv[k][1] == '\0'))
            return k;
      }
   }
   /* Compare for strings. */
   else {
      for (k=0; k < INPUT_NUMKEYS; k++) {
         /* Must not be NULL. */
         if (keyconv[k] == NULL)
            continue;

         /* Compare strings. */
         if (strcmp(name , keyconv[k])==0)
            return k;
      }
   }

   WARN("Keyname '%s' doesn't match any key.", name);
   return SDLK_UNKNOWN;
}


/**
 * @brief Binds key of type type to action keybind.
 *
 *    @param keybind The name of the keybind defined above.
 *    @param type The type of the keybind.
 *    @param key The key to bind to.
 *    @param mod Modifiers to check for.
 */
void input_setKeybind( const char *keybind, KeybindType type, int key, SDLMod mod )
{  
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (strcmp(keybind, input_keybinds[i]->name)==0) {
         input_keybinds[i]->type = type;
         input_keybinds[i]->key = key;
         /* Non-keyboards get mod KMOD_ALL to always match. */
         input_keybinds[i]->mod = (type==KEYBIND_KEYBOARD) ? mod : KMOD_ALL;
         return;
      }
   WARN("Unable to set keybinding '%s', that command doesn't exist", keybind);
}


/**
 * @brief Gets the value of a keybind.
 *
 *    @brief keybind Name of the keybinding to get.
 *    @param[out] type Stores the type of the keybinding.
 *    @param[out] mod Stores the modifiers used with the keybinding.
 *    @return The key assosciated with the keybinding.
 */
SDLKey input_getKeybind( const char *keybind, KeybindType *type, SDLMod *mod )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (strcmp(keybind, input_keybinds[i]->name)==0) {
         if (type != NULL)
            (*type) = input_keybinds[i]->type;
         if (mod != NULL)
            (*mod) = input_keybinds[i]->mod;
         return input_keybinds[i]->key;
      }
   WARN("Unable to get keybinding '%s', that command doesn't exist", keybind);
   return (SDLKey)-1;
}


/**
 * @brief Gets the description of the keybinding.
 *
 *    @param keybind Keybinding to get the description of.
 *    @return Description of the keybinding.
 */
const char* input_getKeybindDescription( const char *keybind )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (strcmp(keybind, input_keybinds[i]->name)==0)
         return keybindDescription[i];
   WARN("Unable to get keybinding description '%s', that command doesn't exist", keybind);
   return NULL;
}


#define KEY(s)    (strcmp(input_keybinds[keynum]->name,s)==0) /**< Shortcut for ease. */
#define INGAME()  (!toolkit_isOpen() && !paused) /**< Makes sure player is in game. */
#define NOHYP()   \
(player && !pilot_isFlag(player,PILOT_HYP_PREP) &&\
!pilot_isFlag(player,PILOT_HYP_BEGIN) &&\
!pilot_isFlag(player,PILOT_HYPERSPACE)) /**< Make sure the player isn't jumping. */
#define NODEAD()  (player && !pilot_isFlag(player,PILOT_DEAD)) /**< Player isn't dead. */
#define NOLAND()  (!landed) /**< Player isn't landed. */
/**
 * @brief Runs the input command.
 *
 *    @param keynum The index of the  keybind.
 *    @param value The value of the keypress (defined above).
 *    @param kabs The absolute value.
 */
static void input_key( int keynum, double value, double kabs )
{
   unsigned int t;


   /*
    * movement
    */
   /* accelerating */
   if (KEY("accel")) {
      if (kabs >= 0.) {
         player_abortAutonav(NULL);
         player_accel(kabs);
      }
      else { /* prevent it from getting stuck */
         if (value==KEY_PRESS) {
            player_abortAutonav(NULL);
            player_accel(1.);
         }
            
         else if (value==KEY_RELEASE)
            player_accelOver();

         /* double tap accel = afterburn! */
         t = SDL_GetTicks();
         if ((conf.afterburn_sens!= 0) &&
               (value==KEY_PRESS) && INGAME() && NOHYP() && NODEAD() &&
               (t-input_accelLast <= conf.afterburn_sens))
            player_afterburn();
         else if ((value==KEY_RELEASE) && player_isFlag(PLAYER_AFTERBURNER))
            player_afterburnOver();

         if (value==KEY_PRESS) input_accelLast = t;
      }
   /* Afterburning. */
   } else if (KEY("afterburn") && INGAME() && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS)
         player_afterburn();
      else if ((value==KEY_RELEASE) && player_isFlag(PLAYER_AFTERBURNER))
         player_afterburnOver();

   /* turning left */
   } else if (KEY("left")) {
      if (kabs >= 0.) {
         player_abortAutonav(NULL);
         player_setFlag(PLAYER_TURN_LEFT); 
         player_left = kabs;
      }
      else {
         /* set flags for facing correction */
         if (value==KEY_PRESS) { 
            player_abortAutonav(NULL);
            player_setFlag(PLAYER_TURN_LEFT); 
            player_left = 1.;
         }
         else if (value==KEY_RELEASE) {
            player_rmFlag(PLAYER_TURN_LEFT);
            player_left = 0.;
         }
      }

   /* turning right */
   } else if (KEY("right")) {
      if (kabs >= 0.) {
         player_abortAutonav(NULL);
         player_setFlag(PLAYER_TURN_RIGHT);
         player_right = kabs;
      }
      else {
         /* set flags for facing correction */
         if (value==KEY_PRESS) {
            player_abortAutonav(NULL);
            player_setFlag(PLAYER_TURN_RIGHT);
            player_right = 1.;
         }
         else if (value==KEY_RELEASE) {
            player_rmFlag(PLAYER_TURN_RIGHT);
            player_right = 0.;
         }
      }
   
   /* turn around to face vel */
   } else if (KEY("reverse")) {
      if (value==KEY_PRESS) {
         player_abortAutonav(NULL);
         player_setFlag(PLAYER_REVERSE);
      }
      else if ((value==KEY_RELEASE) && player_isFlag(PLAYER_REVERSE))
         player_rmFlag(PLAYER_REVERSE);


   /*
    * combat
    */
   /* shooting primary weapon */
   } else if (KEY("primary") && NODEAD()) {
      if (value==KEY_PRESS) { 
         player_abortAutonav(NULL);
         player_setFlag(PLAYER_PRIMARY);
      }
      else if (value==KEY_RELEASE) 
         player_rmFlag(PLAYER_PRIMARY);
   /* targetting */
   } else if (INGAME() && NODEAD() && KEY("target_next")) {
      if (value==KEY_PRESS) player_targetNext(0);
   } else if (INGAME() && NODEAD() && KEY("target_prev")) {
      if (value==KEY_PRESS) player_targetPrev(0);
   } else if (INGAME() && NODEAD() && KEY("target_nearest")) {
      if (value==KEY_PRESS) player_targetNearest();
   } else if (INGAME() && NODEAD() && KEY("target_nextHostile")) {
      if (value==KEY_PRESS) player_targetNext(1);
   } else if (INGAME() && NODEAD() && KEY("target_prevHostile")) {
      if (value==KEY_PRESS) player_targetPrev(1);
   } else if (INGAME() && NODEAD() && KEY("target_hostile")) {
      if (value==KEY_PRESS) player_targetHostile();
   /* face the target */
   } else if (KEY("face")) {
      if (value==KEY_PRESS) { 
         player_abortAutonav(NULL);
         player_setFlag(PLAYER_FACE);
      }
      else if ((value==KEY_RELEASE) && player_isFlag(PLAYER_FACE))
         player_rmFlag(PLAYER_FACE);

   /* board them ships */
   } else if (KEY("board") && INGAME() && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS) {
         player_abortAutonav(NULL);
         player_board();
      }
   } else if (KEY("safety") && INGAME()) {
      if (value==KEY_PRESS)
         weapon_toggleSafety();


   /* 
    * Weapon selection.
    */
   } else if (KEY("weap_all") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) player_setFireMode( 0 );
   } else if (KEY("weap_turret") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) player_setFireMode( 1 );
   } else if (KEY("weap_forward") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) player_setFireMode( 2 );


   /*
    * Escorts.
    */
   } else if (INGAME() && NODEAD() && KEY("e_targetNext")) {
      if (value==KEY_PRESS) player_targetEscort(0);
   } else if (INGAME() && NODEAD() && KEY("e_targetPrev")) {
      if (value==KEY_PRESS) player_targetEscort(1);
   } else if (INGAME() && NODEAD() && KEY("e_attack")) {
      if (value==KEY_PRESS) escorts_attack(player);
   } else if (INGAME() && NODEAD() && KEY("e_hold")) {
      if (value==KEY_PRESS) escorts_hold(player);
   } else if (INGAME() && NODEAD() && KEY("e_return")) {
      if (value==KEY_PRESS) escorts_return(player);
   } else if (INGAME() && NODEAD() && KEY("e_clear")) {
      if (value==KEY_PRESS) escorts_clear(player);


   /*
    * secondary weapons
    */
   /* shooting secondary weapon */
   } else if (KEY("secondary") && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS) {
         player_abortAutonav(NULL);
         player_setFlag(PLAYER_SECONDARY);
      }
      else if (value==KEY_RELEASE)
         player_rmFlag(PLAYER_SECONDARY);

   /* selecting secondary weapon */
   } else if (KEY("secondary_next") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) player_secondaryNext();
   } else if (KEY("secondary_prev") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) player_secondaryPrev();


   /*                                                                     
    * space
    */
   } else if (KEY("autonav") && INGAME() && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS) player_startAutonav();
   /* target planet (cycles like target) */
   } else if (KEY("target_planet") && INGAME() && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS) player_targetPlanet();
   /* target nearest planet or attempt to land */
   } else if (KEY("land") && INGAME() && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS) {
         player_abortAutonav(NULL);
         player_land();
      }
   } else if (KEY("thyperspace") && NOHYP() && NOLAND() && NODEAD()) {
      if (value==KEY_PRESS) {
         player_abortAutonav(NULL);
         player_targetHyperspace();
      }
   } else if (KEY("starmap") && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS) map_open();
   } else if (KEY("jump") && INGAME()) {
      if (value==KEY_PRESS) {
         player_abortAutonav(NULL);
         player_jump();
      }


   /*
    * Communication.
    */
   } else if (KEY("hail") && INGAME() && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS) {
         player_hail();
      }


   /*
    * misc
    */
   /* zooming in */
   } else if (KEY("mapzoomin") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) gui_setRadarRel(-1);
   /* zooming out */
   } else if (KEY("mapzoomout") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) gui_setRadarRel(1);
   /* take a screenshot */
   } else if (KEY("screenshot")) {
      if (value==KEY_PRESS) player_screenshot();
   /* pause the games */
   } else if (KEY("pause") && NOHYP()) {
      if (value==KEY_PRESS) {
         if (!toolkit_isOpen()) {
            if (paused)
               unpause_game();
            else
               pause_game();
         }
      }
   /* toggle speed mode */
   } else if (KEY("speed")) {
      if (value==KEY_PRESS) {
         if (dt_mod == 1.) pause_setSpeed(2.);
         else pause_setSpeed(1.);
      }
   /* opens a small menu */
   } else if (KEY("menu") && NODEAD()) {
      if (value==KEY_PRESS) menu_small();
   
   /* shows pilot information */
   } else if (KEY("info") && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS) menu_info();

   /* Opens the Lua console. */
   } else if (KEY("console") && NODEAD()) {
      if (value==KEY_PRESS) cli_open();
   }
}
#undef KEY


/*
 * events
 */
/* prototypes */
static void input_joyaxis( const SDLKey axis, const int value );
static void input_joyevent( const int event, const SDLKey button );
static void input_keyevent( const int event, const SDLKey key, const SDLMod mod );

/*
 * joystick
 */
/**
 * @brief Filters a joystick axis event.
 *    @param axis Axis generated by the event.
 *    @param value Value of the axis.
 */
static void input_joyaxis( const SDLKey axis, const int value )
{
   int i, k;
   for (i=0; strcmp(keybindNames[i],"end"); i++) {
      if (input_keybinds[i]->key == axis) {
         /* Positive axis keybinding. */
         if ((input_keybinds[i]->type == KEYBIND_JAXISPOS)
               && (value >= 0)) {
            k = (value > 0) ? KEY_PRESS : KEY_RELEASE;
            input_key( i, k, fabs(((double)value)/32767.) );
         }

         /* Negative axis keybinding. */
         if ((input_keybinds[i]->type == KEYBIND_JAXISNEG)
               && (value <= 0)) {
            k = (value < 0) ? KEY_PRESS : KEY_RELEASE;
            input_key( i, k, fabs(((double)value)/32767.) );
         }
      }
   }
}
/**
 * @brief Filters a joystick button event.
 *    @param event Event type (down/up).
 *    @param button Button generating the event.
 */
static void input_joyevent( const int event, const SDLKey button )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if ((input_keybinds[i]->type == KEYBIND_JBUTTON) &&
            (input_keybinds[i]->key == button))
         input_key(i, event, -1.);
}


/*
 * keyboard
 */
/**
 * @brief Filters a keyboard event.
 *    @param event Event type (down/up).
 *    @param key Key generating the event.
 *    @param mod Modifiers active when event was generated.
 */
static void input_keyevent( const int event, SDLKey key, const SDLMod mod )
{
   int i;
   SDLMod mod_filtered;

   /* We want to ignore "global" modifiers. */
   mod_filtered = mod & ~(KMOD_CAPS | KMOD_NUM | KMOD_MODE);

   for (i=0; strcmp(keybindNames[i],"end"); i++) {
      if ((input_keybinds[i]->type == KEYBIND_KEYBOARD) &&
            (input_keybinds[i]->key == key)) {
         if ((input_keybinds[i]->mod == mod_filtered) ||
               (input_keybinds[i]->mod == KMOD_ALL) ||
               (event == KEY_RELEASE)) /**< Release always gets through. */
            input_key(i, event, -1.);
            /* No break so all keys get pressed if needed. */
      }
   }
}


/**
 * @brief Handles global input.
 *
 * Basically seperates the event types
 *
 *    @param event Incoming SDL_Event.
 */
void input_handle( SDL_Event* event )
{
   if (toolkit_isOpen()) /* toolkit handled seperately completely */
      if (toolkit_input(event))
         return; /* we don't process it if toolkit grabs it */

   switch (event->type) {

      /*
       * game itself
       */
      case SDL_JOYAXISMOTION:
         input_joyaxis(event->jaxis.axis, event->jaxis.value);
         break;

      case SDL_JOYBUTTONDOWN:
         input_joyevent(KEY_PRESS, event->jbutton.button);
         break;

      case SDL_JOYBUTTONUP:
         input_joyevent(KEY_RELEASE, event->jbutton.button);
         break;

      case SDL_KEYDOWN:
         input_keyevent(KEY_PRESS, event->key.keysym.sym, event->key.keysym.mod);
         break;

      case SDL_KEYUP:
         input_keyevent(KEY_RELEASE, event->key.keysym.sym, event->key.keysym.mod);
         break;
   }
}

