/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file player.c
 *
 * @brief Contains all the player.p related stuff.
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
#include "escort.h"
#include "event.h"
#include "conf.h"
#include "nebula.h"


#define XML_START_ID "Start" /**< Module start xml document identifier. */

#define START_DATA   "dat/start.xml" /**< Module start information file. */


/*
 * player.p stuff
 */
Player_t player; /**< Local player. */
static Ship* player_ship      = NULL; /**< Temporary ship to hold when naming it */
static double player_px       = 0.; /**< Temporary X position. */
static double player_py       = 0.; /**< Temporary Y position. */
static double player_vx       = 0.; /**< Temporory X velocity. */
static double player_vy       = 0.; /**< Temporary Y velocity. */
static double player_dir      = 0.; /**< Temporary direction. */
static unsigned long player_creds = 0; /**< Temporary hack for when creating. */
static char *player_mission   = NULL; /**< More hack. */


/*
 * Licenses.
 */
static char **player_licenses = NULL; /**< Licenses player.p has. */
static int player_nlicenses   = 0; /**< Number of licenses player.p has. */


/*
 * player.p sounds.
 */
static int player_engine_group = 0; /**< Player engine sound group. */
static int player_gui_group   = 0; /**< Player gui sound group. */
int snd_target                = -1; /**< Sound when targetting. */
int snd_jump                  = -1; /**< Sound when can jump. */
int snd_nav                   = -1; /**< Sound when changing nav computer. */
int snd_hail                  = -1; /**< Sound when being hailed. */
/* Hyperspace sounds. */
int snd_hypPowUp              = -1; /**< Hyperspace power up sound. */
int snd_hypEng                = -1; /**< Hyperspace engine sound. */
int snd_hypPowDown            = -1; /**< Hyperspace power down sound. */
int snd_hypPowUpJump          = -1; /**< Hyperspace Power up to jump sound. */
int snd_hypJump               = -1; /**< Hyperspace jump sound. */
static int player_lastEngineSound = -1; /**< Last engine sound. */
static int player_hailCounter = 0; /**< Number of times to play the hail. */
static double player_hailTimer = 0.; /**< Timer for hailing. */


/* 
 * player.p pilot stack - ships he has 
 */
/**
 * @brief Player ship.
 */
typedef struct PlayerShip_s {
   Pilot* p; /**< Pilot. */
   char *loc; /**< Location. */
} PlayerShip_t;
static PlayerShip_t* player_stack   = NULL; /**< Stack of ships player.p has. */
static int player_nstack            = 0; /**< Number of ships player.p has. */


/*
 * player.p outfit stack - outfits he has
 */
/**
 * @brief Wrapper for outfits.
 */
typedef struct PlayerOutfit_s {
   const Outfit *o; /**< Actual assosciated outfit. */
   int q; /**< Amount of outfit owned. */
} PlayerOutfit_t;
static PlayerOutfit_t *player_outfits  = NULL; /**< Outfits player.p has. */
static int player_noutfits             = 0; /**< Number of outfits player.p has. */
static int player_moutfits             = 0; /**< Current allocated memory. */
#define OUTFIT_CHUNKSIZE               32 /**< Allocation chunk size. */


/* 
 * player.p global properties
 */
/* used in input.c */
double player_left         = 0.; /**< Player left turn velocity from input. */
double player_right        = 0.; /**< Player right turn velocity from input. */
static double player_acc   = 0.; /**< Accel velocity from input. */
static int player_firemode = 0; /**< Player fire mode. */
/* for death and such */
static double player_timer = 0.; /**< For death and such. */


/* 
 * unique mission stack.
 */
static int* missions_done  = NULL; /**< Saves position of completed missions. */
static int missions_mdone  = 0; /**< Memory size of completed missions. */
static int missions_ndone  = 0; /**< Number of completed missions. */


/*
 * unique event stack.
 */
static int* events_done  = NULL; /**< Saves position of completed events. */
static int events_mdone  = 0; /**< Memory size of completed events. */
static int events_ndone  = 0; /**< Number of completed events. */


/*
 * Extern stuff for player.p ships.
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
 * internal
 */
static void player_updateZoom( double dt );
/* creation */
static int player_newMake (void);
static void player_newShipMake( char *name );
/* sound */
static void player_initSound (void);
/* save/load */
static int player_saveEscorts( xmlTextWriterPtr writer );
static int player_saveShipSlot( xmlTextWriterPtr writer, PilotOutfitSlot *slot, int i );
static int player_saveShip( xmlTextWriterPtr writer, 
      Pilot* ship, char* loc );
static int player_parse( xmlNodePtr parent );
static int player_parseDoneMissions( xmlNodePtr parent );
static int player_parseDoneEvents( xmlNodePtr parent );
static int player_parseLicenses( xmlNodePtr parent );
static void player_parseShipSlot( xmlNodePtr node, Pilot *ship, PilotOutfitSlot *slot );
static int player_parseShip( xmlNodePtr parent, int is_player, char *planet );
static int player_parseEscorts( xmlNodePtr parent );
static void player_addOutfitToPilot( Pilot* pilot, Outfit* outfit, PilotOutfitSlot *s );
/* Misc. */
static void player_autonav (void);
static int player_outfitCompare( const void *arg1, const void *arg2 );
static int player_shipPriceRaw( Pilot *ship );
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
   memset( &player, 0, sizeof(Player_t) );
   player_setFlag(PLAYER_CREATING);
   gl_cameraStatic( 0., 0. );

   /* Set up GUI. */
   gui_setDefaults();

   /* Setup sound */
   player_initSound();

   /* Clean up player.p stuff if we'll be recreating. */
   diff_clear();
   player_cleanup();
   var_cleanup();
   missions_cleanup();
   events_cleanup();
   space_clearKnown();
   land_cleanup();
   factions_reset();
   map_cleanup();

   player.name = dialogue_input( "Player Name", 2, 20,
         "Please write your name:" );

   /* Player cancelled dialogue. */
   if (player.name == NULL) {
      menu_main();
      return;
   }

   if (nfile_fileExists("%ssaves/%s.ns", nfile_basePath(), player.name)) {
      r = dialogue_YesNo("Overwrite",
            "You already have a pilot named %s. Overwrite?",player.name);
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
   ship           = NULL;
   sysname        = NULL;
   player_mission = NULL;
   l              = 0;
   h              = 0;
   tl             = 0;
   th             = 0;

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
      if (xml_isNode(node, "player")) { /* we are interested in the player.p */
         cur = node->children;
         do {
            if (xml_isNode(cur,"ship"))
               ship = ship_get( xml_get(cur) );
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
            xmlr_float(cur,"player_crating",player.crating);
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
   xmlCleanupParser();

   /* Time. */
   if ((tl==0) && (th==0)) {
      WARN("Time not set by module.");
      ntime_set(0);
   }
   else
      ntime_set( RNG(tl*1000*NTIME_UNIT_LENGTH,th*1000*NTIME_UNIT_LENGTH) );

   /* Welcome message - must be before space_init. */
   player_message( "\egWelcome to "APPNAME"!" );
   player_message( "\eg v%d.%d.%d", VMAJOR, VMINOR, VREV );

   /* Try to create the pilot, if fails reask for player.p name. */
   if (ship==NULL) {
      WARN("Ship not set by module.");
      return -1;
   }
   if (player_newShip( ship, x, y, 0., 0., RNGF() * 2.*M_PI, NULL ) != 0) {
      player_new();
      return -1;
   }
   space_init(sysname);
   free(sysname);

   /* Monies. */
   if ((l==0) && (h==0)) {
      WARN("Credits not set by module.");
      player.p->credits = 0;
   }
   else
      player.p->credits = RNG(l,h);

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
      double vx, double vy, double dir, const char *def_name )
{
   char* ship_name;
   int i, len;

   /* temporary values while player.p doesn't exist */
   player_creds = (player.p != NULL) ? player.p->credits : 0;
   player_ship    = ship;
   player_px      = px;
   player_py      = py;
   player_vx      = vx;
   player_vy      = vy;
   player_dir     = dir;
   ship_name      = dialogue_input( "Ship Name", 3, 20,
         "Please name your new ship:" );

   /* Dialogue cancelled. */
   if (ship_name == NULL) {
      /* No default name, fail. */
      if (def_name == NULL)
         return -1;

      /* Add default name. */
      i = 2;
      len = strlen(def_name)+10;
      ship_name = malloc( len );
      strncpy( ship_name, def_name, len );
      while (player_hasShip(ship_name)) {
         snprintf( ship_name, len, "%s %d", def_name, i );
         i++;
      }
   }

   /* Must not have same name. */
   if (player_hasShip(ship_name)) {
      dialogue_msg( "Name collision",
            "Please do not give the ship the same name as another of your ships.");
      return -1;
   }

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
   PilotFlags flags;
   PlayerShip_t *ship;

   /* store the current ship if it exists */
   pilot_clearFlagsRaw( flags );
   pilot_setFlagRaw( flags, PILOT_PLAYER );

   /* in case we're respawning */
   player_rmFlag(PLAYER_CREATING);

   /* create the player.p */
   if (player.p == NULL) {
      /* Hackish position setting */
      vect_cset( &vp, player_px, player_py );
      vect_cset( &vv, player_vx, player_vy );

      /* Create the player. */
      pilot_create( player_ship, name, faction_get("Player"), NULL,
            player_dir, &vp, &vv, flags, -1 );
   }
   else {
      /* Grow memory. */
      player_stack = realloc(player_stack, sizeof(PlayerShip_t)*(player_nstack+1));
      ship        = &player_stack[player_nstack];
      /* Create the ship. */
      ship->p     = pilot_createEmpty( player_ship, name, faction_get("Player"), NULL, flags );
      ship->loc   = strdup( land_planet->name );
      player_nstack++;
   }
   gl_cameraBind( &player.p->solid->pos ); /* set opengl camera */

   /* money. */
   player.p->credits = player_creds;
   player_creds = 0;
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
   Vector2d v;

   for (i=0; i<player_nstack; i++) {
      if (strcmp(shipname,player_stack[i].p->name)==0) { /* swap player.p and ship */
         ship = player_stack[i].p;

         /* move credits over */
         ship->credits = player.p->credits;

         /* move cargo over */
         pilot_moveCargo( ship, player.p );

         /* Store position. */
         vectcpy( &v, &player.p->solid->pos );

         /* extra pass to calculate stats */
         pilot_calcStats( ship );
         pilot_calcStats( player.p );

         /* now swap the players */
         player_stack[i].p = player.p;
         for (j=0; j<pilot_nstack; j++) /* find pilot in stack to swap */
            if (pilot_stack[j] == player.p) {
               player.p         = ship;
               pilot_stack[j] = ship;
               break;
            }

         /* Copy position back. */
         vectcpy( &player.p->solid->pos, &v );

         /* Fill the tank. */
         land_checkAddRefuel();         

         gl_cameraBind( &player.p->solid->pos ); /* don't forget the camera */
         return;
      }
   }
   WARN( "Unable to swap player.p with ship '%s': ship does not exist!", shipname );
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
   Pilot *ship = NULL;

   if (strcmp(shipname,player.p->name)==0) {
      ship = player.p;
   }
   else {
      /* Find the ship. */
      for (i=0; i<player_nstack; i++) {
         if (strcmp(shipname,player_stack[i].p->name)==0) {
            ship = player_stack[i].p;
            break;
         }
      }
   }

   /* Not found. */
   if (ship == NULL) {
      WARN( "Unable to find price for player's ship '%s': ship does not exist!", shipname );
      return -1;
   }

   return player_shipPriceRaw( ship );
}


/**
 * @brief Calculates the price of one of the player's ships.
 *
 *    @param ship Ship to calculate price of.
 *    @return The price of the ship in credits.
 */
static int player_shipPriceRaw( Pilot *ship )
{
   int price;
   int i;

   /* Ship price is base price + outfit prices. */
   price = ship_basePrice( ship->ship );
   for (i=0; i<ship->noutfits; i++) {
      if (ship->outfits[i]->outfit == NULL)
         continue;
      price += ship->outfits[i]->outfit->price;
   }

   return price;
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
      /* Not the ship we are looking for. */
      if (strcmp(shipname,player_stack[i].p->name)!=0)
         continue;

      /* Free player.p ship and location. */
      pilot_free(player_stack[i].p);
      free(player_stack[i].loc);

      /* Move memory to make adjacent. */
      memmove( player_stack+i, player_stack+i+1,
            sizeof(PlayerShip_t) * (player_nstack-i-1) );
      player_nstack--; /* Shrink stack. */
      /* Realloc memory to smaller size. */
      player_stack = realloc( player_stack,
            sizeof(PlayerShip_t) * (player_nstack) );
   }
}


/**
 * @brief Cleans up player.p stuff like player_stack.
 */
void player_cleanup (void)
{
   int i;

   /* Reset controls. */
   player_accelOver();
   player_left  = 0.;
   player_right = 0.;

   /* Clear player. */
   player_clear();

   /* Clear messages. */
   gui_clearMessages();

   /* clean up name */
   if (player.name != NULL) {
      free(player.name);
      player.name = NULL;
   }

   /* Clean up gui. */
   gui_cleanup();

   /* clean up the stack */
   for (i=0; i<player_nstack; i++) {
      pilot_free(player_stack[i].p);
      free(player_stack[i].loc);
   }
   if (player_stack != NULL)
      free(player_stack);
   player_stack = NULL;
   /* nothing left */
   player_nstack = 0;

   /* Free outfits. */
   if (player_outfits != NULL)
      free(player_outfits);
   player_outfits  = NULL;
   player_noutfits = 0;
   player_moutfits = 0;

   /* clean up missions */
   if (missions_done != NULL)
      free(missions_done);
   missions_done = NULL;
   missions_ndone = 0;
   missions_mdone = 0;

   /* Clean up events. */
   if (events_done != NULL)
      free(events_done);
   events_done = NULL;
   events_ndone = 0;
   events_mdone = 0;

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

   /* Reset some player.p stuff. */
   player_creds   = 0;
   player.crating = 0;

   /* Stop the sounds. */
   sound_stopAll();
}


static int player_soundReserved = 0; /**< Has the player.p already reserved sound? */
/**
 * @brief Initializes the player.p sounds.
 */
static void player_initSound (void)
{
   if (player_soundReserved) return;

   /* Allocate channels. */
   player_engine_group  = sound_createGroup(1); /* Channel for engine noises. */
   player_gui_group     = sound_createGroup(4);
   player_soundReserved = 1;

   /* Get sounds. */
   snd_target           = sound_get("target");
   snd_jump             = sound_get("jump");
   snd_nav              = sound_get("nav");
   snd_hail             = sound_get("hail");
   snd_hypPowUp         = sound_get("hyperspace_powerup");
   snd_hypEng           = sound_get("hyperspace_engine");
   snd_hypPowDown       = sound_get("hyperspace_powerdown");
   snd_hypPowUpJump     = sound_get("hyperspace_powerupjump");
   snd_hypJump          = sound_get("hyperspace_jump");
}


/**
 * @brief Plays a sound at the player.
 *
 *    @param sound ID of the sound to play.
 *    @param once Play only once?
 */
void player_playSound( int sound, int once )
{
   sound_playGroup( player_gui_group, sound, once );
}


/**
 * @brief Stops playing player.p sounds.
 */
void player_stopSound (void)
{
   sound_stopGroup( player_gui_group );
   sound_stopGroup( player_engine_group );

   /* No last engine sound. */
   player_lastEngineSound = -1;
}


/**
 * @brief Warps the player.p to the new position
 *
 *    @param x X value of the position to warp to.
 *    @param y Y value of the position to warp to.
 */
void player_warp( const double x, const double y )
{
   vect_cset( &player.p->solid->pos, x, y );
}


/**
 * @brief Clears the targets.
 */
void player_clear (void)
{
   if (player.p != NULL)
      player.p->target = PLAYER_ID;
}


static char* player_ratings[] = {
      "Harmless",
      "Mostly Harmless",
      "Smallfry",
      "Average",
      "Above Average",
      "Major",
      "Intimidating",
      "Fearsome",
      "Terrifying",
      "Unstoppable",
      "Godlike"
}; /**< Combat ratings. */
/**
 * @brief Gets the player's combat rating in a human-readable string.
 *
 *    @return The player's combat rating in a human readable string.
 */
const char* player_rating (void)
{
   if (player.crating == 0.) return player_ratings[0];
   else if (player.crating < 25.) return player_ratings[1];
   else if (player.crating < 50.) return player_ratings[2];
   else if (player.crating < 100.) return player_ratings[3];
   else if (player.crating < 200.) return player_ratings[4];
   else if (player.crating < 500.) return player_ratings[5];
   else if (player.crating < 1000.) return player_ratings[6];
   else if (player.crating < 2000.) return player_ratings[7];
   else if (player.crating < 5000.) return player_ratings[8];
   else if (player.crating < 10000.) return player_ratings[9];
   else return player_ratings[10];
}


/**
 * @brief Checks to see if the player.p has enough credits.
 *
 *    @param amount Amount of credits to check to see if the player.p has.
 *    @return 1 if the player.p has enough credits.
 */
int player_hasCredits( int amount )
{
   return pilot_hasCredits( player.p, amount );
}


/**
 * @brief Modifies the amount of credits the player.p has.
 *
 *    @param amount Quantity to modify player's credits by.
 *    @return Amount of credits the player.p has.
 */
unsigned long player_modCredits( int amount )
{
   return pilot_modCredits( player.p, amount );
}


/**
 * @brief Gets how many of the commodity the player.p has.
 *
 *    @param commodityname Commodity to check how many the player.p owns.
 *    @return The number of commodities owned matching commodityname.
 */
int player_cargoOwned( const char* commodityname )
{
   int i;

   for (i=0; i<player.p->ncommodities; i++)
      if (!player.p->commodities[i].id &&
            strcmp(commodityname, player.p->commodities[i].commodity->name)==0)
         return player.p->commodities[i].quantity;
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
   if ((player.p != NULL) && !player_isFlag(PLAYER_CREATING)) {
      pilot_render(player.p, dt);
   }
}


/**
 * @brief Starts autonav.
 */
void player_startAutonav (void)
{
   if (player.p->nav_hyperspace == -1)
      return;

   if (player.p->fuel < HYPERSPACE_FUEL) {
      player_message("\erNot enough fuel to jump for autonav.");
      return;
   }

   player_message("\epAutonav initialized.");
   player_setFlag(PLAYER_AUTONAV);
   player.autonav = AUTONAV_APPROACH;
}

/**
 * @brief Starts autonav and closes the window.
 */
void player_startAutonavWindow( unsigned int wid, char *str)
{
   (void) str;

   if (player.p->nav_hyperspace == -1)
      return;

   if (player.p->fuel < HYPERSPACE_FUEL) {
      player_message("\erNot enough fuel to jump for autonav.");
      return;
   }

   player_message("\epAutonav initialized.");
   player_setFlag(PLAYER_AUTONAV);

   window_destroy( wid );
}

/**
 * @brief Aborts autonav.
 */
void player_abortAutonav( char *reason )
{
   /* No point if player.p is beyond aborting. */
   if ((player.p==NULL) || ((player.p != NULL) && pilot_isFlag(player.p, PILOT_HYPERSPACE)))
      return;

   if (player_isFlag(PLAYER_AUTONAV)) {
      if (reason != NULL)
         player_message("\erAutonav aborted: %s!", reason);
      else
         player_message("\erAutonav aborted!");
      player_rmFlag(PLAYER_AUTONAV);

      /* Get rid of acceleration. */
      player_accelOver();

      /* Drop out of possible different speed modes. */
      if (dt_mod != 1.)
         pause_setSpeed(1.);

      /* Break possible hyperspacing. */
      if (pilot_isFlag(player.p, PILOT_HYP_PREP)) {
         pilot_hyperspaceAbort(player.p);
         player_message("\epAborting hyperspace sequence.");
      }
   }
}


/**
 * @brief Basically uses keyboard input instead of AI input. Used in pilot.c.
 *
 *    @param pplayer Player to think.
 */
void player_think( Pilot* pplayer, const double dt )
{
   (void) dt;
   Pilot *target;
   double turn;
   int facing;
   Outfit *afb;
   int ret;

   /* last i heard, the dead don't think */
   if (pilot_isFlag(pplayer,PILOT_DEAD)) {
      /* no sense in accelerating or turning */
      pilot_setThrust( pplayer, 0. );
      pilot_setTurn( pplayer, 0. );
      return;
   }

   /* Not facing anything yet. */
   facing = 0;

   /* Autonav takes over normal controls. */
   if (player_isFlag(PLAYER_AUTONAV)) {
      /* Abort if lockons detected. */
      if (pplayer->lockons > 0)
         player_abortAutonav("Missile Lockon Detected");

      /* If we're already at the target. */
      else if (player.p->nav_hyperspace == -1)
         player_abortAutonav("Target changed to current system");

      /* Need fuel. */
      else if (pplayer->fuel < HYPERSPACE_FUEL)
         player_abortAutonav("Not enough fuel for autonav to continue");

      /* Keep on moving. */
      else 
         player_autonav();

      /* Disable turning. */
      facing = 1;
   }

   /* turning taken over by PLAYER_FACE */
   else if (player_isFlag(PLAYER_FACE)) { 
      /* Try to face pilot target. */
      if (player.p->target != PLAYER_ID) {
         target = pilot_get(player.p->target);
         if (target != NULL) {
            pilot_face( pplayer,
                  vect_angle( &player.p->solid->pos, &target->solid->pos ));

            /* Disable turning. */
            facing = 1;
         }
      }
      /* If not try to face planet target. */
      else if (player.p->nav_planet != -1) {
         pilot_face( pplayer,
               vect_angle( &player.p->solid->pos,
                  &cur_system->planets[ player.p->nav_planet ]->pos ));
         /* Disable turning. */
         facing = 1;
      }
      else if (player.p->nav_hyperspace != -1) {
         pilot_face( pplayer,
               vect_angle( &player.p->solid->pos,
                  &cur_system->jumps[ player.p->nav_hyperspace ].pos ));
         /* Disable turning. */
         facing = 1;
      }
   }

   /* turning taken over by PLAYER_REVERSE */
   else if (player_isFlag(PLAYER_REVERSE)) {
      
      /* Check to see if already stopped. */
      /*
      if (VMOD(pplayer->solid->vel) < MIN_VEL_ERR)
         player_accel( 0. );

      else {
         d = pilot_face( pplayer, VANGLE(player.p->solid->vel) + M_PI );
         if ((player_acc < 1.) && (d < MAX_DIR_ERR))
            player_accel( 1. );
      }
      */

      /*
       * I don't think automatic braking is good.
       */
      pilot_face( pplayer, VANGLE(player.p->solid->vel) + M_PI );

      /* Disable turning. */
      facing = 1;
   }

   /* normal turning scheme */
   if (!facing) {
      turn = 0;
      if (player_isFlag(PLAYER_TURN_LEFT))
         turn -= player_left;
      if (player_isFlag(PLAYER_TURN_RIGHT))
         turn += player_right;
      turn = CLAMP( -1., 1., turn );
      pilot_setTurn( pplayer, -turn );
   }

   /*
    * Weapon shooting stuff
    */
   /* Primary weapon. */
   if (player_isFlag(PLAYER_PRIMARY)) {
      ret = pilot_shoot( pplayer, player_firemode );
      player_setFlag(PLAYER_PRIMARY_L);
      if (ret)
         player_abortAutonav(NULL);
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
      else {
         ret = pilot_shootSecondary( pplayer );
         if (ret)
            player_abortAutonav(NULL);
      }

      player_setFlag(PLAYER_SECONDARY_L);
   }
   else if (player_isFlag(PLAYER_SECONDARY_L)) {
      player_rmFlag(PLAYER_SECONDARY_L);
   }


   /* 
    * Afterburn!
    */
   if (player_isFlag(PLAYER_AFTERBURNER)) {
      if (pilot_isFlag(player.p,PILOT_AFTERBURNER)) {
         afb = pplayer->afterburner->outfit;
         pilot_setThrust( pplayer, 1. + afb->u.afb.thrust );
      }
      else /* Ran out of energy */
         player_afterburnOver();
   }
   else
      pilot_setThrust( pplayer, player_acc );
}


/**
 * @brief Handles the autonavigation process for the player.
 */
static void player_autonav (void)
{
   JumpPoint *jp;
   double d, time, vel, dist;

   /* Target jump. */
   jp = &cur_system->jumps[ player.p->nav_hyperspace ];

   switch (player.autonav) {
      case AUTONAV_APPROACH:
         /* Only accelerate if facing move dir. */
         d = pilot_face( player.p, vect_angle( &player.p->solid->pos, &jp->pos ) );
         if (FABS(d) < MIN_DIR_ERR) {
            if (player_acc < 1.)
               player_accel( 1. );
         }
         else if (player_acc > 0.)
            player_accelOver();

         /* Get current time to reach target. */
         time  = MIN( 1.5*player.p->speed, VMOD(player.p->solid->vel) ) /
            (player.p->thrust / player.p->solid->mass);

         /* Get velocity. */
         vel   = MIN( player.p->speed, VMOD(player.p->solid->vel) );

         /* Get distance. */
         dist  = vel*(time+1.1*180./player.p->turn) -
               0.5*(player.p->thrust/player.p->solid->mass)*time*time;

         /* See if should start braking. */
         if (dist*dist > vect_dist2( &jp->pos, &player.p->solid->pos )) {
            player_accelOver();
            player.autonav = AUTONAV_BRAKE;
         }

         break;

      case AUTONAV_BRAKE:
         /* Braking procedure. */
         d = pilot_face( player.p, VANGLE(player.p->solid->vel) + M_PI );
         if (FABS(d) < MIN_DIR_ERR) {
            if (player_acc < 1.)
               player_accel( 1. );
         }
         else if (player_acc > 0.)
            player_accelOver();

         /* Try to jump or see if braked. */
         if (space_canHyperspace(player.p)) {
            player.autonav = AUTONAV_APPROACH;
            player_accelOver();
            player_jump();
         }
         else if (VMOD(player.p->solid->vel) < MIN_VEL_ERR) {
            player.autonav = AUTONAV_APPROACH;
            player_accelOver();
         }

         break;
   }
}


/**
 * @brief Player update function.
 *
 *    @param pplayer Player to update.
 *    @param dt Current deltatick.
 */
void player_update( Pilot *pplayer, const double dt )
{
   /* Update normally. */
   pilot_update( pplayer, dt );

   /* Update player.p specific stuff. */
   if (!player_isFlag(PLAYER_DESTROYED))
      player_updateSpecific( pplayer, dt );
}


/**
 * @brief Does a pleyr specific update.
 *
 *    @param pplayer Player to update.
 *    @param dt Current deltatick.
 */
void player_updateSpecific( Pilot *pplayer, const double dt )
{
   int engsound;

   /* Calculate engine sound to use. */
   if (player_isFlag(PLAYER_AFTERBURNER))
      engsound = pplayer->afterburner->outfit->u.afb.sound;
   else if (pplayer->solid->force_x > 0.) {
      /* See if is in hyperspace. */
      if (pilot_isFlag(pplayer, PILOT_HYPERSPACE))
         engsound = snd_hypEng;
      else
         engsound = pplayer->ship->sound;
   }
   else
      engsound = -1;
   /* See if sound must change. */
   if (player_lastEngineSound != engsound) {
      sound_stopGroup( player_engine_group );
      if (engsound >= 0)
         sound_playGroup( player_engine_group, engsound, 0 );
   }
   player_lastEngineSound = engsound;

   /* Sound. */
   sound_updateListener( pplayer->solid->dir,
         pplayer->solid->pos.x, pplayer->solid->pos.y,
         pplayer->solid->vel.x, pplayer->solid->vel.y );

   /* See if must playe hail sound. */
   if (player_hailCounter > 0) {
      player_hailTimer -= dt;
      if (player_hailTimer < 0.) {
         player_playSound( snd_hail, 1 );
         player_hailCounter--;
         player_hailTimer = 3.;
      }
   }

   /* Update zoom. */
   player_updateZoom( dt );
}


/**
 * @brief Updates the player.p zoom.
 * 
 *    @param dt Current deltatick.
 */
static void player_updateZoom( double dt )
{
   Pilot *target;
   double d, x,y, z,tz, dx, dy;
   double zfar, znear;
   double c;

   /* Minimum depends on velocity normally.
    *
    * w*h = A, cte    area constant
    * w/h = K, cte    proportion constant
    * d^2 = A, cte    geometric longitud
    *
    * A_v = A*(1+v/d)   area of view is based on speed
    * A_v / A = (1 + v/d)
    *
    * z = A / A_v = 1. / (1 + v/d)
    */
   d     = sqrt(SCREEN_W*SCREEN_H);
   znear = MAX( conf.zoom_far, 1. / (1. + VMOD(player.p->solid->vel)/d) );

   /* Maximum is limited by nebulae. */
   if (cur_system->nebu_density > 0.) {
      c    = MIN( SCREEN_W, SCREEN_H ) / 2;
      zfar = CLAMP( conf.zoom_far, conf.zoom_near, c / nebu_getSightRadius() );
   }
   else {
      zfar = conf.zoom_far;
   }

   /*
    * Set Zoom to pilot target.
    */
   gl_cameraZoomGet( &z );
   if (player.p->target != PLAYER_ID) {
      target = pilot_get(player.p->target);
      if (target != NULL) {

         /* Get current relative target position. */
         gui_getOffset( &x, &y );
         x += target->solid->pos.x - player.p->solid->pos.x;
         y += target->solid->pos.y - player.p->solid->pos.y;

         /* Get distance ratio. */
         dx = (SCREEN_W/2.) / (FABS(x) + 2*target->ship->gfx_space->sw);
         dy = (SCREEN_H/2.) / (FABS(y) + 2*target->ship->gfx_space->sh);

         /* Get zoom. */
         tz = MIN( dx, dy );
      }
      else /* Avoid using uninitialized data .*/
         tz = z;
   }
   else {
      tz = znear; /* Aim at in. */
   }

   /* Gradually zoom in/out. */
   d  = CLAMP(-conf.zoom_speed, conf.zoom_speed, tz - z);
   d *= dt / dt_mod; /* Remove dt dependence. */
   if (d < 0) /** Speed up if needed. */
      d *= 2.;
   gl_cameraZoom( CLAMP( zfar, znear, z + d) );
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
   int found;
   Outfit *o;

   found = !!(player.p->secondary == NULL);
   for (i=0; i<player.p->noutfits; i++) {
      o = player.p->outfits[i]->outfit;

      /* Make sure is secondary weapon. */
      if ((o == NULL) || !(outfit_isProp(o, OUTFIT_PROP_WEAP_SECONDARY)))
         continue;

      /* Make sure it isn't the same as the current one. */
      if ((player.p->secondary != NULL) &&
            (player.p->secondary->outfit == o)) {
         if (player.p->secondary == player.p->outfits[i])
            found = 1;
         continue;
      }

      /* No secondary, grab first. */
      if (found==1) {
         player.p->secondary = player.p->outfits[i];
         return;
      }
   }
   player.p->secondary = NULL;
}


/**
 * @brief Get the previous secondary weapon.
 */
void player_secondaryPrev (void)
{
   int i;
   int found;
   Outfit *o;

   found = !!(player.p->secondary == NULL);
   for (i=player.p->noutfits-1; i>=0; i--) {
      o = player.p->outfits[i]->outfit;

      /* Make sure is secondary weapon. */
      if ((o == NULL) || !(outfit_isProp(o, OUTFIT_PROP_WEAP_SECONDARY)))
         continue;

      /* Make sure it isn't the same as the current one. */
      if ((player.p->secondary != NULL) &&
            (player.p->secondary->outfit == o)) {
         if (player.p->secondary == player.p->outfits[i])
            found = 1;
         continue;
      }

      /* No secondary, grab first. */
      if (found==1) {
         player.p->secondary = player.p->outfits[i];
         return;
      }
   }
   player.p->secondary = NULL;
}


/**
 * @brief Cycle through planet targets.
 */
void player_targetPlanet (void)
{
   /* Can't be landing. */
   if (pilot_isFlag( player.p, PILOT_LANDING))
      return;

   /* Clean up some stuff. */
   player_rmFlag(PLAYER_LANDACK);

   /* Find next planet target. */
   player.p->nav_planet++;
   while (player.p->nav_planet < cur_system->nplanets) {

      /* In range, target planet. */
      if ((cur_system->planets[ player.p->nav_planet ]->real == ASSET_REAL)
            && pilot_inRangePlanet( player.p, player.p->nav_planet )) {
         player_playSound(snd_nav, 1);
         player.p->nav_hyperspace = -1;
         return;
      }

      player.p->nav_planet++;
   }

   /* Untarget if out of range. */
   player.p->nav_planet = -1;
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

   if (landed) { /* player.p is already landed */
      takeoff(1);
      return;
   }

   /* Already landing. */
   if ((pilot_isFlag( player.p, PILOT_LANDING) ||
         pilot_isFlag( player.p, PILOT_TAKEOFF)))
      return;

   /* Check if there are planets to land on. */
   if (cur_system->nplanets == 0) {
      player_message( "\erThere are no planets to land on." );
      return;
   }

   if (player.p->nav_planet >= 0) { /* attempt to land */
      planet = cur_system->planets[player.p->nav_planet];
      if (!planet_hasService(planet, PLANET_SERVICE_LAND)) {
         player_message( "\erYou can't land here." );
         return;
      }
      else if (!player_isFlag(PLAYER_LANDACK)) { /* no landing authorization */
         if (planet_hasService(planet,PLANET_SERVICE_INHABITED)) { /* Basic services */
            if (!areEnemies( player.p->faction, planet->faction ) ||  /* friendly */
                  planet->bribed ) { /* Bribed. */
               player_message( "\e%c%s>\e0 Permission to land granted.", faction_getColourChar(planet->faction), planet->name );
               player_setFlag(PLAYER_LANDACK);
               player_playSound(snd_nav,1);
            }
            else /* Hostile */
               player_message( "\e%c%s>\e0 Landing request denied.", faction_getColourChar(planet->faction), planet->name );
         }
         else { /* No shoes, no shirt, no lifeforms, no service. */
            player_message( "\epReady to land on %s.", planet->name );
            player_setFlag(PLAYER_LANDACK);
            player_playSound(snd_nav,1);
         }
         return;
      }
      else if (vect_dist(&player.p->solid->pos,&planet->pos) > planet->gfx_space->sw) {
         player_message("\erYou are too far away to land on %s.", planet->name);
         return;
      } else if ((pow2(VX(player.p->solid->vel)) + pow2(VY(player.p->solid->vel))) >
            (double)pow2(MAX_HYPERSPACE_VEL)) {
         player_message("\erYou are going too fast to land on %s.", planet->name);
         return;
      }

      /* Stop afterburning. */
      player_afterburnOver();
      /* Stop accelerating. */
      player_accelOver();

      /* Open land menu. */
      player_soundPause();
      player.p->ptimer = PILOT_LANDING_DELAY;
      pilot_setFlag( player.p, PILOT_LANDING );
   }
   else { /* get nearest planet target */

      if (cur_system->nplanets == 0) {
         player_message("\erThere are no planets to land on.");
         return;
      }

      td = -1; /* temporary distance */
      tp = -1; /* temporary planet */
      for (i=0; i<cur_system->nplanets; i++) {
         planet = cur_system->planets[i];
         d = vect_dist(&player.p->solid->pos,&planet->pos);
         if (pilot_inRangePlanet( player.p, i ) &&
               planet_hasService(planet,PLANET_SERVICE_LAND) &&
               ((tp==-1) || ((td == -1) || (td > d)))) {
            tp = i;
            td = d;
         }
      }
      player.p->nav_planet       = tp;
      player.p->nav_hyperspace   = -1;
      player_rmFlag(PLAYER_LANDACK);

      /* no landable planet */
      if (player.p->nav_planet < 0)
         return;

      player_land(); /* rerun land protocol */
   }
}


/**
 * @brief Gets a hyperspace target.
 */
void player_targetHyperspace (void)
{
   /* Can't be landing. */
   if (pilot_isFlag( player.p, PILOT_LANDING))
      return;

   player.p->nav_planet = -1; /* get rid of planet target */
   player_rmFlag(PLAYER_LANDACK); /* get rid of landing permission */
   player.p->nav_hyperspace++;
   map_clear(); /* clear the current map path */

   if (player.p->nav_hyperspace >= cur_system->njumps)
      player.p->nav_hyperspace = -1;
   else
      player_playSound(snd_nav,1);

   /* Map gets special treatment if open. */
   if (player.p->nav_hyperspace == -1)
      map_select( NULL , 0);
   else
      map_select( cur_system->jumps[player.p->nav_hyperspace].target, 0 );
}


/**
 * @brief Starts the hail sounds.
 */
void player_hailStart (void)
{
   player_hailCounter = 5;
}


/**
 * @brief Actually attempts to jump in hyperspace.
 */
void player_jump (void)
{
   int i, j;
   double dist, mindist;

   /* Must have a jump target and not be already jumping. */
   if (pilot_isFlag(player.p, PILOT_HYPERSPACE))
      return;

   if (player.p->nav_hyperspace == -1) {
      j        = -1;
      mindist  = INFINITY;
      for (i=0; i<cur_system->njumps; i++) {
         dist = vect_dist2( &player.p->solid->pos, &cur_system->jumps[i].pos );
         if (dist < mindist) {
            mindist  = dist;
            j        = i;
         }
      }
      if (j  < 0)
         return;

      player.p->nav_hyperspace = j;
      player_playSound(snd_nav,1);

      /* Only follow through if within range. */
      if (mindist > pow2( cur_system->jumps[j].radius ))
         return;
   }

   /* Already jumping, so we break jump. */
   if (pilot_isFlag(player.p, PILOT_HYP_PREP)) {
      pilot_hyperspaceAbort(player.p);
      player_message("\erAborting hyperspace sequence.");
      return;
   }

   i = space_hyperspace(player.p);

   if (i == -1)
      player_message("\erYou are too far from a jump point to initiate hyperspace.");
   else if (i == -2)
      player_message("\erYou are moving too fast to enter hyperspace.");
   else if (i == -3)
      player_message("\erYou do not have enough fuel to hyperspace jump.");
   else {
      player_message("\epPreparing for hyperspace.");
      /* Stop acceleration noise. */
      player_accelOver();
      /* Stop possible shooting. */
      pilot_shootStop( player.p, 0 );
      pilot_shootStop( player.p, 1 );
   }
}

/**
 * @brief Player actually broke hyperspace (entering new system).
 */
void player_brokeHyperspace (void)
{
   double d;
   StarSystem *sys;

   /* First run jump hook. */
   hooks_run( "jumpout" );

   /* calculates the time it takes, call before space_init */
   d  = pilot_hyperspaceDelay( player.p );
   d += RNG_1SIGMA() * 0.2 * d;
   ntime_inc( (unsigned int)(d*NTIME_UNIT_LENGTH) );

   /* Save old system. */
   sys = cur_system;

   /* enter the new system */
   space_init( cur_system->jumps[player.p->nav_hyperspace].target->name );

   /* set position, the pilot_update will handle lowering vel */
   space_calcJumpInPos( cur_system, sys, &player.p->solid->pos, &player.p->solid->vel, &player.p->solid->dir );

   /* reduce fuel */
   player.p->fuel -= HYPERSPACE_FUEL;

   /* stop hyperspace */
   pilot_rmFlag( player.p, PILOT_HYPERSPACE );
   pilot_rmFlag( player.p, PILOT_HYP_BEGIN );
   pilot_rmFlag( player.p, PILOT_HYP_PREP );

   /* update the map */
   map_jump();

   /* Add the escorts. */
   player_addEscorts();

   /* Disable autonavigation if arrived. */
   if (player_isFlag(PLAYER_AUTONAV)) {
      if (player.p->nav_hyperspace == -1) {
         player_message( "\epAutonav arrived at destination.");
         player_rmFlag(PLAYER_AUTONAV);
      }
      else {
         player_message( "\epAutonav continuing until destination (%d jump%s left).",
               map_npath, (map_npath==1) ? "" : "s" );
      }
   }

   /* run the jump hooks */
   hooks_run( "jumpin" );
   hooks_run( "enter" );
   events_trigger( EVENT_TRIGGER_ENTER );

   /* Player sound. */
   player_playSound( snd_hypJump, 1 );
}


/**
 * @brief Activate the afterburner.
 */
void player_afterburn (void)
{
   if (pilot_isFlag(player.p, PILOT_HYP_PREP) || pilot_isFlag(player.p, PILOT_HYPERSPACE))
      return;

   /** @todo fancy effect? */
   if ((player.p != NULL) && (player.p->afterburner!=NULL)) {
      player_setFlag(PLAYER_AFTERBURNER);
      pilot_setFlag(player.p,PILOT_AFTERBURNER);
      spfx_shake(player.p->afterburner->outfit->u.afb.rumble * SHAKE_MAX);
      if (toolkit_isOpen() || paused)
         player_soundPause();
   }
}


/**
 * @brief Deactivates the afterburner.
 */
void player_afterburnOver (void)
{
   if ((player.p != NULL) && (player.p->afterburner!=NULL)) {
      player_rmFlag(PLAYER_AFTERBURNER);
      pilot_rmFlag(player.p,PILOT_AFTERBURNER);
   }
}


/**
 * @brief Start accelerating.
 *
 *    @param acc How much thrust should beb applied of maximum (0 - 1).
 */
void player_accel( double acc )
{
   if ((player.p == NULL) || pilot_isFlag(player.p, PILOT_HYP_PREP) ||
         pilot_isFlag(player.p, PILOT_HYPERSPACE))
      return;


   player_acc = acc;
   if (toolkit_isOpen() || paused)
      player_soundPause();
}


/**
 * @brief Done accelerating.
 */
void player_accelOver (void)
{
   player_acc = 0.;
}


/**
 * @brief Pauses the ship's sounds.
 */
void player_soundPause (void)
{
   sound_pauseGroup(player_engine_group);
}


/**
 * @brief Resumes the ship's sounds.
 */
void player_soundResume (void)
{
   sound_resumeGroup(player_engine_group);
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
      if (!pilot_inRangePilot( player.p, pilot_stack[i] ))
         continue;

      /* Normal unbribed check. */
      if (pilot_isHostile(pilot_stack[i])) {
         td = vect_dist(&pilot_stack[i]->solid->pos, &player.p->solid->pos);       
         if (!pilot_isDisabled(pilot_stack[i]) && ((tp==PLAYER_ID) || (td < d))) {
            d = td;
            tp = pilot_stack[i]->id;
         }
      }
   }

   if ((tp != PLAYER_ID) && (tp != player.p->target))
      player_playSound( snd_target, 1 );

   player.p->target = tp;
}


/**
 * @brief Cycles to next target.
 *
 *    @param mode Mode to target. 0 is normal, 1 is hostiles.
 */
void player_targetNext( int mode )
{
   player.p->target = pilot_getNextID(player.p->target, mode);

   if (player.p->target != PLAYER_ID)
      player_playSound( snd_target, 1 );
}


/**
 * @brief Cycles to previous target.
 *
 *    @param mode Mode to target. 0 is normal, 1 is hostiles.
 */
void player_targetPrev( int mode )
{
   player.p->target = pilot_getPrevID(player.p->target, mode);

   if (player.p->target != PLAYER_ID)
      player_playSound( snd_target, 1 );
}


/**
 * @brief Clearcs the player's ship target.
 */
void player_targetClear (void)
{
   player.p->target = PLAYER_ID;
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
   for (i=0; i<player.p->nescorts; i++) {
      if (player.p->target == player.p->escorts[i].id) {

         /* Cycle targets. */
         if (prev)
            player.p->target = (i > 0) ?
                  player.p->escorts[i-1].id : PLAYER_ID;
         else
            player.p->target = (i < player.p->nescorts-1) ?
                  player.p->escorts[i+1].id : PLAYER_ID;

         break;
      }
   }

   /* Not found in loop. */
   if (i >= player.p->nescorts) {

      /* Check to see if he actually has escorts. */
      if (player.p->nescorts > 0) {

         /* Cycle forward or backwards. */
         if (prev)
            player.p->target = player.p->escorts[player.p->nescorts-1].id;
         else
            player.p->target = player.p->escorts[0].id;
      }
      else
         player.p->target = PLAYER_ID;
   }


   if (player.p->target != PLAYER_ID)
      player_playSound( snd_target, 1 );
}



/**
 * @brief Player targets nearest pilot.
 */
void player_targetNearest (void)
{
   unsigned int t;

   t = player.p->target;
   player.p->target = pilot_getNearestPilot(player.p);

   if ((player.p->target != PLAYER_ID) && (t != player.p->target))
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
   if (player.p->target != player.p->id)
      comm_openPilot(player.p->target);
   else if(player.p->nav_planet != -1)
      comm_openPlanet( cur_system->planets[ player.p->nav_planet ] );
   else
      player_message("\erNo target selected to hail.");
}


/**
 * @brief Automatically tries to hail a pilot that hailed the player.
 */
void player_autohail (void)
{
   int i;
   Pilot *p;

   /* Find pilot to autohail. */
   for (i=0; i<pilot_nstack; i++) {
      p = pilot_stack[i];

      /* Must be hailing. */
      if (pilot_isFlag(p, PILOT_HAILING))
         break;
   }

   /* Not found any. */
   if (i >= pilot_nstack) {
      player_message("\erYou haven't been hailed by any pilots.");
      return;
   }

   /* Try o hail. */
   player.p->target = p->id;
   player_hail();
}


/**
 * @brief Sets the ship fire mode.
 */
void player_setFireMode( int mode )
{
   if (player_firemode == mode)
      return;

   player_firemode = mode;

   if (player_firemode == 0)
      player_message("\epFire mode set to all weapons.");
   else if (player_firemode == 1)
      player_message("\epFire mode set to turret weapons.");
   else if (player_firemode == 2)
      player_message("\epFire mode set to forward weapons.");
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

   /* Mark as destroyed. */
   player_setFlag(PLAYER_DESTROYED);

   /* Stop camera. */
   gl_cameraStatic( player.p->solid->pos.x, player.p->solid->pos.y );

   /* Set timer for death menu. */
   player_timer = 5.;

   /* Stop sounds. */
   player_stopSound();
}


/**
 * @brief PlayerShip_t compare function for qsort().
 */
static int player_shipsCompare( const void *arg1, const void *arg2 )
{
   PlayerShip_t *ps1, *ps2;
   int p1, p2;

   /* Get the arguments. */
   ps1 = (PlayerShip_t*) arg1;
   ps2 = (PlayerShip_t*) arg2;

   /* Get prices. */
   p1 = player_shipPriceRaw( ps1->p );
   p2 = player_shipPriceRaw( ps2->p );

   /* Compare price INVERSELY */
   if (p1 < p2)
      return +1;
   else if (p1 > p2)
      return -1;

   /* In case of tie sort by name so they don't flip or something. */
   return strcmp( ps1->p->name, ps2->p->name );
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

   if (player_nstack == 0)
      return;

   /* Sort. */
   qsort( player_stack, player_nstack, sizeof(PlayerShip_t), player_shipsCompare );

   /* Create the struct. */
   for (i=0; i < player_nstack; i++) {
      sships[i] = strdup(player_stack[i].p->name);
      tships[i] = player_stack[i].p->ship->gfx_target;
   }
}


/**
 * @brief Gets the amount of ships player.p has in storage.
 *
 *    @return The number of ships the player.p has.
 */
int player_nships (void)
{
   return player_nstack;
}


/**
 * @brief Sees if player.p has a ship of a name.
 *
 *    @param shipname Nome of the ship to get.
 *    @return 1 if ship exists.
 */
int player_hasShip( char* shipname )
{
   int i;

   /* Check current ship. */
   if ((player.p != NULL) && (strcmp(player.p->name,shipname)==0))
      return 1;

   /* Check stocked ships. */
   for (i=0; i < player_nstack; i++)
      if (strcmp(player_stack[i].p->name, shipname)==0)
         return 1;
   return 0;
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

   if ((player.p != NULL) && (strcmp(shipname,player.p->name)==0))
      return player.p;

   for (i=0; i < player_nstack; i++)
      if (strcmp(player_stack[i].p->name, shipname)==0)
         return player_stack[i].p;

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
      if (strcmp(player_stack[i].p->name, shipname)==0)
         return player_stack[i].loc;

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
      if (strcmp(player_stack[i].p->name, shipname)==0) {
         free(player_stack[i].loc);
         player_stack[i].loc = strdup(loc);
         return;
      }
   }

   WARN("Player ship '%s' not found in stack", shipname);
}


/**
 * @brief Gets how many of the outfit the player.p owns.
 *
 *    @param outfitname Outfit to check how many the player.p owns.
 *    @return The number of outfits matching outfitname owned.
 */
int player_outfitOwned( const Outfit* o )
{
   int i;

   /* Special case map. */
   if ((outfit_isMap(o)) &&
         map_isMapped( NULL, o->u.map.radius ))
      return 1;

   /* Special case license. */
   if (outfit_isLicense(o) &&
         player_hasLicense(o->name))
      return 1;

   /* Try to find it. */
   for (i=0; i<player_noutfits; i++) {
      if (player_outfits[i].o == o) {
         return player_outfits[i].q;
      }
   }

   return 0;
}


/**
 * @brief qsort() compare function for PlayerOutfit_t sorting.
 */
static int player_outfitCompare( const void *arg1, const void *arg2 )
{
   PlayerOutfit_t *po1, *po2;

   /* Get type. */
   po1 = (PlayerOutfit_t*) arg1;
   po2 = (PlayerOutfit_t*) arg2;

   /* Compare. */
   return outfit_compareTech( &po1->o, &po2->o );
}


/**
 * @brief Prepares two arrays for displaying in an image array.
 *
 *    @param[out] soutfits Names of outfits to .
 *    @param[out] toutfits Textures of outfits for image array.
 */
void player_getOutfits( char** soutfits, glTexture** toutfits )
{
   int i;

   if (player_noutfits == 0) {
      soutfits[0] = strdup( "None" );
      toutfits[0] = NULL;
      return;
   }

   /* We'll sort. */
   qsort( player_outfits, player_noutfits,
         sizeof(PlayerOutfit_t), player_outfitCompare );

   /* Now built name and texture structure. */
   for (i=0; i<player_noutfits; i++) {
      soutfits[i] = strdup( player_outfits[i].o->name );
      toutfits[i] = player_outfits[i].o->gfx_store;
   }
}


/**
 * @brief Gets the amount of different outfits in the player.p outfit stack.
 *
 *    @return Amount of different outfits.
 */
int player_numOutfits (void)
{
   return player_noutfits;
}


/**
 * @brief Adds an outfit to the player.p outfit stack.
 *
 *    @param o Outfit to add.
 *    @param quantity Amount to add.
 *    @return Amount added.
 */
int player_addOutfit( const Outfit *o, int quantity )
{
   int i;

   /* Sanity check. */
   if (quantity == 0)
      return 0;

   /* special case if it's a map */
   if (outfit_isMap(o)) {
      map_map(NULL,o->u.map.radius);
      return 1; /* Success. */
   }
   /* special case if it's a license. */
   else if (outfit_isLicense(o)) {
      player_addLicense(o->name);
      return 1; /* Success. */
   }

   /* Try to find it. */
   for (i=0; i<player_noutfits; i++) {
      if (player_outfits[i].o == o) {
         player_outfits[i].q += quantity;
         return quantity;
      }
   }

   /* Allocate if needed. */
   player_noutfits++;
   if (player_noutfits > player_moutfits) {
      player_moutfits += OUTFIT_CHUNKSIZE;
      player_outfits = realloc( player_outfits,
            sizeof(PlayerOutfit_t) * player_moutfits );
   }

   /* Add the outfit. */
   player_outfits[player_noutfits-1].o = o;
   player_outfits[player_noutfits-1].q = quantity;
   return quantity;
}


/**
 * @brief Remove an outfit from the player's outfit stack.
 *
 *    @param o Outfit to remove.
 *    @param quantity Amount to remove.
 *    @return Amount removed.
 */
int player_rmOutfit( const Outfit *o, int quantity )
{
   int i, q;

   /* Try to find it. */
   for (i=0; i<player_noutfits; i++) {
      if (player_outfits[i].o == o) {
         /* See how many to remove. */
         q = MIN( player_outfits[i].q, quantity );
         player_outfits[i].q -= q;

         /* See if must remove element. */
         if (player_outfits[i].q <= 0) {
            player_noutfits--;
            memmove( &player_outfits[i], &player_outfits[i+1],
                  sizeof(PlayerOutfit_t) * (player_noutfits-i) );
         }

         /* Return removed outfits. */
         return q;
      }
   }

   /* Nothing removed. */
   return 0;
}


/**
 * @brief Marks a mission as completed.
 *
 *    @param id ID of the mission to mark as completed.
 */
void player_missionFinished( int id )
{
   /* Make sure not already marked. */
   if (player_missionAlreadyDone(id))
      return;

   /* Mark as done. */
   missions_ndone++;
   if (missions_ndone > missions_mdone) { /* need to grow */
      missions_mdone += 25;
      missions_done = realloc( missions_done, sizeof(int) * missions_mdone);
   }
   missions_done[ missions_ndone-1 ] = id;
}


/**
 * @brief Checks to see if player.p has already completed a mission.
 *
 *    @param id ID of the mission to see if player.p has completed.
 *    @return 1 if player.p has completed the mission, 0 otherwise.
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
 * @brief Marks a event as completed.
 *
 *    @param id ID of the event to mark as completed.
 */
void player_eventFinished( int id )
{
   /* Make sure not already done. */
   if (player_eventAlreadyDone(id))
      return;

   /* Add to done. */
   events_ndone++;
   if (events_ndone > events_mdone) { /* need to grow */
      events_mdone += 25;
      events_done = realloc( events_done, sizeof(int) * events_mdone);
   }
   events_done[ events_ndone-1 ] = id;
}


/**
 * @brief Checks to see if player.p has already completed a event.
 *
 *    @param id ID of the event to see if player.p has completed.
 *    @return 1 if player.p has completed the event, 0 otherwise.
 */
int player_eventAlreadyDone( int id )
{
   int i;
   for (i=0; i<events_ndone; i++)
      if (events_done[i] == id)
         return 1;
   return 0;
}


/**
 * @brief Checks to see if player.p has license.
 *
 *    @param license License to check to see if the player.p has.
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
 * @brief Gives the player.p a license.
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
 *    @param nlicenses Amount of licenses the player.p has.
 *    @return Name of the licenses he has.
 */
char **player_getLicenses( int *nlicenses )
{
   *nlicenses = player_nlicenses;
   return player_licenses;
}


/**
 * @brief Clears escorts to make sure deployment is sane.
 */
void player_clearEscorts (void)
{
   int i;

   for (i=0; i<player.p->noutfits; i++) {
      if (player.p->outfits[i]->outfit == NULL)
         continue;

      if (outfit_isFighterBay(player.p->outfits[i]->outfit)) {
         player.p->outfits[i]->u.ammo.deployed = 0;
      }
   }
}


/**
 * @brief Adds the player's escorts.
 *
 *    @return 0 on success.
 */
int player_addEscorts (void)
{
   int i, j;
   double a;
   Vector2d v;
   unsigned int e;
   Outfit *o;
   int q;

   /* Clear escorts first. */
   player_clearEscorts();

   for (i=0; i<player.p->nescorts; i++) {
      a = RNGF() * 2 * M_PI;
      vect_cset( &v, player.p->solid->pos.x + 50.*cos(a),
            player.p->solid->pos.y + 50.*sin(a) );
      e = escort_create( player.p, player.p->escorts[i].ship,
            &v, &player.p->solid->vel, player.p->solid->dir,
            player.p->escorts[i].type, 0 );
      player.p->escorts[i].id = e; /* Important to update ID. */

      /* Update outfit if needed. */
      if (player.p->escorts[i].type != ESCORT_TYPE_BAY)
         continue;

      for (j=0; j<player.p->noutfits; j++) {
         /* Must have outfit. */
         if (player.p->outfits[j]->outfit == NULL)
            continue;

         /* Must be fighter bay. */
         if (!outfit_isFighterBay(player.p->outfits[j]->outfit))
            continue;

         /* Ship must match. */
         o = outfit_ammo(player.p->outfits[j]->outfit);
         if (!outfit_isFighter(o) ||
               (strcmp(player.p->escorts[i].ship,o->u.fig.ship)!=0))
            continue;

         /* Must not have all deployed. */
         q = player.p->outfits[j]->u.ammo.deployed + player.p->outfits[j]->u.ammo.quantity;
         if (q >= outfit_amount(player.p->outfits[j]->outfit))
            continue;

         /* Mark as deployed. */
         player.p->outfits[j]->u.ammo.deployed += 1;
         break;
      }
      if (j >= player.p->noutfits) {
         WARN("Unable to mark escort as deployed");
      }
   }

   return 0;
}


/**
 * @brief Saves the player's escorts.
 */
static int player_saveEscorts( xmlTextWriterPtr writer )
{
   int i;

   for (i=0; i<player.p->nescorts; i++) {
      xmlw_startElem(writer, "escort");
      xmlw_attr(writer,"type","bay"); /**< @todo other types. */
      xmlw_str(writer, "%s", player.p->escorts[i].ship);
      xmlw_endElem(writer); /* "escort" */
   }

   return 0;
}


/**
 * @brief Save the freaking player.p in a freaking xmlfile.
 *
 *    @param writer xml Writer to use.
 *    @return 0 on success.
 */
int player_save( xmlTextWriterPtr writer )
{
   int i;
   MissionData *m;
   const char *ev;

   xmlw_startElem(writer,"player");

   /* Standard player.p details. */
   xmlw_attr(writer,"name","%s",player.name);
   xmlw_elem(writer,"rating","%f",player.crating);
   xmlw_elem(writer,"credits","%"PRIu64,player.p->credits);
   xmlw_elem(writer,"time","%u",ntime_get());

   /* Current ship. */
   xmlw_elem(writer,"location","%s",land_planet->name);
   player_saveShip( writer, player.p, NULL ); /* current ship */

   /* Ships. */
   xmlw_startElem(writer,"ships");
   for (i=0; i<player_nstack; i++)
      player_saveShip( writer, player_stack[i].p, player_stack[i].loc );
   xmlw_endElem(writer); /* "ships" */

   /* Outfits. */
   xmlw_startElem(writer,"outfits");
   for (i=0; i<player_noutfits; i++) {
      xmlw_startElem(writer,"outfit");
      xmlw_attr(writer,"quantity","%d",player_outfits[i].q);
      xmlw_str(writer,"%s",player_outfits[i].o->name);
      xmlw_endElem(writer); /* "outfit" */
   }
   xmlw_endElem(writer); /* "outfits" */

   /* Licenses. */
   xmlw_startElem(writer,"licenses");
   for (i=0; i<player_nlicenses; i++)
      xmlw_elem(writer,"license","%s",player_licenses[i]);
   xmlw_endElem(writer); /* "licenses" */

   xmlw_endElem(writer); /* "player" */

   /* Mission the player.p has done. */
   xmlw_startElem(writer,"missions_done");
   for (i=0; i<missions_ndone; i++) {
      m = mission_get(missions_done[i]);
      if (m != NULL) /* In case mission name changes between versions */
         xmlw_elem(writer,"done","%s",m->name);
   }
   xmlw_endElem(writer); /* "missions_done" */

   /* Events the player.p has done. */
   xmlw_startElem(writer,"events_done");
   for (i=0; i<events_ndone; i++) {
      ev = event_dataName(events_done[i]);
      if (ev != NULL) /* In case mission name changes between versions */
         xmlw_elem(writer,"done","%s",ev);
   }
   xmlw_endElem(writer); /* "events_done" */

   /* Escorts. */
   xmlw_startElem(writer, "escorts");
   player_saveEscorts(writer);
   xmlw_endElem(writer); /* "escorts" */

   return 0;
}


/**
 * @brief Saves an outfit slot.
 */
static int player_saveShipSlot( xmlTextWriterPtr writer, PilotOutfitSlot *slot, int i )
{
   Outfit *o;
   o = slot->outfit;
   xmlw_startElem(writer,"outfit");
   xmlw_attr(writer,"slot","%d",i);
   if ((outfit_ammo(o) != NULL) &&
         (slot->u.ammo.outfit != NULL)) {
      xmlw_attr(writer,"ammo","%s",slot->u.ammo.outfit->name);
      xmlw_attr(writer,"quantity","%d", slot->u.ammo.quantity);
   }
   xmlw_str(writer,"%s",o->name);
   xmlw_endElem(writer); /* "outfit" */

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
   xmlw_attr(writer,"name","%s",ship->name);
   xmlw_attr(writer,"model","%s",ship->ship->name);

   if (loc != NULL)
      xmlw_elem(writer,"location","%s",loc);

   /* save the fuel */
   xmlw_elem(writer,"fuel","%f",ship->fuel);

   /* save the outfits */
   xmlw_startElem(writer,"outfits_low");
   for (i=0; i<ship->outfit_nlow; i++) {
      if (ship->outfit_low[i].outfit==NULL)
         continue;
      player_saveShipSlot( writer, &ship->outfit_low[i], i );
   }
   xmlw_endElem(writer); /* "outfits_low" */
   xmlw_startElem(writer,"outfits_medium");
   for (i=0; i<ship->outfit_nmedium; i++) {
      if (ship->outfit_medium[i].outfit==NULL)
         continue;
      player_saveShipSlot( writer, &ship->outfit_medium[i], i );
   }
   xmlw_endElem(writer); /* "outfits_medium" */
   xmlw_startElem(writer,"outfits_high");
   for (i=0; i<ship->outfit_nhigh; i++) {
      if (ship->outfit_high[i].outfit==NULL)
         continue;
      player_saveShipSlot( writer, &ship->outfit_high[i], i );
   }
   xmlw_endElem(writer); /* "outfits_high" */

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
      xmlw_str(writer,"%s",ship->commodities[i].commodity->name);

      xmlw_endElem(writer); /* commodity */
   }
   xmlw_endElem(writer); /* "commodities" */

   xmlw_endElem(writer); /* "ship" */

   return 0;
}


/**
 * @brief Loads the player.p stuff.
 *
 *    @param parent Node where the player.p stuff is to be found.
 *    @return 0 on success.
 */
int player_load( xmlNodePtr parent )
{
   xmlNodePtr node;

   /* some cleaning up */
   memset( &player, 0, sizeof(Player_t) );
   map_cleanup();

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"player"))
         player_parse( node );
      else if (xml_isNode(node,"missions_done"))
         player_parseDoneMissions( node );
      else if (xml_isNode(node,"events_done"))
         player_parseDoneEvents( node );
      else if (xml_isNode(node,"escorts"))
         player_parseEscorts(node);
   } while (xml_nextNode(node));

   return 0;
}


/**
 * @brief Parses the player.p node.
 *
 *    @param parent The player.p node.
 *    @return 0 on success.
 */
static int player_parse( xmlNodePtr parent )
{
   unsigned int player_time;
   char* planet, *str;
   Planet* pnt;
   int sw,sh;
   xmlNodePtr node, cur;
   int q;
   Outfit *o;
   int i, hunting;

   xmlr_attr(parent,"name",player.name);

   /* Make sure player.p is NULL. */
   player.p = NULL;

   /* Sane defaults. */
   planet = NULL;
   player_time = 0;

   /* Must get planet first. */
   node = parent->xmlChildrenNode;
   do {
      xmlr_str(node,"location",planet);
   } while (xml_nextNode(node));

   /* Parse rest. */
   node = parent->xmlChildrenNode;
   do {

      /* global stuff */
      xmlr_float(node,"rating",player.crating);
      xmlr_ulong(node,"credits",player_creds);
      xmlr_uint(node,"time",player_time);

      if (xml_isNode(node,"ship"))
         player_parseShip(node, 1, planet);
     
      /* Parse ships. */
      else if (xml_isNode(node,"ships")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"ship"))
               player_parseShip(cur, 0, planet);
         } while (xml_nextNode(cur));
      }

      /* Parse outfits. */
      else if (xml_isNode(node,"outfits")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"outfit")) {
               o = outfit_get( xml_get(cur) );
               if (o == NULL) {
                  WARN("Outfit '%s' was saved but does not exist!", xml_get(cur));
                  continue;
               }

               xmlr_attr( cur, "quantity", str );
               if (str != NULL) {
                  q = atof(str);
                  free(str);
               }
               else {
                  WARN("Outfit '%s' was saved without quantity!", o->name);
                  continue;
               }

               player_addOutfit( o, q );
            }
         } while (xml_nextNode(cur));
      }

      /* Parse licenses. */
      else if (xml_isNode(node,"licenses"))
         player_parseLicenses(node);

   } while (xml_nextNode(node));

   /* Handle cases where ship is missing. */
   if (player.p == NULL) {
      if (player_nstack == 0) {
         WARN("Player has no ships!");
         return -1;
      }

      /* Just give player.p a random ship in the stack. */
      player.p = player_stack[player_nstack-1].p;
      player_nstack--;
   }

   /* set global thingies */
   player.p->credits = player_creds;
   if (player_time==0)
      WARN("Save has no time information, setting to 0.");
   ntime_set(player_time);

   /* set player.p in system */
   pnt = planet_get( planet );
   /* Get random planet if it's NULL. */
   if (pnt == NULL)
      pnt = planet_get( space_getRndPlanet() );
   /* In case the planet does not exist, we need to update some variables.
    * While we're at it, we'll also make sure the system exists as well. */
   hunting  = 1;
   i        = 0;
   while (hunting && (i<100)) {
      planet = pnt->name;
      if (planet_getSystem( planet ) == NULL) {
         WARN("Planet '%s' found, but its system isn't. Trying again.", planet);
         pnt = planet_get( space_getRndPlanet() );
      }
      else {
         hunting = 0;
      }
      i++;
   }
   sw = pnt->gfx_space->sw;
   sh = pnt->gfx_space->sh;
   player_warp( pnt->pos.x + RNG(-sw/2,sw/2),
         pnt->pos.y + RNG(-sh/2,sh/2) );
   player.p->solid->dir = RNG(0,359) * M_PI/180.;
   gl_cameraBind(&player.p->solid->pos);

   /* initialize the system */
   music_choose("takeoff");
   planet = pnt->name;
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
static int player_parseDoneMissions( xmlNodePtr parent )
{
   xmlNodePtr node;
   int id;

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"done")) {
         id = mission_getID( xml_get(node) );
         if (id < 0)
            DEBUG("Mission '%s' doesn't seem to exist anymore, removing from save.",
                  xml_get(node));
         else
            player_missionFinished( id );
      }
   } while (xml_nextNode(node));

   return 0;
}


/**
 * @brief Parses player's done missions.
 *
 *    @param parent Node of the missions.
 *    @return 0 on success.
 */
static int player_parseDoneEvents( xmlNodePtr parent )
{
   xmlNodePtr node;
   int id;

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"done")) {
         id = event_dataID( xml_get(node) );
         if (id < 0)
            DEBUG("Event '%s' doesn't seem to exist anymore, removing from save.",
                  xml_get(node));
         else
            player_eventFinished( id );
      }
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
 * @brief Parses the escorts from the escort node.
 *
 *    @param parent "escorts" node to parse.
 *    @return 0 on success.
 */
static int player_parseEscorts( xmlNodePtr parent )
{
   xmlNodePtr node;
   char *buf, *ship;
   EscortType_t type;

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"escort")) {
         xmlr_attr( node, "type", buf );
         if (strcmp(buf,"bay")==0)
            type = ESCORT_TYPE_BAY;
         else {
            WARN("Escort has invalid type '%s'.", buf);
            type = ESCORT_TYPE_NULL;
         }
         free(buf);

         ship = xml_get(node);

         /* Add escort to the list. */
         escort_addList( player.p, ship, type, 0 );
      }
   } while (xml_nextNode(node));

   return 0;
}


/**
 * @brief Adds outfit to pilot if it can.
 */
static void player_addOutfitToPilot( Pilot* pilot, Outfit* outfit, PilotOutfitSlot *s )
{
   int ret;
   ret = pilot_addOutfitRaw( pilot, outfit, s );
   if (ret != 0) {
      DEBUG("Outfit '%s' does not fit on player's pilot '%s', adding to stock.",
            outfit->name, pilot->name);
      player_addOutfit( outfit, 1 );
   }

   /* Update stats. */
   pilot_calcStats( pilot );
}


/**
 * @brief Parses a ship outfit slot.
 */
static void player_parseShipSlot( xmlNodePtr node, Pilot *ship, PilotOutfitSlot *slot )
{
   Outfit *o, *ammo;
   char *buf;
   int q;

   /* Add the outfit. */
   o = outfit_get( xml_get(node) );
   if (o==NULL)
      return;
   player_addOutfitToPilot( ship, o, slot );

   /* Doesn't have ammo. */
   if (outfit_ammo(o)==NULL)
      return;

   /* See if has ammo. */
   xmlr_attr(node,"ammo",buf);
   if (buf == NULL)
      return;

   /* Get the ammo. */
   ammo = outfit_get(buf);
   free(buf);
   if (ammo==NULL)
      return;

   /* See if has quantity. */
   xmlr_attr(node,"quantity",buf);
   if (buf == NULL)
      return;

   /* Get quantity. */
   q = atoi(buf);
   free(buf);

   /* Add ammo. */
   pilot_addAmmo( ship, slot, ammo, q );
}


/**
 * @brief Parses a player's ship.
 *
 *    @param parent Node of the ship.
 *    @param is_player Is it the ship the player is currently in?
 *    @param planet Default planet in case ship location not found.
 *    @return 0 on success.
 */
static int player_parseShip( xmlNodePtr parent, int is_player, char *planet )
{
   char *name, *model, *loc, *q, *id;
   int i, n;
   double fuel;
   Ship *ship_parsed;
   Pilot* ship;
   xmlNodePtr node, cur;
   int quantity;
   Outfit *o;
   int ret;
   const char *str;
   Commodity *com;
   PilotFlags flags;
   
   xmlr_attr(parent,"name",name);
   xmlr_attr(parent,"model",model);

   /* Sane defaults. */
   loc = NULL;
   pilot_clearFlagsRaw( flags );
   pilot_setFlagRaw( flags, PILOT_PLAYER );
   pilot_setFlagRaw( flags, PILOT_NO_OUTFITS );

   /* Get the ship. */
   ship_parsed = ship_get(model);
   if (ship_parsed == NULL) {
      WARN("Player ship '%s' not found", model);
      return 0;
   }

   /* player.p is currently on this ship */
   if (is_player != 0) {
      pilot_create( ship_parsed, name, faction_get("Player"), NULL, 0., NULL, NULL, flags, -1 );
      ship = player.p;
   }
   else
      ship = pilot_createEmpty( ship_parsed, name, faction_get("Player"), NULL, flags );

   free(name);
   free(model);

   node = parent->xmlChildrenNode;

   fuel = -1;

   do {
      /* Get location. */
      if (is_player == 0)
         xmlr_str(node,"location",loc);

      /* get fuel */
      xmlr_float(node,"fuel",fuel);

      /*
       * LEGACY LAYER TO NOT LOSE OUTFITS FROM OLD GAMES
       * @todo Remove it at 0.5.0 or earlier
       */
      if (xml_isNode(node,"outfits")) {
         cur = node->xmlChildrenNode;
         DEBUG("Using legacy loading for old outfits.");
         do { /* load each outfit */
            if (xml_isNode(cur,"outfit")) {
               xmlr_attr(cur,"quantity",q);
               n = 0;
               if (q != NULL) {
                  n = atoi(q);
                  free(q);
               }
               if (n <= 0) {
                  WARN("Outfit '%s' has no quantity", xml_get(cur));
                  continue;
               }
               /* Get the outfit. */
               o = outfit_get(xml_get(cur));
               if (o==NULL)
                  continue;
               /* Add the outfit. */
               player_addOutfit( o, n );
            }
         } while (xml_nextNode(cur));
      }

      /* New outfit loading. */
      if (xml_isNode(node,"outfits_low")) {
         cur = node->xmlChildrenNode;
         do { /* load each outfit */
            if (xml_isNode(cur,"outfit")) {
               xmlr_attr(cur,"slot",q);
               n = -1;
               if (q != NULL) {
                  n = atoi(q);
                  free(q);
               }
               if ((n<0) || (n >= ship->outfit_nlow)) {
                  WARN("Outfit slot out of range, not adding.");
                  continue;
               }
               player_parseShipSlot( cur, ship, &ship->outfit_low[n] );
            }
         } while (xml_nextNode(cur));
      }
      else if (xml_isNode(node,"outfits_medium")) {
         cur = node->xmlChildrenNode;
         do { /* load each outfit */
            if (xml_isNode(cur,"outfit")) {
               xmlr_attr(cur,"slot",q);
               n = -1;
               if (q != NULL) {
                  n = atoi(q);
                  free(q);
               }
               if ((n<0) || (n >= ship->outfit_nmedium)) {
                  WARN("Outfit slot out of range, not adding.");
                  continue;
               }
               player_parseShipSlot( cur, ship, &ship->outfit_medium[n] );
            }
         } while (xml_nextNode(cur));
      }
      else if (xml_isNode(node,"outfits_high")) {
         cur = node->xmlChildrenNode;
         do { /* load each outfit */
            if (xml_isNode(cur,"outfit")) {
               xmlr_attr(cur,"slot",q);
               n = -1;
               if (q != NULL) {
                  n = atoi(q);
                  free(q);
               }
               if ((n<0) || (n >= ship->outfit_nhigh)) {
                  WARN("Outfit slot out of range, not adding.");
                  continue;
               }
               player_parseShipSlot( cur, ship, &ship->outfit_high[n] );
            }
         } while (xml_nextNode(cur));
      }
      else if (xml_isNode(node,"commodities")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"commodity")) {
               xmlr_attr(cur,"quantity",q);
               xmlr_attr(cur,"id",id);
               quantity = atoi(q);
               i = (id==NULL) ? 0 : atoi(id);
               free(q);
               if (id != NULL)
                  free(id);

               /* Get the commodity. */
               com = commodity_get(xml_get(cur));
               if (com == NULL) {
                  WARN("Unknown commodity '%s' detected, removing.", xml_get(cur));
                  continue;
               }

               /* actually add the cargo with id hack */
               pilot_addCargo( ship, com, quantity );
               if (i != 0)
                  ship->commodities[ ship->ncommodities-1 ].id = i;
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   /* Update stats. */
   pilot_calcStats( ship );

   /* Test for sanity. */
   if (fuel >= 0)
      ship->fuel = MIN(ship->fuel_max, fuel);
   if ((is_player == 0) && (planet_get(loc)==NULL))
      loc = planet;
   str = pilot_checkSanity( ship );
   if (str != NULL) {
      DEBUG("Player ship '%s' failed sanity check (%s), removing all outfits and adding to stock.",
            ship->name, str );
      /* Remove all outfits. */
      for (i=0; i<ship->noutfits; i++) {
         o = ship->outfits[i]->outfit;
         ret = pilot_rmOutfitRaw( ship, ship->outfits[i] );
         if (ret==0)
            player_addOutfit( o, 1 );
      }
      pilot_calcStats( ship );
   }

   /* add it to the stack if it's not what the player.p is in */
   if (is_player == 0) {
      player_stack = realloc(player_stack, sizeof(PlayerShip_t)*(player_nstack+1));
      player_stack[player_nstack].p    = ship;
      player_stack[player_nstack].loc  = (loc!=NULL) ? strdup(loc) : strdup("Uknown");
      player_nstack++;
   }

   return 0;
}

