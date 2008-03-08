/*
 * See Licensing and Copyright notice in naev.h
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


#define KEY_PRESS    ( 1.)
#define KEY_RELEASE  (-1.)


/* keybinding structure */
typedef struct Keybind_ {
   char *name; /* keybinding name, taken from keybindNames */
   KeybindType type; /* type, defined in playe.h */
   unsigned int key; /* key/axis/button event number */
   double reverse; /* 1. if normal, -1. if reversed, only useful for joystick axis */
} Keybind;


static Keybind** input_keybinds; /* contains the players keybindings */

/* name of each keybinding */
const char *keybindNames[] = {
   "accel", "left", "right", "reverse", /* movement */
   "primary", "target", "target_nearest", "face", "board", /* fighting */
   "secondary", "secondary_next", /* secondary weapons */
   "target_planet", "land", "thyperspace", "starmap", "jump", /* space navigation */
   "mapzoomin", "mapzoomout", "screenshot", "pause", "menu", "info",  /* misc */
   "end" }; /* must terminate in "end" */


/*
 * accel hacks
 */
static unsigned int input_accelLast = 0; /* used to see if double tap */
unsigned int input_afterburnSensibility = 500; /* ms between taps to afterburn */


/*
 * from player.c
 */
extern double player_turn;
extern double player_acc;
extern unsigned int player_target;
/*
 * from main.c
 */
extern int show_fps;


/*
 * sets the default input keys
 */
void input_setDefault (void)
{
   /* movement */
   input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_UP, 0 );
   input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_LEFT, 0 );
   input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_RIGHT, 0 );
   input_setKeybind( "reverse", KEYBIND_KEYBOARD, SDLK_DOWN, 0 );
   /* combat */
   input_setKeybind( "primary", KEYBIND_KEYBOARD, SDLK_SPACE, 0 );
   input_setKeybind( "target", KEYBIND_KEYBOARD, SDLK_TAB, 0 );
   input_setKeybind( "target_nearest", KEYBIND_KEYBOARD, SDLK_r, 0 );
   input_setKeybind( "face", KEYBIND_KEYBOARD, SDLK_a, 0 );
   input_setKeybind( "board", KEYBIND_KEYBOARD, SDLK_b, 0 );
   /* secondary weap */
   input_setKeybind( "secondary", KEYBIND_KEYBOARD, SDLK_LSHIFT, 0 );
   input_setKeybind( "secondary_next", KEYBIND_KEYBOARD, SDLK_w, 0 );
   /* space */
   input_setKeybind( "target_planet", KEYBIND_KEYBOARD, SDLK_p, 0 );
   input_setKeybind( "land", KEYBIND_KEYBOARD, SDLK_l, 0 );
   input_setKeybind( "thyperspace", KEYBIND_KEYBOARD, SDLK_h, 0 );
   input_setKeybind( "starmap", KEYBIND_KEYBOARD, SDLK_m, 0 );
   input_setKeybind( "jump", KEYBIND_KEYBOARD, SDLK_j, 0 );
   /* misc */
   input_setKeybind( "mapzoomin", KEYBIND_KEYBOARD, SDLK_9, 0 );
   input_setKeybind( "mapzoomout", KEYBIND_KEYBOARD, SDLK_0, 0 );
   input_setKeybind( "screenshot", KEYBIND_KEYBOARD, SDLK_KP_MINUS, 0 );
   input_setKeybind( "pause", KEYBIND_KEYBOARD, SDLK_z, 0 );
   input_setKeybind( "menu", KEYBIND_KEYBOARD, SDLK_ESCAPE, 0 );
   input_setKeybind( "info", KEYBIND_KEYBOARD, SDLK_i, 0 );
}


/*
 * initialization/exit functions (does not assign keys)
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
      temp->key = 0;
      temp->reverse = 1.;
      input_keybinds[i] = temp;
   }
}
void input_exit (void)
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      free(input_keybinds[i]);
   free(input_keybinds);
}


/*
 * binds key of type type to action keybind
 *
 * @param keybind is the name of the keybind defined above
 * @param type is the type of the keybind
 * @param key is the key to bind to
 * @param reverse is whether to reverse it or not
 */
void input_setKeybind( char *keybind, KeybindType type, int key, int reverse )
{  
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (strcmp(keybind, input_keybinds[i]->name)==0) {
         input_keybinds[i]->type = type;
         input_keybinds[i]->key = key;
         input_keybinds[i]->reverse = (reverse) ? -1. : 1. ;
         return;
      }
   WARN("Unable to set keybinding '%s', that command doesn't exist", keybind);
}


/*
 * gets the value of a keybind
 */
int input_getKeybind( char *keybind, KeybindType *type, int *reverse )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (strcmp(keybind, input_keybinds[i]->name)==0) {
         if (type != NULL) (*type) = input_keybinds[i]->type;
         if (reverse != NULL) (*reverse) = input_keybinds[i]->reverse;
         return input_keybinds[i]->key;
      }
   WARN("Unable to get keybinding '%s', that command doesn't exist", keybind);
   return -1;
}


/*
 * runs the input command
 *
 * @param keynum is the index of the  keybind
 * @param value is the value of the keypress (defined above)
 * @param abs is whether or not it's an absolute value (for them joystick)
 */
#define KEY(s)    (strcmp(input_keybinds[keynum]->name,s)==0)
#define INGAME()  (!toolkit)
#define NOHYP()   \
(player && !pilot_isFlag(player,PILOT_HYP_PREP) &&\
!pilot_isFlag(player,PILOT_HYP_BEGIN) &&\
!pilot_isFlag(player,PILOT_HYPERSPACE))
static void input_key( int keynum, double value, int abs )
{
   unsigned int t;

   /*
    * movement
    */
   /* accelerating */
   if (INGAME() && KEY("accel")) {
      if (abs) player_acc = value;
      else { /* prevent it from getting stuck */
         if (value==KEY_PRESS) player_acc = 1.;
         else if (value==KEY_RELEASE) player_acc = 0.;
      }

      /* double tap accel = afterburn! */
      t = SDL_GetTicks();
      if ((value==KEY_PRESS) && (t-input_accelLast <= input_afterburnSensibility)) {
         player_afterburn();
      }
      else if ((value==KEY_RELEASE) && player_isFlag(PLAYER_AFTERBURNER))
         player_afterburnOver();
      else
         player_acc = ABS(player_acc); /* make sure value is sane */

      if (value==KEY_PRESS) input_accelLast = t;

   /* turning left */
   } else if (INGAME() && KEY("left")) {

      /* set flags for facing correction */
      if (value==KEY_PRESS) { player_setFlag(PLAYER_TURN_LEFT); }
      else if (value==KEY_RELEASE) { player_rmFlag(PLAYER_TURN_LEFT); }

      if (abs) { player_turn = -value; }
      else { player_turn -= value; }
      if (player_turn < -1.) { player_turn = -1.; } /* make sure value is sane */

   /* turning right */
   } else if (INGAME() && KEY("right")) {

      /* set flags for facing correction */
      if (value==KEY_PRESS) { player_setFlag(PLAYER_TURN_RIGHT); }
      else if (value==KEY_RELEASE) { player_rmFlag(PLAYER_TURN_RIGHT); }

      if (abs) { player_turn = value; }
      else { player_turn += value; }
      if (player_turn > 1.) { player_turn = 1.; } /* make sure value is sane */
   
   /* turn around to face vel */
   } else if (INGAME() && KEY("reverse")) {
      if (value==KEY_PRESS) { player_setFlag(PLAYER_REVERSE); }
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
   } else if (INGAME() && KEY("primary")) {
      if (value==KEY_PRESS) { player_setFlag(PLAYER_PRIMARY); }
      else if (value==KEY_RELEASE) { player_rmFlag(PLAYER_PRIMARY); }
   /* targetting */
   } else if (INGAME() && KEY("target")) {
      if (value==KEY_PRESS) player_target = pilot_getNext(player_target);
   } else if (INGAME() && KEY("target_nearest")) {
      if (value==KEY_PRESS) player_target = pilot_getHostile();
   /* face the target */
   } else if (KEY("face")) {
      if (value==KEY_PRESS) { player_setFlag(PLAYER_FACE); }
      else if (value==KEY_RELEASE) {
         player_rmFlag(PLAYER_FACE);
         player_turn = 0; /* turning corrections */
         if (player_isFlag(PLAYER_TURN_LEFT)) { player_turn -= 1; }
         if (player_isFlag(PLAYER_TURN_RIGHT)) { player_turn += 1; }
      }
   /* board them ships */
   } else if (KEY("board") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) player_board();


   /*
    * secondary weapons
    */
   /* shooting secondary weapon */
   } else if (KEY("secondary") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) { player_setFlag(PLAYER_SECONDARY); }
      else if (value==KEY_RELEASE) { player_rmFlag(PLAYER_SECONDARY); }

   /* selecting secondary weapon */
   } else if (KEY("secondary_next") && INGAME()) {
      if (value==KEY_PRESS) player_secondaryNext();


   /*                                                                     
    * space
    */
   /* target planet (cycles like target) */
   } else if (KEY("target_planet") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) player_targetPlanet();
   /* target nearest planet or attempt to land */
   } else if (KEY("land") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) player_land();
   } else if (KEY("thyperspace") && INGAME() && NOHYP()) {
      if (value==KEY_PRESS) player_targetHyperspace();
   } else if (KEY("starmap") && NOHYP()) {
      if (value==KEY_PRESS) map_open();
   } else if (KEY("jump") && INGAME()) {
      if (value==KEY_PRESS) player_jump();


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
static void input_joydown( const unsigned int button );
static void input_joyup( const unsigned int button );
static void input_keydown( SDLKey key );
static void input_keyup( SDLKey key );

/*
 * joystick
 */
/* joystick axis */
static void input_joyaxis( const unsigned int axis, const int value )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (input_keybinds[i]->type == KEYBIND_JAXIS && input_keybinds[i]->key == axis) {
         input_key(i,-(input_keybinds[i]->reverse)*(double)value/32767.,1);
         return;
      }
}
/* joystick button down */
static void input_joydown( const unsigned int button )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (input_keybinds[i]->type == KEYBIND_JBUTTON && input_keybinds[i]->key == button) {
         input_key(i,KEY_PRESS,0);
         return;
      }
}
/* joystick button up */
static void input_joyup( const unsigned int button )
{  
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (input_keybinds[i]->type == KEYBIND_JBUTTON && input_keybinds[i]->key == button) {
         input_key(i,KEY_RELEASE,0);
         return;                                                          
      }
}


/*
 * keyboard
 */
/* key down */
static void input_keydown( SDLKey key )
{
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (input_keybinds[i]->type == KEYBIND_KEYBOARD && input_keybinds[i]->key == key) {
         input_key(i,KEY_PRESS,0);
         return;
      }
}
/* key up */
static void input_keyup( SDLKey key )
{  
   int i;
   for (i=0; strcmp(keybindNames[i],"end"); i++)
      if (input_keybinds[i]->type == KEYBIND_KEYBOARD && input_keybinds[i]->key == key) {
         input_key(i,KEY_RELEASE,0);
         return;                                                          
      }
}


/*
 * global input
 *
 * basically seperates the event types
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
         input_joydown(event->jbutton.button);
         break;

      case SDL_JOYBUTTONUP:
         input_joyup(event->jbutton.button);
         break;

      case SDL_KEYDOWN:
         input_keydown(event->key.keysym.sym);
         break;

      case SDL_KEYUP:
         input_keyup(event->key.keysym.sym);
         break;
   }
}

