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


#define KEY_PRESS    ( 1.) /**< Key is pressed. */
#define KEY_RELEASE  (-1.) /**< Key is released. */


/* keybinding structure */
typedef struct Keybind_ {
   char *name; /**< keybinding name, taken from keybindNames */
   KeybindType type; /**< type, defined in playe.h */
   SDLKey key; /**< key/axis/button event number */
   double reverse; /**< 1. if normal, -1. if reversed, only useful for joystick axis */
   SDLMod mod; /**< Key modifiers (where applicable). */
} Keybind;


static Keybind** input_keybinds; /**< contains the players keybindings */

/* name of each keybinding */
const char *keybindNames[] = {
   /* Movement. */
   "accel", "left", "right", "reverse", "afterburn",
   /* Targetting. */
   "target", "target_nearest", "target_hostile",
   /* Fighting. */
   "primary", "face", "board",
   /* Secondary weapons. */
   "secondary", "secondary_next",
   /* Space navigation. */
   "autonav", "target_planet", "land", "thyperspace", "starmap", "jump",
   /* Misc. */
   "mapzoomin", "mapzoomout", "screenshot", "pause", "menu", "info",
   "end" }; /* must terminate in "end" */


/*
 * accel hacks
 */
static unsigned int input_accelLast = 0; /**< Used to see if double tap */
unsigned int input_afterburnSensibility = 250; /**< ms between taps to afterburn */


/*
 * from player.c
 */
extern double player_turn;
extern unsigned int player_target;


/**
 * @fn void input_setDefault (void)
 *
 * @brief Sets the default input keys.
 */
void input_setDefault (void)
{
   /* movement */
   input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_UP, KMOD_ALL, 0 );
   input_setKeybind( "afterburn", KEYBIND_KEYBOARD, SDLK_UNKNOWN, KMOD_ALL, 0 ); /* not set */
   input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_LEFT, KMOD_ALL, 0 );
   input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_RIGHT, KMOD_ALL, 0 );
   input_setKeybind( "reverse", KEYBIND_KEYBOARD, SDLK_DOWN, KMOD_ALL, 0 );
   /* targetting */
   input_setKeybind( "target", KEYBIND_KEYBOARD, SDLK_TAB, KMOD_NONE, 0 );
   input_setKeybind( "target_nearest", KEYBIND_KEYBOARD, SDLK_t, KMOD_NONE, 0 );
   input_setKeybind( "target_hostile", KEYBIND_KEYBOARD, SDLK_r, KMOD_NONE, 0 );
   /* combat */
   input_setKeybind( "primary", KEYBIND_KEYBOARD, SDLK_SPACE, KMOD_NONE, 0 );
   input_setKeybind( "face", KEYBIND_KEYBOARD, SDLK_a, KMOD_NONE, 0 );
   input_setKeybind( "board", KEYBIND_KEYBOARD, SDLK_b, KMOD_NONE, 0 );
   /* secondary weap */
   input_setKeybind( "secondary", KEYBIND_KEYBOARD, SDLK_LSHIFT, KMOD_ALL, 0 );
   input_setKeybind( "secondary_next", KEYBIND_KEYBOARD, SDLK_w, KMOD_NONE, 0 );
   /* space */
   input_setKeybind( "autonav", KEYBIND_KEYBOARD, SDLK_j, KMOD_LCTRL, 0 );
   input_setKeybind( "target_planet", KEYBIND_KEYBOARD, SDLK_p, KMOD_NONE, 0 );
   input_setKeybind( "land", KEYBIND_KEYBOARD, SDLK_l, KMOD_NONE, 0 );
   input_setKeybind( "thyperspace", KEYBIND_KEYBOARD, SDLK_h, KMOD_NONE, 0 );
   input_setKeybind( "starmap", KEYBIND_KEYBOARD, SDLK_m, KMOD_NONE, 0 );
   input_setKeybind( "jump", KEYBIND_KEYBOARD, SDLK_j, KMOD_NONE, 0 );
   /* misc */
   input_setKeybind( "mapzoomin", KEYBIND_KEYBOARD, SDLK_KP_PLUS, KMOD_NONE, 0 );
   input_setKeybind( "mapzoomout", KEYBIND_KEYBOARD, SDLK_KP_MINUS, KMOD_NONE, 0 );
   input_setKeybind( "screenshot", KEYBIND_KEYBOARD, SDLK_KP_MULTIPLY, KMOD_NONE, 0 );
   input_setKeybind( "pause", KEYBIND_KEYBOARD, SDLK_z, KMOD_NONE, 0 );
   input_setKeybind( "menu", KEYBIND_KEYBOARD, SDLK_ESCAPE, KMOD_NONE, 0 );
   input_setKeybind( "info", KEYBIND_KEYBOARD, SDLK_i, KMOD_NONE, 0 );
}


/**
 * @fn void input_init (void)
 *
 * @brief Initializes the input subsystem (does not set keys).
 */
void input_init (void)
{  
   Keybind *temp;
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++); /* gets number of bindings */
      input_keybinds = malloc(i*sizeof(Keybind*));

   /* creates a null keybinding for each */
   for (i=0; strcmp(keybindNames[i],"end"); i++) {
      temp = MALLOC_ONE(Keybind);
      temp->name = (char*)keybindNames[i];
      temp->type = KEYBIND_NULL;
      temp->key = SDLK_UNKNOWN;
      temp->mod = KMOD_NONE;
      temp->reverse = 1.;
      input_keybinds[i] = temp;
   }
}


/**
 * @fn void input_exit (void)
 *
 * @brief Exits the input subsystem.
 */
void input_exit (void)
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      free(input_keybinds[i]);
   free(input_keybinds);
}


/**
 * @fn void input_setKeybind( char *keybind, KeybindType type, int key, int reverse )
 *
 * @brief Binds key of type type to action keybind.
 *
 *    @param keybind The name of the keybind defined above.
 *    @param type The type of the keybind.
 *    @param key The key to bind to.
 *    @param mod Modifiers to check for.
 *    @param reverse Whether to reverse it or not.
 */
void input_setKeybind( char *keybind, KeybindType type, int key,
      SDLMod mod, int reverse )
{  
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (strcmp(keybind, input_keybinds[i]->name)==0) {
         input_keybinds[i]->type = type;
         input_keybinds[i]->key = key;
         /* Non-keyboards get mod KMOD_ALL to always match. */
         input_keybinds[i]->mod = (type==KEYBIND_KEYBOARD) ? mod : KMOD_ALL;
         input_keybinds[i]->reverse = (reverse) ? -1. : 1. ;
         return;
      }
   WARN("Unable to set keybinding '%s', that command doesn't exist", keybind);
}


/**
 * @fn int input_getKeybind( char *keybind, KeybindType *type, int *reverse )
 *
 * @brief Gets the value of a keybind.
 */
int input_getKeybind( char *keybind, KeybindType *type, SDLMod *mod, int *reverse )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (strcmp(keybind, input_keybinds[i]->name)==0) {
         if (type != NULL) (*type) = input_keybinds[i]->type;
         if (mod != NULL) (*mod) = input_keybinds[i]->mod;
         if (reverse != NULL) (*reverse) = input_keybinds[i]->reverse;
         return input_keybinds[i]->key;
      }
   WARN("Unable to get keybinding '%s', that command doesn't exist", keybind);
   return -1;
}


/**
 * @fn static void input_key( int keynum, double value, int kabs )
 *
 * @brief Runs the input command.
 *
 *    @param keynum The index of the  keybind.
 *    @param value The value of the keypress (defined above).
 *    @param abs Whether or not it's an absolute value (for them joystick).
 */
#define KEY(s)    (strcmp(input_keybinds[keynum]->name,s)==0) /**< Shortcut for ease. */
#define INGAME()  (!toolkit) /**< Makes sure player is in game. */
#define NOHYP()   \
(player && !pilot_isFlag(player,PILOT_HYP_PREP) &&\
!pilot_isFlag(player,PILOT_HYP_BEGIN) &&\
!pilot_isFlag(player,PILOT_HYPERSPACE)) /**< Make sure the player isn't jumping. */
static void input_key( int keynum, double value, int kabs )
{
   unsigned int t;


   /*
    * movement
    */
   /* accelerating */
   if (KEY("accel")) {
      if (kabs) {
         player_abortAutonav();
         player_accel(value);
      }
      else { /* prevent it from getting stuck */
         if (value==KEY_PRESS) {
            player_abortAutonav();
            player_accel(1.);
         }
            
         else if (value==KEY_RELEASE) player_accelOver();
      }

      /* double tap accel = afterburn! */
      t = SDL_GetTicks();
      if ((value==KEY_PRESS) && INGAME() && NOHYP() &&
            (t-input_accelLast <= input_afterburnSensibility))
         player_afterburn();
      else if ((value==KEY_RELEASE) && player_isFlag(PLAYER_AFTERBURNER))
         player_afterburnOver();

      if (value==KEY_PRESS) input_accelLast = t;
   /* Afterburning. */
   } else if (KEY("afterburn") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) player_afterburn();
      else if (value==KEY_RELEASE) player_afterburnOver();

   /* turning left */
   } else if (KEY("left")) {
      /* set flags for facing correction */
      if (value==KEY_PRESS) { 
         player_abortAutonav();
         player_setFlag(PLAYER_TURN_LEFT); 
      }
      else if (value==KEY_RELEASE) player_rmFlag(PLAYER_TURN_LEFT);

      if (kabs) { player_turn = -value; }
      else { player_turn -= value; }
      if (player_turn < -1.) player_turn = -1.; /* make sure value is sane */

   /* turning right */
   } else if (KEY("right")) {
      /* set flags for facing correction */
      if (value==KEY_PRESS) {
         player_abortAutonav();
         player_setFlag(PLAYER_TURN_RIGHT);
      }
      else if (value==KEY_RELEASE) player_rmFlag(PLAYER_TURN_RIGHT);

      if (kabs) { player_turn = value; }
      else { player_turn += value; }
      if (player_turn > 1.) { player_turn = 1.; } /* make sure value is sane */
   
   /* turn around to face vel */
   } else if (KEY("reverse")) {
      if (value==KEY_PRESS) {
         player_abortAutonav();
         player_setFlag(PLAYER_REVERSE);
      }
      else if (value==KEY_RELEASE) {
         player_rmFlag(PLAYER_REVERSE);
         player_turn = 0; /* turning corrections */
         if (player_isFlag(PLAYER_TURN_LEFT)) { player_turn -= 1; }
         if (player_isFlag(PLAYER_TURN_RIGHT)) { player_turn += 1; }
      }


   /*
    * combat
    */
   /* shooting primary weapon */
   } else if (KEY("primary")) {
      if (value==KEY_PRESS) { 
         player_abortAutonav();
         player_setFlag(PLAYER_PRIMARY);
      }
      else if (value==KEY_RELEASE) { player_rmFlag(PLAYER_PRIMARY); }
   /* targetting */
   } else if (INGAME() && KEY("target")) {
      if (value==KEY_PRESS) player_targetNext();
   } else if (INGAME() && KEY("target_nearest")) {
      if (value==KEY_PRESS) player_targetNearest();
   } else if (INGAME() && KEY("target_hostile")) {
      if (value==KEY_PRESS) player_targetHostile();
   /* face the target */
   } else if (KEY("face")) {
      if (value==KEY_PRESS) { 
         player_abortAutonav();
         player_setFlag(PLAYER_FACE);
      }
      else if (value==KEY_RELEASE) {
         player_rmFlag(PLAYER_FACE);
         player_turn = 0; /* turning corrections */
         if (player_isFlag(PLAYER_TURN_LEFT)) { player_turn -= 1; }
         if (player_isFlag(PLAYER_TURN_RIGHT)) { player_turn += 1; }
      }
   /* board them ships */
   } else if (KEY("board") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) {
         player_abortAutonav();
         player_board();
      }


   /*
    * secondary weapons
    */
   /* shooting secondary weapon */
   } else if (KEY("secondary") && NOHYP()) {
      player_abortAutonav();
      if (value==KEY_PRESS) { player_setFlag(PLAYER_SECONDARY); }
      else if (value==KEY_RELEASE) { player_rmFlag(PLAYER_SECONDARY); }

   /* selecting secondary weapon */
   } else if (KEY("secondary_next") && INGAME()) {
      if (value==KEY_PRESS) player_secondaryNext();


   /*                                                                     
    * space
    */
   } else if (KEY("autonav") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) player_startAutonav();
   /* target planet (cycles like target) */
   } else if (KEY("target_planet") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) player_targetPlanet();
   /* target nearest planet or attempt to land */
   } else if (KEY("land") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) {
         player_abortAutonav();
         player_land();
      }
   } else if (KEY("thyperspace") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) {
         player_abortAutonav();
         player_targetHyperspace();
      }
   } else if (KEY("starmap") && NOHYP()) {
      if (value==KEY_PRESS) map_open();
   } else if (KEY("jump") && INGAME()) {
      if (value==KEY_PRESS) {
         player_abortAutonav();
         player_jump();
      }


   /*
    * misc
    */
   /* zooming in */
   } else if (KEY("mapzoomin") && INGAME()) {
      if (value==KEY_PRESS) player_setRadarRel(1);
   /* zooming out */
   } else if (KEY("mapzoomout") && INGAME()) {
      if (value==KEY_PRESS) player_setRadarRel(-1);
   /* take a screenshot */
   } else if (KEY("screenshot")) {
      if (value==KEY_PRESS) player_screenshot();
   /* pause the games */
   } else if (KEY("pause") && NOHYP()) {
      if (value==KEY_PRESS) {
         if (!toolkit) {
            if (paused) unpause_game();
            else pause_game();
         }
      }
   /* opens a small menu */
   } else if (KEY("menu")) {
      if (value==KEY_PRESS) menu_small();
   
   /* shows pilot information */
   } else if (KEY("info") && NOHYP()) {
      if (value==KEY_PRESS) menu_info();
   }
}
#undef KEY


/*
 * events
 */
/* prototypes */
static void input_joyaxis( const unsigned int axis, const int value );
static void input_joyevent( int event, const unsigned int button );
static void input_keyevent( int event, SDLKey key, SDLMod mod );

/*
 * joystick
 */
/* joystick axis */
static void input_joyaxis( const unsigned int axis, const int value )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (input_keybinds[i]->type == KEYBIND_JAXIS && input_keybinds[i]->key == axis)
         input_key(i,-(input_keybinds[i]->reverse)*(double)value/32767.,1);
}
/* joystick button down */
static void input_joyevent( int event, const unsigned int button )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (input_keybinds[i]->type == KEYBIND_JBUTTON && input_keybinds[i]->key == button)
         input_key(i, event, 0);
}


/*
 * keyboard
 */
/* key event */
static void input_keyevent( int event, SDLKey key, SDLMod mod )
{
   int i;

   mod &= ~(KMOD_CAPS | KMOD_NUM | KMOD_MODE); /* We want to ignore "global" modifiers. */

   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if ((input_keybinds[i]->type == KEYBIND_KEYBOARD) &&
            (input_keybinds[i]->key == key) &&
            ((input_keybinds[i]->mod == mod) || (input_keybinds[i]->mod == KMOD_ALL))) 
         input_key(i, event, 0);
}


/**
 * @fn void input_handle( SDL_Event* event )
 *
 * @brief Handles global input.
 *
 * Basically seperates the event types
 *
 *    @param event Incoming SDL_Event.
 */
void input_handle( SDL_Event* event )
{
   if (toolkit) /* toolkit handled seperately completely */
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

