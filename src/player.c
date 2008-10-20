/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file player.c
 *
 * @brief Contains all the player related stuff.
 */


#include "player.h"

#include <malloc.h>

#include "xml.h"
#include "naev.h"
#include "pilot.h"
#include "log.h"
#include "opengl.h"
#include "font.h"
#include "pack.h"
#include "space.h"
#include "rng.h"
#include "land.h"
#include "sound.h"
#include "economy.h"
#include "pause.h"
#include "menu.h"
#include "toolkit.h"
#include "dialogue.h"
#include "mission.h"
#include "misn_lua.h"
#include "ntime.h"
#include "hook.h"
#include "map.h"
#include "nfile.h"
#include "spfx.h"
#include "unidiff.h"
#include "comm.h"


#define XML_GUI_ID   "GUIs" /**< XML section identifier for GUI document. */
#define XML_GUI_TAG  "gui" /**<  XML Section identifier for GUI tags. */

#define XML_START_ID "Start" /**< Module start xml document identifier. */

#define GUI_DATA     "dat/gui.xml" /**< Global GUI configuration file. */
#define GUI_GFX      "gfx/gui/" /**< Location of the GUI graphics. */

#define START_DATA   "dat/start.xml" /**< Module start information file. */

#define TARGET_WIDTH 128 /**< Width of target graphics. */
#define TARGET_HEIGHT 96 /**< Height of target graphics. */

#define PLAYER_RESERVED_CHANNELS 6 /**< Number of channels to reserve for player sounds. */
#define PLAYER_ENGINE_CHANNEL    8 /**< Player channel for engine noises. */
#define PLAYER_GUI_CHANNEL       9 /**< Player channel. */


/*
 * player stuff
 */
Pilot* player = NULL; /**< Ze player. */
static Ship* player_ship = NULL; /**< Temporary ship to hold when naming it */
static double player_px, player_py, player_vx, player_vy, player_dir; /**< More hack. */
static int player_credits = 0; /**< Temporary hack for when creating. */


/*
 * player sounds.
 */
static int snd_target = -1; /**< Sound when targetting. */
static int snd_jump = -1; /**< Sound when can jump. */
static int snd_nav = -1; /**< Sound when changing nav computer. */


/* 
 * player pilot stack - ships he has 
 */
static Pilot** player_stack = NULL; /**< Stack of ships player has. */
static char** player_lstack = NULL; /**< Names of the planet the ships are at. */
static int player_nstack = 0; /**< Number of ships player has. */


/* 
 * player global properties
 */
char* player_name = NULL; /**< Ze name. */
double player_crating = 0; /**< Ze combat rating. */
unsigned int player_flags = 0; /**< Player flags. */
/* used in input.c */
double player_turn = 0.; /**< Turn velocity from input. */
static double player_acc = 0.; /**< Accel velocity from input. */
/* used in map.c */
int planet_target = -1; /**< Targetted planet. -1 is none. */
int hyperspace_target = -1; /**< Targetted hyperspace route. -1 is none. */
/* for death and such */
static unsigned int player_timer = 0; /**< For death and such. */
static Vector2d player_cam; /**< For death and such. */


/* 
 * unique mission stack
 */
static int* missions_done = NULL; /**< Saves position of completed missions. */
static int missions_mdone = 0; /**< Memory size of completed missions. */
static int missions_ndone = 0; /**< Number of completed missions. */


/*
 * pilot stuff for GUI
 */
extern Pilot** pilot_stack;
extern int pilot_nstack;

/*
 * space stuff for GUI
 */
extern StarSystem *systems_stack;

/*
 * map stuff for autonav
 */
extern int map_npath;


/**
 * @struct Radar
 *
 * @brief Represents the player's radar.
 */
typedef struct Radar_ {
   double x; /**< X position. */
   double y; /**< Y Position */
   double w; /**< Width. */
   double h; /**< Hegiht. */
   RadarShape shape; /**< Shape */
   double res; /**< Resolution */
} Radar;
/* radar resolutions */
#define RADAR_RES_MAX      100. /**< Maximum radar resolution. */
#define RADAR_RES_MIN      10. /**< Minimum radar resolution. */
#define RADAR_RES_INTERVAL 10. /**< Steps used to increase/decrease resolution. */
#define RADAR_RES_DEFAULT  40. /**< Default resolution. */

/**
 * @struct Rect
 *
 * @brief Represents a rectangle.
 */
typedef struct Rect_ {
   double x; /**< X position. */
   double y; /**< Y position. */
   double w; /**< Width. */
   double h; /**< Height. */
} Rect;

/**
 * @struct GUI
 *
 * @brief Represents the ingame player graphical user interface.
 */
typedef struct GUI_ {
   /* graphics */
   glTexture *gfx_frame; /**< Frame of the GUI. */
   glTexture *gfx_targetPilot; /**< Graphics used to target pilot. */
   glTexture *gfx_targetPlanet; /**< Graphics used to target planets. */

   /* rects */
   Radar radar; /**< The radar. */
   Rect nav; /**< Navigation computer. */
   Rect shield; /**< Shield bar. */
   Rect armour; /**< Armour bar. */
   Rect energy; /**< Energy bar. */
   Rect weapon; /**< Weapon targetting system. */
   Rect target_health; /**< Target health. */
   Rect target_name; /**< Name of the target. */
   Rect target_faction; /**< Faction of the target. */
   Rect misc; /**< Misc stuff: credits, cargo... */
   Rect mesg; /**< Where messages go. */
   
   /* positions */
   Vector2d frame; /**< Global frame position. */
   Vector2d target; /**< Global target position. */

} GUI;
static GUI gui = { .gfx_frame = NULL,
      .gfx_targetPilot = NULL,
      .gfx_targetPlanet = NULL }; /**< Ze GUI. */
/* needed to render properly */
double gui_xoff = 0.; /**< X Offset that GUI introduces. */
double gui_yoff = 0.; /**< Y offset that GUI introduces. */

/* messages */
#define MESG_SIZE_MAX   120 /**< Maxmimu message length. */
int mesg_timeout = 5000; /**< How long it takes for a message to timeout. */
int mesg_max = 5; /**< Maximum messages onscreen */
/**
 * @struct Mesg
 * 
 * @brief On screen player message.
 */
typedef struct Mesg_ {
   char str[MESG_SIZE_MAX]; /**< The message. */
   unsigned int t; /**< Time of creation. */
} Mesg;
static Mesg* mesg_stack; /**< Stack of mesages, will be of mesg_max size. */


/* 
 * prototypes
 */
/* 
 * external
 */
extern void pilot_render( const Pilot* pilot ); /* from pilot.c */
extern void weapon_minimap( const double res, const double w, const double h,
      const RadarShape shape ); /* from weapon.c */
extern void planets_minimap( const double res, const double w, const double h,
      const RadarShape shape ); /* from space.c */
/* 
 * internal
 */
/* creation */
static void player_newMake (void);
static void player_newShipMake( char *name );
/* sound */
static void player_initSound (void);
/* static void player_stopSound (void);*/
/* gui */
static void rect_parse( const xmlNodePtr parent,
      double *x, double *y, double *w, double *h );
static int gui_parse( const xmlNodePtr parent, const char *name );
static void gui_renderPilot( const Pilot* p );
static void gui_renderBar( const glColour* c,
      const Rect* r, const double w );
/* save/load */
static int player_saveShip( xmlTextWriterPtr writer, 
      Pilot* ship, char* loc );
static int player_parse( xmlNodePtr parent );
static int player_parseDone( xmlNodePtr parent );
static int player_parseShip( xmlNodePtr parent, int is_player );
/* 
 * externed
 */
void player_dead (void); /* pilot.c */
void player_destroyed (void); /* pilot.c */
int player_save( xmlTextWriterPtr writer ); /* save.c */
int player_load( xmlNodePtr parent ); /* save.c */
void player_think( Pilot* player ); /* pilot.c */
void player_brokeHyperspace (void); /* pilot.c */
double player_faceHyperspace (void); /* pilot.c */


/**
 * @fn void player_new (void)
 * 
 * @brief Creates a new player.
 *
 *   - Cleans up after old players.
 *   - Prompts for name.
 * 
 * @sa player_newMake
 */
void player_new (void)
{
   int r;

   /* to not segfault due to lack of environment */
   player_flags = 0;
   player_setFlag(PLAYER_CREATING);
   vectnull( &player_cam );
   gl_bindCamera( &player_cam );

   /* setup sound */
   player_initSound();

   /* cleanup player stuff if we'll be recreating */
   player_cleanup();
   var_cleanup();
   missions_cleanup();
   space_clearKnown();
   land_cleanup();
   diff_clear();
   factions_reset();
   map_close();

   player_name = dialogue_input( "Player Name", 3, 20,
         "Please write your name:" );

   /* Player cancelled dialogue. */
   if (player_name == NULL) {
      menu_main();
      return;
   }

   if (nfile_fileExists("saves/%s.ns",player_name)) {
      r = dialogue_YesNo("Overwrite",
            "You already have a pilot named %s. Overwrite?",player_name);
      if (r==0) { /* no */
         player_new();
         return;
      }
   }

   player_newMake();
}


/**
 * @fn static void player_newMake (void)
 *
 * @brief Actually creates a new player.
 */
static void player_newMake (void)
{
   Ship *ship;
   char *sysname;;
   uint32_t bufsize;
   char *buf;
   int l,h, tl,th;
   double x,y;

   sysname = NULL;

   buf = pack_readfile( DATA, START_DATA, &bufsize );

   xmlNodePtr node, cur, tmp;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_START_ID)) {
      ERR("Malformed '"START_DATA"' file: missing root element '"XML_START_ID"'");
      return;
   }

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"START_DATA"' file: does not contain elements");
      return;
   }
   do {
      if (xml_isNode(node, "player")) { /* we are interested in the player */
         cur = node->children;
         do {
            if (xml_isNode(cur,"ship")) ship = ship_get( xml_get(cur) );
            else if (xml_isNode(cur,"credits")) { /* monies range */
               tmp = cur->children;
               do {
                  xmlr_int(tmp, "low", l);
                  xmlr_int(tmp, "high", h);
               } while (xml_nextNode(tmp));
            }
            else if (xml_isNode(cur,"system")) {
               tmp = cur->children;
               do {
                  /** system name, @todo percent chance */
                  if (xml_isNode(tmp,"name")) 
                     sysname = strdup(xml_get(tmp));
                  /* position */
                  xmlr_float(tmp,"x",x);
                  xmlr_float(tmp,"y",y);
               } while (xml_nextNode(tmp));
            }
            xmlr_float(cur,"player_crating",player_crating);
            if (xml_isNode(cur,"date")) {
               tmp = cur->children;
               do {
                  xmlr_int(tmp, "low", tl);
                  xmlr_int(tmp, "high", th);
               } while (xml_nextNode(tmp));
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);

   /* monies */
   player_credits = RNG(l,h);

   /* time */
   ntime_set( RNG(tl*1000*NTIME_UNIT_LENGTH,th*1000*NTIME_UNIT_LENGTH) );

   /* welcome message - must be before space_init */
   player_message( "Welcome to "APPNAME"!" );
   player_message( " v%d.%d.%d", VMAJOR, VMINOR, VREV );

   /* Try to create the pilot, if fails reask for player name. */
   if (player_newShip( ship, x, y, 0., 0., RNG(0,359)/180.*M_PI ) != 0) {
      player_new();
      return;
   }
   space_init(sysname);
   free(sysname);

   /* clear the map */
   map_clear();
}


/**
 * @fn void player_newShip( Ship* ship, double px, double py,
 *           double vx, double vy, double dir )
 *
 * @brief Creates a new ship for player.
 *
 *    @return 0 indicates success, -1 means dialogue was cancelled.
 *
 * @sa player_newShipMake
 */
int player_newShip( Ship* ship, double px, double py,
      double vx, double vy, double dir )
{
   char* ship_name;

   /* temporary values while player doesn't exist */
   player_ship = ship;
   player_px = px;
   player_py = py;
   player_vx = vx;
   player_vy = vy;
   player_dir = dir;
   ship_name = dialogue_input( "Ship Name", 3, 20,
         "Please name your brand new %s:", ship->name );

   /* Dialogue cancelled. */
   if (ship_name == NULL)
      return -1;

   player_newShipMake(ship_name);

   free(ship_name);

   return 0;
}

/**
 * @fn static void player_newShipMake( char* name )
 *
 * @brief Actually creates the new ship.
 */
static void player_newShipMake( char* name )
{
   Vector2d vp, vv;

   /* store the current ship if it exists */
   if (player != NULL) {
      player_stack = realloc(player_stack, sizeof(Pilot*)*(player_nstack+1));
      player_stack[player_nstack] = pilot_copy( player );
      player_lstack = realloc(player_lstack, sizeof(char*)*(player_nstack+1));
      player_lstack[player_nstack] = strdup( land_planet->name );
      player_nstack++;
      if (!player_credits) player_credits = player->credits;
      pilot_destroy( player );
   }

   /* in case we're respawning */
   player_rmFlag(PLAYER_CREATING);

   /* hackish position setting */
   vect_cset( &vp, player_px, player_py );
   vect_cset( &vv, player_vx, player_vy );

   /* create the player */
   pilot_create( player_ship, name, faction_get("Player"), NULL,
         player_dir,  &vp, &vv, PILOT_PLAYER );
   gl_bindCamera( &player->solid->pos ); /* set opengl camera */

   /* money */
   player->credits = player_credits;
   player_credits = 0;
}


/**
 * @fn void player_swapShip( char* shipname )
 *
 * @brief Swaps player's current ship with his ship named shipname.
 *
 *    @param shipname Ship to change to.
 */
void player_swapShip( char* shipname )
{
   int i, j;
   Pilot* ship;

   for (i=0; i<player_nstack; i++) {
      if (strcmp(shipname,player_stack[i]->name)==0) { /* swap player and ship */
         ship = player_stack[i];

         /* move credits over */
         ship->credits = player->credits;

         /* move cargo over */
         for (j=0; j<player->ncommodities; j++) {
            pilot_addCargo( ship, player->commodities[j].commodity,
                  player->commodities[j].quantity );
            pilot_rmCargo( player, player->commodities[j].commodity,
                  player->commodities[j].quantity );
         }

         /* extra pass to calculate stats */
         pilot_calcStats( ship );
         pilot_calcStats( player );

         /* now swap the players */
         player_stack[i] = player;
         for (j=0; j<pilot_nstack; j++) /* find pilot in stack to swap */
            if (pilot_stack[j] == player) {
               pilot_stack[j] = player = ship;
               break;
            }

         gl_bindCamera( &player->solid->pos ); /* don't forget the camera */
         return;
      }
   }
   WARN( "Unable to swap player with ship '%s': ship does not exist!", shipname );
}


/**
 * @fn int player_shipPrice( char* shipname )
 *
 * @brief Calculates the price of one of the player's ships.
 *
 *    @param shipname Name of the ship.
 *    @return The price of the ship in credits.
 */
int player_shipPrice( char* shipname )
{
   int i;
   int price;
   Pilot *ship;

   /* Find the ship. */
   for (i=0; i<player_nstack; i++) {
      if (strcmp(shipname,player_stack[i]->name)==0) {
         ship = player_stack[i];

         /* Ship price is base price + outfit prices. */
         price = ship_basePrice( ship->ship );
         for (i=0; i<ship->noutfits; i++)
            price += ship->outfits[i].quantity * ship->outfits[i].outfit->price;

         return price;
      }
   }

   WARN( "Unable to find price for player's ship '%s': ship does not exist!", shipname );
   return -1;
}


/**
 * @brief void player_rmShip( char* shipname )
 * 
 * @brief Removes one of the player's ships.
 *
 *    @param shipname Name of the ship to remove.
 */
void player_rmShip( char* shipname )
{
   int i;

   for (i=0; i<player_nstack; i++) {
      if (strcmp(shipname,player_stack[i]->name)==0) {

         /* Free player ship and location. */
         pilot_free(player_stack[i]);
         free(player_lstack[i]);

         /* Move memory to make adjacent. */
         memmove( player_stack+i, player_stack+i+1,
               sizeof(Pilot*) * (player_nstack-i-1) );
         memmove( player_lstack+i, player_lstack+i+1,
               sizeof(char*) * (player_nstack-i-1) );
         player_nstack--; /* Shrink stack. */
         /* Realloc memory to smaller size. */
         player_stack = realloc( player_stack,
               sizeof(Pilot*) * (player_nstack) );
         player_lstack = realloc( player_lstack,
               sizeof(char*) * (player_nstack) );
      }
   }
}


/**
 * @brief void player_cleanup (void)
 *
 * @brief Cleans up player stuff like player_stack.
 */
void player_cleanup (void)
{
   int i;

   player_clear();

   /* clean up name */
   if (player_name != NULL) free(player_name);

   /* clean up messages */
   for (i=0; i<mesg_max; i++)
      memset( mesg_stack[i].str, '\0', MESG_SIZE_MAX );

   /* clean up the stack */
   if (player_stack != NULL) {                
      for (i=0; i<player_nstack; i++) {
         pilot_free(player_stack[i]);
         free(player_lstack[i]);
      }
      free(player_stack);
      player_stack = NULL;
      free(player_lstack);
      player_lstack = NULL;
      /* nothing left */
      player_nstack = 0;
   }

   /* clean up misions */
   if (missions_done != NULL) {
      free(missions_done);
      missions_done = NULL;
      missions_ndone = 0;
      missions_mdone = 0;
   }

   /* just in case purge the pilot stack */
   pilots_cleanAll();
   if (player != NULL) { /* and the player */
      pilot_free(player);
      player = NULL;
   }
}


/**
 * @fn static void player_initSound (void)
 *
 * @brief Initializes the player sounds.
 */
static int player_soundReserved = 0; /**< Has the player already reserved sound? */
static void player_initSound (void)
{
   if (player_soundReserved) return;

   /* Allocate channels. */
   sound_reserve(PLAYER_RESERVED_CHANNELS);
   sound_createGroup(PLAYER_ENGINE_CHANNEL, 0, 1); /* Channel for engine noises. */
   sound_createGroup(PLAYER_GUI_CHANNEL, 1, PLAYER_RESERVED_CHANNELS-1);
   player_soundReserved = 1;

   /* Get sounds. */
   snd_target = sound_get("target");
   snd_jump = sound_get("jump");
   snd_nav = sound_get("nav");
}


/**
 * @fn static void player_playSound( int sound, int once )
 *
 * @brief Plays a sound at the player.
 *
 *    @param sound ID of the sound to play.
 *    @param once Play only once?
 */
void player_playSound( int sound, int once )
{
   sound_playGroup( PLAYER_GUI_CHANNEL, sound, once );
}


#if 0
/**
 * @fn static void player_stopSound (void)
 *
 * @brief Stops playing player sounds.
 */
static void player_stopSound (void)
{
   sound_stopGroup( PLAYER_GUI_CHANNEL );
}
#endif


/**
 * @fn void player_message ( const char *fmt, ... )
 *
 * @brief Adds a mesg to the queue to be displayed on screen.
 *
 *    @param fmt String with formatting like printf.
 */
void player_message ( const char *fmt, ... )
{
   va_list ap;
   int i;

   if (fmt == NULL) return; /* message not valid */

   /* copy old messages back */
   for (i=1; i<mesg_max; i++) {
      if (mesg_stack[mesg_max-i-1].str[0] != '\0') {
         strcpy(mesg_stack[mesg_max-i].str, mesg_stack[mesg_max-i-1].str);
         mesg_stack[mesg_max-i].t = mesg_stack[mesg_max-i-1].t;
      }
   }

   /* add the new one */
   va_start(ap, fmt);
   vsprintf( mesg_stack[0].str, fmt, ap );
   va_end(ap);

   mesg_stack[0].t = SDL_GetTicks() + mesg_timeout;
}


/**
 * @fn void player_warp( const double x, const double y )
 *
 * @brief Warps the player to the new position
 *
 *    @param x X value of the position to warp to.
 *    @param y Y value of the position to warp to.
 */
void player_warp( const double x, const double y )
{
   vect_cset( &player->solid->pos, x, y );
}


/**
 * @fn void player_clear (void)
 *
 * @brief Clears the targets.
 */
void player_clear (void)
{
   if (player != NULL)
      player->target = PLAYER_ID;
   planet_target = -1;
   hyperspace_target = -1;
}


/**
 * @fn const char* player_rating (void)
 *
 * @brief Gets the player's combat rating in a human-readable string.
 *
 *    @return The player's combat rating in a human readable string.
 */
static char* player_ratings[] = {
      "None",
      "Smallfry",
      "Weak",
      "Minor",
      "Average",
      "Major",
      "Fearsome",
      "Godlike"
};
const char* player_rating (void)
{
   if (player_crating == 0.) return player_ratings[0];
   else if (player_crating < 50.) return player_ratings[1];
   else if (player_crating < 200.) return player_ratings[2];
   else if (player_crating < 500.) return player_ratings[3];
   else if (player_crating < 1000.) return player_ratings[4];
   else if (player_crating < 2500.) return player_ratings[5];
   else if (player_crating < 10000.) return player_ratings[6];
   else return player_ratings[7];
}


/**
 * @fn int player_outfitOwned( const char* outfitname )
 *
 * @brief Gets how many of the outfit the player owns.
 *
 *    @param outfitname Outfit to check how many the player owns.
 *    @return The number of outfits matching outfitname owned.
 */
int player_outfitOwned( const char* outfitname )
{
   int i;

   for (i=0; i<player->noutfits; i++)
      if (strcmp(outfitname, player->outfits[i].outfit->name)==0)
         return player->outfits[i].quantity;
   return 0;
}


/**
 * @fn int player_cargoOwned( const char* commodityname )
 *
 * @brief Gets how many of the commodity the player has.
 *
 *    @param commodityname Commodity to check how many the player owns.
 *    @return The number of commodities owned matching commodityname.
 */
int player_cargoOwned( const char* commodityname )
{
   int i;

   for (i=0; i<player->ncommodities; i++)
      if (!player->commodities[i].id &&
            strcmp(commodityname, player->commodities[i].commodity->name)==0)
         return player->commodities[i].quantity;
   return 0;
}


/*
 * removes mission cargo from all player ships
 */
void player_rmMissionCargo( unsigned int cargo_id )
{
   int i;
   
   /* check if already done */
   if ((player != NULL) && !pilot_rmMissionCargo(player, cargo_id)) return;

   for (i=0; i<player_nstack; i++)
      if (!pilot_rmMissionCargo( player_stack[i], cargo_id ))
            return; /* success */
}



/**
 * @fn void player_renderBG (void)
 *
 * @brief Renders the background player stuff, namely planet target gfx
 */
void player_renderBG (void)
{
   double x,y;
   glColour *c;
   Planet* planet;

   /* no need to draw if pilot is dead */
   if (player_isFlag(PLAYER_DESTROYED) || player_isFlag(PLAYER_CREATING) ||
      pilot_isFlag(player,PILOT_DEAD)) return;

   if (planet_target >= 0) {
      planet = cur_system->planets[planet_target];

      c = faction_getColour(planet->faction);

      x = planet->pos.x - planet->gfx_space->sw/2.;
      y = planet->pos.y + planet->gfx_space->sh/2.;
      gl_blitSprite( gui.gfx_targetPlanet, x, y, 0, 0, c ); /* top left */

      x += planet->gfx_space->sw;
      gl_blitSprite( gui.gfx_targetPlanet, x, y, 1, 0, c ); /* top right */

      y -= planet->gfx_space->sh;
      gl_blitSprite( gui.gfx_targetPlanet, x, y, 1, 1, c ); /* bottom right */

      x -= planet->gfx_space->sw;
      gl_blitSprite( gui.gfx_targetPlanet, x, y, 0, 1, c ); /* bottom left */
   }
}

/**
 * @fn void player_render (void)
 *
 * @brief Renders the player
 */
void player_render (void)
{
   Pilot *p;
   glColour *c;
   double x,y;

   if ((player != NULL) && !player_isFlag(PLAYER_CREATING)) {

      /* renders the player target graphics */
      if (player->target != PLAYER_ID) p = pilot_get(player->target);
      else p = NULL;
      if ((p==NULL) || pilot_isFlag(p,PILOT_DEAD))
         player->target = PLAYER_ID; /* no more pilot_target */
      else { /* still is a pilot_target */
         if (pilot_isDisabled(p)) c = &cInert;
         else if (pilot_isFlag(p,PILOT_BRIBED)) c = &cNeutral;
         else if (pilot_isFlag(p,PILOT_HOSTILE)) c = &cHostile;
         else c = faction_getColour(p->faction);

         x = p->solid->pos.x - p->ship->gfx_space->sw * PILOT_SIZE_APROX/2.;
         y = p->solid->pos.y + p->ship->gfx_space->sh * PILOT_SIZE_APROX/2.;
         gl_blitSprite( gui.gfx_targetPilot, x, y, 0, 0, c ); /* top left */

         x += p->ship->gfx_space->sw * PILOT_SIZE_APROX;
         gl_blitSprite( gui.gfx_targetPilot, x, y, 1, 0, c ); /* top right */

         y -= p->ship->gfx_space->sh * PILOT_SIZE_APROX;
         gl_blitSprite( gui.gfx_targetPilot, x, y, 1, 1, c ); /* bottom right */

         x -= p->ship->gfx_space->sw * PILOT_SIZE_APROX;
         gl_blitSprite( gui.gfx_targetPilot, x, y, 0, 1, c ); /* bottom left */
      }


      /* Player is ontop of targeting graphic */
      pilot_render(player);
   }
}


/**
 * @fn void player_renderGUI (void)
 *
 * @brief Renders the player's GUI.
 */
static int can_jump = 0; /**< Stores whether or not the player is able to jump. */
void player_renderGUI (void)
{
   int i, j;
   double x, y;
   char str[10];
   Pilot* p;
   glColour* c, c2;
   glFont* f;
   StarSystem *sys;
   unsigned int t;
   int quantity, delay;

   t = SDL_GetTicks();

      /* pilot is dead or being created, just render him and stop */
   if (player_isFlag(PLAYER_DESTROYED) || player_isFlag(PLAYER_CREATING) ||
        pilot_isFlag(player,PILOT_DEAD)) {
      if (player_isFlag(PLAYER_DESTROYED)) {
         if (!toolkit && !player_isFlag(PLAYER_CREATING) &&
               (t > player_timer)) {
            menu_death();
         }
      }

      /*
       * draw fancy cinematic scene borders
       */
      spfx_cinematic();

      return;
   }

   if (player==NULL) return;


   /* Lockon warning */
   if (player->lockons > 0)
      gl_printMid( NULL, SCREEN_W - gui_xoff, 0., SCREEN_H-gl_defFont.h-25.,
            &cRed, "LOCKON DETECTED");

   /* Volatile environment. */
   if (cur_system->nebu_volatility > 0.)
      gl_printMid( NULL, SCREEN_W - gui_xoff, 0., SCREEN_H-gl_defFont.h*2.-35.,
            &cRed, "VOLATILE ENVIRONMENT DETECTED");

   /*
    *    G U I
    */
   /*
    * frame
    */
   gl_blitStatic( gui.gfx_frame, gui.frame.x, gui.frame.y, NULL );

   /*
    * radar
    */
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   if (gui.radar.shape==RADAR_RECT)
      glTranslated( gui.radar.x - SCREEN_W/2. + gui.radar.w/2.,
            gui.radar.y - SCREEN_H/2. - gui.radar.h/2., 0.);
   else if (gui.radar.shape==RADAR_CIRCLE)
      glTranslated( gui.radar.x - SCREEN_W/2.,
            gui.radar.y - SCREEN_H/2., 0.);

   /*
    * planets
    */
   planets_minimap(gui.radar.res, gui.radar.w, gui.radar.h, gui.radar.shape);

   /*
    * weapons
    */
   glBegin(GL_POINTS);
      COLOUR(cRadar_weap);
      weapon_minimap(gui.radar.res, gui.radar.w, gui.radar.h, gui.radar.shape);
   glEnd(); /* GL_POINTS */


   /* render the pilot_nstack */
   for (j=0, i=1; i<pilot_nstack; i++) { /* skip the player */
      if (pilot_stack[i]->id == player->target) j = i;
      else gui_renderPilot(pilot_stack[i]);
   }
   /* render the targetted pilot */
   if (j!=0) gui_renderPilot(pilot_stack[j]);


   /* the + sign in the middle of the radar representing the player */
   glBegin(GL_LINES);
      COLOUR(cRadar_player);
      glVertex2d(  0., -3. );
      glVertex2d(  0.,  3. );
      glVertex2d( -3.,  0. );
      glVertex2d(  3.,  0. );
   glEnd(); /* GL_LINES */

   glPopMatrix(); /* GL_PROJECTION */


   /*
    * NAV 
    */
   if (planet_target >= 0) { /* planet landing target */
      gl_printMid( NULL, (int)gui.nav.w,
            gui.nav.x, gui.nav.y - 5,
            &cConsole, "Land" );

      gl_printMid( &gl_smallFont, (int)gui.nav.w,
            gui.nav.x, gui.nav.y - 10 - gl_smallFont.h,
            NULL, "%s", cur_system->planets[planet_target]->name );
   }
   else if (hyperspace_target >= 0) { /* hyperspace target */

      sys = &systems_stack[cur_system->jumps[hyperspace_target]];

      /* Determine if we have to play the "enter hyperspace range" sound. */
      i = space_canHyperspace(player);
      if ((i != 0) && (i != can_jump))
         if (!pilot_isFlag(player,PILOT_HYPERSPACE))
            player_playSound(snd_jump, 1);
      can_jump = i;

      /* Determine the colour of the NAV text. */
      if (can_jump || pilot_isFlag(player,PILOT_HYPERSPACE) ||
             pilot_isFlag(player,PILOT_HYP_PREP) ||
             pilot_isFlag(player,PILOT_HYP_BEGIN))
         c = &cConsole;
      else c = NULL;
      gl_printMid( NULL, (int)gui.nav.w,
            gui.nav.x, gui.nav.y - 5,
            c, "Hyperspace" );

      gl_printMid( &gl_smallFont, (int)gui.nav.w,
            gui.nav.x, gui.nav.y - 10 - gl_smallFont.h,
            NULL, "%d - %s", pilot_getJumps(player),
            (sys_isKnown(sys)) ? sys->name : "Unknown" );
   }
   else { /* no NAV target */
      gl_printMid( NULL, (int)gui.nav.w,
            gui.nav.x, gui.nav.y - 5,
            &cConsole, "Navigation" );

      gl_printMid( &gl_smallFont, (int)gui.nav.w,
            gui.nav.x, gui.nav.y - 10 - gl_smallFont.h,
            &cGrey, "Off" );
   }


   /*
    * health
    */
   gui_renderBar( &cShield,  &gui.shield,
         player->shield / player->shield_max );
   gui_renderBar( &cArmour, &gui.armour,
         player->armour / player->armour_max );
   gui_renderBar( &cEnergy, &gui.energy,
         player->energy / player->energy_max );


   /* 
    * weapon
    */ 
   if (player->secondary==NULL) { /* no secondary weapon */ 
      gl_printMid( NULL, (int)gui.weapon.w,
            gui.weapon.x, gui.weapon.y - 5,
            &cConsole, "Secondary" ); 

      gl_printMid( &gl_smallFont, (int)gui.weapon.w,
            gui.weapon.x, gui.weapon.y - 10 - gl_defFont.h,
            &cGrey, "None"); 
   }  
   else {
      f = &gl_defFont;

      quantity = pilot_oquantity(player,player->secondary);
      delay = outfit_delay(player->secondary->outfit);

      /* check to see if weapon is ready */
      if ((player->secondary->timer > 0) &&
            (SDL_GetTicks() - player->secondary->timer) < (unsigned int)(delay / quantity))
         c = &cGrey;
      else
         c = &cConsole;


      /* Launcher. */
      if (player->ammo != NULL) {
         /* use the ammunition's name */
         i = gl_printWidth( f, "%s", outfit_ammo(player->secondary->outfit)->name);
         if (i > gui.weapon.w) /* font is too big */
            f = &gl_smallFont;

         /* Weapon name. */
         gl_printMid( f, (int)gui.weapon.w,
               gui.weapon.x, gui.weapon.y - 5,
               (player->ammo) ? c : &cGrey, "%s",
               outfit_ammo(player->secondary->outfit)->name );

         /* Print ammo left underneath. */
         gl_printMid( &gl_smallFont, (int)gui.weapon.w,
               gui.weapon.x, gui.weapon.y - 10 - gl_defFont.h,
               NULL, "%d", (player->ammo) ? player->ammo->quantity : 0 );
      }
      /* Other. */
      else { /* just print the item name */
         i = gl_printWidth( f, "%s", player->secondary->outfit->name);
         if (i > (int)gui.weapon.w) /* font is too big */
            f = &gl_smallFont;
         gl_printMid( f, (int)gui.weapon.w,
               gui.weapon.x, gui.weapon.y - (gui.weapon.h - f->h)/2.,
               c, "%s", player->secondary->outfit->name );

      }
   } 


   /*
    * target
    */
   if (player->target != PLAYER_ID) {
      p = pilot_get(player->target);

      /* blit the pilot target */
      gl_blitStatic( p->ship->gfx_target, gui.target.x, gui.target.y, NULL );
      /* blit the pilot space image */
      /*x = gui.target.x + (TARGET_WIDTH - p->ship->gfx_space->sw)/2.;
      y = gui.target.y + (TARGET_HEIGHT - p->ship->gfx_space->sh)/2.;
      gl_blitStaticSprite( p->ship->gfx_space,
            x, y, p->tsx, p->tsy, NULL );*/


      /* target name */
      gl_print( NULL,
            gui.target_name.x,
            gui.target_name.y,
            NULL, "%s", p->name );
      gl_print( &gl_smallFont,
            gui.target_faction.x,
            gui.target_faction.y,
            NULL, "%s", faction_name(p->faction) );

      /* target status */
      if (pilot_isDisabled(p)) /* pilot is disabled */
         gl_print( &gl_smallFont,
               gui.target_health.x,
               gui.target_health.y,
               NULL, "Disabled" );

      else if (p->shield > p->shield_max/100.) /* on shields */
         gl_print( &gl_smallFont,
            gui.target_health.x,
               gui.target_health.y, NULL,
               "%s: %.0f%%", "Shield", p->shield/p->shield_max*100. );

      else /* on armour */
         gl_print( &gl_smallFont,
               gui.target_health.x,
               gui.target_health.y, NULL, 
               "%s: %.0f%%", "Armour", p->armour/p->armour_max*100. );
   }
   else { /* no target */
      gl_printMid( NULL, SHIP_TARGET_W,
            gui.target.x, gui.target.y  + (SHIP_TARGET_H - gl_defFont.h)/2.,
            &cGrey, "No Target" );
   }


   /*
    * misc
    */
   /* monies */
   j = gui.misc.y - 8 - gl_smallFont.h;
   gl_print( &gl_smallFont,
         gui.misc.x + 8, j,
         &cConsole, "Creds:" );
   credits2str( str, player->credits, 2 );
   i = gl_printWidth( &gl_smallFont, str );
   gl_print( &gl_smallFont,
         gui.misc.x + gui.misc.w - 8 - i, j,
         NULL, str );
   /* cargo and friends */
   if (player->ncommodities > 0) {
      j -= gl_smallFont.h + 5;
      gl_print( &gl_smallFont,
            gui.misc.x + 8, j,
            &cConsole, "Cargo:" );
      for (i=0; i < MIN(player->ncommodities,3); i++) { 
         j -= gl_smallFont.h + 3;
         if (player->commodities[i].quantity) /* quantity is over */
            gl_printMax( &gl_smallFont, gui.misc.w - 15,
                  gui.misc.x + 13, j,
                  NULL, "%d %s%s", player->commodities[i].quantity,
                  player->commodities[i].commodity->name,
                  (player->commodities[i].id) ? "*" : "" );
         else /* basically for weightless mission stuff */ 
            gl_printMax( &gl_smallFont, gui.misc.w - 15,
                  gui.misc.x + 13, j,
                  NULL, "%s%s",  player->commodities[i].commodity->name,
                  (player->commodities[i].id) ? "*" : "" );

      }
   }

   j -= gl_smallFont.h + 5;
   gl_print( &gl_smallFont,
         gui.misc.x + 8, j,
         &cConsole, "Free:" );
   i = gl_printWidth( &gl_smallFont, "%d", player->ship->cap_cargo );
   gl_print( &gl_smallFont,
         gui.misc.x + gui.misc.w - 8 - i, j,
         NULL, "%d", pilot_cargoFree(player) );


   /*
    * messages
    */
   x = gui.mesg.x;
   y = gui.mesg.y + (double)(gl_defFont.h*mesg_max)*1.2;
   c2.r = c2.g = c2.b = 1.;
   for (i=0; i<mesg_max; i++) {
      y -= (double)gl_defFont.h*1.2;
      if (mesg_stack[mesg_max-i-1].str[0]!='\0') {
         if (mesg_stack[mesg_max-i-1].t < t)
            mesg_stack[mesg_max-i-1].str[0] = '\0';
         else {
            if (mesg_stack[mesg_max-i-1].t - mesg_timeout/2 < t)
               c2.a = (double)(mesg_stack[mesg_max-i-1].t - t) / (double)(mesg_timeout/2);
            else
               c2.a = 1.;
            gl_print( NULL, x, y, &c2, "%s", mesg_stack[mesg_max-i-1].str );
         }
      }
   }


   /*
    * hyperspace
    */
   if (pilot_isFlag(player, PILOT_HYPERSPACE)) {
      i = (int)player->ptimer - HYPERSPACE_FADEOUT;
      if (paused) i += t;
      j = (int)t;
      if (i < j) {
         x = (double)(j-i) / HYPERSPACE_FADEOUT;
         glColor4d(1.,1.,1., x );
         glBegin(GL_QUADS);
            glVertex2d( -SCREEN_W/2., -SCREEN_H/2. );
            glVertex2d( -SCREEN_W/2.,  SCREEN_H/2. );
            glVertex2d(  SCREEN_W/2.,  SCREEN_H/2. );
            glVertex2d(  SCREEN_W/2., -SCREEN_H/2. );
         glEnd(); /* GL_QUADS */
      }
   }
}

/*
 * renders a pilot
 */
static void gui_renderPilot( const Pilot* p )
{
   int x, y, sx, sy;
   double w, h;
   glColour *col;

   x = (p->solid->pos.x - player->solid->pos.x) / gui.radar.res;
   y = (p->solid->pos.y - player->solid->pos.y) / gui.radar.res;
   sx = PILOT_SIZE_APROX/2. * p->ship->gfx_space->sw / gui.radar.res;
   sy = PILOT_SIZE_APROX/2. * p->ship->gfx_space->sh / gui.radar.res;
   if (sx < 1.) sx = 1.;
   if (sy < 1.) sy = 1.;

   if ( ((gui.radar.shape==RADAR_RECT) &&
            ((ABS(x) > gui.radar.w/2+sx) || (ABS(y) > gui.radar.h/2.+sy)) ) ||
         ((gui.radar.shape==RADAR_CIRCLE) &&
            ((x*x+y*y) > (int)(gui.radar.w*gui.radar.w))) )
      return; /* pilot not in range */

   if (gui.radar.shape==RADAR_RECT) {
      w = gui.radar.w/2.;
      h = gui.radar.h/2.;
   }
   else if (gui.radar.shape==RADAR_CIRCLE) {
      w = gui.radar.w;
      h = gui.radar.w;
   }

   glBegin(GL_QUADS);
      /* colors */
      if (p->id == player->target) col = &cRadar_targ;
      else if (pilot_isDisabled(p)) col = &cInert;
      else if (pilot_isFlag(p,PILOT_BRIBED)) col = &cNeutral;
      else if (pilot_isFlag(p,PILOT_HOSTILE)) col = &cHostile;
      else col = faction_getColour(p->faction);
      COLOUR(*col);

      /* image */
      glVertex2d( MAX(x-sx,-w), MIN(y+sy, h) ); /* top-left */
      glVertex2d( MIN(x+sx, w), MIN(y+sy, h) );/* top-right */
      glVertex2d( MIN(x+sx, w), MAX(y-sy,-h) );/* bottom-right */
      glVertex2d( MAX(x-sx,-w), MAX(y-sy,-h) );/* bottom-left */
   glEnd(); /* GL_QUADS */
}


/*
 * renders a bar (health)
 */
static void gui_renderBar( const glColour* c,
      const Rect* r, const double w )
{
   int x, y, sx, sy;

   glBegin(GL_QUADS); /* shield */
      COLOUR(*c); 
      x = r->x - SCREEN_W/2.;
      y = r->y - SCREEN_H/2.;
      sx = w * r->w;
      sy = r->h;
      glVertex2d( x, y );
      glVertex2d( x + sx, y );
      glVertex2d( x + sx, y - sy );
      glVertex2d( x, y - sy );                                            
   glEnd(); /* GL_QUADS */

}


/**
 * @fn int gui_init (void)
 *
 * @brief Initializes the GUI system.
 *
 *    @return 0 on success;
 */
int gui_init (void)
{
   /*
    * set gfx to NULL
    */
   gui.gfx_frame = NULL;
   gui.gfx_targetPilot = NULL;
   gui.gfx_targetPlanet = NULL;

   /*
    * radar
    */
   gui.radar.res = RADAR_RES_DEFAULT;

   /*
    * messages
    */
   gui.mesg.x = 20;
   gui.mesg.y = 30;
   mesg_stack = calloc(mesg_max, sizeof(Mesg));
   if (mesg_stack == NULL) {
      ERR("Out of memory!");
      return -1;
   }

   return 0;
}


/**
 * @fn int gui_load (const char* name)
 *
 * @brief Attempts to load the actual GUI.
 *
 *    @param name Name of the GUI to load.
 *    @return 0 on success.
 */
int gui_load( const char* name )
{
   uint32_t bufsize;
   char *buf = pack_readfile( DATA, GUI_DATA, &bufsize );
   char *tmp;
   int found = 0;

   xmlNodePtr node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_GUI_ID)) {
      ERR("Malformed '"GUI_DATA"' file: missing root element '"XML_GUI_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"GUI_DATA"' file: does not contain elements");
      return -1;
   }                                                                                       
   do {
      if (xml_isNode(node, XML_GUI_TAG)) {

         tmp = xml_nodeProp(node,"name"); /* mallocs */

         /* is the gui we are looking for? */
         if (strcmp(tmp,name)==0) {
            found = 1;

            /* parse the xml node */
            if (gui_parse(node,name)) WARN("Trouble loading GUI '%s'", name);
            free(tmp);
            break;
         }

         free(tmp);
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);

   if (!found) {
      WARN("GUI '%s' not found in '"GUI_DATA"'",name);
      return -1;
   }

   return 0;
}


/*
 * used to pull out a rect from an xml node (<x><y><w><h>)
 */
static void rect_parse( const xmlNodePtr parent,
      double *x, double *y, double *w, double *h )
{
   xmlNodePtr cur;
   int param;

   param = 0;

   cur = parent->children;
   do {
      if (xml_isNode(cur,"x")) {
         if (x!=NULL) {
            *x = xml_getFloat(cur);
            param |= (1<<0);
         }
         else WARN("Extra parameter 'x' found for GUI node '%s'", parent->name);
      }
      else if (xml_isNode(cur,"y")) {
         if (y!=NULL) {
            *y = xml_getFloat(cur);
            param |= (1<<1);
         }
         else WARN("Extra parameter 'y' found for GUI node '%s'", parent->name);
      }
      else if (xml_isNode(cur,"w")) {
         if (w!=NULL) {
            *w = xml_getFloat(cur);
            param |= (1<<2);
         }
         else WARN("Extra parameter 'w' found for GUI node '%s'", parent->name);
      }
      else if (xml_isNode(cur,"h")) {
         if (h!=NULL) {
            *h = xml_getFloat(cur);
            param |= (1<<3);
         }
         else WARN("Extra parameter 'h' found for GUI node '%s'", parent->name);
      }
   } while (xml_nextNode(cur));

   /* check to see if we got everything we asked for */
   if (x && !(param & (1<<0)))
      WARN("Missing parameter 'x' for GUI node '%s'", parent->name);
   else if (y && !(param & (1<<1))) 
      WARN("Missing parameter 'y' for GUI node '%s'", parent->name);
   else if (w && !(param & (1<<2))) 
      WARN("Missing parameter 'w' for GUI node '%s'", parent->name);
   else if (h && !(param & (1<<3))) 
      WARN("Missing parameter 'h' for GUI node '%s'", parent->name);
}


/*
 * parse a gui node
 */
#define RELATIVIZE(a)   \
{(a).x+=VX(gui.frame); (a).y=VY(gui.frame)+gui.gfx_frame->h-(a).y;}
static int gui_parse( const xmlNodePtr parent, const char *name )
{
   xmlNodePtr cur, node;
   char *tmp, buf[PATH_MAX];


   /*
    * gfx
    */
   /* set as a property and not a node because it must be loaded first */
   tmp = xml_nodeProp(parent,"gfx");
   if (tmp==NULL) {
      ERR("GUI '%s' has no gfx property",name);
      return -1;
   }

   /* load gfx */
   /* frame */
   snprintf( buf, PATH_MAX, GUI_GFX"%s.png", tmp );
   if (gui.gfx_frame) gl_freeTexture(gui.gfx_frame); /* free if needed */
   gui.gfx_frame = gl_newImage( buf );
   /* pilot */
   snprintf( buf, PATH_MAX, GUI_GFX"%s_pilot.png", tmp );
   if (gui.gfx_targetPilot) gl_freeTexture(gui.gfx_targetPilot); /* free if needed */
   gui.gfx_targetPilot = gl_newSprite( buf, 2, 2 );
   /* planet */
   snprintf( buf, PATH_MAX, GUI_GFX"%s_planet.png", tmp );
   if (gui.gfx_targetPlanet) gl_freeTexture(gui.gfx_targetPlanet); /* free if needed */
   gui.gfx_targetPlanet = gl_newSprite( buf, 2, 2 );
   free(tmp);

   /*
    * frame (based on gfx)
    */
   vect_csetmin( &gui.frame,
         SCREEN_W - gui.gfx_frame->w,     /* x */
         SCREEN_H - gui.gfx_frame->h );   /* y */

   /* now actually parse the data */
   node = parent->children;
   do { /* load all the data */

      /*
       * offset
       */
      if (xml_isNode(node,"offset"))
         rect_parse( node, &gui_xoff, &gui_yoff, NULL, NULL );

      /*
       * radar
       */
      else if (xml_isNode(node,"radar")) {

         tmp = xml_nodeProp(node,"type");

         /* make sure type is valid */
         if (strcmp(tmp,"rectangle")==0) gui.radar.shape = RADAR_RECT;
         else if (strcmp(tmp,"circle")==0) gui.radar.shape = RADAR_CIRCLE;
         else {
            WARN("Radar for GUI '%s' is missing 'type' tag or has invalid 'type' tag",name);
            gui.radar.shape = RADAR_RECT;
         }

         free(tmp);
      
         /* load the appropriate measurements */
         if (gui.radar.shape == RADAR_RECT)
            rect_parse( node, &gui.radar.x, &gui.radar.y, &gui.radar.w, &gui.radar.h );
         else if (gui.radar.shape == RADAR_CIRCLE)
            rect_parse( node, &gui.radar.x, &gui.radar.y, &gui.radar.w, NULL );
         RELATIVIZE(gui.radar);
      }

      /*
       * nav computer
       */
      else if (xml_isNode(node,"nav")) {
         rect_parse( node, &gui.nav.x, &gui.nav.y, &gui.nav.w, &gui.nav.h );
         RELATIVIZE(gui.nav);
         gui.nav.y -= gl_defFont.h;
      }

      /*
       * health bars
       */
      else if (xml_isNode(node,"health")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"shield")) {
               rect_parse( cur, &gui.shield.x, &gui.shield.y,
                     &gui.shield.w, &gui.shield.h );
               RELATIVIZE(gui.shield);
            }
            if (xml_isNode(cur,"armour")) {
               rect_parse( cur, &gui.armour.x, &gui.armour.y,
                     &gui.armour.w, &gui.armour.h );
               RELATIVIZE(gui.armour);
            }
            if (xml_isNode(cur,"energy")) {
               rect_parse( cur, &gui.energy.x, &gui.energy.y,
                     &gui.energy.w, &gui.energy.h );
               RELATIVIZE(gui.energy);
            }
         } while (xml_nextNode(cur));
      }

      /*
       * secondary weapon
       */
      else if (xml_isNode(node,"weapon")) {
         rect_parse( node, &gui.weapon.x, &gui.weapon.y,
               &gui.weapon.w, &gui.weapon.h );
         RELATIVIZE(gui.weapon);
         gui.weapon.y -= gl_defFont.h;
      }

      /*
       * target
       */
      else if (xml_isNode(node,"target")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"gfx")) {
               rect_parse( cur, &gui.target.x, &gui.target.y, NULL, NULL );
               RELATIVIZE(gui.target);
               gui.target.y -= SHIP_TARGET_H;
            }
            else if (xml_isNode(cur,"name")) {
               rect_parse( cur, &gui.target_name.x, &gui.target_name.y, NULL, NULL );
               RELATIVIZE(gui.target_name);
               gui.target_name.y -= gl_defFont.h;
            }
            else if (xml_isNode(cur,"faction")) {
               rect_parse( cur, &gui.target_faction.x, &gui.target_faction.y, NULL, NULL );
               RELATIVIZE(gui.target_faction);
               gui.target_faction.y -= gl_smallFont.h;
            }
            else if (xml_isNode(cur,"health")) {
               rect_parse( cur, &gui.target_health.x, &gui.target_health.y, NULL, NULL );
               RELATIVIZE(gui.target_health);
               gui.target_health.y -= gl_smallFont.h;
            }
         } while (xml_nextNode(cur));
      }

      /*
       * misc
       */
      else if (xml_isNode(node,"misc")) {
         rect_parse( node, &gui.misc.x, &gui.misc.y, &gui.misc.w, &gui.misc.h );
         RELATIVIZE(gui.misc);
      }
   } while (xml_nextNode(node));

   return 0;
}
#undef RELATIVIZE
/**
 * @fn void gui_free (void)
 *
 * @brief Frees the GUI.
 */
void gui_free (void)
{
   if (gui.gfx_frame) gl_freeTexture( gui.gfx_frame );
   if (gui.gfx_targetPilot) gl_freeTexture( gui.gfx_targetPilot );
   if (gui.gfx_targetPlanet) gl_freeTexture( gui.gfx_targetPlanet );

   free(mesg_stack);
}


/**
 * @fn void player_startAutonav (void)
 *
 * @brief Starts autonav.
 */
void player_startAutonav (void)
{
   if (hyperspace_target == -1)
      return;

   player_message("Autonav initialized.");
   player_setFlag(PLAYER_AUTONAV);
}


/**
 * @fn void player_abortAutonav (void)
 *
 * @brief Aborts autonav.
 */
void player_abortAutonav (void)
{
   if (player_isFlag(PLAYER_AUTONAV)) {
      player_message("Autonav aborted!");
      player_rmFlag(PLAYER_AUTONAV);

      /* Get rid of acceleration. */
      player_accelOver();

      /* Break possible hyperspacing. */
      if (pilot_isFlag(player, PILOT_HYP_PREP)) {
         pilot_hyperspaceAbort(player);
         player_message("Aborting hyperspace sequence.");
      }
   }
}


/**
 * @fn void player_think( Pilot* pplayer )
 *
 * @brief Basically uses keyboard input instead of AI input. Used in pilot.c.
 *
 *    @param pplayer Player to think.
 */
void player_think( Pilot* pplayer )
{
   /* last i heard, the dead don't think */
   if (pilot_isFlag(pplayer,PILOT_DEAD)) {
      /* no sense in accelerating or turning */
      pplayer->solid->dir_vel = 0.;
      vect_pset( &player->solid->force, 0., 0. );
      return;
   }

   /* Autonav takes over normal controls. */
   if (player_isFlag(PLAYER_AUTONAV)) {
      if (pplayer->lockons > 0)
         player_abortAutonav();

      if (space_canHyperspace(pplayer)) {
         player_jump();
      }
      else  {
         pilot_face( pplayer, VANGLE(pplayer->solid->pos) );
         if (player_acc < 1.)
            player_accel( 1. );
      }
   }

   /* turning taken over by PLAYER_FACE */
   if (player_isFlag(PLAYER_FACE)) { 
      if (player->target != PLAYER_ID)
         pilot_face( pplayer,
               vect_angle( &player->solid->pos,
                  &pilot_get(player->target)->solid->pos ));
      else if (planet_target != -1)
         pilot_face( pplayer,
               vect_angle( &player->solid->pos,
                  &cur_system->planets[ planet_target ]->pos ));
   }

   /* turning taken over by PLAYER_REVERSE */
   else if (player_isFlag(PLAYER_REVERSE) && (VMOD(pplayer->solid->vel) > 0.))
      pilot_face( pplayer, VANGLE(player->solid->vel) + M_PI );

   /* normal turning scheme */
   else {
      pplayer->solid->dir_vel = 0.;
      if (player_turn)
         pplayer->solid->dir_vel -= pplayer->turn * player_turn;
   }

   /*
    * Weapon shooting stuff
    */
   /* Primary weapon. */
   if (player_isFlag(PLAYER_PRIMARY)) {
      pilot_shoot( pplayer, 0 );
      player_setFlag(PLAYER_PRIMARY_L);
   }
   else if (player_isFlag(PLAYER_PRIMARY_L)) {
      pilot_shootStop( pplayer, 0 );
      player_rmFlag(PLAYER_PRIMARY_L);
   }
   /* Secondary weapon. */
   if (player_isFlag(PLAYER_SECONDARY)) { /* needs target */
      /* Double tap stops beams. */
      if (!player_isFlag(PLAYER_SECONDARY_L) &&
            (pplayer->secondary != NULL) &&
            outfit_isBeam(pplayer->secondary->outfit)) {
         pilot_shootStop( pplayer, 1 );
      }
      else
         pilot_shoot( pplayer, 1 );

      player_setFlag(PLAYER_SECONDARY_L);
   }
   else if (player_isFlag(PLAYER_SECONDARY_L)) {
      player_rmFlag(PLAYER_SECONDARY_L);
   }


   /* 
    * Afterburn!
    */
   if (player_isFlag(PLAYER_AFTERBURNER)) {
      if (pilot_isFlag(player,PILOT_AFTERBURNER))
         vect_pset( &pplayer->solid->force,
               pplayer->thrust * pplayer->afterburner->outfit->u.afb.thrust_perc + 
               pplayer->afterburner->outfit->u.afb.thrust_abs, pplayer->solid->dir );
      else /* Ran out of energy */
         player_afterburnOver();
   }
   else
      vect_pset( &pplayer->solid->force, pplayer->thrust * player_acc,
            pplayer->solid->dir );

   /*
    * Sound
    */
   sound_updateListener( pplayer->solid->dir,
         pplayer->solid->pos.x, pplayer->solid->pos.y );
}


/*
 *
 *    For use in keybindings
 *
 */
/**
 * @fn void player_setRadarRel( int mod )
 *
 * @brief Modifies the radar resolution.
 *
 *    @param mod Number of intervals to jump (up or down).
 */
void player_setRadarRel( int mod )
{
   gui.radar.res += mod * RADAR_RES_INTERVAL;
   if (gui.radar.res > RADAR_RES_MAX) gui.radar.res = RADAR_RES_MAX;
   else if (gui.radar.res < RADAR_RES_MIN) gui.radar.res = RADAR_RES_MIN;

   player_message( "Radar set to %dx.", (int)gui.radar.res );
}


/**
 * @fn void player_secondaryNext (void)
 *
 * @brief Get the next secondary weapon.
 */
void player_secondaryNext (void)
{
   int i = 0;
   
   /* get current secondary weapon pos */
   if (player->secondary != NULL)   
      for (i=0; i<player->noutfits; i++)
         if (&player->outfits[i] == player->secondary) {
            i++;
            break;
         }

   /* get next secondary weapon */
   for (; i<player->noutfits; i++)
      if (outfit_isProp(player->outfits[i].outfit, OUTFIT_PROP_WEAP_SECONDARY)) {
         pilot_switchSecondary( player, i );
         break;
      }

   /* found no bugger outfit */
   if (i >= player->noutfits)
      pilot_switchSecondary( player, -1 );

   /* set ammo */
   pilot_setAmmo(player);
}


/**
 * @fn void player_targetPlanet (void)
 *
 * @brief Cycle through planet targets.
 */
void player_targetPlanet (void)
{
   hyperspace_target = -1;
   player_rmFlag(PLAYER_LANDACK);

   /* no target */
   if ((planet_target==-1) && (cur_system->nplanets > 0)) {
      planet_target = 0;
      player_playSound(snd_nav, 1);
      return;
   }
   
   planet_target++;

   if (planet_target >= cur_system->nplanets) /* last system */
      planet_target = -1;
   else
      player_playSound(snd_nav, 1);
}


/**
 * @fn void player_land (void)
 *
 * @brief Try to land or target closest planet if no land target.
 */
void player_land (void)
{
   int i;
   int tp;
   double td, d;
   Planet *planet;

   if (landed) { /* player is already landed */
      takeoff();
      return;
   }

   /* Check if there are planets to land on. */
   if (cur_system->nplanets == 0) {
      player_message("There are no planets to land on.");
      return;
   }

   if (planet_target >= 0) { /* attempt to land */
      planet = cur_system->planets[planet_target];
      if (!planet_hasService(planet, PLANET_SERVICE_LAND)) {
         player_message( "You can't land here." );
         return;
      }
      else if (!player_isFlag(PLAYER_LANDACK)) { /* no landing authorization */
         if (planet_hasService(planet,PLANET_SERVICE_BASIC)) { /* Basic services */
            if (!areEnemies( player->faction, planet->faction )) { /* Friendly */
               player_message( "%s> Permission to land granted.", planet->name );
               player_setFlag(PLAYER_LANDACK);
               player_playSound(snd_nav,1);
            }
            else /* Hostile */
               player_message( "%s> Land request denied.", planet->name );
         }
         else { /* No shoes, no shirt, no lifeforms, no service. */
            player_message( "Ready to land on %s.", planet->name );
            player_setFlag(PLAYER_LANDACK);
            player_playSound(snd_nav,1);
         }
         return;
      }
      else if (vect_dist(&player->solid->pos,&planet->pos) > planet->gfx_space->sw) {
         player_message("You are too far away to land on %s.", planet->name);
         return;
      } else if ((pow2(VX(player->solid->vel)) + pow2(VY(player->solid->vel))) >
            (double)pow2(MAX_HYPERSPACE_VEL)) {
         player_message("You are going too fast to land on %s.", planet->name);
         return;
      }

      land(planet); /* land the player */
   }
   else { /* get nearest planet target */

      if (cur_system->nplanets == 0) {
         player_message("There are no planets to land on.");
         return;
      }

      td = -1; /* temporary distance */
      tp = -1; /* temporary planet */
      for (i=0; i<cur_system->nplanets; i++) {
         planet = cur_system->planets[i];
         d = vect_dist(&player->solid->pos,&planet->pos);
         if (planet_hasService(planet,PLANET_SERVICE_LAND) &&
                  ((tp==-1) || ((td == -1) || (td > d)))) {
            tp = i;
            td = d;
         }
      }
      planet_target = tp;
      player_rmFlag(PLAYER_LANDACK);

      /* no landable planet */
      if (planet_target < 0) return;

      player_land(); /* rerun land protocol */
   }
}


/**
 * @fn void player_targetHyperspace (void)
 *
 * @brief Gets a hyperspace target.
 */
void player_targetHyperspace (void)
{
   planet_target = -1; /* get rid of planet target */
   player_rmFlag(PLAYER_LANDACK); /* get rid of landing permission */
   hyperspace_target++;
   map_clear(); /* clear the current map path */

   if (hyperspace_target >= cur_system->njumps)
      hyperspace_target = -1;
   else
      player_playSound(snd_nav,1);

   /* Map gets special treatment if open. */
   if (map_isOpen()) {
      if (hyperspace_target == -1)
         map_select( NULL );
      else
         map_select( &systems_stack[cur_system->jumps[hyperspace_target]] );
   }

}


/**
 * @fn void player_jump (void)
 *
 * @brief Actually attempts to jump in hyperspace.
 */
void player_jump (void)
{
   int i;

   /* Must have a jump target and not be already jumping. */
   if ((hyperspace_target == -1) || pilot_isFlag(player, PILOT_HYPERSPACE))
      return;

   /* Already jumping, so we break jump. */
   if (pilot_isFlag(player, PILOT_HYP_PREP)) {
      pilot_hyperspaceAbort(player);
      player_message("Aborting hyperspace sequence.");
      return;
   }

   i = space_hyperspace(player);

   if (i == -1)
      player_message("You are too close to gravity centers to initiate hyperspace.");
   else if (i == -2)
      player_message("You are moving too fast to enter hyperspace.");
   else if (i == -3)
      player_message("You do not have enough fuel to hyperspace jump.");
   else {
      player_message("Preparing for hyperspace.");
      /* Stop acceleration noise. */
      player_accelOver();
      /* Stop possible shooting. */
      pilot_shootStop( player, 0 );
      pilot_shootStop( player, 1 );
   }
}


/**
 * @fn void player_brokeHyperspace (void)
 *
 * @brief Player actually broke hyperspace (entering new system).
 */
void player_brokeHyperspace (void)
{
   unsigned int tl, th;
   double d;

   /* calculates the time it takes, call before space_init */
   tl = (unsigned int) floor( sqrt( (double)player->solid->mass)/5. );
   th = (unsigned int) ceil( sqrt( (double)player->solid->mass)/5. );
   tl *= NTIME_UNIT_LENGTH;
   th *= NTIME_UNIT_LENGTH;
   ntime_inc( RNG( tl, th ) );

   /* enter the new system */
   space_init( systems_stack[cur_system->jumps[hyperspace_target]].name );

   /* set position, the pilot_update will handle lowering vel */
   d = RNGF()*(HYPERSPACE_ENTER_MAX-HYPERSPACE_ENTER_MIN) + HYPERSPACE_ENTER_MIN;
   player_warp( -cos( player->solid->dir ) * d, -sin( player->solid->dir ) * d );

   /* reduce fuel */
   player->fuel -= HYPERSPACE_FUEL;

   /* stop hyperspace */
   pilot_rmFlag( player, PILOT_HYPERSPACE | PILOT_HYP_BEGIN | PILOT_HYP_PREP );

   /* update the map */
   map_jump();

   /* Disable autonavigation if arrived. */
   if (player_isFlag(PLAYER_AUTONAV)) {
      if (hyperspace_target == -1) {
         player_message( "Autonav arrived at destination.");
         player_rmFlag(PLAYER_AUTONAV);
      }
      else {
         player_message( "Autonav continuing until destination (%d jump%s left).",
               map_npath, (map_npath==1) ? "" : "s" );
      }
   }

   /* run the jump hooks */
   hooks_run( "jump" );
   hooks_run( "enter" );
}


/**
 * @fn double player_faceHyperspace (void)
 *
 * @brief Makes player face his hyperspace target.
 *
 *    @return direction to face.
 */
double player_faceHyperspace (void)
{
   double a;
   a = ANGLE( systems_stack[ cur_system->jumps[hyperspace_target] ].pos.x -
            cur_system->pos.x,
         systems_stack[ cur_system->jumps[hyperspace_target] ].pos.y -
            cur_system->pos.y );
   return pilot_face( player, a );
}


/**
 * @fn void player_afterburn (void)
 *
 * @brief Activate the afterburner.
 */
void player_afterburn (void)
{
   /** @todo fancy effect? */
   if ((player != NULL) && (player->afterburner!=NULL)) {
      player_setFlag(PLAYER_AFTERBURNER);
      pilot_setFlag(player,PILOT_AFTERBURNER);
      spfx_shake(player->afterburner->outfit->u.afb.rumble * SHAKE_MAX);
      sound_stopGroup( PLAYER_ENGINE_CHANNEL );
      sound_playGroup( PLAYER_ENGINE_CHANNEL, 
            player->afterburner->outfit->u.afb.sound, 0 );
   }
}


/**
 * @fn void player_afterburnOver (void)
 *
 * @brief Deactivates the afterburner.
 */
void player_afterburnOver (void)
{
   if ((player != NULL) && (player->afterburner!=NULL)) {
      player_rmFlag(PLAYER_AFTERBURNER);
      pilot_rmFlag(player,PILOT_AFTERBURNER);
      sound_stopGroup( PLAYER_ENGINE_CHANNEL );
   }
}


/**
 * @fn void player_accel( double acc )
 *
 * @brief Start accelerating.
 *
 *    @param acc How much thrust should beb applied of maximum (0 - 1).
 */
void player_accel( double acc )
{
   if (player != NULL) {
      player_acc = acc;
      sound_stopGroup( PLAYER_ENGINE_CHANNEL );
      sound_playGroup( PLAYER_ENGINE_CHANNEL,
            player->ship->sound, 0 );
   }
}


/**
 * @fn void player_accelOver (void)
 *
 * @brief Done accelerating.
 */
void player_accelOver (void)
{
   player_acc = 0.;
   sound_stopGroup( PLAYER_ENGINE_CHANNEL );
}


/**
 * @fn void player_targetHostile (void)
 *
 * @brief Targets the nearest hostile enemy to the player.
 */
void player_targetHostile (void)
{  
   unsigned int tp;
   int i;
   double d, td;

   tp=PLAYER_ID;
   d=0;
   for (i=0; i<pilot_nstack; i++) {
      /* Don't get if is bribed. */
      if (pilot_isFlag(pilot_stack[i],PILOT_BRIBED))
         continue;
   
      /* Normal unbribed check. */
      if (pilot_isFlag(pilot_stack[i],PILOT_HOSTILE) ||
            areEnemies(FACTION_PLAYER,pilot_stack[i]->faction)) {
         td = vect_dist(&pilot_stack[i]->solid->pos, &player->solid->pos);       
         if (!pilot_isDisabled(pilot_stack[i]) && ((tp==PLAYER_ID) || (td < d))) {
            d = td;
            tp = pilot_stack[i]->id;
         }
      }
   }

   if ((tp != PLAYER_ID) && (tp != player->target))
      player_playSound( snd_target, 1 );

   player->target = tp;
}


/**
 * @fn void player_targetNext (void)
 *
 * @brief Cycles to next target.
 */
void player_targetNext (void)
{
   player->target = pilot_getNextID(player->target);

   if (player->target != PLAYER_ID)
      player_playSound( snd_target, 1 );
}


/**
 * @fn void player_targetPrev (void)
 *
 * @brief Cycles to previous target.
 */
void player_targetPrev (void)
{
   player->target = pilot_getPrevID(player->target);

   if (player->target != PLAYER_ID)
      player_playSound( snd_target, 1 );
}


/**
 * @fn player_targetNearest (void)
 *
 * @brief Player targets nearest pilot.
 */
void player_targetNearest (void)
{
   unsigned int t;

   t = player->target;
   player->target = pilot_getNearestPilot(player);

   if ((player->target != PLAYER_ID) && (t != player->target))
      player_playSound( snd_target, 1 );
}


/**
 * @fn void player_screenshot (void)
 *
 * @brief Takes a screenshot.
 */
static int screenshot_cur = 0; /**< Current screenshot at. */
void player_screenshot (void)
{
   FILE *fp;
   int done;
   char filename[PATH_MAX];

   if (nfile_dirMakeExist("screenshots")) {
      WARN("Aborting screenshot");
      return;
   }

   done = 0;
   do {
      if (screenshot_cur >= 999) { /* in case the crap system breaks :) */
         WARN("You have reached the maximum amount of screenshots [999]");
         return;
      }
      snprintf( filename, PATH_MAX, "%sscreenshots/screenshot%03d.png",
            nfile_basePath(), screenshot_cur );
      fp = fopen( filename, "r" ); /* yes i know it's a cheesy way to check */
      if (fp==NULL) done = 1;
      else { /* next */
         screenshot_cur++;
         fclose(fp);
      }
      fp = NULL;
   } while (!done);


   /* now proceed to take the screenshot */
   DEBUG( "Taking screenshot [%03d]...", screenshot_cur );
   gl_screenshot(filename);
}


/**
 * @fn void player_hail (void)
 */
void player_hail (void)
{
   if (player->target != player->id)
      comm_open(player->target);
}


/**
 * @fn void player_dead (void)
 *
 * @brief Player got pwned.
 */
void player_dead (void)
{
   gui_xoff = 0.;
   gui_yoff = 0.;
}


/**
 * @fn void player_destroyed (void)
 *
 * @brief Player blew up in a fireball.
 */
void player_destroyed (void)
{
   if (player_isFlag(PLAYER_DESTROYED)) return;

   vectcpy( &player_cam, &player->solid->pos );
   gl_bindCamera( &player_cam );
   player_setFlag(PLAYER_DESTROYED);
   player_timer = SDL_GetTicks() + 5000;
}


/**
 * @fn void player_ships( char** sships, glTexture** tships )
 *
 * @brief Returns a buffer with all the player's ships names
 *        or "None" if there are no ships.
 *
 *    @param sships Fills sships with player_nships ship names.
 *    @param tships Fills sships with player_nships ship target textures.
 *    @return Freshly allocated array with allocated ship names.
 */
void player_ships( char** sships, glTexture** tships )
{
   int i;

   if (player_nstack==0) {
      sships[0] = strdup("None");
      tships[0] = NULL;
   }
   else {
      for (i=0; i < player_nstack; i++) {
         sships[i] = strdup(player_stack[i]->name);
         tships[i] = player_stack[i]->ship->gfx_target;
      }
   }
}


/**
 * @fn int player_nships (void)
 *
 * @brief Gets the amount of ships player has in storage.
 *
 *    @return The number of ships the player has.
 */
int player_nships (void)
{
   return player_nstack;
}


/*
 * returns a specific ship
 */
Pilot* player_getShip( char* shipname )
{
   int i;

   for (i=0; i < player_nstack; i++)
      if (strcmp(player_stack[i]->name, shipname)==0)
         return player_stack[i];

   WARN("Player ship '%s' not found in stack", shipname);
   return NULL;
}


/*
 * returns where a specific ship is
 */
char* player_getLoc( char* shipname )
{
   int i;

   for (i=0; i < player_nstack; i++)
      if (strcmp(player_stack[i]->name, shipname)==0)
         return player_lstack[i];

   WARN("Player ship '%s' not found in stack", shipname);
   return NULL;
}


/*
 * sets the location of a specific ship
 */
void player_setLoc( char* shipname, char* loc )
{
   int i;

   for (i=0; i < player_nstack; i++) {
      if (strcmp(player_stack[i]->name, shipname)==0) {
         free(player_lstack[i]);
         player_lstack[i] = strdup(loc);
         return;
      }
   }

   WARN("Player ship '%s' not found in stack", shipname);
}


/*
 * marks a mission as completed
 */
void player_missionFinished( int id )
{
   missions_ndone++;
   if (missions_ndone > missions_mdone) { /* need to grow */
      missions_mdone += 25;
      missions_done = realloc( missions_done, sizeof(int) * missions_mdone);
   }
   missions_done[ missions_ndone-1 ] = id;
}


/*
 * has player already completed a mission?
 */
int player_missionAlreadyDone( int id )
{
   int i;

   for (i=0; i<missions_ndone; i++)
      if (missions_done[i] == id)
         return 1;
   return 0;
}


/*
 * save the freaking player in a freaking xmlfile
 */
int player_save( xmlTextWriterPtr writer )
{
   int i;
   MissionData *m;

   xmlw_startElem(writer,"player");
   xmlw_attr(writer,"name",player_name);

   xmlw_elem(writer,"rating","%f",player_crating);
   xmlw_elem(writer,"credits","%d",player->credits);
   xmlw_elem(writer,"time","%d",ntime_get());

   xmlw_elem(writer,"location",land_planet->name);
   player_saveShip( writer, player, NULL ); /* current ship */

   xmlw_startElem(writer,"ships");
   for (i=0; i<player_nstack; i++)
      player_saveShip( writer, player_stack[i], player_lstack[i] );
   xmlw_endElem(writer); /* "ships" */

   xmlw_endElem(writer); /* "player" */


   /* Mission the player has done */
   xmlw_startElem(writer,"missions_done");
   for (i=0; i<missions_ndone; i++) {
      m = mission_get(missions_done[i]);
      if (m != NULL) /* In case mission name changes between versions */
         xmlw_elem(writer,"done",mission_get(missions_done[i])->name);
   }
   xmlw_endElem(writer); /* "missions_done" */

   return 0;
}


static int player_saveShip( xmlTextWriterPtr writer,
      Pilot* ship, char* loc )
{
   int i;

   xmlw_startElem(writer,"ship");
   xmlw_attr(writer,"name",ship->name);
   xmlw_attr(writer,"model",ship->ship->name);

   if (loc != NULL) xmlw_elem(writer,"location",loc);

   /* save the fuel */
   xmlw_elem(writer,"fuel","%f",ship->fuel);

   /* save the outfits */
   xmlw_startElem(writer,"outfits");
   for (i=0; i<ship->noutfits; i++) {
      xmlw_startElem(writer,"outfit");

      xmlw_attr(writer,"quantity","%d",ship->outfits[i].quantity);
      xmlw_str(writer,ship->outfits[i].outfit->name);

      xmlw_endElem(writer); /* "outfit" */
   }
   xmlw_endElem(writer); /* "outfits" */

   /* save the commodities */
   xmlw_startElem(writer,"commodities");
   for (i=0; i<ship->ncommodities; i++) {
      xmlw_startElem(writer,"commodity");

      xmlw_attr(writer,"quantity","%d",ship->commodities[i].quantity);
      if (ship->commodities[i].id > 0)
         xmlw_attr(writer,"id","%d",ship->commodities[i].id);
      xmlw_str(writer,ship->commodities[i].commodity->name);

      xmlw_endElem(writer); /* commodity */
   }
   xmlw_endElem(writer); /* "commodities" */

   xmlw_endElem(writer); /* "ship" */

   return 0;
}


/*
 * loads the player stuff
 */
int player_load( xmlNodePtr parent )
{
   xmlNodePtr node;

   /* some cleaning up */
   player_flags = 0;
   player_cleanup();
   var_cleanup();
   missions_cleanup();
   map_close();

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"player"))
         player_parse( node );
      else if (xml_isNode(node,"missions_done"))
         player_parseDone( node );
   } while (xml_nextNode(node));

   return 0;
}
static int player_parse( xmlNodePtr parent )
{
   unsigned int player_time;
   char* planet;
   Planet* pnt;
   int sw,sh;
   xmlNodePtr node, cur;

   node = parent->xmlChildrenNode;

   xmlr_attr(parent,"name",player_name);

   do {

      /* global stuff */
      xmlr_float(node,"rating",player_crating);
      xmlr_int(node,"credits",player_credits);
      xmlr_long(node,"time",player_time);
      xmlr_str(node,"location",planet);

      if (xml_isNode(node,"ship"))
         player_parseShip(node, 1);
      
      if (xml_isNode(node,"ships")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"ship"))
               player_parseShip(cur, 0);
         } while (xml_nextNode(cur));
      }

      if (xml_isNode(node,"missions_done"))
         player_parseDone(node);
   } while (xml_nextNode(node));

   /* set global thingies */
   player->credits =player_credits;
   ntime_set(player_time);

   /* set player in system */
   pnt = planet_get( planet );
   sw = pnt->gfx_space->sw;
   sh = pnt->gfx_space->sh;
   player_warp( pnt->pos.x + RNG(-sw/2,sw/2),
         pnt->pos.y + RNG(-sh/2,sh/2) );
   player->solid->dir = RNG(0,359) * M_PI/180.;
   gl_bindCamera(&player->solid->pos);

   /* initialize the system */
   space_init( planet_getSystem(planet) );
   map_clear(); /* sets the map up */

   /* initialize the sound */
   player_initSound();

   return 0;
}
static int player_parseDone( xmlNodePtr parent )
{
   xmlNodePtr node;

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"done"))
         player_missionFinished( mission_getID( xml_get(node) ) );
   } while (xml_nextNode(node));

   return 0;
}
static int player_parseShip( xmlNodePtr parent, int is_player )
{
   char *name, *model, *loc, *q, *id;
   int i, n;
   double fuel;
   Pilot* ship;
   Outfit* o;
   xmlNodePtr node, cur;
   
   xmlr_attr(parent,"name",name);
   xmlr_attr(parent,"model",model);

   /* player is currently on this ship */
   if (is_player != 0) {
      pilot_create( ship_get(model), name, faction_get("Player"), NULL, 0., NULL, NULL,
            PILOT_PLAYER | PILOT_NO_OUTFITS );
      ship = player;
   }
   else
      ship = pilot_createEmpty( ship_get(model), name, faction_get("Player"), NULL,
            PILOT_PLAYER | PILOT_NO_OUTFITS );

   free(name);
   free(model);

   node = parent->xmlChildrenNode;

   fuel = 0;

   do {
      if (is_player == 0) xmlr_str(node,"location",loc);

      /* get fuel */
      xmlr_float(node,"fuel",fuel);

      if (xml_isNode(node,"outfits")) {
         cur = node->xmlChildrenNode;
         do { /* load each outfit */
            if (xml_isNode(cur,"outfit")) {
               xmlr_attr(cur,"quantity",q);
               n = atoi(q);
               free(q);
               /* adding the outfit */
               o = outfit_get(xml_get(cur));
               if (o != NULL)
                  pilot_addOutfit( ship, o, n );
            }
         } while (xml_nextNode(cur));
      }
      if (xml_isNode(node,"commodities")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"commodity")) {
               xmlr_attr(cur,"quantity",q);
               xmlr_attr(cur,"id",id);
               n = atoi(q);
               if (id == NULL) i = 0;
               else i = atoi(id);
               free(q);
               if (id != NULL) free(id);

               /* actually add the cargo with id hack */
               pilot_addCargo( ship, commodity_get(xml_get(cur)), n );
               if (i != 0) ship->commodities[ ship->ncommodities-1 ].id = i;
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   /* set fuel */
   if (fuel != 0)
      ship->fuel = MIN(ship->fuel_max, fuel);

   /* add it to the stack if it's not what the player is in */
   if (is_player == 0) {
      player_stack = realloc(player_stack, sizeof(Pilot*)*(player_nstack+1));
      player_stack[player_nstack] = ship;
      player_lstack = realloc(player_lstack, sizeof(char*)*(player_nstack+1));
      player_lstack[player_nstack] = strdup(loc);
      player_nstack++;
   }

   return 0;
}


