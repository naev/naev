/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file player.c
 *
 * @brief Contains all the player related stuff.
 */


#include "player.h"

#include "naev.h"

#include <stdlib.h>

#include "nxml.h"
#include "pilot.h"
#include "log.h"
#include "opengl.h"
#include "font.h"
#include "ndata.h"
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
#include "nlua_misn.h"
#include "ntime.h"
#include "hook.h"
#include "map.h"
#include "nfile.h"
#include "spfx.h"
#include "unidiff.h"
#include "comm.h"
#include "intro.h"
#include "perlin.h"
#include "ai.h"
#include "music.h"
#include "gui.h"
#include "nlua_var.h"


#define XML_START_ID "Start" /**< Module start xml document identifier. */

#define START_DATA   "dat/start.xml" /**< Module start information file. */

#define PLAYER_RESERVED_CHANNELS 6 /**< Number of channels to reserve for player sounds. */
#define PLAYER_ENGINE_CHANNEL    8 /**< Player channel for engine noises. */
#define PLAYER_GUI_CHANNEL       9 /**< Player channel. */


/*
 * player stuff
 */
Pilot* player                 = NULL; /**< Ze player. */
static Ship* player_ship      = NULL; /**< Temporary ship to hold when naming it */
static double player_px       = 0.; /**< Temporary X position. */
static double player_py       = 0.; /**< Temporary Y position. */
static double player_vx       = 0.; /**< Temporory X velocity. */
static double player_vy       = 0.; /**< Temporary Y velocity. */
static double player_dir      = 0.; /**< Temporary direction. */
static int player_credits     = 0; /**< Temporary hack for when creating. */
static char *player_mission   = NULL; /**< More hack. */
int player_enemies            = 0; /**< Number of enemies player has in system. */


/*
 * Licenses.
 */
static char **player_licenses = NULL; /**< Licenses player has. */
static int player_nlicenses   = 0; /**< Number of licenses player has. */


/*
 * player sounds.
 */
int snd_target    = -1; /**< Sound when targetting. */
int snd_jump      = -1; /**< Sound when can jump. */
int snd_nav       = -1; /**< Sound when changing nav computer. */


/* 
 * player pilot stack - ships he has 
 */
static Pilot** player_stack   = NULL; /**< Stack of ships player has. */
static char** player_lstack   = NULL; /**< Names of the planet the ships are at. */
static int player_nstack      = 0; /**< Number of ships player has. */


/* 
 * player global properties
 */
char* player_name          = NULL; /**< Ze name. */
double player_crating      = 0; /**< Ze combat rating. */
unsigned int player_flags  = 0; /**< Player flags. */
/* used in input.c */
double player_left         = 0.; /**< Player left turn velocity from input. */
double player_right        = 0.; /**< Player right turn velocity from input. */
static double player_acc   = 0.; /**< Accel velocity from input. */
static int player_firemode = 0; /**< Player fire mode. */
/* used in map.c */
int planet_target          = -1; /**< Targetted planet. -1 is none. */
int hyperspace_target      = -1; /**< Targetted hyperspace route. -1 is none. */
/* for death and such */
static double player_timer = 0.; /**< For death and such. */


/* 
 * unique mission stack
 */
static int* missions_done  = NULL; /**< Saves position of completed missions. */
static int missions_mdone  = 0; /**< Memory size of completed missions. */
static int missions_ndone  = 0; /**< Number of completed missions. */


/*
 * Extern stuff for player ships.
 */
extern Pilot** pilot_stack;
extern int pilot_nstack;


/*
 * map stuff for autonav
 */
extern int map_npath;


/* 
 * prototypes
 */
/*
 * external
 */
extern void pilot_render( const Pilot* pilot ); /**< from pilot.c */
/* 
 * internal
 */
/* creation */
static int player_newMake (void);
static void player_newShipMake( char *name );
/* sound */
static void player_initSound (void);
/* save/load */
static int player_saveShip( xmlTextWriterPtr writer, 
      Pilot* ship, char* loc );
static int player_parse( xmlNodePtr parent );
static int player_parseDone( xmlNodePtr parent );
static int player_parseLicenses( xmlNodePtr parent );
static int player_parseShip( xmlNodePtr parent, int is_player );
/* 
 * externed
 */
int player_save( xmlTextWriterPtr writer ); /* save.c */
int player_load( xmlNodePtr parent ); /* save.c */


/**
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
   gl_cameraStatic( 0., 0. );

   /* Set up GUI. */
   gui_setDefaults();

   /* Setup sound */
   player_initSound();

   /* Clean up player stuff if we'll be recreating. */
   diff_clear();
   player_cleanup();
   var_cleanup();
   missions_cleanup();
   space_clearKnown();
   land_cleanup();
   factions_reset();
   map_cleanup();

   player_name = dialogue_input( "Player Name", 3, 20,
         "Please write your name:" );

   /* Player cancelled dialogue. */
   if (player_name == NULL) {
      menu_main();
      return;
   }

   if (nfile_fileExists("%ssaves/%s.ns", nfile_basePath(), player_name)) {
      r = dialogue_YesNo("Overwrite",
            "You already have a pilot named %s. Overwrite?",player_name);
      if (r==0) { /* no */
         player_new();
         return;
      }
   }

   if (player_newMake())
      return;

   /* Display the intro. */
   intro_display( "dat/intro", "intro" );

   /* Play music. */
   music_choose( "ambient" );

   /* Add the mission if found. */
   if (player_mission != NULL) {
      if (mission_start( player_mission ) < 0)
         WARN("Failed to run start mission '%s'.", player_mission);
      free(player_mission);
      player_mission = NULL;
   }
}


/**
 * @brief Actually creates a new player.
 *
 *    @return 0 on success.
 */
static int player_newMake (void)
{
   Ship *ship;
   char *sysname;;
   uint32_t bufsize;
   char *buf;
   int l,h, tl,th;
   double x,y;
   xmlNodePtr node, cur, tmp;
   xmlDocPtr doc;

   /* Sane defaults. */
   sysname = NULL;
   player_mission = NULL;

   /* Try to read teh file. */
   buf = ndata_read( START_DATA, &bufsize );
   if (buf == NULL)
      return -1;

   /* Load the XML file. */
   doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_START_ID)) {
      ERR("Malformed '"START_DATA"' file: missing root element '"XML_START_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"START_DATA"' file: does not contain elements");
      return -1;
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
                  xmlr_strd(tmp, "name", sysname);
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
            /* Check for mission. */
            if (xml_isNode(cur,"mission")) {
               if (player_mission != NULL) {
                  WARN("start.xml already contains a mission node!");
                  continue;
               }
               player_mission = xml_getStrd(cur);
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   /* Clean up. */
   xmlFreeDoc(doc);
   free(buf);

   /* Monies. */
   player_credits = RNG(l,h);

   /* Time. */
   ntime_set( RNG(tl*1000*NTIME_UNIT_LENGTH,th*1000*NTIME_UNIT_LENGTH) );

   /* Welcome message - must be before space_init. */
   player_message( "Welcome to "APPNAME"!" );
   player_message( " v%d.%d.%d", VMAJOR, VMINOR, VREV );

   /* Try to create the pilot, if fails reask for player name. */
   if (player_newShip( ship, x, y, 0., 0., RNGF() * 2.*M_PI ) != 0) {
      player_new();
      return -1;
   }
   space_init(sysname);
   free(sysname);

   /* clear the map */
   map_clear();

   /* Start the economy. */
   economy_init();

   return 0;
}


/**
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
   player_ship    = ship;
   player_px      = px;
   player_py      = py;
   player_vx      = vx;
   player_vy      = vy;
   player_dir     = dir;
   ship_name      = dialogue_input( "Ship Name", 3, 20,
         "Please name your brand new %s %s:", ship->fabricator, ship->name );

   /* Dialogue cancelled. */
   if (ship_name == NULL)
      return -1;

   player_newShipMake(ship_name);

   free(ship_name);

   return 0;
}

/**
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

      player_credits = player->credits;
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
   gl_cameraBind( &player->solid->pos ); /* set opengl camera */

   /* copy cargo over. */
   if (player_nstack > 0) { /* not during creation though. */
      pilot_moveCargo( player, player_stack[player_nstack-1] );

      /* recalculate stats after cargo movement. */
      pilot_calcStats( player );
      pilot_calcStats( player_stack[player_nstack-1] );
   }

   /* money. */
   player->credits = player_credits;
   player_credits = 0;
}


/**
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
         pilot_moveCargo( ship, player );

         /* extra pass to calculate stats */
         pilot_calcStats( ship );
         pilot_calcStats( player );

         /* now swap the players */
         player_stack[i] = player;
         for (j=0; j<pilot_nstack; j++) /* find pilot in stack to swap */
            if (pilot_stack[j] == player) {
               player         = ship;
               pilot_stack[j] = ship;
               break;
            }

         gl_cameraBind( &player->solid->pos ); /* don't forget the camera */
         return;
      }
   }
   WARN( "Unable to swap player with ship '%s': ship does not exist!", shipname );
}


/**
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
 * @brief Cleans up player stuff like player_stack.
 */
void player_cleanup (void)
{
   int i;

   player_clear();

   /* clean up name */
   if (player_name != NULL) {
      free(player_name);
      player_name = NULL;
   }

   /* Clean up gui. */
   gui_cleanup();

   /* clean up the stack */
   if (player_nstack > 0) {                
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
   if (missions_ndone > 0) {
      free(missions_done);
      missions_done = NULL;
      missions_ndone = 0;
      missions_mdone = 0;
   }

   /* Clean up licenses. */
   if (player_nlicenses > 0) {
      for (i=0; i<player_nlicenses; i++)
         free(player_licenses[i]);
      free(player_licenses);
      player_licenses = NULL;
      player_nlicenses = 0;
   }

   /* just in case purge the pilot stack */
   pilots_cleanAll();
}


static int player_soundReserved = 0; /**< Has the player already reserved sound? */
/**
 * @brief Initializes the player sounds.
 */
static void player_initSound (void)
{
   if (player_soundReserved) return;

   /* Allocate channels. */
   sound_reserve(PLAYER_RESERVED_CHANNELS);
   sound_createGroup(PLAYER_ENGINE_CHANNEL, 0, 1); /* Channel for engine noises. */
   sound_createGroup(PLAYER_GUI_CHANNEL, 1, PLAYER_RESERVED_CHANNELS-1);
   player_soundReserved = 1;

   /* Get sounds. */
   snd_target  = sound_get("target");
   snd_jump    = sound_get("jump");
   snd_nav     = sound_get("nav");
}


/**
 * @brief Plays a sound at the player.
 *
 *    @param sound ID of the sound to play.
 *    @param once Play only once?
 */
void player_playSound( int sound, int once )
{
   sound_playGroup( PLAYER_GUI_CHANNEL, sound, once );
}


/**
 * @brief Stops playing player sounds.
 */
void player_stopSound (void)
{
   sound_stopGroup( PLAYER_GUI_CHANNEL );
   sound_stopGroup( PLAYER_ENGINE_CHANNEL );
}


/**
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
 * @brief Clears the targets.
 */
void player_clear (void)
{
   if (player != NULL)
      player->target = PLAYER_ID;
   planet_target     = -1;
   hyperspace_target = -1;
}


static char* player_ratings[] = {
      "None",
      "Smallfry",
      "Weak",
      "Minor",
      "Average",
      "Major",
      "Fearsome",
      "Godlike"
}; /**< Combat ratings. */
/**
 * @brief Gets the player's combat rating in a human-readable string.
 *
 *    @return The player's combat rating in a human readable string.
 */
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


/**
 * @brief Renders the player
 */
void player_render( double dt )
{
   /*
    * Check to see if the death menu should pop up.
    */
   if (player_isFlag(PLAYER_DESTROYED)) {
      player_timer -= dt;
      if (!toolkit_isOpen() && !player_isFlag(PLAYER_CREATING) &&
            (player_timer < 0.)) {
         menu_death();
      }
   }

   /*
    * Render the player.
    */
   if ((player != NULL) && !player_isFlag(PLAYER_CREATING)) {

      /* Render player's target first. */
      gui_renderTarget(dt);

      /* Player is ontop of targeting graphic */
      pilot_render(player);

   /* Use to test and debug mounts. */
#if 0
      int i;
      Vector2d v;
      double a;
      for (i=0; i<player->ship->nmounts; i++) {
         a  = (double)(player->tsy * player->ship->gfx_space->sx + player->tsx);
         a *= player->ship->mangle;
         ship_getMount( player->ship, a, i, &v );
         glBegin(GL_LINES);
            COLOUR(cRadar_player);
            glVertex2d( v.x + 0., v.y + -3. );
            glVertex2d( v.x + 0., v.y + 3. );
            glVertex2d( v.x +-3., v.y + 0. );
            glVertex2d( v.x + 3., v.y + 0. );
         glEnd(); /* GL_LINES */
      }
#endif
   }
}


/**
 * @brief Starts autonav.
 */
void player_startAutonav (void)
{
   if (hyperspace_target == -1)
      return;

   if (player->fuel < HYPERSPACE_FUEL) {
      player_message("Not enough fuel to jump for autonav.");
      return;
   }

   player_message("Autonav initialized.");
   player_setFlag(PLAYER_AUTONAV);
}


/**
 * @brief Aborts autonav.
 */
void player_abortAutonav( char *reason )
{
   /* No point if player is beyond aborting. */
   if ((player != NULL) && pilot_isFlag(player, PILOT_HYPERSPACE))
      return;

   if (player_isFlag(PLAYER_AUTONAV)) {
      if (reason != NULL)
         player_message("Autonav aborted: %s!", reason);
      else
         player_message("Autonav aborted!");
      player_rmFlag(PLAYER_AUTONAV);

      /* Get rid of acceleration. */
      player_accelOver();

      /* Drop out of possible different speed modes. */
      if (dt_mod != 1.)
         pause_setSpeed(1.);

      /* Break possible hyperspacing. */
      if (pilot_isFlag(player, PILOT_HYP_PREP)) {
         pilot_hyperspaceAbort(player);
         player_message("Aborting hyperspace sequence.");
      }
   }
}


/**
 * @brief Basically uses keyboard input instead of AI input. Used in pilot.c.
 *
 *    @param pplayer Player to think.
 */
void player_think( Pilot* pplayer )
{
   Pilot *target;
   double d;
   double turn;

   /* last i heard, the dead don't think */
   if (pilot_isFlag(pplayer,PILOT_DEAD)) {
      /* no sense in accelerating or turning */
      pplayer->solid->dir_vel = 0.;
      vect_pset( &player->solid->force, 0., 0. );
      return;
   }

   /* Autonav takes over normal controls. */
   if (player_isFlag(PLAYER_AUTONAV)) {
      /* Abort if lockons detected. */
      if (pplayer->lockons > 0)
         player_abortAutonav("Missile Lockon Detected");

      /* Need fuel. */
      else if (pplayer->fuel < HYPERSPACE_FUEL)
         player_abortAutonav("Not enough fuel for autonav to continue.");

      /* Try to jump. */
      else if (space_canHyperspace(pplayer))
         player_jump();

      /* Keey on moving. */
      else  {
         /* Only accelerate if facing move dir. */
         d = pilot_face( pplayer, VANGLE(pplayer->solid->pos) );
         if ((player_acc < 1.) && (d < MIN_DIR_ERR))
            player_accel( 1. );
      }
   }

   /* turning taken over by PLAYER_FACE */
   else if (player_isFlag(PLAYER_FACE)) { 
      /* Try to face pilot target. */
      if (player->target != PLAYER_ID) {
         target = pilot_get(player->target);
         if (target != NULL)
            pilot_face( pplayer,
                  vect_angle( &player->solid->pos, &target->solid->pos ));
      }
      /* If not try to face planet target. */
      else if (planet_target != -1)
         pilot_face( pplayer,
               vect_angle( &player->solid->pos,
                  &cur_system->planets[ planet_target ]->pos ));
   }

   /* turning taken over by PLAYER_REVERSE */
   else if (player_isFlag(PLAYER_REVERSE)) {
      
      /* Check to see if already stopped. */
      /*
      if (VMOD(pplayer->solid->vel) < MIN_VEL_ERR)
         player_accel( 0. );

      else {
         d = pilot_face( pplayer, VANGLE(player->solid->vel) + M_PI );
         if ((player_acc < 1.) && (d < MAX_DIR_ERR))
            player_accel( 1. );
      }
      */

      /*
       * I don't think automatic braking is good.
       */
      pilot_face( pplayer, VANGLE(player->solid->vel) + M_PI );
   }

   /* normal turning scheme */
   else {
      pplayer->solid->dir_vel = 0.;
      turn = 0;
      if (player_isFlag(PLAYER_TURN_LEFT))
         turn -= player_left;
      if (player_isFlag(PLAYER_TURN_RIGHT))
         turn += player_right;
      turn = CLAMP( -1., 1., turn );
      pplayer->solid->dir_vel -= pplayer->turn * turn;
   }

   /*
    * Weapon shooting stuff
    */
   /* Primary weapon. */
   if (player_isFlag(PLAYER_PRIMARY)) {
      pilot_shoot( pplayer, player_firemode );
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
         pilot_shootSecondary( pplayer );

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
 * @brief Get the next secondary weapon.
 */
void player_secondaryNext (void)
{
   int i;
   
   /* get current secondary weapon pos */
   if (player->secondary != NULL)   
      i = player->secondary - player->outfits + 1;
   else
      i = 0;

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
 * @brief Get the previous secondary weapon.
 */
void player_secondaryPrev (void)
{
   int i;
   
   /* get current secondary weapon pos */
   if (player->secondary != NULL)   
      i = player->secondary - player->outfits - 1;
   else
      i = player->noutfits - 1;

   /* get next secondary weapon */
   for (; i>= 0; i--)
      if (outfit_isProp(player->outfits[i].outfit, OUTFIT_PROP_WEAP_SECONDARY)) {
         pilot_switchSecondary( player, i );
         break;
      }

   /* found no bugger outfit */
   if (i < 0)
      pilot_switchSecondary( player, -1 );

   /* set ammo */
   pilot_setAmmo(player);
}


/**
 * @brief Cycle through planet targets.
 */
void player_targetPlanet (void)
{
   /* Clean up some stuff. */
   hyperspace_target = -1;
   player_rmFlag(PLAYER_LANDACK);

   /* Find next planet target. */
   planet_target++;
   while (planet_target < cur_system->nplanets) {

      /* In range, target planet. */
      if (pilot_inRangePlanet( player, planet_target )) {
         player_playSound(snd_nav, 1);
         return;
      }

      planet_target++;
   }

   /* Untarget if out of range. */
   planet_target = -1;
}


/**
 * @brief Try to land or target closest planet if no land target.
 */
void player_land (void)
{
   int i;
   int tp;
   double td, d;
   Planet *planet;

   if (landed) { /* player is already landed */
      takeoff(1);
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
            if (!areEnemies( player->faction, planet->faction ) ||  /* friendly */
                  planet->bribed ) { /* Bribed. */
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

      /* Stop afterburning. */
      player_afterburnOver();
      /* Stop accelerating. */
      player_accelOver();

      /* Open land menu. */
      player_soundPause();
      land(planet);
      player_soundResume();
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
         if (pilot_inRangePlanet( player, i ) &&
               planet_hasService(planet,PLANET_SERVICE_LAND) &&
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
         map_select( system_getIndex( cur_system->jumps[hyperspace_target] ) );
   }

}


/**
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
 * @brief Player actually broke hyperspace (entering new system).
 */
void player_brokeHyperspace (void)
{
   unsigned int tl, th;
   double d;

   /* calculates the time it takes, call before space_init */
   tl = (unsigned int) floor( sqrt( player->solid->mass)/5. );
   th = (unsigned int) ceil( sqrt( player->solid->mass)/5. );
   tl *= NTIME_UNIT_LENGTH;
   th *= NTIME_UNIT_LENGTH;
   ntime_inc( RNG( tl, th ) );

   /* enter the new system */
   space_init( system_getIndex( cur_system->jumps[hyperspace_target] )->name );

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
   hooks_run( "enter" );
}


/**
 * @brief Makes player face his hyperspace target.
 *
 *    @return direction to face.
 */
double player_faceHyperspace (void)
{
   double a;
   StarSystem *sys;

   sys = system_getIndex( cur_system->jumps[hyperspace_target] );
   a = ANGLE( sys->pos.x - cur_system->pos.x, sys->pos.y - cur_system->pos.y );
   return pilot_face( player, a );
}


/**
 * @brief Activate the afterburner.
 */
void player_afterburn (void)
{
   if (pilot_isFlag(player, PILOT_HYP_PREP) || pilot_isFlag(player, PILOT_HYPERSPACE))
      return;

   /** @todo fancy effect? */
   if ((player != NULL) && (player->afterburner!=NULL)) {
      player_setFlag(PLAYER_AFTERBURNER);
      pilot_setFlag(player,PILOT_AFTERBURNER);
      spfx_shake(player->afterburner->outfit->u.afb.rumble * SHAKE_MAX);
      sound_stopGroup( PLAYER_ENGINE_CHANNEL );
      sound_playGroup( PLAYER_ENGINE_CHANNEL, 
            player->afterburner->outfit->u.afb.sound, 0 );
      if (toolkit_isOpen() || paused)
         player_soundPause();
   }
}


/**
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
 * @brief Start accelerating.
 *
 *    @param acc How much thrust should beb applied of maximum (0 - 1).
 */
void player_accel( double acc )
{
   if ((player == NULL) || pilot_isFlag(player, PILOT_HYP_PREP) ||
         pilot_isFlag(player, PILOT_HYPERSPACE))
      return;


   player_acc = acc;
   sound_stopGroup( PLAYER_ENGINE_CHANNEL );
   sound_playGroup( PLAYER_ENGINE_CHANNEL,
         player->ship->sound, 0 );
   if (toolkit_isOpen() || paused)
      player_soundPause();
}


/**
 * @brief Done accelerating.
 */
void player_accelOver (void)
{
   player_acc = 0.;
   sound_stopGroup( PLAYER_ENGINE_CHANNEL );
}


/**
 * @brief Pauses the ship's sounds.
 *
 * @todo Not use hardcoded PLAYER_ENGINE_CHANNEL sound...  Ideally add support
 *  for pausing/resuming groups in SDL_Mixer.
 */
void player_soundPause (void)
{
   sound_pauseChannel(0);
}


/**
 * @brief Resumes the ship's sounds.
 *
 * @todo Not use hardcoded PLAYER_ENGINE_CHANNEL sound...  Ideally add support
 *  for pausing/resuming groups in SDL_Mixer.
 */
void player_soundResume (void)
{
   sound_resumeChannel(0);
}


/**
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
 
      /* Must be in range. */
      if (!pilot_inRangePilot( player, pilot_stack[i] ))
         continue;

      /* Normal unbribed check. */
      if (pilot_isHostile(pilot_stack[i])) {
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
 * @brief Cycles to next target.
 *
 *    @param mode Mode to target. 0 is normal, 1 is hostiles.
 */
void player_targetNext( int mode )
{
   player->target = pilot_getNextID(player->target, mode);

   if (player->target != PLAYER_ID)
      player_playSound( snd_target, 1 );
}


/**
 * @brief Cycles to previous target.
 *
 *    @param mode Mode to target. 0 is normal, 1 is hostiles.
 */
void player_targetPrev( int mode )
{
   player->target = pilot_getPrevID(player->target, mode);

   if (player->target != PLAYER_ID)
      player_playSound( snd_target, 1 );
}


/**
 * @brief Targets the pilot.
 *
 *    @param prev 1 if is cycling backwards.
 */
void player_targetEscort( int prev )
{
   int i;

   /* Check if current target is an escort. */
   for (i=0; i<player->nescorts; i++) {
      if (player->target == player->escorts[i]) {

         /* Cycle targets. */
         if (prev)
            player->target = (i > 0) ?
                  player->escorts[i-1] : PLAYER_ID;
         else
            player->target = (i < player->nescorts-1) ?
                  player->escorts[i+1] : PLAYER_ID;

         break;
      }
   }

   /* Not found in loop. */
   if (i >= player->nescorts) {

      /* Check to see if he actually has escorts. */
      if (player->nescorts > 0) {

         /* Cycle forward or backwards. */
         if (prev)
            player->target = player->escorts[player->nescorts-1];
         else
            player->target = player->escorts[0];
      }
      else
         player->target = PLAYER_ID;
   }


   if (player->target != PLAYER_ID)
      player_playSound( snd_target, 1 );
}



/**
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


static int screenshot_cur = 0; /**< Current screenshot at. */
/**
 * @brief Takes a screenshot.
 */
void player_screenshot (void)
{
   char filename[PATH_MAX];

   if (nfile_dirMakeExist("%sscreenshots", nfile_basePath())) {
      WARN("Aborting screenshot");
      return;
   }

   /* Try to find current screenshots. */
   for ( ; screenshot_cur < 1000; screenshot_cur++) {
      snprintf( filename, PATH_MAX, "%sscreenshots/screenshot%03d.png",
            nfile_basePath(), screenshot_cur );
      if (!nfile_fileExists( filename ))
         break;
   }

   if (screenshot_cur >= 999) { /* in case the crap system breaks :) */
      WARN("You have reached the maximum amount of screenshots [999]");
      return;
   }

   /* now proceed to take the screenshot */
   DEBUG( "Taking screenshot [%03d]...", screenshot_cur );
   gl_screenshot(filename);
}


/**
 * @brief Opens communication with the player's target.
 */
void player_hail (void)
{
   if (player->target != player->id)
      comm_openPilot(player->target);
   else if(planet_target != -1)
      comm_openPlanet( cur_system->planets[ planet_target ] );
   else
      player_message("Who are you hailing?");
}


void player_setFireMode( int mode )
{
   if (player_firemode == mode)
      return;

   player_firemode = mode;

   if (player_firemode == 0)
      player_message("Fire mode set to all weapons.");
   else if (player_firemode == 1)
      player_message("Fire mode set to turret weapons.");
   else if (player_firemode == 2)
      player_message("Fire mode set to forward weapons.");
}


/**
 * @brief Player got pwned.
 */
void player_dead (void)
{
   gui_cleanup();
}


/**
 * @brief Player blew up in a fireball.
 */
void player_destroyed (void)
{
   if (player_isFlag(PLAYER_DESTROYED))
      return;

   gl_cameraStatic( player->solid->pos.x, player->solid->pos.y );
   player_setFlag(PLAYER_DESTROYED);
   player_timer = 5.;
}


/**
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
 * @brief Gets the amount of ships player has in storage.
 *
 *    @return The number of ships the player has.
 */
int player_nships (void)
{
   return player_nstack;
}


/**
 * @brief Gets a specific ship.
 *
 *    @param shipname Nome of the ship to get.
 *    @return The ship matching name.
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


/**
 * @brief Gets where a specific ship is.
 *
 *    @param shipname Ship to check where it is.
 *    @return The location of the ship.
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


/**
 * @brief Sets the location of a specific ship.
 *
 *    @param shipname Name of the ship to change location of.
 *    @param loc Location of the ship to change to.
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


/**
 * @brief Marks a mission as completed.
 *
 *    @param id ID of the mission to mark as completed.
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


/**
 * @brief Checks to see if player has already completed a mission.
 *
 *    @param id ID of the mission to see if player has completed.
 *    @return 1 if player has completed the mission, 0 otherwise.
 */
int player_missionAlreadyDone( int id )
{
   int i;
   for (i=0; i<missions_ndone; i++)
      if (missions_done[i] == id)
         return 1;
   return 0;
}


/**
 * @brief Checks to see if player has license.
 *
 *    @param license License to check to see if the player has.
 *    @return 1 if has license,  0 if doesn't.
 */
int player_hasLicense( char *license )
{
   int i;
   for (i=0; i<player_nlicenses; i++)
      if (strcmp(license, player_licenses[i])==0)
         return 1;
   return 0;
}


/**
 * @brief Gives the player a license.
 *
 *    @brief license License to give the player.
 */
void player_addLicense( char *license )
{
   /* Player already has license. */
   if (player_hasLicense(license))
      return;

   /* Add the license. */
   player_nlicenses++;
   player_licenses = realloc( player_licenses, sizeof(char*)*player_nlicenses );
   player_licenses[player_nlicenses-1] = strdup(license);
}


/**
 * @brief Gets the player's licenses.
 *
 *    @param nlicenses Amount of licenses the player has.
 *    @return Name of the licenses he has.
 */
char **player_getLicenses( int *nlicenses )
{
   *nlicenses = player_nlicenses;
   return player_licenses;
}


/**
 * @brief Save the freaking player in a freaking xmlfile.
 *
 *    @param writer xml Writer to use.
 *    @return 0 on success.
 */
int player_save( xmlTextWriterPtr writer )
{
   int i;
   MissionData *m;

   xmlw_startElem(writer,"player");

   /* Standard player details. */
   xmlw_attr(writer,"name",player_name);
   xmlw_elem(writer,"rating","%f",player_crating);
   xmlw_elem(writer,"credits","%d",player->credits);
   xmlw_elem(writer,"time","%d",ntime_get());

   /* Current ship. */
   xmlw_elem(writer,"location",land_planet->name);
   player_saveShip( writer, player, NULL ); /* current ship */

   /* Ships. */
   xmlw_startElem(writer,"ships");
   for (i=0; i<player_nstack; i++)
      player_saveShip( writer, player_stack[i], player_lstack[i] );
   xmlw_endElem(writer); /* "ships" */

   /* Licenses. */
   xmlw_startElem(writer,"licenses");
   for (i=0; i<player_nlicenses; i++)
      xmlw_elem(writer,"license",player_licenses[i]);
   xmlw_endElem(writer); /* "licenses" */

   xmlw_endElem(writer); /* "player" */


   /* Mission the player has done */
   xmlw_startElem(writer,"missions_done");
   for (i=0; i<missions_ndone; i++) {
      m = mission_get(missions_done[i]);
      if (m != NULL) /* In case mission name changes between versions */
         xmlw_elem(writer,"done",m->name);
   }
   xmlw_endElem(writer); /* "missions_done" */

   return 0;
}


/**
 * @brief Saves a ship.
 *
 *    @param writer XML writer.
 *    @param ship Ship to save.
 *    @param loc Location of the ship.
 *    @return 0 on success.
 */
static int player_saveShip( xmlTextWriterPtr writer,
      Pilot* ship, char* loc )
{
   int i, j, k;
   int found;

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
      /* Remove cargo with id and no mission. */
      if (ship->commodities[i].id > 0) {
         found = 0;
         for (j=0; j<MISSION_MAX; j++) {
            /* Only check active missions. */
            if (player_missions[j].id > 0) {
               /* Now check if it's in the cargo list. */
               for (k=0; k<player_missions[j].ncargo; k++) {
                  /* See if it matches a cargo. */
                  if (player_missions[j].cargo[k] == ship->commodities[i].id) {
                     found = 1;
                     break;
                  }
               }
            }
            if (found)
               break;
         }

         if (!found) {
            WARN("Found mission cargo without associated mission.");
            WARN("Please reload save game to remove the dead cargo.");
            continue;
         }
      }

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


/**
 * @brief Loads the player stuff.
 *
 *    @param parent Node where the player stuff is to be found.
 *    @return 0 on success.
 */
int player_load( xmlNodePtr parent )
{
   xmlNodePtr node;

   /* some cleaning up */
   player_flags = 0;
   map_cleanup();

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"player"))
         player_parse( node );
      else if (xml_isNode(node,"missions_done"))
         player_parseDone( node );
   } while (xml_nextNode(node));

   return 0;
}


/**
 * @brief Parses the player node.
 *
 *    @param parent The player node.
 *    @return 0 on success.
 */
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
      
      else if (xml_isNode(node,"ships")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"ship"))
               player_parseShip(cur, 0);
         } while (xml_nextNode(cur));
      }

      else if (xml_isNode(node,"licenses"))
         player_parseLicenses(node);

   } while (xml_nextNode(node));

   /* Make sure player exists. */
   if (player == NULL) {
      WARN("Savegame has no primary ship node!");
      return -1;
   }

   /* set global thingies */
   player->credits = player_credits;
   ntime_set(player_time);

   /* set player in system */
   pnt = planet_get( planet );
   sw = pnt->gfx_space->sw;
   sh = pnt->gfx_space->sh;
   player_warp( pnt->pos.x + RNG(-sw/2,sw/2),
         pnt->pos.y + RNG(-sh/2,sh/2) );
   player->solid->dir = RNG(0,359) * M_PI/180.;
   gl_cameraBind(&player->solid->pos);

   /* initialize the system */
   music_choose("takeoff");
   space_init( planet_getSystem(planet) );
   map_clear(); /* sets the map up */

   /* initialize the sound */
   player_initSound();

   return 0;
}


/**
 * @brief Parses player's done missions.
 *
 *    @param parent Node of the missions.
 *    @return 0 on success.
 */
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


/**
 * @brief Parses player's licenses.
 *
 *    @param parent Node of the licenses.
 *    @return 0 on success.
 */
static int player_parseLicenses( xmlNodePtr parent )
{
   xmlNodePtr node;

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"license"))
         player_addLicense( xml_get(node) );
   } while (xml_nextNode(node));

   return 0;
}


/**
 * @brief Parses a player's ship.
 *
 *    @param parent Node of the ship.
 *    @param is_player Is it the ship the player is currently in?
 *    @return 0 on success.
 */
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
               i = (id==NULL) ? 0 : atoi(id);
               free(q);
               if (id != NULL)
                  free(id);

               /* actually add the cargo with id hack */
               pilot_addCargo( ship, commodity_get(xml_get(cur)), n );
               if (i != 0)
                  ship->commodities[ ship->ncommodities-1 ].id = i;
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

