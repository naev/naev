/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file player.c
 *
 * @brief Contains all the player related stuff.
 */
/** @cond */
#include <stdlib.h>
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "player.h"

#include "ai.h"
#include "board.h"
#include "camera.h"
#include "claim.h"
#include "comm.h"
#include "conf.h"
#include "dialogue.h"
#include "difficulty.h"
#include "economy.h"
#include "equipment.h"
#include "escort.h"
#include "event.h"
#include "font.h"
#include "gui.h"
#include "gui_omsg.h"
#include "hook.h"
#include "info.h"
#include "input.h"
#include "intro.h"
#include "land.h"
#include "land_outfits.h"
#include "load.h"
#include "log.h"
#include "map.h"
#include "map_overlay.h"
#include "menu.h"
#include "mission.h"
#include "music.h"
#include "ndata.h"
#include "news.h"
#include "nfile.h"
#include "nlua_misn.h"
#include "nlua_outfit.h"
#include "nlua_ship.h"
#include "nlua_var.h"
#include "nstring.h"
#include "ntracing.h"
#include "ntime.h"
#include "nxml.h"
#include "opengl.h"
#include "pause.h"
#include "pilot.h"
#include "player_gui.h"
#include "player_fleet.h"
#include "player_inventory.h"
#include "rng.h"
#include "shiplog.h"
#include "sound.h"
#include "space.h"
#include "spfx.h"
#include "start.h"
#include "toolkit.h"
#include "unidiff.h"

/*
 * Player stuff
 */
Player_t player; /**< Local player. */
static const Ship* player_ship = NULL; /**< Temporary ship to hold when naming it */
static credits_t player_creds = 0; /**< Temporary hack for when creating. */
static credits_t player_payback = 0; /**< Temporary hack for when creating. */
static int player_ran_updater = 0; /**< Temporary hack for when creating. */
static char *player_message_noland = NULL; /**< No landing message (when PLAYER_NOLAND is set). */

/*
 * Licenses.
 */
static char **player_licenses = NULL; /**< Licenses player has. */

/*
 * Default radar resolution.
 */
#define RADAR_RES_DEFAULT  50. /**< Default resolution. */

/*
 * player sounds.
 */
static int player_engine_group = -1; /**< Player engine sound group. */
static int player_hyper_group = -1; /**< Player hyperspace sound group. */
static int player_gui_group   = -1; /**< Player GUI sound group. */
int snd_target                = -1; /**< Sound when targeting. */
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
 * Player pilot stack (ships they have) and outfit (outfits they have) stacks (array.h)
 */
static PlayerShip_t* player_stack      = NULL;  /**< Stack of ships player has, excluding their current one (player.ps). */
static PlayerOutfit_t *player_outfits  = NULL;  /**< Outfits player has. */

/*
 * player global properties
 */
/* used in input.c */
double player_left         = 0.; /**< Player left turn velocity from input. */
double player_right        = 0.; /**< Player right turn velocity from input. */
double player_acc          = 0.; /**< Accel velocity from input. */
/* for death and such */
static double player_timer = 0.; /**< For death and such. */

/*
 * unique mission and event stack.
 */
static int* missions_done  = NULL; /**< Array (array.h): Saves position of completed missions. */
static int* events_done  = NULL; /**< Array (array.h): Saves position of completed events. */

/*
 * prototypes
 */
/*
 * internal
 */
static void player_checkHail (void);
/* creation */
static void player_newSetup();
static int player_newMake (void);
static PlayerShip_t* player_newShipMake( const char* name );
/* sound */
static void player_initSound (void);
/* save/load */
static int player_saveEscorts( xmlTextWriterPtr writer );
static int player_saveShipSlot( xmlTextWriterPtr writer, const PilotOutfitSlot *slot, int i );
static int player_saveShip( xmlTextWriterPtr writer, PlayerShip_t *pship );
static int player_saveMetadata( xmlTextWriterPtr writer );
static Spob* player_parse( xmlNodePtr parent );
static int player_parseDoneMissions( xmlNodePtr parent );
static int player_parseDoneEvents( xmlNodePtr parent );
static int player_parseLicenses( xmlNodePtr parent );
static int player_parseInventory( xmlNodePtr parent );
static void player_parseShipSlot( xmlNodePtr node, Pilot *ship, PilotOutfitSlot *slot );
static int player_parseShip( xmlNodePtr parent, int is_player );
static int player_parseEscorts( xmlNodePtr parent );
static int player_parseMetadata( xmlNodePtr parent );
static void player_addOutfitToPilot( Pilot* pilot, const Outfit* outfit, PilotOutfitSlot *s );
static int player_runUpdaterScript( const char* type, const char* name, int q );
static const Outfit* player_tryGetOutfit( const char* name, int q );
static const Ship* player_tryGetShip( const char* name );
static void player_tryAddLicense( const char* name );
/* Render. */
static void player_renderStealthUnderlay( double dt );
static void player_renderStealthOverlay( double dt );
static void player_renderAimHelper( double dt );
/* Misc. */
static int player_filterSuitableSpob( Spob *p );
static void player_spobOutOfRangeMsg (void);
static int player_outfitCompare( const void *arg1, const void *arg2 );
static int player_thinkMouseFly( double dt );
static int preemption = 0; /* Hyperspace target/untarget preemption. */

/*
 * externed
 */
int player_save( xmlTextWriterPtr writer ); /* save.c */
Spob* player_load( xmlNodePtr parent ); /* save.c */

/**
 * @brief Initializes player stuff.
 */
int player_init (void)
{
   if (player_stack==NULL)
      player_stack = array_create( PlayerShip_t );
   if (player_outfits==NULL)
      player_outfits = array_create( PlayerOutfit_t );
   player_initSound();
   memset( &player, 0, sizeof(PlayerShip_t) );

   player_autonavInit();

   return 0;
}

/**
 * @brief Sets up a new player.
 */
static void player_newSetup()
{
   double x, y;

   /* Setup sound */
   player_initSound();

   /* Clean up player stuff if we'll be recreating. */
   player_cleanup();

   /* Set up GUI. */
   player.radar_res = RADAR_RES_DEFAULT;
   gui_setDefaults();

   /* Sane time defaults. */
   player.last_played = time(NULL);
   player.date_created = player.last_played;
   player.time_since_save = player.last_played;
   player.chapter = strdup( start_chapter() );

   /* For pretty background. */
   pilots_cleanAll();
   space_init( start_system(), 1 );
   start_position( &x, &y );

   cam_setTargetPos( x, y, 0 );
   cam_setZoom( conf.zoom_far );

   /* Clear the init message for new game. */
   gui_clearMessages();
}

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
   int invalid = 1;

   /* Set up new player. */
   player_newSetup();

   /* Some meta-data. */
   player.date_created = time(NULL);

   do {
      const char *SAVEPATH = "_tmp";
      char buf[PATH_MAX];

      /* Get the name. */
      player.name = dialogue_input( _("Player Name"), 1, 60,
            _("Please write your name:") );

      /* Player cancelled dialogue. */
      if (player.name == NULL) {
         menu_main();
         return;
      }

      /* Try to see if we can save the game for a valid player name. */
      snprintf( buf, sizeof(buf), "%s/%s", SAVEPATH, player.name );
      if (PHYSFS_mkdir(buf)!=0) { /* In particular should be PHYSFS_ERR_BAD_FILENAME erro when mkdir==0. */
         PHYSFS_Stat stat;
         int ret = PHYSFS_stat( buf, &stat );
         /* When ret==0, we somehow created a directory, but we don't actually know the name
          * nor where it is. This can happen on Windows when using a '.' as the
          * final character.  */
         if ((ret!=0) && (stat.filetype==PHYSFS_FILETYPE_DIRECTORY)) {
            /* Here the directory should have been properly created, so we can tell the player it's good. */
            ret = PHYSFS_delete( buf );
            if (ret==0)
               WARN(_("Unable to delete temporary file '%s': %s"), buf, PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ));
            else {
               ret = PHYSFS_delete( SAVEPATH );
               if (ret==0)
                  WARN(_("Unable to delete temporary file '%s': %s"), SAVEPATH, PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ));
            }
            invalid = 0;
         }
      }
      if (invalid)
         dialogue_alert(_("'%s' is an invalid player name as it can not be saved to your filesystem! Please choose another."), player.name);
      PHYSFS_getLastErrorCode(); /* Clear error code. */
   } while (invalid);

   load_refresh();
   if (array_size( load_getList( player.name ) ) > 0) {
      int r = dialogue_YesNo(_("Overwrite"),
            _("You already have a pilot named %s. Their autosave and backup save will be overwritten. Do you wish to continue?"), player.name);
      if (r==0) { /* no */
         player_new();
         return;
      }
   }

   if (player_newMake())
      return;

   /* Display the intro. */
   intro_display( INTRO_PATH, "intro" );

   /* Play music. */
   music_choose( "ambient" );

   /* Set loaded version. */
   player.loaded_version = strdup( naev_version(0) );

   /* Add the mission if found. */
   if (start_mission() != NULL) {
      if (mission_start(start_mission(), NULL) < 0)
         WARN(_("Failed to run start mission '%s'."), start_mission());
   }

   /* Add the event if found. */
   if (start_event() != NULL) {
      if (event_start( start_event(), NULL ))
         WARN(_("Failed to run start event '%s'."), start_event());
   }

   /* Run the load event trigger. */
   events_trigger( EVENT_TRIGGER_LOAD );

   /* Load the GUI. */
   gui_load( gui_pick() );
}

/**
 * @brief Actually creates a new player.
 *
 *    @return 0 on success.
 */
static int player_newMake (void)
{
   const Ship *ship;
   const char *shipname, *acquired;
   double x,y;
   PlayerShip_t *ps;

   if (player_stack==NULL)
      player_stack = array_create( PlayerShip_t );
   if (player_outfits==NULL)
      player_outfits = array_create( PlayerOutfit_t );

   /* Time. */
   ntime_set( start_date() );
   /* Clear known economy info */
   economy_clearKnown();
   /* Welcome message - must be before space_init. */
   player_message( _("#gWelcome to %s!"), APPNAME );
   player_message( "#g v%s", naev_version(0) );

   /* Try to create the pilot, if fails reask for player name. */
   ship = ship_get( start_ship() );
   shipname = _(start_shipname());
   if (ship==NULL) {
      WARN(_("Ship not properly set by module."));
      return -1;
   }
   acquired = _(start_acquired());
   /* Setting a default name in the XML prevents naming prompt. */
   ps = player_newShip( ship, shipname, 0, acquired, (shipname==NULL) ? 0 : 1 );
   if (ps == NULL) {
      player_new();
      return -1;
   }
   assert( &player.ps == ps );
   start_position( &x, &y );
   vec2_cset( &player.p->solid.pos, x, y );
   vectnull( &player.p->solid.vel );
   player.p->solid.dir = RNGF() * 2.*M_PI;
   space_init( start_system(), 1 );

   /* Bind camera. */
   cam_setTargetPilot( player.p->id, 0 );

   /* Set player speed to default 1 */
   player.speed = conf.game_speed;

   /* Reset speed (to make sure time dilation stuff is accounted for). */
   player_autonavResetSpeed();

   /* Monies. */
   player.p->credits = start_credits();

   /* clear the map */
   map_clear();

   /* Start the economy. */
   economy_init();

   /* clear the shiplog*/
   shiplog_clear();

   /* Start the news */
   news_init();

   return 0;
}

/**
 * @brief Creates a new ship for player.
 *
 *    @param ship New ship to get.
 *    @param def_name Default name to give it if cancelled.
 *    @param trade Whether or not to trade player's current ship with the new ship.
 *    @param acquired Description of how the ship was acquired.
 *    @param noname Whether or not to let the player name it.
 *    @return Newly created pilot on success or NULL if dialogue was cancelled.
 *
 * @sa player_newShipMake
 */
PlayerShip_t* player_newShip( const Ship* ship, const char *def_name,
      int trade, const char *acquired, int noname )
{
   char *ship_name;
   PlayerShip_t *ps;

   /* temporary values while player doesn't exist */
   player_creds = (player.p != NULL) ? player.p->credits : 0;
   player_ship  = ship;
   if (!noname)
      ship_name = dialogue_input( _("Ship Name"), 1, 60,
            _("Please name your new ship:") );
   else
      ship_name = NULL;

   /* Dialogue cancelled. */
   if (ship_name == NULL) {
      int i, len;

      /* No default name, fail. */
      if (def_name == NULL)
         return NULL;

      /* Add default name. */
      i = 2;
      len = strlen(def_name)+10;
      ship_name = malloc( len );
      strcpy( ship_name, def_name );
      while (player_hasShip(ship_name)) {
         snprintf( ship_name, len, "%s %d", def_name, i );
         i++;
      }
   }

   /* Must not have same name. */
   if (player_hasShip(ship_name)) {
      dialogue_msg( _("Name collision"),
            _("Please do not give the ship the same name as another of your ships."));
      free( ship_name );
      return NULL;
   }
   if (trade && player.p == NULL)
      ERR(_("Player ship isn't valid… This shouldn't happen!"));

   ps = player_newShipMake( ship_name );
   ps->autoweap  = 1;
   ps->favourite = 0;
   ps->p->shipvar= array_create( lvar );
   ps->acquired  = (acquired!=NULL) ? strdup( acquired ) : NULL;
   ps->acquired_date = ntime_get();

   /* Player is trading ship in. */
   if (trade) {
      const char *old_name = player.p->name;
      player_swapShip( ship_name, 1 ); /* Move to the new ship. */
      player_rmShip( old_name );
   }

   free(ship_name);
   pfleet_update();

   /* Update ship list if landed. */
   if (landed) {
      int w = land_getWid( LAND_WINDOW_EQUIPMENT );
      equipment_regenLists( w, 0, 1 );
   }

   return ps;
}

/**
 * @brief Actually creates the new ship.
 */
static PlayerShip_t *player_newShipMake( const char *name )
{
   PilotFlags flags;
   PlayerShip_t *ps;

   /* store the current ship if it exists */
   pilot_clearFlagsRaw( flags );
   pilot_setFlagRaw( flags, PILOT_PLAYER );
   pilot_setFlagRaw( flags, PILOT_NO_EQUIP ); /* We want to give default outfits though. */

   /* in case we're respawning */
   player_rmFlag( PLAYER_CREATING );

   /* Grow memory. */
   ps = (player.p == NULL) ? &player.ps : &array_grow( &player_stack );
   memset( ps, 0, sizeof(PlayerShip_t) );
   pilot_setFlagRaw( flags, PILOT_PLAYER_FLEET );
   /* Create the ship. */
   ps->p = pilot_createEmpty( player_ship, name, faction_get("Player"), flags );
   if (player.p == NULL) {
      pilot_reset( ps->p );
      pilot_setPlayer( ps->p );
   }
   /* Initialize parent weapon sets. */
   ws_copy( ps->weapon_sets, ps->p->weapon_sets );

   if (player.p == NULL)
      ERR(_("Something seriously wonky went on, newly created player does not exist, bailing!"));

   /* money. */
   player.p->credits = player_creds;
   player_creds = 0;
   player_payback = 0;

   return ps;
}

/**
 * @brief Swaps player's current ship with their ship named shipname.
 *
 *    @param shipname Ship to change to.
 *    @param move_cargo Whether or not to move the cargo over or ignore it.
 */
void player_swapShip( const char *shipname, int move_cargo )
{
   HookParam hparam[5];
   Pilot *ship;
   vec2 v;
   double dir;
   int removed, hyptarget;
   PlayerShip_t *ps = NULL;
   PlayerShip_t ptemp;

   /* Try to find the ship. */
   for (int i=0; i<array_size(player_stack); i++) {
      if (strcmp(shipname,player_stack[i].p->name)==0) {
         ps = &player_stack[i];
         break;
      }
   }
   if (ps==NULL) {
      WARN( _("Unable to swap player.p with ship '%s': ship does not exist!"), shipname );
      return;
   }

   /* Save some variables to restore later. */
   hyptarget = player.p->nav_hyperspace;

   /* Run onremove hook for all old outfits. */
   for (int i=0; i<array_size(player.p->outfits); i++)
      pilot_outfitLRemove( player.p, player.p->outfits[i] );

   /* Get rid of deployed escorts and swap existing escorts. */
   escort_clearDeployed( player.p );
   escort_freeList( ps->p );
   ps->p->escorts = array_create( Escort_t );
   /* Just copying the array over has unforeseen consequences, so recreate. */
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      Escort_t ne = *e;

      /* Must not be new ship. */
      if (e->id == ps->p->id )
         continue;

      ne.ship = e->ship; /* Might be worth having an escort_copy function. */
      array_push_back( &ps->p->escorts, ne );
   }
   escort_freeList( player.p );

   /* Swap information over. */
   ptemp    = player.ps;
   player.ps= *ps;
   *ps      = ptemp;
   ship     = player.ps.p;

   /* Move credits over */
   ship->credits = player.p->credits;

   /* Copy target info */
   ship->target      = player.p->target;
   ship->nav_spob    = player.p->nav_spob;
   ship->nav_hyperspace = player.p->nav_hyperspace;
   ship->nav_anchor  = player.p->nav_anchor;
   ship->nav_asteroid= player.p->nav_asteroid;

   /* Store position. */
   v     = player.p->solid.pos;
   dir   = player.p->solid.dir;

   /* Copy over weapon sets. */
   ws_copy( player.ps.p->weapon_sets, player.ps.weapon_sets );

   /* If the pilot is deployed, we must redeploy. */
   removed = 0;
   if (ps->p->id > 0) {
      pilot_stackRemove( ps->p );
      removed = 1;
   }
   pilot_setPlayer( ship );
   player.ps.deployed = 0; /* Player themselves can't be deployed. */
   if (ps->deployed)
      pfleet_deploy( ps );

   /* Extra pass to calculate stats */
   pilot_calcStats( player.p );
   pilot_calcStats( ps->p );

   /* Run onadd hook for all new outfits. */
   for (int j=0; j<array_size(ship->outfits); j++)
      pilot_outfitLAdd( ship, ship->outfits[j] );

   /* Move cargo over. */
   if (move_cargo) {
      pilot_cargoMoveRaw( player.p, ps->p );
      pfleet_update(); /* Update fleet and move cargo. */
   }

   /* Clean up, AFTER cargo is updated. */
   if (!ps->deployed && removed)
      pilot_free( ps->p ); /* Has PILOT_NOFREE flag. */

   /* Copy position back. */
   player.p->solid.pos = v;
   player.p->solid.dir = dir;

   /* Fill the tank. */
   if (landed)
      land_refuel();

   /* Clear targets. */
   player_targetClearAll();
   player.p->nav_hyperspace = hyptarget; /* Special case restore hyperspace target. */

   /* Set some gui stuff. */
   gui_load( gui_pick() );

   /* Bind camera. */
   cam_setTargetPilot( player.p->id, 0 );

   /* Recompute stuff if necessary. */
   pilot_calcStats( player.p );
   player_resetSpeed();

   /* Run hook. */
   hparam[0].type    = HOOK_PARAM_STRING;
   hparam[0].u.str   = player.p->name;
   hparam[1].type    = HOOK_PARAM_SHIP;
   hparam[1].u.ship  = player.p->ship;
   hparam[2].type    = HOOK_PARAM_STRING;
   hparam[2].u.str   = ps->p->name;
   hparam[3].type    = HOOK_PARAM_SHIP;
   hparam[3].u.ship  = ps->p->ship;
   hparam[4].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "ship_swap", hparam );
}

/**
 * @brief Calculates the price of one of the player's ships.
 *
 *    @param shipname Name of the ship.
 *    @param count_unique Whether or not to count unique outfits too.
 *    @return The price of the ship in credits.
 */
credits_t player_shipPrice( const char *shipname, int count_unique )
{
   Pilot *ship = NULL;

   if (strcmp(shipname,player.p->name)==0)
      ship = player.p;
   else {
      /* Find the ship. */
      for (int i=0; i<array_size(player_stack); i++) {
         if (strcmp(shipname,player_stack[i].p->name)==0) {
            ship = player_stack[i].p;
            break;
         }
      }
   }

   /* Not found. */
   if (ship == NULL) {
      WARN( _("Unable to find price for player's ship '%s': ship does not exist!"), shipname );
      return -1;
   }

   return pilot_worth( ship, count_unique );
}

void player_rmPlayerShip( PlayerShip_t *ps )
{
   pilot_rmFlag( ps->p, PILOT_NOFREE );
   pilot_free( ps->p );
   ws_free( ps->weapon_sets );
   free( ps->acquired );
}

/**
 * @brief Removes one of the player's ships.
 *
 *    @param shipname Name of the ship to remove.
 */
void player_rmShip( const char *shipname )
{
   for (int i=0; i<array_size(player_stack); i++) {
      PlayerShip_t *ps = &player_stack[i];

      /* Not the ship we are looking for. */
      if (strcmp(shipname,ps->p->name)!=0)
         continue;

      /* Free player ship. */
      player_rmPlayerShip( ps );

      array_erase( &player_stack, ps, ps+1 );
   }

   /* Update ship list if landed. */
   if (landed) {
      int w = land_getWid( LAND_WINDOW_EQUIPMENT );
      equipment_regenLists( w, 0, 1 );
   }
}

/**
 * @brief Cleans up player stuff like player_stack.
 */
void player_cleanup (void)
{
   /* Enable all input. */
   input_enableAll();

   /* Clean up other stuff. */
   land_cleanup(); /* Should be first. */
   diff_clear();
   var_cleanup();
   missions_cleanup();
   events_cleanup();
   space_clearKnown();
   map_cleanup();
   factions_clearDynamic();
   player_inventoryClear();

   /* Reset controls. */
   player_accelOver();
   player_left  = 0.;
   player_right = 0.;

   /* Clear player. */
   player_clear();

   /* Clear hail timer. */
   player_hailCounter   = 0;
   player_hailTimer     = 0.;

   /* Clear messages. */
   gui_clearMessages();

   /* Reset factions. */
   factions_reset();

   /* Free stuff. */
   free(player.name);
   player.name = NULL;
   free( player.ps.acquired );
   player.ps.acquired = NULL;
   ws_free( player.ps.weapon_sets );

   free(player_message_noland);
   player_message_noland = NULL;

   /* Clean up gui. */
   gui_cleanup();
   player_guiCleanup();
   ovr_setOpen(0);

   /* Clear up info buttons. */
   info_buttonClear();

   array_free(player_outfits);
   player_outfits = NULL;

   array_free(missions_done);
   missions_done = NULL;

   array_free(events_done);
   events_done = NULL;

   /* Clean up licenses. */
   for (int i=0; i<array_size(player_licenses); i++)
      free(player_licenses[i]);
   array_free(player_licenses);
   player_licenses = NULL;

   /* Clear claims. */
   claim_clear();

   /* Purge the pilot stack, and player.p. */
   pilots_cleanAll();

   /* clean up the stack */
   for (int i=0; i<array_size(player_stack); i++)
      player_rmPlayerShip( &player_stack[i] );
   array_free(player_stack);
   player_stack = NULL;
   /* nothing left */

   /* Reset some player stuff. */
   player_creds   = 0;
   player_payback = 0;
   free( player.gui );
   player.gui = NULL;
   free( player.chapter );
   player.chapter = NULL;
   free( player.difficulty );
   player.difficulty = NULL;

   /* Clear omsg. */
   omsg_cleanup();

   /* Stop the sounds. */
   sound_stopAll();

   /* Clean up local difficulty. */
   difficulty_setLocal( NULL );

   /* Reset time compression. */
   pause_setSpeed( 1. );
   sound_setSpeed( 1. );

   free( player.loaded_version );
   player.loaded_version = NULL;

   /* Clean up. */
   memset( &player, 0, sizeof(Player_t) );
   player_setFlag(PLAYER_CREATING);
}

static int player_soundReserved = 0; /**< Has the player already reserved sound? */
/**
 * @brief Initializes the player sounds.
 */
static void player_initSound (void)
{
   if (player_soundReserved)
      return;

   /* Allocate channels. */
   player_engine_group  = sound_createGroup(1); /* Channel for engine noises. */
   player_gui_group     = sound_createGroup(4);
   player_hyper_group   = sound_createGroup(4);
   sound_speedGroup( player_gui_group, 0 ); /* Disable pitch shift. */
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
 * @brief Plays a GUI sound (unaffected by time accel).
 *
 *    @param sound ID of the sound to play.
 *    @param once Play only once?
 */
void player_soundPlayGUI( int sound, int once )
{
   sound_playGroup( player_gui_group, sound, once );
}

/**
 * @brief Plays a sound at the player.
 *
 *    @param sound ID of the sound to play.
 *    @param once Play only once?
 */
void player_soundPlay( int sound, int once )
{
   sound_playGroup( player_hyper_group, sound, once );
}

/**
 * @brief Stops playing player sounds.
 */
void player_soundStop (void)
{
   if (player_gui_group >= 0)
      sound_stopGroup( player_gui_group );
   if (player_engine_group >= 0)
      sound_stopGroup( player_engine_group );
   if (player_hyper_group >= 0)
      sound_stopGroup( player_hyper_group );

   /* No last engine sound. */
   player_lastEngineSound = -1;
}

/**
 * @brief Pauses the ship's sounds.
 */
void player_soundPause (void)
{
   if (player_engine_group >= 0)
      sound_pauseGroup(player_engine_group);
   if (player_hyper_group >= 0)
      sound_pauseGroup(player_hyper_group);
}

/**
 * @brief Resumes the ship's sounds.
 */
void player_soundResume (void)
{
   if (player_engine_group >= 0)
      sound_resumeGroup(player_engine_group);
   if (player_hyper_group >= 0)
      sound_resumeGroup(player_hyper_group);
}

/**
 * @brief Warps the player to the new position
 *
 *    @param x X value of the position to warp to.
 *    @param y Y value of the position to warp to.
 */
void player_warp( double x, double y )
{
   unsigned int target = cam_getTarget();
   vec2_cset( &player.p->solid.pos, x, y );
   /* Have to move camera over to avoid moving stars when loading. */
   if (target == player.p->id)
      cam_setTargetPilot( target, 0 );
}

/**
 * @brief Clears the targets.
 */
void player_clear (void)
{
   if (player.p != NULL) {
      pilot_setTarget( player.p, player.p->id );
      gui_setTarget();
   }

   /* Clear the noland flag. */
   player_rmFlag( PLAYER_NOLAND );
}

/**
 * @brief Checks to see if the player has enough credits.
 *
 *    @param amount Amount of credits to check to see if the player has.
 *    @return 1 if the player has enough credits.
 */
int player_hasCredits( credits_t amount )
{
   return pilot_hasCredits( player.p, amount );
}

/**
 * @brief Modifies the amount of credits the player has.
 *
 *    @param amount Quantity to modify player's credits by.
 *    @return Amount of credits the player has.
 */
credits_t player_modCredits( credits_t amount )
{
   return pilot_modCredits( player.p, amount );
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
            (player_timer < 0.))
         menu_death();
   }

   /* Skip rendering. */
   if ((player.p == NULL) || (player.p->id == 0) || player_isFlag(PLAYER_CREATING) ||
         pilot_isFlag( player.p, PILOT_HIDE))
      return;

   NTracingZone( _ctx, 1 );

   /* Render stealth overlay. */
   if (pilot_isFlag( player.p, PILOT_STEALTH ))
      player_renderStealthOverlay( dt );

   /* Render the aiming lines. */
   if ((player.p->target != PLAYER_ID) && player.p->aimLines
        && !pilot_isFlag( player.p, PILOT_HYPERSPACE ) && !pilot_isFlag( player.p, PILOT_DISABLED )
        && !pilot_isFlag( player.p, PILOT_LANDING ) && !pilot_isFlag( player.p, PILOT_TAKEOFF )
        && !player_isFlag( PLAYER_CINEMATICS_GUI ))
      player_renderAimHelper( dt );

   /* Render the player's pilot. */
   pilot_render( player.p );

   /* Render the player's overlay. */
   pilot_renderOverlay( player.p );

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Renders the player underlay.
 */
void player_renderUnderlay( double dt )
{
   /* Skip rendering. */
   if ((player.p == NULL) || player_isFlag(PLAYER_CREATING) ||
         pilot_isFlag( player.p, PILOT_HIDE))
      return;

   if (pilot_isFlag( player.p, PILOT_STEALTH ))
      player_renderStealthUnderlay( dt );
}

/**
 * @brief Renders the stealth overlay for the player.
 */
static void player_renderStealthUnderlay( double dt )
{
   (void) dt;
   double detectz;
   glColour col;
   Pilot *const* ps;

   /* Don't display if overlay is open. */
   if (ovr_isOpen())
      return;

   /* Iterate and draw for all pilots. */
   detectz = player.p->ew_stealth * cam_getZoom();
   col = cRed;
   col.a = 0.3;
   ps = pilot_getAll();
   for (int i=0; i<array_size(ps); i++) {
      double x, y, r;
      Pilot *t = ps[i];
      if (areAllies( player.p->faction, t->faction ) || pilot_isFriendly(t))
         continue;
      if (pilot_isDisabled(t))
         continue;
      /* Only show pilots the player can see. */
      if (!pilot_validTarget( player.p, t ))
         continue;

      gl_gameToScreenCoords( &x, &y, t->solid.pos.x, t->solid.pos.y );
      r = detectz * t->stats.ew_detect;
      if (r > 0.) {
         glUseProgram( shaders.stealthaura.program );
         gl_renderShader( x, y, r, r, 0., &shaders.stealthaura, &col, 1 );
      }
   }
}

/**
 * @brief Renders the stealth overlay for the player.
 */
static void player_renderStealthOverlay( double dt )
{
   (void) dt;
   double x, y, r, st, z;
   glColour col;

   z = cam_getZoom();
   gl_gameToScreenCoords( &x, &y, player.p->solid.pos.x, player.p->solid.pos.y );

   /* Determine the arcs. */
   st    = player.p->ew_stealth_timer;

   /* We do red to yellow. */
   col_blend( &col, &cYellow, &cRed, st );
   col.a = 0.5;

   /* Determine size. */
   r = 1.2/2. * (double)player.p->ship->gfx_space->sw;

   /* Draw the main circle. */
   glUseProgram( shaders.stealthmarker.program );
   glUniform1f( shaders.stealthmarker.paramf, st );
   gl_renderShader( x, y, r*z, r*z, 0., &shaders.stealthmarker, &col, 1 );
}

/**
 * @brief Renders the aim helper.
 */
static void player_renderAimHelper( double dt )
{
   (void) dt;
   double a, b, d, x1, y1, x2, y2, r, theta;
   glColour c, c2;
   Pilot *target;

   target = pilot_getTarget( player.p );
   if (target == NULL)
      return;

   a = player.p->solid.dir;
   r = 200.;
   gl_gameToScreenCoords( &x1, &y1, player.p->solid.pos.x, player.p->solid.pos.y );

   b = pilot_aimAngle( player.p, &target->solid.pos, &target->solid.vel );

   theta = 22.*M_PI/180.;

   /* The angular error will give the exact colour that is used. */
   d = ABS( angle_diff(a,b) / (2*theta) );
   d = MIN( 1, d );

   c = cInert;
   c.a = 0.3;
   gl_gameToScreenCoords( &x2, &y2, player.p->solid.pos.x + r*cos( a+theta ),
                           player.p->solid.pos.y + r*sin( a+theta ) );
   gl_renderLine( x1, y1, x2, y2, &c );
   gl_gameToScreenCoords( &x2, &y2, player.p->solid.pos.x + r*cos( a-theta ),
                           player.p->solid.pos.y + r*sin( a-theta ) );
   gl_renderLine( x1, y1, x2, y2, &c );

   c.r = d*0.9;
   c.g = d*0.2 + (1.-d)*0.8;
   c.b = (1-d)*0.2;
   c.a = 0.7;
   col_gammaToLinear( &c );
   gl_gameToScreenCoords( &x2, &y2, player.p->solid.pos.x + r*cos( a ),
                           player.p->solid.pos.y + r*sin( a ) );

   gl_renderLine( x1, y1, x2, y2, &c );

   c2 = cWhite;
   c2.a = 0.7;
   glUseProgram(shaders.crosshairs.program);
   glUniform1f(shaders.crosshairs.paramf, 1.);
   gl_renderShader( x2, y2, 7, 7, 0., &shaders.crosshairs, &c2, 1 );

   gl_gameToScreenCoords( &x2, &y2, player.p->solid.pos.x + r*cos( b ),
                           player.p->solid.pos.y + r*sin( b ) );

   c.a = 0.4;
   gl_renderLine( x1, y1, x2, y2, &c );

   /* TODO this should be converted into a single SDF call. */
   glColour c3 = cBlack;
   c3.a = c2.a;
   gl_renderCircle( x2, y2, 8., &c3, 0 );
   gl_renderCircle( x2, y2, 10., &c3, 0 );
   gl_renderCircle( x2, y2, 9., &c2, 0 );
}

/**
 * @brief Basically uses keyboard input instead of AI input. Used in pilot.c.
 *
 *    @param pplayer Player to think.
 *    @param dt Current delta tick.
 */
void player_think( Pilot* pplayer, const double dt )
{
   Pilot *target;
   int facing, fired;

   /* last i heard, the dead don't think */
   if (pilot_isFlag(pplayer,PILOT_DEAD)) {
      /* no sense in accelerating or turning */
      pilot_setAccel( pplayer, 0. );
      pilot_setTurn( pplayer, 0. );
      return;
   }

   /* We always have to run ai_think in the case the player has escorts so that
    * they properly form formations, however, we only have to do the task under manual control.. */
   ai_think( pplayer, dt, pilot_isFlag(pplayer, PILOT_MANUAL_CONTROL) );

   /* Under manual control is special. */
   if (pilot_isFlag( pplayer, PILOT_MANUAL_CONTROL ) || pilot_isFlag( pplayer, PILOT_HIDE ))
      return;

   /* Not facing anything yet. */
   facing = 0;

   /* Autonav takes over normal controls. */
   if (player_isFlag(PLAYER_AUTONAV)) {
      player_thinkAutonav( pplayer, dt );

      /* Disable turning. */
      facing = 1;
   }

   /* Mouse-flying is enabled. */
   if (!facing && player_isFlag(PLAYER_MFLY))
      facing = player_thinkMouseFly( dt );

   /* turning taken over by PLAYER_FACE */
   if (!facing && player_isFlag(PLAYER_FACE)) {
      /* Try to face pilot target. */
      if (player.p->target != PLAYER_ID) {
         target = pilot_getTarget( player.p );
         if (target != NULL) {
            pilot_face( pplayer,
                  vec2_angle( &player.p->solid.pos, &target->solid.pos ),
                  dt );

            /* Disable turning. */
            facing = 1;
         }
      }
      /* Try to face asteroid. */
      else if (player.p->nav_asteroid != -1) {
         AsteroidAnchor *field = &cur_system->asteroids[player.p->nav_anchor];
         Asteroid *ast = &field->asteroids[player.p->nav_asteroid];
         pilot_face( pplayer,
               vec2_angle( &player.p->solid.pos, &ast->sol.pos ),
               dt);
         /* Disable turning. */
         facing = 1;
      }
      /* If not try to face spob target. */
      else if ((player.p->nav_spob != -1) && ((preemption == 0) || (player.p->nav_hyperspace == -1))) {
         pilot_face( pplayer,
               vec2_angle( &player.p->solid.pos,
                  &cur_system->spobs[ player.p->nav_spob ]->pos ),
               dt);
         /* Disable turning. */
         facing = 1;
      }
      else if (player.p->nav_hyperspace != -1) {
         pilot_face( pplayer,
               vec2_angle( &player.p->solid.pos,
                  &cur_system->jumps[ player.p->nav_hyperspace ].pos ),
               dt);
         /* Disable turning. */
         facing = 1;
      }
   }

   /* turning taken over by PLAYER_REVERSE */
   if (player_isFlag(PLAYER_REVERSE)) {
      /*
       * If the player has reverse thrusters, fire those.
       */
      if (!player.p->stats.misc_reverse_thrust && !facing) {
         pilot_face( pplayer, VANGLE(player.p->solid.vel) + M_PI, dt );
         /* Disable turning. */
         facing = 1;
      }
   }

   /* Normal turning scheme */
   if (!facing) {
      double turn = 0;
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
   fired = 0;
   pilot_shoot( pplayer, player_isFlag(PLAYER_PRIMARY), player_isFlag(PLAYER_SECONDARY) );

   if (fired)
      player_autonavReset( 1. );

   if (!player_isFlag(PLAYER_AUTONAV)) {
      double acc = player_acc;
      /* Have to handle the case the player is doing reverse. This takes priority
      * over normal accel. */
      if (player_isFlag(PLAYER_REVERSE) && player.p->stats.misc_reverse_thrust
            && !pilot_isFlag(player.p, PILOT_HYP_PREP)
            && !pilot_isFlag(player.p, PILOT_HYPERSPACE) )
         acc = -PILOT_REVERSE_THRUST;

      pilot_setAccel( pplayer, acc );
   }
}

/**
 * @brief Player update function.
 *
 *    @param pplayer Player to update.
 *    @param dt Current delta tick.
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
 * @brief Does a player specific update.
 *
 *    @param pplayer Player to update.
 *    @param dt Current delta tick.
 */
void player_updateSpecific( Pilot *pplayer, const double dt )
{
   int engsound;
   double pitch = 1.;

   /* Calculate engine sound to use. */
   if (pilot_isFlag(pplayer, PILOT_AFTERBURNER))
      engsound = pplayer->afterburner->outfit->u.afb.sound;
   else if (pilot_isFlag(pplayer, PILOT_HYPERSPACE))
      engsound = snd_hypEng;
   else if (pplayer->engine_glow > 0.) {
      engsound = pplayer->ship->sound;
      pitch = pplayer->ship->engine_pitch;
   }
   else
      engsound = -1;
   if (engsound >= 0)
      sound_volumeGroup( player_engine_group, conf.engine_vol * pplayer->engine_glow );
   /* See if sound must change. */
   if (player_lastEngineSound != engsound) {
      sound_stopGroup( player_engine_group );
      if (engsound >= 0) {
         sound_pitchGroup( player_engine_group, pitch );
         sound_playGroup( player_engine_group, engsound, 0 );
      }
   }
   player_lastEngineSound = engsound;

   /* Sound. */
   /*
    * Sound is now camera-specific and thus not player specific. A bit sad really.
   sound_updateListener( pplayer->solid.dir,
         pplayer->solid.pos.x, pplayer->solid.pos.y,
         pplayer->solid.vel.x, pplayer->solid.vel.y );
   */

   /* See if must play hail sound. */
   if (player_hailCounter > 0) {
      player_hailTimer -= dt;
      if (player_hailTimer < 0.) {
         player_soundPlayGUI( snd_hail, 1 );
         player_hailCounter--;
         player_hailTimer = 3.;
      }
   }

   /* Handle passive scanning of nearby asteroids. */
   /* TODO should probably handle player escorts in the future. */
   if (player.p->stats.asteroid_scan > 0.) {
      double range = player.p->stats.asteroid_scan;
      for (int i=0; i<array_size(cur_system->asteroids); i++) {
         double r2;
         AsteroidAnchor *ast = &cur_system->asteroids[i];

         /* Field out of range. */
         if (vec2_dist2( &ast->pos, &player.p->solid.pos ) > pow2(range+ast->radius+ast->margin))
            continue;

         r2 = pow2(range);
         for (int j=0; j<array_size(ast->asteroids); j++) {
            HookParam hparam[2];
            Asteroid *a = &ast->asteroids[j];

            if (a->scanned) /* Ignore scanned outfits. */
               continue;

            if (vec2_dist2( &a->sol.pos, &player.p->solid.pos ) > r2)
               continue;

            a->scanned = 1;

            /* Run the hook. */
            hparam[0].type = HOOK_PARAM_ASTEROID;
            hparam[0].u.ast.parent = ast->id;
            hparam[0].u.ast.id = a->id;
            hparam[1].type = HOOK_PARAM_SENTINEL;
            hooks_runParamDeferred( "asteroid_scan", hparam );
         }
      }
   }
}

/*
 *    For use in keybindings
 */
/**
 * @brief Handles keyboard events involving the player's weapon-set keys. It's valid to call this while gameplay is paused.
 */
void player_weapSetPress( int id, double value, int repeat )
{
   int type;

   if (repeat || (player.p == NULL))
      return;

   type = (value>=0) ? +1 : -1;

   if (type > 0) {
      if (toolkit_isOpen())
         return;

      if ((pilot_isFlag(player.p, PILOT_HYP_PREP) ||
            pilot_isFlag(player.p, PILOT_HYPERSPACE) ||
            pilot_isFlag(player.p, PILOT_LANDING) ||
            pilot_isFlag(player.p, PILOT_TAKEOFF)))
         return;
   }

   pilot_weapSetPress( player.p, id, type );
}

/**
 * @brief Resets the player speed stuff.
 */
void player_resetSpeed (void)
{
   double spd = player.speed * player_dt_default();
   pause_setSpeed( spd );
   sound_setSpeed( spd / conf.game_speed );
}

/**
 * @brief Aborts autonav and other states that take control of the ship.
 *
 *    @param reason Reason for aborting (see player.h)
 *    @param str String accompanying the reason.
 */
void player_restoreControl( int reason, const char *str )
{
   if (player.p==NULL)
      return;

   if (reason != PINPUT_AUTONAV) {
      /* Autonav should be harder to abort when paused. */
      if ((!paused || reason != PINPUT_MOVEMENT))
         player_autonavAbort(str);
   }

   if (reason != PINPUT_BRAKING) {
      pilot_rmFlag(player.p, PILOT_BRAKING);
      pilot_rmFlag(player.p, PILOT_COOLDOWN_BRAKE);
      if (pilot_isFlag(player.p, PILOT_COOLDOWN))
         pilot_cooldownEnd(player.p, str);
   }
}

/**
 * @brief Sets the player's target spob.
 *
 *    @param id Target spob or -1 if none should be selected.
 */
void player_targetSpobSet( int id )
{
   int old;

   /* Player must exist. */
   if (player.p == NULL)
      return;

   if (id >= array_size(cur_system->spobs)) {
      WARN(_("Trying to set player's spob target to invalid ID '%d'"), id);
      return;
   }

   if ((player.p == NULL) || pilot_isFlag( player.p, PILOT_LANDING ))
      return;

   old = player.p->nav_spob;
   player.p->nav_spob = id;
   player_hyperspacePreempt((id < 0) ? 1 : 0);
   if (old != id) {
      player_rmFlag(PLAYER_LANDACK);
      if (id >= 0)
         player_soundPlayGUI(snd_nav, 1);
   }
   gui_forceBlink();
   gui_setNav();

   if (player.autonav==AUTONAV_SPOB)
      player_autonavAbort(NULL);
}

/**
 * @brief Sets the player's target asteroid.
 *
 *    @param field Index of the parent field of the asteoid.
 *    @param id Target spob or -1 if none should be selected.
 */
void player_targetAsteroidSet( int field, int id )
{
   int old;

   if ((player.p == NULL) || pilot_isFlag( player.p, PILOT_LANDING ))
      return;

   old = player.p->nav_asteroid;
   player.p->nav_asteroid = id;
   if (old != id) {
      if (id >= 0) {
         player_soundPlayGUI(snd_nav, 1);
      }
   }

   player.p->nav_anchor = field;

   /* Untarget pilot. */
   player.p->target = player.p->id;
}

/**
 * @brief Cycle through spob targets.
 */
void player_targetSpob (void)
{
   int id;

   /* Not under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   /* Find next spob target. */
   for (id=player.p->nav_spob+1; id<array_size(cur_system->spobs); id++)
      if (spob_isKnown( cur_system->spobs[id] ))
         break;

   /* Try to select the lowest-indexed valid spob. */
   if (id >= array_size(cur_system->spobs) ) {
      id = -1;
      for (int i=0; i<array_size(cur_system->spobs); i++)
         if (spob_isKnown( cur_system->spobs[i] )) {
            id = i;
            break;
         }
   }

   /* Untarget if out of range. */
   player_targetSpobSet( id );
}

/**
 * @brief Try to land or target closest spob if no land target.
 *
 *    @param loud Whether or not to show messages irrelevant when auto-landing.
 *    @return One of PLAYER_LAND_OK, PLAYER_LAND_AGAIN, or PLAYER_LAND_DENIED.
 */
int player_land( int loud )
{
   Spob *spob;
   int silent = 0; /* Whether to suppress the land ack noise. */

   if (landed) { /* player is already landed */
      takeoff( 1, 0 );
      return PLAYER_LAND_DENIED;
   }

   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return PLAYER_LAND_DENIED;

   /* Already landing. */
   if ((pilot_isFlag( player.p, PILOT_LANDING) ||
         pilot_isFlag( player.p, PILOT_TAKEOFF)))
      return PLAYER_LAND_DENIED;

   /* Check if there are spobs to land on. */
   if (array_size(cur_system->spobs) == 0) {
      player_message( "#r%s", _("There are no spobs to land on.") );
      return PLAYER_LAND_DENIED;
   }

   if (player_isFlag(PLAYER_NOLAND)) {
      player_message( "#r%s", player_message_noland );
      return PLAYER_LAND_DENIED;
   }
   else if (pilot_isFlag( player.p, PILOT_NOLAND)) {
      player_message( "#r%s", _("Docking stabilizers malfunctioning, cannot land.") );
      return PLAYER_LAND_DENIED;
   }

   /* No target means no land. */
   if (player.p->nav_spob == -1)
      return PLAYER_LAND_DENIED;
   /* Check if spob is in range when not uninhabited. */
   else if (!spob_isFlag(cur_system->spobs[ player.p->nav_spob ], SPOB_UNINHABITED) && !pilot_inRangeSpob( player.p, player.p->nav_spob )) {
      player_spobOutOfRangeMsg();
      return PLAYER_LAND_AGAIN;
   }

   /* attempt to land at selected spob */
   spob = cur_system->spobs[player.p->nav_spob];
   spob_updateLand( spob ); /* Update if necessary. */
   if ((spob->lua_can_land==LUA_NOREF) && !spob_hasService(spob, SPOB_SERVICE_LAND)) {
      player_message( "#r%s", _("You can't land here.") );
      return PLAYER_LAND_DENIED;
   }
   else if ((spob->lua_can_land!=LUA_NOREF) && !spob->can_land) {
      if (spob->land_msg)
         player_message( _("#%c%s>#0 %s"), spob_getColourChar(spob),
               spob_name(spob), spob->land_msg );
      else
         player_message( "#r%s", _("You can't land here.") );
      return PLAYER_LAND_DENIED;
   }
   else if (!player_isFlag(PLAYER_LANDACK)) { /* no landing authorization */
      if (spob_hasService(spob,SPOB_SERVICE_INHABITED)) { /* Basic services */
         if (spob->can_land)
            player_message( _("#%c%s>#0 %s"), spob_getColourChar(spob),
                  spob_name(spob), spob->land_msg );
         else if (spob->land_override > 0)
            player_message( _("#%c%s>#0 %s"), spob_getColourChar(spob),
                  spob_name(spob), _("Landing authorized.") );
         else { /* Hostile */
            player_message( _("#%c%s>#0 %s"), spob_getColourChar(spob),
                  spob_name(spob), spob->land_msg );
            return PLAYER_LAND_DENIED;
         }
      }
      else /* No shoes, no shirt, no lifeforms, no service. */
         player_message( _("#oReady to land on %s."), spob_name(spob) );

      player_setFlag(PLAYER_LANDACK);
      if (!silent)
         player_soundPlayGUI(snd_nav, 1);

      return player_land(loud);
   }
   else if (vec2_dist2(&player.p->solid.pos,&spob->pos) > pow2(spob->radius)) {
      if (loud)
         player_message(_("#rYou are too far away to land on %s."), spob_name(spob));
      return PLAYER_LAND_AGAIN;
   }
   else if (vec2_odist2( &player.p->solid.vel ) > pow2(MAX_HYPERSPACE_VEL)) {
      if (loud)
         player_message(_("#rYou are going too fast to land on %s."), spob_name(spob));
      return PLAYER_LAND_AGAIN;
   }

   /* End autonav. */
   player_autonavEnd();

   /* Stop afterburning. */
   pilot_afterburnOver( player.p );
   /* Stop accelerating. */
   player_accelOver();
   /* Stop stealth. */
   pilot_destealth( player.p );

   /* Stop all on outfits. */
   if (pilot_outfitOffAll( player.p ) > 0)
      pilot_calcStats( player.p );

   /* Do whatever the spob wants to do. */
   if (spob->lua_land != LUA_NOREF) {
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, spob->lua_land); /* f */
      lua_pushspob( naevL, spob_index(spob) );
      lua_pushpilot( naevL, player.p->id );
      if (nlua_pcall( spob->lua_env, 2, 0 )) {
         WARN(_("Spob '%s' failed to run '%s':\n%s"), spob->name, "land", lua_tostring(naevL,-1));
         lua_pop(naevL,1);
      }

      return PLAYER_LAND_OK;
   }

   /* Start landing. */
   player_soundPause();
   player.p->landing_delay = PILOT_LANDING_DELAY * player_dt_default();
   player.p->ptimer = player.p->landing_delay;
   pilot_setFlag( player.p, PILOT_LANDING );
   pilot_setAccel( player.p, 0. );
   pilot_setTurn( player.p, 0. );

   return PLAYER_LAND_OK;
}

/**
 * @brief Revokes landing authorization if the player's reputation is too low.
 */
void player_checkLandAck( void )
{
   Spob *p;

   /* No authorization to revoke. */
   if ((player.p == NULL) || !player_isFlag(PLAYER_LANDACK))
      return;

   /* Avoid a potential crash if PLAYER_LANDACK is set inappropriately. */
   if (player.p->nav_spob < 0) {
      WARN(_("Player has landing permission, but no valid spob targeted."));
      return;
   }

   p = cur_system->spobs[ player.p->nav_spob ];

   /* Player can still land. */
   if (p->can_land || (p->land_override > 0))
      return;

   player_rmFlag(PLAYER_LANDACK);
   player_message( _("#%c%s>#0 Landing permission revoked."),
         spob_getColourChar(p), spob_name(p) );
}

/**
 * @brief Sets the no land message.
 *
 *    @brief str Message to set when the player is not allowed to land temporarily.
 */
void player_nolandMsg( const char *str )
{
   free(player_message_noland);

   /* Duplicate so that Lua memory which might be garbage-collected isn't relied on. */
   if (str != NULL)
      player_message_noland = strdup(str);
   else
      player_message_noland = strdup(_("You are not allowed to land at this moment."));
}

/**
 * @brief Logic to make the player approach a target pilot to board or spob to land on.
 */
void player_approach (void)
{
   int plt = (player.p->target!=PLAYER_ID);
   int lnd = (player.p->nav_spob != -1);

   if (plt && (player_canBoard(0)!=PLAYER_BOARD_IMPOSSIBLE)) {
      if (player_tryBoard(1) == PLAYER_BOARD_RETRY)
         player_autonavBoard( player.p->target );
      return;
   }
   else if (lnd) {
      int canland = player_land(1);
      if (canland == PLAYER_LAND_AGAIN)
         player_autonavSpob( cur_system->spobs[player.p->nav_spob]->name, 1 );
      else if (canland == PLAYER_LAND_DENIED)
         player_autonavSpob( cur_system->spobs[player.p->nav_spob]->name, 0 );
      return;
   }
   else {
      /* In the case they have no target already, we just try to find a target
       * first, with priority for boarding. */
      if (!plt) {
         Pilot *nearp;
         double d = pilot_getNearestPosPilot( player.p, &nearp, player.p->solid.pos.x, player.p->solid.pos.y, 1 );
         if ((nearp!=NULL) && !pilot_isFlag(nearp,PILOT_NOBOARD) && (d<pow2(5e3)) &&
               (pilot_isDisabled(nearp) || pilot_isFlag(nearp,PILOT_BOARDABLE))) {
            player_targetSet( nearp->id );
            player_tryBoard(0); /* Try to board if can. */
            return;
         }
      }

      /* Now try to find a landing target. */
      if (!lnd) {
         double td = -1.; /* temporary distance */
         int tp = -1; /* temporary spob */
         for (int i=0; i<array_size(cur_system->spobs); i++) {
            const Spob *spob = cur_system->spobs[i];
            double d = vec2_dist(&player.p->solid.pos,&spob->pos);
            if (!pilot_inRangeSpob( player.p, i ))
               continue;
            if (!spob_hasService(spob,SPOB_SERVICE_LAND))
               continue;
            if ((tp==-1) || ((td==-1) || (td > d))) {
               tp = i;
               td = d;
            }
         }
         if (tp>=0) {
            player_targetSpobSet( tp );
            player_hyperspacePreempt(0);
            player_land(0); /* Try to land if can. */
            return;
         }
      }
   }
}

/**
 * @brief Sets the player's hyperspace target.
 *
 *    @param id ID of the hyperspace target.
 *    @param nomsg Whether or not to display a message regarding aborting autonav.
 */
void player_targetHyperspaceSet( int id, int nomsg )
{
   int old;

   /* Player must exist. */
   if (player.p == NULL)
      return;

   if (id >= array_size(cur_system->jumps)) {
      WARN(_("Trying to set player's hyperspace target to invalid ID '%d'"), id);
      return;
   }

   if (pilot_isFlag(player.p, PILOT_HYP_PREP) ||
         pilot_isFlag(player.p, PILOT_HYP_BEGIN) ||
         pilot_isFlag(player.p, PILOT_HYPERSPACE))
      return;

   old = player.p->nav_hyperspace;
   player.p->nav_hyperspace = id;
   player_hyperspacePreempt((id < 0) ? 0 : 1);
   if ((old != id) && (id >= 0))
      player_soundPlayGUI(snd_nav,1);
   gui_setNav();

   if (!nomsg && (old != id) && (player.autonav==AUTONAV_JUMP))
      player_autonavAbort(NULL);

   hooks_run( "target_hyperspace" );
}

/**
 * @brief Gets a hyperspace target.
 */
void player_targetHyperspace (void)
{
   int id;

   /* Not under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   map_clear(); /* clear the current map path */

   for (id=player.p->nav_hyperspace+1; id<array_size(cur_system->jumps); id++)
      if (jp_isKnown( &cur_system->jumps[id]))
         break;

   /* Try to find the lowest-indexed valid jump. */
   if (id >= array_size(cur_system->jumps)) {
      id = -1;
      for (int i=0; i<array_size(cur_system->jumps); i++)
         if (jp_isUsable( &cur_system->jumps[i])) {
            id = i;
            break;
         }
   }

   player_targetHyperspaceSet( id, 0 );

   /* Map gets special treatment if open. */
   if (id == -1)
      map_select( NULL , 0);
   else
      map_select( cur_system->jumps[ id ].target, 0 );
}

/**
 * @brief Enables or disables jump points preempting spobs in autoface and target clearing.
 *
 *    @param preempt Boolean; 1 preempts spob target.
 */
void player_hyperspacePreempt( int preempt )
{
   preemption = preempt;
}

/**
 * @brief Returns whether the jump point target should preempt the spob target.
 *
 *    @return Boolean; 1 preempts spob target.
 */
int player_getHypPreempt(void)
{
   return preemption;
}

/**
 * @brief Returns the player's total default time delta based on time dilation stuff.
 *
 *    @return The default/minimum time delta
 */
double player_dt_default (void)
{
   if (player.p != NULL && player.p->ship != NULL)
      return player.p->stats.time_mod * player.p->ship->dt_default;
   return 1.;
}

/**
 * @brief Starts the hail sounds and aborts autoNav
 */
void player_hailStart (void)
{
   char buf[128];

   player_hailCounter = 5;

   input_getKeybindDisplay( "autohail", buf, sizeof(buf) );
   player_message( _("#rReceiving hail! Press #b%s#0 to respond."), buf );

   /* Reset speed. */
   player_autonavReset( 10. );
}

/**
 * @brief Actually attempts to jump in hyperspace.
 *
 *    @return 1 if actually started a jump, 0 otherwise.
 */
int player_jump (void)
{
   int h;

   /* Must have a jump target and not be already jumping. */
   if (pilot_isFlag(player.p, PILOT_HYPERSPACE))
      return 0;

   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return 0;

   /* Select nearest jump if not target. */
   if (player.p->nav_hyperspace == -1) {
      int j    = -1;
      double mindist  = INFINITY;
      for (int i=0; i<array_size(cur_system->jumps); i++) {
         double dist = vec2_dist2( &player.p->solid.pos, &cur_system->jumps[i].pos );
         if (dist < mindist && jp_isUsable(&cur_system->jumps[i])) {
            mindist  = dist;
            j        = i;
         }
      }
      if (j  < 0)
         return 0;

      player.p->nav_hyperspace = j;
      player_soundPlayGUI(snd_nav,1);
      map_select( cur_system->jumps[player.p->nav_hyperspace].target, 0 );
      gui_setNav();

      /* Only follow through if within range. */
      if (mindist > pow2( cur_system->jumps[j].radius ))
         return 0;
   }

   /* Already jumping, so we break jump. */
   if (pilot_isFlag(player.p, PILOT_HYP_PREP)) {
      pilot_hyperspaceAbort(player.p);
      player_message( "#r%s", _("Aborting hyperspace sequence."));
      return 0;
   }

   /* Try to hyperspace. */
   h = space_hyperspace(player.p);
   if (h == -1) {
      player_hyperspacePreempt( 1 );
      player_autonavStart();
      //player_message( "#r%s", _("You are too far from a jump point to initiate hyperspace."));
   }
   else if (h == -2)
      player_message( "#r%s", _("Hyperspace drive is offline."));
   else if (h == -3)
      player_message( "#r%s", _("You do not have enough fuel to hyperspace jump."));
   else {
      player_message( "#o%s", _("Preparing for hyperspace."));
      /* Stop acceleration noise. */
      player_accelOver();
      /* Stop possible shooting. */
      pilot_shoot( player.p, 0, 0 );

      /* Order escorts to jump; just for aesthetics (for now) */
      escorts_jump( player.p, &cur_system->jumps[player.p->nav_hyperspace] );

      return 1;
   }

   return 0;
}

/**
 * @brief Player actually broke hyperspace (entering new system).
 */
void player_brokeHyperspace (void)
{
   ntime_t t;
   StarSystem *sys;
   JumpPoint *jp;
   Pilot *const* pilot_stack;

   /* First run jump hook. */
   hooks_run( "jumpout" );

   /* Prevent targeted spob # from carrying over. */
   gui_setNav();
   gui_setTarget();
   player_targetSpobSet( -1 );
   player_targetAsteroidSet( -1, -1 );

   /* calculates the time it takes, call before space_init */
   t  = pilot_hyperspaceDelay( player.p );
   ntime_inc( t );

   /* Save old system. */
   sys = cur_system;

   /* Free old graphics. */
   space_gfxUnload( sys );

   /* enter the new system */
   jp = &cur_system->jumps[player.p->nav_hyperspace];
   space_init( jp->target->name, 1 );

   /* Set up the overlay. */
   ovr_initAlpha();

   /* set position, the pilot_update will handle lowering vel */
   space_calcJumpInPos( cur_system, sys, &player.p->solid.pos, &player.p->solid.vel, &player.p->solid.dir, player.p );
   cam_setTargetPilot( player.p->id, 0 );

   /* reduce fuel */
   player.p->fuel -= player.p->fuel_consumption;

   /* Set the ptimer. */
   player.p->ptimer = HYPERSPACE_FADEIN;

   /* Update the map, we have to remove the player flags first or it breaks down. */
   pilot_rmFlag( player.p, PILOT_HYPERSPACE );
   pilot_rmFlag( player.p, PILOT_HYP_BEGIN );
   pilot_rmFlag( player.p, PILOT_HYP_BRAKE );
   pilot_rmFlag( player.p, PILOT_HYP_PREP );
   map_jump();

   /* Add persisted pilots */
   pilot_stack = pilot_getAll();
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];

      if (pilot_isFlag(p, PILOT_PERSIST) || pilot_isFlag(p, PILOT_PLAYER)) {
         if (p != player.p)
            space_calcJumpInPos( cur_system, sys, &p->solid.pos, &p->solid.vel, &p->solid.dir, player.p );

         /* Run Lua stuff for all persistant pilots. */
         pilot_outfitLInitAll( p );
         pilot_outfitLOnjumpin( p );

         /* Invulnerable delay too. */
         p->itimer = PILOT_PLAYER_NONTARGETABLE_JUMPIN_DELAY;
         pilot_setFlag( p, PILOT_NONTARGETABLE );

         /* Clear flags as necessary. */
         pilot_rmFlag( p, PILOT_HYPERSPACE );
         pilot_rmFlag( p, PILOT_HYP_BEGIN );
         pilot_rmFlag( p, PILOT_HYP_BRAKE );
         pilot_rmFlag( p, PILOT_HYP_PREP );
      }
   }

   /* Disable autonavigation if arrived. */
   player_autonavEnter();

   /* Safe since this is run in the player hook section. */
   hooks_run( "jumpin" );
   hooks_run( "enter" );
   events_trigger( EVENT_TRIGGER_ENTER );
   missions_run( MIS_AVAIL_ENTER, -1, NULL, NULL );

   /* Player sound. */
   player_soundPlay( snd_hypJump, 1 );

   /* Increment times jumped. */
   player.jumped_times++;
   player.ps.jumped_times++;
}

/**
 * @brief Start accelerating.
 *
 *    @param acc How much thrust should be applied of maximum (0 - 1).
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
 * @brief Sets the player's target.
 *
 *    @param id Target to set for the player.
 */
void player_targetSet( unsigned int id )
{
   unsigned int old;
   old = player.p->target;
   pilot_setTarget( player.p, id );
   if ((old != id) && (player.p->target != PLAYER_ID)) {
      gui_forceBlink();
      player_soundPlayGUI( snd_target, 1 );
   }
   gui_setTarget();

   /* Clear the asteroid target. */
   player.p->nav_asteroid = -1;
   player.p->nav_anchor = -1;

   /* The player should not continue following if the target pilot has been changed. */
   if ((old != id) && player_isFlag(PLAYER_AUTONAV) && (player.autonav==AUTONAV_PILOT))
      player_autonavAbort(NULL);
}

/**
 * @brief Targets the nearest hostile enemy to the player.
 *
 * @note This function largely duplicates pilot_getNearestEnemy, because the
 *       player's hostility with AIs is more nuanced than AI vs AI.
 */
void player_targetHostile (void)
{
   unsigned int tp;
   double d, td;
   Pilot *const* pilot_stack;

   tp = PLAYER_ID;
   d  = 0;
   pilot_stack = pilot_getAll();
   for (int i=0; i<array_size(pilot_stack); i++) {
      /* Shouldn't be disabled. */
      if (pilot_isDisabled(pilot_stack[i]))
         continue;

      /* Must be a valid target. */
      if (!pilot_canTarget( pilot_stack[i] ))
         continue;

      /* Must be hostile. */
      if (!pilot_isHostile(pilot_stack[i]))
         continue;

      if (pilot_inRangePilot(player.p, pilot_stack[i], &td) != 1 )
         continue;

      if (tp == PLAYER_ID || ((td < d))) {
         d  = td;
         tp = pilot_stack[i]->id;
      }
   }

   player_targetSet( tp );
}

/**
 * @brief Cycles to next target.
 *
 *    @param mode Mode to target. 0 is normal, 1 is hostiles.
 */
void player_targetNext( int mode )
{
   player_targetSet( pilot_getNextID(player.p->target, mode) );
}

/**
 * @brief Cycles to previous target.
 *
 *    @param mode Mode to target. 0 is normal, 1 is hostiles.
 */
void player_targetPrev( int mode )
{
   player_targetSet( pilot_getPrevID(player.p->target, mode) );
}

/**
 * @brief Clears the player's ship, spob or hyperspace target, in that order.
 */
void player_targetClear (void)
{
   gui_forceBlink();
   if (player.p->target != PLAYER_ID)
      player_targetSet( PLAYER_ID );
   else if (player.p->nav_asteroid >= 0)
      player_targetAsteroidSet( -1, -1 );
   else if (player.p->nav_spob >= 0)
      player_targetSpobSet( -1 );
   else if ((preemption == 1 || player.p->nav_spob == -1) &&
         !pilot_isFlag(player.p, PILOT_HYP_PREP)) {
      player.p->nav_hyperspace = -1;
      player_hyperspacePreempt(0);
      map_clear();
   }
   gui_setNav();
}

/**
 * @brief Clears all player targets: hyperspace, spob, asteroid, etc...
 */
void player_targetClearAll (void)
{
   player_targetSpobSet( -1 );
   player_targetHyperspaceSet( -1, 0 );
   player_targetAsteroidSet( -1, -1 );
   player_targetSet( PLAYER_ID );
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
   for (i=0; i<array_size(player.p->escorts); i++) {
      if (player.p->target == player.p->escorts[i].id) {

         /* Cycle targets. */
         if (prev)
            pilot_setTarget( player.p, (i > 0) ?
                  player.p->escorts[i-1].id : player.p->id );
         else
            pilot_setTarget( player.p, (i < array_size(player.p->escorts)-1) ?
                  player.p->escorts[i+1].id : player.p->id );

         break;
      }
   }

   /* Not found in loop. */
   if (i >= array_size(player.p->escorts)) {
      /* Check to see if he actually has escorts. */
      if (array_size(player.p->escorts) > 0) {
         /* Cycle forward or backwards. */
         if (prev)
            pilot_setTarget( player.p, array_back(player.p->escorts).id );
         else
            pilot_setTarget( player.p, array_front(player.p->escorts).id );
      }
      else
         pilot_setTarget( player.p, player.p->id );
   }

   if (player.p->target != PLAYER_ID) {
      gui_forceBlink();
      player_soundPlayGUI( snd_target, 1 );
   }
   gui_setTarget();
}

/**
 * @brief Player targets nearest pilot.
 */
void player_targetNearest (void)
{
   unsigned int t, dt;
   double d = pilot_getNearestPos( player.p, &dt, player.p->solid.pos.x,
         player.p->solid.pos.y, 1 );
   t = dt;

   /* Disabled ships are typically only valid if within 500 px of the player. */
   if ((d > pow2(500.)) && (pilot_isDisabled( pilot_get(dt) ))) {
      t = pilot_getNearestPilot(player.p);
      /* Try to target a disabled ship if there are no active ships in range. */
      if (t == PLAYER_ID)
         t = dt;
   }

   player_targetSet( t );
}

static int screenshot_cur = 0; /**< Current screenshot at. */
/**
 * @brief Takes a screenshot.
 */
void player_screenshot (void)
{
   char filename[PATH_MAX];

   if (PHYSFS_mkdir("screenshots") == 0) {
      WARN(_("Aborting screenshot"));
      return;
   }

   /* Try to find current screenshots. */
   for ( ; screenshot_cur < 1000; screenshot_cur++) {
      snprintf( filename, sizeof(filename), "screenshots/screenshot%03d.png", screenshot_cur );
      if (!PHYSFS_exists( filename ))
         break;
   }

   if (screenshot_cur >= 999) { /* in case the crap system breaks :) */
      WARN(_("You have reached the maximum amount of screenshots [999]"));
      return;
   }

   /* now proceed to take the screenshot */
   DEBUG( _("Taking screenshot [%03d]..."), screenshot_cur );
   gl_screenshot( filename );
}

/**
 * @brief Checks to see if player is still being hailed and clears hail counters
 *        if he isn't.
 */
static void player_checkHail (void)
{
   Pilot *const* pilot_stack = pilot_getAll();
   for (int i=0; i<array_size(pilot_stack); i++) {
      const Pilot *p = pilot_stack[i];

      /* Must be hailing. */
      if (pilot_isFlag(p, PILOT_HAILING))
         return;
   }

   /* Clear hail timer. */
   player_hailCounter   = 0;
   player_hailTimer     = 0.;
}

/**
 * @brief Displays an out of range message for the player's currently selected spob.
 */
static void player_spobOutOfRangeMsg (void)
{
   const Spob *spob = cur_system->spobs[player.p->nav_spob];
   const char *name = spob_name(spob);
   player_message( _("#r%s is out of comm range, unable to contact."), name );
}

/**
 * @brief Opens communication with the player's target.
 */
void player_hail (void)
{
   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return;

   if (player.p->target != player.p->id)
      comm_openPilot(player.p->target);
   else if (player.p->nav_spob != -1) {
      Spob *spob = cur_system->spobs[ player.p->nav_spob ];
      if (spob_isFlag(spob, SPOB_UNINHABITED))
         player_message( _("#r%s does not respond."), spob_name(spob) );
      else if (pilot_inRangeSpob( player.p, player.p->nav_spob ))
         comm_openSpob( spob );
      else
         player_spobOutOfRangeMsg();
   }
   else
      player_message( "#r%s", _("No target selected to hail.") );

   /* Clear hails if none found. */
   player_checkHail();
}

/**
 * @brief Opens communication with the player's spob target.
 */
void player_hailSpob (void)
{
   /* Not under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   if (player.p->nav_spob != -1) {
      if (pilot_inRangeSpob( player.p, player.p->nav_spob ))
         comm_openSpob( cur_system->spobs[ player.p->nav_spob ] );
      else
         player_spobOutOfRangeMsg();
   }
   else
      player_message( "#r%s", _("No target selected to hail.") );
}

/**
 * @brief Automatically tries to hail a pilot that hailed the player.
 */
void player_autohail (void)
{
   Pilot *const* pilot_stack;

   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return;

   /* Find pilot to autohail. */
   pilot_stack = pilot_getAll();
   for (int i=0; i<array_size(pilot_stack); i++) {
      const Pilot *p = pilot_stack[i];

      /* Must be hailing. */
      if (pilot_isFlag(p, PILOT_HAILING)) {
         /* Try to hail. */
         pilot_setTarget( player.p, p->id );
         gui_setTarget();
         player_hail();

         /* Clear hails if none found. */
         player_checkHail();
         return;
      }
   }

   player_message( "#r%s", _("You haven't been hailed by any pilots.") );
}

/**
 * @brief Toggles mouse flying.
 */
void player_toggleMouseFly (void)
{
   if (!conf.mouse_fly)
      return;

   if (!player_isFlag(PLAYER_MFLY)) {
      input_mouseShow();
      player_message( "#o%s", _("Mouse flying enabled.") );
      player_setFlag(PLAYER_MFLY);
   }
   else {
      input_mouseHide();
      player_rmFlag(PLAYER_MFLY);
      player_message( "#o%s", _("Mouse flying disabled.") );

      if (conf.mouse_accel)
         player_accelOver();
   }
}

/**
 * @brief Starts braking or active cooldown.
 */
void player_cooldownBrake(void)
{
   int stopped;

   if (pilot_isFlag(player.p, PILOT_TAKEOFF))
      return;

   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return;

   stopped = pilot_isStopped(player.p);
   if (stopped && !pilot_isFlag(player.p, PILOT_COOLDOWN))
      pilot_cooldown(player.p, 1);
   else {
      pilot_setFlag(player.p, PILOT_BRAKING);
      pilot_setFlag(player.p, PILOT_COOLDOWN_BRAKE);
   }
}

/**
 * @brief Handles mouse flying based on cursor position.
 *
 *    @param dt Current delta tick.
 *    @return 1 if cursor is outside the dead zone, 0 if it isn't.
 */
static int player_thinkMouseFly( double dt )
{
   double px, py, r, x, y;

   px = player.p->solid.pos.x;
   py = player.p->solid.pos.y;
   gl_screenToGameCoords( &x, &y, player.mousex, player.mousey );
   r = sqrt(pow2(x-px) + pow2(y-py));
   if (r > 50.) { /* Ignore mouse input within a 50 px radius of the centre. */
      pilot_face(player.p, atan2( y-py, x-px), dt);
      if (conf.mouse_accel) { /* Only alter thrust if option is enabled. */
         double acc = CLAMP(0., 1., (r - 100.) / 200.);
         acc = 3. * pow2(acc) - 2. * pow(acc, 3.);
         /* Only accelerate when within 180 degrees of the intended direction. */
         if (ABS(angle_diff(atan2( y - py, x - px), player.p->solid.dir)) < M_PI_2 )
            player_accel(acc);
         else
            player_accel(0.);
      }
      return 1;
   }
   else
      return 0;
}

/**
 * @brief Player got pwned.
 */
void player_dead (void)
{
   /* Explode at normal speed. */
   pause_setSpeed(1.);
   sound_setSpeed(1.);

   /* Close the overlay. */
   ovr_setOpen(0);
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

   /* Set timer for death menu. */
   player_timer = 5.;

   /* Stop sounds. */
   player_soundStop();

   /* Stop autonav */
   player_autonavEnd();

   /* Reset time compression when player dies. */
   pause_setSpeed(1.);
   sound_setSpeed(1.);
}

/**
 * @brief PlayerShip_t compare function for qsort().
 */
static int player_shipsCompare( const void *arg1, const void *arg2 )
{
   PlayerShip_t *ps1, *ps2;
   credits_t p1, p2;

   /* Get the arguments. */
   ps1 = (PlayerShip_t*) arg1;
   ps2 = (PlayerShip_t*) arg2;

   if (ps1->favourite && !ps2->favourite)
      return -1;
   else if (ps2->favourite && !ps1->favourite)
      return +1;

   if (ps1->deployed && !ps2->deployed)
      return -1;
   else if (ps2->deployed && !ps1->deployed)
      return +1;

   /* Get prices. */
   p1 = pilot_worth( ps1->p, 0 );
   p2 = pilot_worth( ps2->p, 0 );

   /* Compare price INVERSELY */
   if (p1 < p2)
      return +1;
   else if (p1 > p2)
      return -1;

   /* In case of tie sort by name so they don't flip or something. */
   return strcmp( ps1->p->name, ps2->p->name );
}

/**
 * @brief Sorts the players ships.
 */
void player_shipsSort (void)
{
   if (array_size(player_stack) == 0)
      return;

   /* Sort. */
   qsort( player_stack, array_size(player_stack), sizeof(PlayerShip_t), player_shipsCompare );
}

/**
 * @brief Returns a buffer with all the player's ships names.
 *
 *    @param sships Fills sships with player_nships ship names.
 *    @param tships Fills sships with player_nships ship target textures.
 *    @return Freshly allocated array with allocated ship names.
 *    @return The number of ships the player has.
 */
int player_ships( char** sships, glTexture** tships )
{
   /* Sort. */
   player_shipsSort();

   /* Create the struct. */
   for (int i=0; i < array_size(player_stack); i++) {
      sships[i] = strdup(player_stack[i].p->name);
      tships[i] = player_stack[i].p->ship->gfx_store;
   }

   return array_size(player_stack);
}

/**
 * @brief Gets the array (array.h) of the player's ships.
 */
const PlayerShip_t* player_getShipStack (void)
{
   return player_stack;
}

/**
 * @brief Gets the amount of ships player has in storage.
 *
 *    @return The number of ships the player has.
 */
int player_nships (void)
{
   return array_size(player_stack);
}

/**
 * @brief Sees if player has a ship of a name.
 *
 *    @param shipname Nome of the ship to get.
 *    @return 1 if ship exists.
 */
int player_hasShip( const char *shipname )
{
   /* Check current ship. */
   if ((player.p != NULL) && (strcmp(player.p->name,shipname)==0))
      return 1;

   /* Check stocked ships. */
   for (int i=0; i < array_size(player_stack); i++)
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
Pilot *player_getShip( const char *shipname )
{
   if ((player.p != NULL) && (strcmp(shipname,player.p->name)==0))
      return player.p;

   for (int i=0; i < array_size(player_stack); i++)
      if (strcmp(player_stack[i].p->name, shipname)==0)
         return player_stack[i].p;

   WARN(_("Player ship '%s' not found in stack"), shipname);
   return NULL;
}

/**
 * @brief Gets a specific ship.
 *
 *    @param shipname Nome of the ship to get.
 *    @return The ship matching name.
 */
PlayerShip_t *player_getPlayerShip( const char *shipname )
{
   if ((player.p != NULL) && (strcmp(shipname,player.p->name)==0))
      return NULL;

   for (int i=0; i < array_size(player_stack); i++)
      if (strcmp(player_stack[i].p->name, shipname)==0)
         return &player_stack[i];

   WARN(_("Player ship '%s' not found in stack"), shipname);
   return NULL;
}

/**
 * @brief Gets how many of the outfit the player owns.
 *
 *    @param o Outfit to check how many the player owns.
 *    @return The number of outfits matching outfitname owned.
 */
int player_outfitOwned( const Outfit* o )
{
   /* Special case map. */
   if ((outfit_isMap(o) && map_isUseless(o)) ||
         (outfit_isLocalMap(o) && localmap_isUseless(o)))
      return 1;

   /* Special case license. */
   if (outfit_isLicense(o) &&
         player_hasLicense(o->u.lic.provides))
      return 1;

   /* Special case GUI. */
   if (outfit_isGUI(o) &&
         player_guiCheck(o->u.gui.gui))
      return 1;

   /* Special case intrinsics. */
   if (o->slot.type==OUTFIT_SLOT_INTRINSIC)
      return pilot_hasIntrinsic( player.p, o );

   /* Try to find it. */
   for (int i=0; i<array_size(player_outfits); i++)
      if (player_outfits[i].o == o)
         return player_outfits[i].q;

   return 0;
}

/**
 * Total number of an outfit owned by the player (including equipped).
 */
int player_outfitOwnedTotal( const Outfit* o )
{
   int q = player_outfitOwned(o);
   q += pilot_numOutfit( player.p, o );
   for (int i=0; i<array_size(player_stack); i++)
      q += pilot_numOutfit( player_stack[i].p, o );

   return q;
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
 * @brief Gets an array (array.h) of the player's outfits.
 */
const PlayerOutfit_t* player_getOutfits (void)
{
   return player_outfits;
}

/**
 * @brief Prepares two arrays for displaying in an image array.
 *
 *    @param[out] outfits Outfits the player owns.
 *    @param[in] filter Function to filter which outfits to get.
 *    @param[in] name Name fragment that each outfit must contain.
 *    @return Number of outfits.
 */
int player_getOutfitsFiltered( const Outfit ***outfits,
      int(*filter)( const Outfit *o ), const char *name )
{
   if (array_size(player_outfits) == 0)
      return 0;

   /* We'll sort. */
   qsort( player_outfits, array_size(player_outfits),
         sizeof(PlayerOutfit_t), player_outfitCompare );

   for (int i=0; i<array_size(player_outfits); i++)
      array_push_back( outfits, (Outfit*)player_outfits[i].o );

   return outfits_filter( *outfits, array_size(player_outfits), filter, name );
}

/**
 * @brief Gets the amount of different outfits in the player outfit stack.
 *
 *    @return Amount of different outfits.
 */
int player_numOutfits (void)
{
   return array_size(player_outfits);
}

/**
 * @brief Adds an outfit to the player outfit stack.
 *
 *    @param o Outfit to add.
 *    @param quantity Amount to add.
 *    @return Amount added.
 */
int player_addOutfit( const Outfit *o, int quantity )
{
   PlayerOutfit_t *po;

   /* Validity check. */
   if (quantity == 0)
      return 0;

   /* Don't readd uniques. */
   if (outfit_isProp(o,OUTFIT_PROP_UNIQUE) && (player_outfitOwned(o)>0))
      return 0;

   /* special case if it's a map */
   if (outfit_isMap(o)) {
      map_map(o);
      return 1; /* Success. */
   }
   else if (outfit_isLocalMap(o)) {
      localmap_map(o);
      return 1;
   }
   /* special case if it's an outfit */
   else if (outfit_isGUI(o)) {
      player_guiAdd(o->u.gui.gui);
      return 1; /* Success. */
   }
   /* special case if it's a license. */
   else if (outfit_isLicense(o)) {
      player_addLicense(o->u.lic.provides);
      return 1; /* Success. */
   }
   /* intrinsic outfits get added as intinsics. */
   else if (o->slot.type==OUTFIT_SLOT_INTRINSIC) {
      if (pilot_hasOutfitLimit( player.p, o->limit ))
         return 0;
      return pilot_addOutfitIntrinsic( player.p, o );
   }

   /* Try to find it. */
   for (int i=0; i<array_size(player_outfits); i++) {
      if (player_outfits[i].o == o) {
         player_outfits[i].q  += quantity;
         return quantity;
      }
   }

   /* Allocate if needed. */
   po = &array_grow( &player_outfits );

   /* Add the outfit. */
   po->o = o;
   po->q = quantity;
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
   if (o->slot.type==OUTFIT_SLOT_INTRINSIC)
      return pilot_rmOutfitIntrinsic( player.p, o );

   /* Try to find it. */
   for (int i=0; i<array_size(player_outfits); i++) {
      if (player_outfits[i].o != o)
         continue;
      /* See how many to remove. */
      int q = MIN( player_outfits[i].q, quantity );
      player_outfits[i].q -= q;

      /* See if must remove element. */
      if (player_outfits[i].q <= 0)
         array_erase( &player_outfits, &player_outfits[i], &player_outfits[i+1] );

      /* Return removed outfits. */
      return q;
   }

   /* Nothing removed. */
   return 0;
}

/*
 * Trivial sorting function for arrays of integers.
 */
static int cmp_int( const void *p1, const void *p2 )
{
   const int *i1 = (const int*) p1;
   const int *i2 = (const int*) p2;
   return (*i1) - (*i2);
}

/**
 * @brief Marks a mission as completed.

 *    @param id ID of the mission to mark as completed.
 */
void player_missionFinished( int id )
{
   HookParam h[2];
   const MissionData *m;

   /* Make sure not already marked. */
   if (player_missionAlreadyDone(id))
      return;

   /* Mark as done. */
   if (missions_done == NULL)
      missions_done = array_create( int );
   array_push_back( &missions_done, id );

   qsort( missions_done, array_size(missions_done), sizeof(int), cmp_int );

   /* Run the completion hook. */
   m = mission_get( id );
   mission_toLuaTable( naevL, m ); /* Push to stack. */
   h[0].type = HOOK_PARAM_REF;
   h[0].u.ref = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* Pops from stack. */
   h[1].type = HOOK_PARAM_SENTINEL;
   hooks_runParam( "mission_done", h );
}

/**
 * @brief Checks to see if player has already completed a mission.
 *
 *    @param id ID of the mission to see if player has completed.
 *    @return 1 if player has completed the mission, 0 otherwise.
 */
int player_missionAlreadyDone( int id )
{
   if (missions_done == NULL)
      return 0;

   const int *i = bsearch( &id, missions_done, array_size(missions_done), sizeof(int), cmp_int );
   return i!=NULL;
}

/**
 * @brief Gets a list of all the missions the player has done.
 *
 *    @return Array (array.c) of all the missions the player has done. Do not free!
 */
int* player_missionsDoneList (void)
{
   return missions_done;
}

/**
 * @brief Marks a event as completed.
 *
 *    @param id ID of the event to mark as completed.
 */
void player_eventFinished( int id )
{
   HookParam h[2];

   /* Make sure not already done. */
   if (player_eventAlreadyDone(id))
      return;

   /* Mark as done. */
   if (events_done == NULL)
      events_done = array_create( int );
   array_push_back( &events_done, id );

   qsort( events_done, array_size(events_done), sizeof(int), cmp_int );

   /* Run the completion hook. */
   event_toLuaTable( naevL, id ); /* Push to stack. */
   h[0].type = HOOK_PARAM_REF;
   h[0].u.ref = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* Pops from stack. */
   h[1].type = HOOK_PARAM_SENTINEL;
   hooks_runParam( "event_done", h );
}

/**
 * @brief Checks to see if player has already completed a event.
 *
 *    @param id ID of the event to see if player has completed.
 *    @return 1 if player has completed the event, 0 otherwise.
 */
int player_eventAlreadyDone( int id )
{
   if (events_done == NULL)
      return 0;

   const int *i = bsearch( &id, events_done, array_size(events_done), sizeof(int), cmp_int );
   return i!=NULL;
}

/**
 * @brief Gets a list of all the events the player has done.
 *
 *    @return Array (array.c) of all the events the player has done. Do not free!
 */
int* player_eventsDoneList (void)
{
   return events_done;
}

/**
 * @brief Checks to see if player has license.
 *
 *    @param license License to check to see if the player has.
 *    @return 1 if has license (or none needed), 0 if doesn't.
 */
int player_hasLicense( const char *license )
{
   if (license == NULL)
      return 1;
   if (player_licenses == NULL)
      return 0;

   const char *s = bsearch( &license, player_licenses, array_size(player_licenses), sizeof(char*), strsort );
   return s!=NULL;
}

/**
 * @brief Gives the player a license.
 *
 *    @brief license License to give the player.
 */
void player_addLicense( const char *license )
{
   if (player_hasLicense(license))
      return;
   if (player_licenses == NULL)
      player_licenses = array_create( char* );
   array_push_back( &player_licenses, strdup(license) );

   qsort( player_licenses, array_size(player_licenses), sizeof(char*), strsort );
}

/**
 * @brief Gets the array (array.h) of license names in the player's inventory.
 */
const char **player_getLicenses ()
{
   return (const char**) player_licenses;
}

/**
 * @brief Runs hooks for the player.
 */
void player_runHooks (void)
{
   if (player_isFlag( PLAYER_HOOK_HYPER )) {
      player_rmFlag( PLAYER_HOOK_HYPER );
      player_brokeHyperspace();
   }
   if (player_isFlag( PLAYER_HOOK_JUMPIN)) {
      player_rmFlag( PLAYER_HOOK_JUMPIN );
      pilot_outfitLOnjumpin( player.p );
      hooks_run( "jumpin" );
      hooks_run( "enter" );
      events_trigger( EVENT_TRIGGER_ENTER );
      missions_run( MIS_AVAIL_ENTER, -1, NULL, NULL );
   }
   if (player_isFlag( PLAYER_HOOK_LAND )) {
      player_rmFlag( PLAYER_HOOK_LAND );
      if (player.p->nav_spob >= 0)
         land( cur_system->spobs[ player.p->nav_spob ], 0 );
   }
}

/**
 * @brief Clears escorts to make sure deployment is safe.
 */
static void player_clearEscorts (void)
{
   for (int i=0; i<array_size(player.p->outfits); i++) {
      if (player.p->outfits[i]->outfit == NULL)
         continue;

      if (outfit_isFighterBay(player.p->outfits[i]->outfit))
         player.p->outfits[i]->u.ammo.deployed = 0;
   }
}

/**
 * @brief Adds the player's escorts.
 *
 *    @return 0 on success.
 */
int player_addEscorts (void)
{
   /* Clear escorts first. */
   player_clearEscorts();

   /* Go over escorts. */
   for (int i=0; i<array_size(player.p->escorts); i++) {
      int q;
      PilotOutfitSlot *po;
      const Escort_t *e = &player.p->escorts[i];
      Pilot *pe = pilot_get( e->id );

      /* Non-persistent pilots should have been wiped already. */
      if (pe == NULL) {
         escort_rmListIndex(player.p, i);
         i--;
         continue;
      }

      /* Update to random position. */
      pe->solid.dir = RNGF() * 2. * M_PI;
      vec2_cset( &pe->solid.pos, player.p->solid.pos.x + 50.*cos(pe->solid.dir),
            player.p->solid.pos.y + 50.*sin(pe->solid.dir) );
      vec2_cset( &pe->solid.vel, 0., 0. );

      /* Update outfit if needed. */
      if (e->type != ESCORT_TYPE_BAY)
         continue;

      po = pilot_getDockSlot( pe );
      if (po == NULL) {
         /* We just want to delete the pilot and not trigger other stuff. */
         pilot_setFlag( pe, PILOT_DELETE );
         WARN(_("Escort is undeployed, removing."));
         escort_rmListIndex(player.p, i);
         i--;
         continue;
      }

      po->u.ammo.deployed++;
      q = po->u.ammo.deployed + po->u.ammo.quantity;
      if (q > pilot_maxAmmoO(player.p,po->outfit)) {
         /* We just want to delete the pilot and not trigger other stuff. */
         pilot_setFlag( pe, PILOT_DELETE );
         WARN(_("Escort is deployed past outfit limits, removing."));
         escort_rmListIndex(player.p, i);
         i--;
         continue;
      }
   }

   /* Add the player fleets. */
   for (int i=0; i<array_size(player_stack); i++) {
      PlayerShip_t *ps = &player_stack[i];

      /* Already exists. */
      if (ps->p->id)
         continue;

      /* Only deploy escorts that are deployed. */
      if (!ps->deployed)
         continue;

      /* Only deploy spaceworthy escorts. */
      if (!pilot_isSpaceworthy(ps->p))
         continue;

      pfleet_deploy( ps );
   }

   return 0;
}

/**
 * @brief Saves the player's escorts.
 */
static int player_saveEscorts( xmlTextWriterPtr writer )
{
   for (int i=0; i<array_size(player.p->escorts); i++) {
      Escort_t *e = &player.p->escorts[i];
      Pilot *pe;
      if (!e->persist)
         continue;
      switch (e->type) {
         case ESCORT_TYPE_BAY:
            xmlw_startElem(writer, "escort");
            xmlw_attr(writer,"type","bay");
            xmlw_attr(writer, "name", "%s", e->ship->name);
            xmlw_endElem(writer); /* "escort" */
            break;

         case ESCORT_TYPE_FLEET:
            pe = pilot_get( e->id );
            if (pe != NULL) {
               xmlw_startElem(writer, "escort");
               xmlw_attr(writer,"type","fleet");
               xmlw_attr(writer, "name", "%s", pe->name);
               xmlw_endElem(writer); /* "escort" */
            }
            break;

         default:
            break;
      }
   }

   return 0;
}

/**
 * @brief Save the freaking player in a freaking xmlfile.
 *
 *    @param writer xml Writer to use.
 *    @return 0 on success.
 */
int player_save( xmlTextWriterPtr writer )
{
   const char **guis;
   int cycles, periods, seconds;
   double rem;
   const PlayerItem *inventory;

   xmlw_startElem(writer,"player");

   /* Standard player details. */
   xmlw_attr(writer,"name","%s",player.name);
   xmlw_elem(writer,"credits","%"CREDITS_PRI,player.p->credits);
   xmlw_elem(writer,"chapter","%s",player.chapter);
   if (player.difficulty != NULL)
      xmlw_elem(writer,"difficulty","%s",player.difficulty);
   if (player.gui != NULL)
      xmlw_elem(writer,"gui","%s",player.gui);
   xmlw_elem(writer,"mapOverlay","%d",ovr_isOpen());
   gui_radarGetRes( &player.radar_res );
   xmlw_elem(writer,"radar_res","%f",player.radar_res);
   xmlw_elem(writer,"eq_outfitMode","%d",player.eq_outfitMode);
   xmlw_elem(writer,"map_minimal","%d",player.map_minimal);
   xmlw_elem(writer,"fleet_capacity","%d",player.fleet_capacity);

   /* Time. */
   xmlw_startElem(writer,"time");
   ntime_getR( &cycles, &periods, &seconds, &rem );
   xmlw_elem(writer,"SCU","%d", cycles);
   xmlw_elem(writer,"STP","%d", periods);
   xmlw_elem(writer,"STU","%d", seconds);
   xmlw_elem(writer,"Remainder","%lf", rem);
   xmlw_endElem(writer); /* "time" */

   /* Current ship. */
   xmlw_elem(writer, "location", "%s", land_spob->name);
   player_saveShip( writer, &player.ps ); /* current ship */

   /* Ships. */
   xmlw_startElem(writer,"ships");
   for (int i=0; i<array_size(player_stack); i++)
      player_saveShip( writer, &player_stack[i] );
   xmlw_endElem(writer); /* "ships" */

   /* GUIs. */
   xmlw_startElem(writer,"guis");
   guis = player_guiList();
   for (int i=0; i<array_size(guis); i++)
      xmlw_elem(writer,"gui","%s",guis[i]);
   xmlw_endElem(writer); /* "guis" */

   /* Outfits. */
   xmlw_startElem(writer,"outfits");
   for (int i=0; i<array_size(player_outfits); i++) {
      xmlw_startElem(writer, "outfit");
      xmlw_attr(writer, "quantity", "%d", player_outfits[i].q);
      xmlw_str(writer, "%s", player_outfits[i].o->name);
      xmlw_endElem(writer); /* "outfit" */
   }
   xmlw_endElem(writer); /* "outfits" */

   /* Licenses. */
   xmlw_startElem(writer, "licenses");
   for (int i=0; i<array_size(player_licenses); i++)
      xmlw_elem(writer, "license", "%s", player_licenses[i]);
   xmlw_endElem(writer); /* "licenses" */

   /* Inventory. */
   xmlw_startElem(writer, "inventory");
   inventory = player_inventory();
   for (int i=0; i<array_size(inventory); i++) {
      const PlayerItem *pi = &inventory[i];
      xmlw_startElem(writer, "item");
      xmlw_attr(writer, "quantity", "%d", pi->quantity);
      xmlw_str(writer, "%s", pi->name);
      xmlw_endElem(writer); /* "item" */
   }
   xmlw_endElem(writer); /* "inventory" */

   xmlw_endElem(writer); /* "player" */

   /* Mission the player has done. */
   xmlw_startElem(writer,"missions_done");
   for (int i=0; i<array_size(missions_done); i++) {
      const MissionData *m = mission_get(missions_done[i]);
      if (m != NULL) /* In case mission name changes between versions */
         xmlw_elem(writer, "done", "%s", m->name);
   }
   xmlw_endElem(writer); /* "missions_done" */

   /* Events the player has done. */
   xmlw_startElem(writer, "events_done");
   for (int i=0; i<array_size(events_done); i++) {
      const char *ev = event_dataName(events_done[i]);
      if (ev != NULL) /* In case mission name changes between versions */
         xmlw_elem(writer, "done", "%s", ev);
   }
   xmlw_endElem(writer); /* "events_done" */

   /* Escorts. */
   xmlw_startElem(writer, "escorts");
   player_saveEscorts(writer);
   xmlw_endElem(writer); /* "escorts" */

   /* Metadata. */
   xmlw_startElem(writer,"metadata");
   player_saveMetadata( writer );
   xmlw_endElem(writer); /* "metadata" */

   return 0;
}

/**
 * @brief Saves an outfit slot.
 */
static int player_saveShipSlot( xmlTextWriterPtr writer, const PilotOutfitSlot *slot, int i )
{
   const Outfit *o = slot->outfit;
   xmlw_startElem(writer,"outfit");
   xmlw_attr(writer,"slot","%d",i);
   if (outfit_isLauncher(o) || outfit_isFighterBay(o))
      xmlw_attr(writer,"quantity","%d", slot->u.ammo.quantity);
   xmlw_str(writer,"%s",o->name);
   xmlw_endElem(writer); /* "outfit" */

   return 0;
}

/**
 * @brief Saves a ship.
 *
 *    @param writer XML writer.
 *    @param pship Ship to save.
 *    @return 0 on success.
 */
static int player_saveShip( xmlTextWriterPtr writer, PlayerShip_t *pship )
{
   Pilot *ship = pship->p;
   xmlw_startElem(writer,"ship");
   xmlw_attr(writer,"name","%s",ship->name);
   xmlw_attr(writer,"model","%s",ship->ship->name);
   xmlw_attr(writer,"favourite", "%d",pship->favourite);
   xmlw_attr(writer,"deployed", "%d",pship->deployed);

   /* Metadata. */
   if (pship->acquired)
      xmlw_elem(writer, "acquired","%s", pship->acquired);
   xmlw_saveTime(writer, "acquired_date", pship->acquired_date);
   xmlw_elem(writer, "time_played","%f", pship->time_played);
   xmlw_elem(writer, "dmg_done_shield", "%f", pship->dmg_done_shield);
   xmlw_elem(writer, "dmg_done_armour", "%f", pship->dmg_done_armour);
   xmlw_elem(writer, "dmg_taken_shield", "%f", pship->dmg_taken_shield);
   xmlw_elem(writer, "dmg_taken_armour", "%f", pship->dmg_taken_armour);
   xmlw_elem(writer, "jumped_times", "%u", pship->jumped_times);
   xmlw_elem(writer, "landed_times", "%u", pship->landed_times);
   xmlw_elem(writer, "death_counter", "%u", pship->death_counter);

   /* Ships destroyed by class. */
   xmlw_startElem(writer,"ships_destroyed");
   for (int i=SHIP_CLASS_NULL+1; i<SHIP_CLASS_TOTAL; i++) {
      char buf[STRMAX_SHORT];
      strncpy( buf, ship_classToString(i), sizeof(buf)-1 );
      for (size_t j=0; j<strlen(buf); j++)
         if (buf[j]==' ')
            buf[j]='_';
      xmlw_elem(writer, buf, "%u", pship->ships_destroyed[i]);
   }
   xmlw_endElem(writer); /* "ships_destroyed" */

   /* save the fuel */
   xmlw_elem(writer,"fuel","%f",ship->fuel);

   /* save the outfits */
   xmlw_startElem(writer,"outfits_intrinsic"); /* Want them to be first. */
   for (int i=0; i<array_size(ship->outfit_intrinsic); i++)
      player_saveShipSlot( writer, &ship->outfit_intrinsic[i], i );
   xmlw_endElem(writer); /* "outfits_intrinsic" */
   xmlw_startElem(writer,"outfits_structure");
   for (int i=0; i<array_size(ship->outfit_structure); i++) {
      if (ship->outfit_structure[i].outfit==NULL)
         continue;
      player_saveShipSlot( writer, &ship->outfit_structure[i], i );
   }
   xmlw_endElem(writer); /* "outfits_structure" */
   xmlw_startElem(writer,"outfits_utility");
   for (int i=0; i<array_size(ship->outfit_utility); i++) {
      if (ship->outfit_utility[i].outfit==NULL)
         continue;
      player_saveShipSlot( writer, &ship->outfit_utility[i], i );
   }
   xmlw_endElem(writer); /* "outfits_utility" */
   xmlw_startElem(writer,"outfits_weapon");
   for (int i=0; i<array_size(ship->outfit_weapon); i++) {
      if (ship->outfit_weapon[i].outfit==NULL)
         continue;
      player_saveShipSlot( writer, &ship->outfit_weapon[i], i );
   }
   xmlw_endElem(writer); /* "outfits_weapon" */

   /* save the commodities */
   xmlw_startElem(writer,"commodities");
   for (int i=0; i<array_size(ship->commodities); i++) {
      PilotCommodity *pc = &ship->commodities[i];
      /* Remove cargo with id and no mission. */
      if (pc->id > 0) {
         int found = 0;
         for (int j=0; j<array_size(player_missions); j++) {
            /* Only check active missions. */
            if (player_missions[j]->id > 0) {
               /* Now check if it's in the cargo list. */
               for (int k=0; k<array_size(player_missions[j]->cargo); k++) {
                  /* See if it matches a cargo. */
                  if (player_missions[j]->cargo[k] == pc->id) {
                     found = 1;
                     break;
                  }
               }
            }
            if (found)
               break;
         }

         if (!found) {
            WARN(_("Found mission cargo '%s' without associated mission."),pc->commodity->name);
            WARN(_("Please reload save game to remove the dead cargo."));
            continue;
         }
      }
      else if (pc->quantity==0) {
         WARN(_("Found cargo '%s' with 0 quantity."),pc->commodity->name);
         WARN(_("Please reload save game to remove the dead cargo."));
         continue;
      }

      xmlw_startElem(writer,"commodity");

      xmlw_attr(writer,"quantity","%d",pc->quantity);
      if (pc->id > 0)
         xmlw_attr(writer,"id","%d",pc->id);
      xmlw_str(writer,"%s",pc->commodity->name);

      xmlw_endElem(writer); /* commodity */
   }
   xmlw_endElem(writer); /* "commodities" */

   xmlw_startElem(writer, "weaponsets");
   xmlw_attr(writer, "autoweap", "%d", ship->autoweap);
   xmlw_attr(writer, "active_set", "%d", ship->active_set);
   xmlw_attr(writer, "aim_lines", "%d", ship->aimLines);
   for (int i=0; i<PILOT_WEAPON_SETS; i++) {
      PilotWeaponSet *ws = &pship->weapon_sets[i];
      PilotWeaponSetOutfit *weaps = ws->slots;
      xmlw_startElem(writer,"weaponset");
      /* Inrange isn't handled by autoweap for the player. */
      xmlw_attr(writer,"inrange","%d",ws->inrange);
      xmlw_attr(writer,"manual","%d",ws->manual);
      xmlw_attr(writer,"volley","%d",ws->volley);
      xmlw_attr(writer,"id","%d",i);
      if (!ship->autoweap) {
         xmlw_attr(writer,"type","%d",ws->type);
         for (int j=0; j<array_size(weaps); j++) {
            xmlw_startElem(writer,"weapon");
            xmlw_attr(writer,"level","%d",weaps[j].level);
            xmlw_str(writer,"%d",weaps[j].slotid);
            xmlw_endElem(writer); /* "weapon" */
         }
      }
      xmlw_endElem(writer); /* "weaponset" */
   }
   xmlw_endElem(writer); /* "weaponsets" */

   /* Ship variables. */
   xmlw_startElem(writer, "vars");
   lvar_save( pship->p->shipvar, writer );
   xmlw_endElem(writer); /* "vars" */

   xmlw_endElem(writer); /* "ship" */

   return 0;
}

/**
 * @brief Saves the player meta-data.
 *
 *    @param writer XML writer.
 *    @return 0 on success.
 */
static int player_saveMetadata( xmlTextWriterPtr writer )
{
   time_t t = time(NULL);
   double diff = difftime( t, player.time_since_save );

   /* Compute elapsed time. */
   player.time_played += diff;
   player.ps.time_played += diff;
   player.time_since_save = t;

   /* Save the stuff. */
   xmlw_saveTime(writer, "last_played", time(NULL));
   xmlw_saveTime(writer, "date_created", player.date_created);

   /* Meta-data. */
   xmlw_elem(writer, "time_played","%f", player.time_played);
   xmlw_elem(writer, "dmg_done_shield", "%f", player.dmg_done_shield);
   xmlw_elem(writer, "dmg_done_armour", "%f", player.dmg_done_armour);
   xmlw_elem(writer, "dmg_taken_shield", "%f", player.dmg_taken_shield);
   xmlw_elem(writer, "dmg_taken_armour", "%f", player.dmg_taken_armour);
   xmlw_elem(writer, "jumped_times", "%u", player.jumped_times);
   xmlw_elem(writer, "landed_times", "%u", player.landed_times);
   xmlw_elem(writer, "death_counter", "%u", player.death_counter);

   /* Ships destroyed by class. */
   xmlw_startElem(writer,"ships_destroyed");
   for (int i=SHIP_CLASS_NULL+1; i<SHIP_CLASS_TOTAL; i++) {
      char buf[STRMAX_SHORT];
      strncpy( buf, ship_classToString(i), sizeof(buf)-1 );
      for (size_t j=0; j<strlen(buf); j++)
         if (buf[j]==' ')
            buf[j]='_';
      xmlw_elem(writer, buf, "%u", player.ships_destroyed[i]);
   }
   xmlw_endElem(writer); /* "ships_destroyed" */

   return 0;
}

/**
 * @brief Loads the player stuff.
 *
 *    @param parent Node where the player stuff is to be found.
 *    @return 0 on success.
 */
Spob* player_load( xmlNodePtr parent )
{
   xmlNodePtr node;
   Spob *pnt;

   /* some cleaning up */
   memset( &player, 0, sizeof(Player_t) );
   player.speed = conf.game_speed;
   pnt = NULL;
   map_cleanup();

   /* Sane time defaults. */
   player.last_played = time(NULL);
   player.date_created = player.last_played;
   player.time_since_save = player.last_played;

   if (player_stack==NULL)
      player_stack = array_create( PlayerShip_t );
   if (player_outfits==NULL)
      player_outfits = array_create( PlayerOutfit_t );

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"metadata"))
         player_parseMetadata(node);
      else if (xml_isNode(node,"player"))
         pnt = player_parse( node );
      else if (xml_isNode(node,"missions_done"))
         player_parseDoneMissions( node );
      else if (xml_isNode(node,"events_done"))
         player_parseDoneEvents( node );
      else if (xml_isNode(node,"escorts"))
         player_parseEscorts(node);
   } while (xml_nextNode(node));

   /* Set up meta-data. */
   player.time_since_save = time(NULL);

   /* Defaults as necessary. */
   if (player.chapter==NULL)
      player.chapter = strdup( start_chapter() );
   if (player.difficulty!=NULL)
      difficulty_setLocal( difficulty_get(player.difficulty) );
   else
      difficulty_setLocal( NULL ); /* Sets the default. */

   /* Updates the fleet internals. */
   pfleet_update();

   /* Update weapon set. */
   pilot_weaponSetDefault( player.p );

   return pnt;
}

/**
 * @brief Runs the save updater script, leaving any result on the stack of naevL.
 *
 *    @param type Type of item to translate. Currently "outfit" and "license" are supported.
 *    @param name Name of the inventory item.
 *    @param q Quantity in possession.
 *    @return Stack depth: 1 if player got a translated item back, 0 if they got nothing or just money.
 */
static int player_runUpdaterScript( const char* type, const char* name, int q )
{
   static nlua_env player_updater_env = LUA_NOREF;

   player_ran_updater = 1;

   /* Load env if necessary. */
   if (player_updater_env == LUA_NOREF) {
      player_updater_env = nlua_newEnv();
      size_t bufsize;
      char *buf = ndata_read( SAVE_UPDATER_PATH, &bufsize );
      if (nlua_dobufenv(player_updater_env, buf, bufsize, SAVE_UPDATER_PATH) != 0) {
         WARN( _("Error loading file: %s\n"
            "%s\n"
            "Most likely Lua file has improper syntax, please check"),
               SAVE_UPDATER_PATH, lua_tostring(naevL,-1));
         free(buf);
         return 0;
      }
      free(buf);
   }

   /* Try to find out equivalent. */
   nlua_getenv( naevL, player_updater_env, type );
   lua_pushstring( naevL, name );
   if (nlua_pcall(player_updater_env, 1, 1)) { /* error has occurred */
      WARN( _("Board: '%s'"), lua_tostring(naevL,-1));
      lua_pop(naevL,1);
      return 0;
   }
   if (lua_type(naevL,-1) == LUA_TNUMBER) {
      player_payback += q * round( lua_tonumber(naevL,-1) );
      lua_pop(naevL,1);
      return 0;
   }

   return 1;
}

/**
 * @brief Tries to get an outfit for the player or looks for equivalents.
 */
static const Outfit* player_tryGetOutfit( const char *name, int q )
{
   const Outfit *o = outfit_getW( name );

   /* Outfit was found normally. */
   if (o != NULL)
      return o;
   player_ran_updater = 1;

   /* Try to find out equivalent. */
   if (player_runUpdaterScript( "outfit", name, q ) == 0)
      return NULL;
   else if (lua_type(naevL,-1) == LUA_TSTRING)
      o = outfit_get( lua_tostring(naevL,-1) );
   else if (lua_isoutfit(naevL,-1))
      o = lua_tooutfit(naevL,-1);
   else
      WARN(_("Outfit '%s' in player save not found!"), name );

   lua_pop(naevL,1);

   return o;
}

/**
 * @brief Tries to get an ship for the player or looks for equivalents.
 */
static const Ship* player_tryGetShip( const char *name )
{
   const Ship *s = ship_getW( name );

   /* Ship was found normally. */
   if (s != NULL)
      return s;
   player_ran_updater = 1;

   /* Try to find out equivalent. */
   if (player_runUpdaterScript( "ship", name, 1 ) == 0)
      return NULL;
   else if (lua_type(naevL,-1) == LUA_TSTRING)
      s = ship_get( lua_tostring(naevL,-1) );
   else if (lua_isship(naevL,-1))
      s = lua_toship(naevL,-1);
   else
      WARN(_("Ship '%s' in player save not found!"), name );

   lua_pop(naevL,1);

   return s;
}

/**
 * @brief Tries to get an outfit for the player or looks for equivalents.
 */
static void player_tryAddLicense( const char *name )
{
   /* Found normally. */
   if (outfit_licenseExists(name)) {
      player_addLicense( name );
      return;
   }
   player_ran_updater = 1;

   /* Try to find out equivalent. */
   if (player_runUpdaterScript( "license", name, 1 ) == 0)
      return;
   else if (lua_type(naevL,-1) == LUA_TSTRING)
      player_addLicense( lua_tostring(naevL,-1) );
   else
      WARN(_("Saved license does not exist and could not be found or updated: '%s'!"), name);
   lua_pop(naevL,1);
}

/**
 * @brief Parses the player node.
 *
 *    @param parent The player node.
 *    @return Spob to start on on success.
 */
static Spob* player_parse( xmlNodePtr parent )
{
   const char *spob = NULL;
   Spob *pnt = NULL;
   xmlNodePtr node, cur;
   int map_overlay_enabled = 0;
   StarSystem *sys;
   double a, r;
   int time_set = 0;

   xmlr_attr_strd(parent, "name", player.name);
   assert( player.p == NULL );
   player_ran_updater = 0;

   player.radar_res = RADAR_RES_DEFAULT;

   /* Must get spob first. */
   node = parent->xmlChildrenNode;
   do {
      xmlr_str(node,"location",spob);
   } while (xml_nextNode(node));

   /* Parse rest. */
   node = parent->xmlChildrenNode;
   do {
      /* global stuff */
      xmlr_ulong(node, "credits", player_creds);
      xmlr_strd(node, "gui", player.gui);
      xmlr_strd(node, "chapter", player.chapter);
      xmlr_int(node, "mapOverlay", map_overlay_enabled);
      xmlr_float(node, "radar_res", player.radar_res);
      xmlr_int(node, "eq_outfitMode", player.eq_outfitMode);
      xmlr_int(node, "map_minimal", player.map_minimal);
      xmlr_int(node, "fleet_capacity", player.fleet_capacity);

      /* Time. */
      if (xml_isNode(node,"time")) {
         double rem = -1.;
         int cycles=-1, periods=-1, seconds=-1;
         cur = node->xmlChildrenNode;
         do {
            xmlr_int(cur, "SCU", cycles);
            xmlr_int(cur, "STP", periods);
            xmlr_int(cur, "STU", seconds);
            xmlr_float(cur, "Remainder", rem);
         } while (xml_nextNode(cur));
         if ((cycles < 0) || (periods < 0) || (seconds < 0) || (rem<0.))
            WARN(_("Malformed time in save game!"));
         ntime_setR( cycles, periods, seconds, rem );
         if ((cycles >= 0) || (periods >= 0) || (seconds >= 0))
            time_set = 1;
      }

      if (xml_isNode(node, "ship"))
         player_parseShip(node, 1);

      /* Parse ships. */
      else if (xml_isNode(node,"ships")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"ship"))
               player_parseShip(cur, 0);
         } while (xml_nextNode(cur));
      }

      /* Parse GUIs. */
      else if (xml_isNode(node,"guis")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"gui"))
               player_guiAdd( xml_get(cur) );
         } while (xml_nextNode(cur));
      }

      /* Parse outfits. */
      else if (xml_isNode(node,"outfits")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"outfit")) {
               int q;
               const Outfit *o;
               const char *oname = xml_get(cur);
               xmlr_attr_float( cur, "quantity", q );
               if (q == 0) {
                  WARN(_("Outfit '%s' was saved without quantity!"), (oname!=NULL) ? oname : "NULL" );
                  continue;
               }

               o = player_tryGetOutfit( oname, q );
               if (o == NULL)
                  continue;

               player_addOutfit( o, q );
            }
         } while (xml_nextNode(cur));
      }

      /* Parse licenses. */
      else if (xml_isNode(node,"licenses"))
         player_parseLicenses(node);

      else if (xml_isNode(node,"inventory"))
         player_parseInventory(node);

   } while (xml_nextNode(node));

   /* Handle cases where ship is missing. */
   if (player.p == NULL) {
      PilotFlags flags;
      pilot_clearFlagsRaw( flags );
      pilot_setFlagRaw( flags, PILOT_PLAYER );
      pilot_setFlagRaw( flags, PILOT_NO_OUTFITS );
      WARN(_("Player ship does not exist!"));

      if (array_size(player_stack) == 0) {
         WARN(_("Player has no other ships, giving starting ship."));
         pilot_create( ship_get(start_ship()), "MIA",
               faction_get("Player"), "player", 0., NULL, NULL, flags, 0, 0 );
      }
      else {
         /* Just give player.p a random ship in the stack. */
         const Pilot *old_ship = player_stack[array_size(player_stack)-1].p;
         pilot_create( old_ship->ship, old_ship->name,
               faction_get("Player"), "player", 0., NULL, NULL, flags, 0, 0 );
         player_rmShip( old_ship->name );
         WARN(_("Giving player ship '%s'."), player.p->name );
      }
   }

   /* Check. */
   if (player.p == NULL) {
      ERR(_("Something went horribly wrong, player does not exist after load..."));
      return NULL;
   }

   /* Reset player speed */
   player.speed = conf.game_speed;

   /* set global thingies */
   player.p->credits = player_creds + player_payback;
   if (!time_set) {
      WARN(_("Save has no time information, setting to start information."));
      ntime_set( start_date() );
   }

   /* Updater message. */
   if (player_ran_updater) {
      DEBUG(_("Player save was updated!"));
      dialogue_msg(_("Save Game Updated"),_("The loaded save games has had outfits and ships updated to the current Naev version. You will find that some outfits and ships you have had have been changed. In the case no equivalent outfit or ship was found, you have been refunded the cost in credits."));
   }

   /* set player in system */
   pnt = spob_get( spob );
   /* Get random spob if it's NULL. */
   if ((pnt == NULL) || (spob_getSystem(spob) == NULL) ||
         !spob_hasService(pnt, SPOB_SERVICE_LAND)) {
      WARN(_("Player starts out in non-existent or invalid spob '%s',"
            "trying to find a suitable one instead."),
            spob );

      /* Find a landable, inhabited spob that's in a system, offers refueling
       * and meets the following additional criteria:
       *
       *    0: Shipyard, outfitter, non-hostile
       *    1: Outfitter, non-hostile
       *    2: None
       *
       * If no spob meeting the current criteria can be found, the next
       * set of criteria is tried until none remain.
       */
      const char *found = NULL;
      for (int i=0; i<3; i++) {
         unsigned int services = SPOB_SERVICE_LAND | SPOB_SERVICE_INHABITED | SPOB_SERVICE_REFUEL;

         if (i == 0)
            services |= SPOB_SERVICE_SHIPYARD;

         if (i != 2)
            services |= SPOB_SERVICE_OUTFITS;

         found = space_getRndSpob( 1, services,
               (i != 2) ? player_filterSuitableSpob : NULL );
         if (found != NULL)
            break;

         WARN(_("Could not find a spob satisfying criteria %d."), i);
      }

      if (found == NULL) {
         WARN(_("Could not find a suitable spob. Choosing a random spob."));
         found = space_getRndSpob(0, 0, NULL); /* This should never, ever fail. */
      }
      pnt = spob_get( found );
   }

   /* Initialize system. */
   sys = system_get( spob_getSystem( pnt->name ) );
   space_gfxLoad( sys );
   a = RNGF() * 2.*M_PI;
   r = RNGF() * pnt->radius * 0.8;
   player_warp( pnt->pos.x + r*cos(a), pnt->pos.y + r*sin(a) );
   player.p->solid.dir = RNG(0,359) * M_PI/180.;

   /* Initialize outfits. */
   pilot_outfitLInitAll( player.p );

   /* initialize the system */
   space_init( sys->name, 0 );
   map_clear(); /* sets the map up */
   ovr_setOpen(map_overlay_enabled);

   /* initialize the sound */
   player_initSound();

   return pnt;
}

/**
 * @brief Filter function for space_getRndSpob
 *
 *    @param p Spob.
 *    @return Whether the spob is suitable for teleporting to.
 */
static int player_filterSuitableSpob( Spob *p )
{
   return !areEnemies(p->presence.faction, FACTION_PLAYER);
}

/**
 * @brief Parses player's done missions.
 *
 *    @param parent Node of the missions.
 *    @return 0 on success.
 */
static int player_parseDoneMissions( xmlNodePtr parent )
{
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      if (!xml_isNode(node,"done"))
         continue;

      int id = mission_getID( xml_get(node) );
      if (id < 0)
         DEBUG(_("Mission '%s' doesn't seem to exist anymore, removing from save."),
               xml_get(node));
      else
         player_missionFinished( id );
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
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      if (!xml_isNode(node,"done"))
         continue;

      int id = event_dataID( xml_get(node) );
      if (id < 0)
         DEBUG(_("Event '%s' doesn't seem to exist anymore, removing from save."),
               xml_get(node));
      else
         player_eventFinished( id );
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
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      if (!xml_isNode( node, "license" ))
         continue;

      const char *name = xml_get( node );
      if (name == NULL) {
         WARN( _( "License node is missing name." ) );
         continue;
      }
      player_tryAddLicense( name );
   } while (xml_nextNode(node));
   return 0;
}

/**
 * @brief Parses player's inventory.
 *
 *    @param parent Node of the inventory.
 *    @return 0 on success.
 */
static int player_parseInventory( xmlNodePtr parent )
{
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      int q;
      xml_onlyNodes(node);

      if (!xml_isNode( node, "item" ))
         continue;

      xmlr_attr_int_def( node, "quantity", q, 1 );
      const char *name = xml_get( node );
      if (name == NULL) {
         WARN( _( "Inventory item node is missing name." ) );
         continue;
      }
      player_inventoryAdd( name, q );
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
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      char *buf, *name;

      /* Skip non-escorts. */
      if (!xml_isNode(node,"escort"))
         continue;

      xmlr_attr_strd( node, "type", buf );
      xmlr_attr_strd( node, "name", name );
      if (name==NULL) /* Workaround for < 0.10.0 old saves, TODO remove around 0.12.0 or 0.13.0. */
         name = xml_getStrd( node );
      if (strcmp(buf,"bay")==0)
         escort_addList( player.p, ship_get(name), ESCORT_TYPE_BAY, 0, 1 );

      else if (strcmp(buf,"fleet")==0) {
         PlayerShip_t *ps = player_getPlayerShip( name );

         /* Only deploy escorts that are deployed. */
         if (!ps->deployed)
            WARN(_("Fleet ship '%s' is deployed despite not being marked for deployal!"), ps->p->name);

         /* Only deploy spaceworthy escorts. */
         if (!pilot_isSpaceworthy(ps->p))
            WARN(_("Fleet ship '%s' is deployed despite not being space worthy!"), ps->p->name);

         pfleet_deploy( ps );
      }
      else
         WARN(_("Escort has invalid type '%s'."), buf);
      free(buf);
      free(name);
   } while (xml_nextNode(node));
   return 0;
}

/**
 * @brief Parses the player metadata.
 *
 *    @param parent "metadata" node to parse.
 *    @return 0 on success.
 */
static int player_parseMetadata( xmlNodePtr parent )
{
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      xmlr_float(node,"dmg_done_shield",player.dmg_done_shield);
      xmlr_float(node,"dmg_done_armour",player.dmg_done_armour);
      xmlr_float(node,"dmg_taken_shield",player.dmg_taken_shield);
      xmlr_float(node,"dmg_taken_armour",player.dmg_taken_armour);
      xmlr_uint(node,"jumped_times",player.jumped_times);
      xmlr_uint(node,"landed_times",player.landed_times);
      xmlr_uint(node,"death_counter",player.death_counter);
      xmlr_float(node,"time_played",player.time_played);

      if (xml_isNode(node,"last_played")) {
         xml_parseTime(node, &player.last_played);
         continue;
      }
      else if (xml_isNode(node,"date_created")) {
         xml_parseTime(node, &player.date_created);
         continue;
      }
      else if (xml_isNode(node,"ships_destroyed")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do {
            char buf[STRMAX_SHORT];
            int class;

            xml_onlyNodes(cur);

            strncpy( buf, (const char*)cur->name, sizeof(buf)-1 );
            for (size_t i=0; i<strlen(buf); i++)
               if (buf[i]=='_')
                  buf[i] = ' ';

            class = ship_classFromString( buf );
            if (class==SHIP_CLASS_NULL) {
               WARN(_("Unknown ship class '%s' when parsing 'ships_destroyed' node!"), (const char*)cur->name );
               continue;
            }

            player.ships_destroyed[class] = xml_getULong(cur);
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));
   return 0;
}

/**
 * @brief Adds outfit to pilot if it can.
 */
static void player_addOutfitToPilot( Pilot* pilot, const Outfit* outfit, PilotOutfitSlot *s )
{
   int ret;

   if (!outfit_fitsSlot( outfit, &s->sslot->slot )) {
      DEBUG( _("Outfit '%s' does not fit designated slot on player's pilot '%s', adding to stock."),
            outfit->name, pilot->name );
      player_addOutfit( outfit, 1 );
      return;
   }

   ret = pilot_addOutfitRaw( pilot, outfit, s );
   if (ret != 0) {
      DEBUG(_("Outfit '%s' does not fit on player's pilot '%s', adding to stock."),
            outfit->name, pilot->name);
      player_addOutfit( outfit, 1 );
      return;
   }

   /* Update stats. */
   pilot_calcStats( pilot );
}

/**
 * @brief Parses a ship outfit slot.
 */
static void player_parseShipSlot( xmlNodePtr node, Pilot *ship, PilotOutfitSlot *slot )
{
   const Outfit *o;
   int q;
   const char *name = xml_get(node);
   if (name == NULL) {
      WARN(_("Empty ship slot node found, skipping."));
      return;
   }

   /* Add the outfit. */
   o = player_tryGetOutfit( name, 1 );
   if (o==NULL)
      return;
   player_addOutfitToPilot( ship, o, slot );

   /* Doesn't have ammo. */
   if (!outfit_isLauncher(o) && !outfit_isFighterBay(o))
      return;

   /* See if has quantity. */
   xmlr_attr_int(node,"quantity",q);
   if (q > 0)
      pilot_addAmmo( ship, slot, q );
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
   char *name, *model;
   int id, autoweap, fuel, aim_lines, active_set;
   const Ship *ship_parsed;
   Pilot* ship;
   xmlNodePtr node;
   Commodity *com;
   PilotFlags flags;
   PlayerShip_t ps;

   memset( &ps, 0, sizeof(PlayerShip_t) );

   /* Parse attributes. */
   xmlr_attr_strd( parent, "name", name );
   xmlr_attr_strd( parent, "model", model );
   xmlr_attr_int_def( parent, "favourite", ps.favourite, 0 );
   xmlr_attr_int_def( parent, "deployed", ps.deployed, 0 );

   /* Safe defaults. */
   pilot_clearFlagsRaw( flags );
   if (is_player)
      pilot_setFlagRaw( flags, PILOT_PLAYER );
   pilot_setFlagRaw( flags, PILOT_NO_OUTFITS );

   /* Handle certain 0.10.0-alpha saves where it's possible that... */
   if (!is_player && strcmp( name, player.p->name ) == 0) {
      DEBUG( _("Ignoring player-owned ship '%s': duplicate of player's current ship."), name );
      free(name);
      free(model);
      return 0;
   }

   /* Get the ship. */
   ship_parsed = player_tryGetShip( model );
   if (ship_parsed == NULL) {
      WARN(_("Player ship '%s' not found!"), model);

      /* TODO we should probably parse the outfits and give them to the player. */

      /* Clean up. */
      free(name);
      free(model);

      return -1;
   }

   /* Create the ship. */
   ship = pilot_createEmpty( ship_parsed, name, faction_get("Player"), flags );
   /* Player is currently on this ship */
   if (is_player)
      ps.deployed = 0; /* Current ship can't be deployed. */
   ps.p = ship;

   /* Ship should not have default outfits. */
   for (int i=0; i<array_size(ship->outfits); i++)
      pilot_rmOutfitRaw( ship, ship->outfits[i] );

   /* Clean up. */
   free(name);
   free(model);

   /* Defaults. */
   fuel      = -1;
   autoweap  = 1;
   aim_lines = 0;

   /* Start parsing. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      /* Meta-data. */
      xmlr_strd(node,"acquired",ps.acquired);
      if (xml_isNode(node,"acquired_date")) {
         xml_parseTime(node, &ps.acquired_date);
         continue;
      }
      xmlr_float(node,"time_played",ps.time_played);
      xmlr_float(node,"dmg_done_shield",ps.dmg_done_shield);
      xmlr_float(node,"dmg_done_armour",ps.dmg_done_armour);
      xmlr_float(node,"dmg_taken_shield",ps.dmg_taken_shield);
      xmlr_float(node,"dmg_taken_armour",ps.dmg_taken_armour);
      xmlr_uint(node,"jumped_times",ps.jumped_times);
      xmlr_uint(node,"landed_times",ps.landed_times);
      xmlr_uint(node,"death_counter",ps.death_counter);
      if (xml_isNode(node,"ships_destroyed")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do {
            char buf[STRMAX_SHORT];
            int class;

            xml_onlyNodes(cur);

            strncpy( buf, (const char*)cur->name, sizeof(buf)-1 );
            for (size_t i=0; i<strlen(buf); i++)
               if (buf[i]=='_')
                  buf[i] = ' ';

            class = ship_classFromString( buf );
            if (class==SHIP_CLASS_NULL) {
               WARN(_("Unknown ship class '%s' when parsing 'ships_destroyed' node!"), (const char*)cur->name );
               continue;
            }

            ps.ships_destroyed[class] = xml_getULong(cur);
         } while (xml_nextNode(cur));
      }

      /* Get fuel. */
      xmlr_int(node,"fuel",fuel);

      /* New outfit loading. */
      if (xml_isNode(node,"outfits_structure")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do { /* load each outfit */
            int n;
            xml_onlyNodes(cur);
            if (!xml_isNode(cur,"outfit")) {
               WARN(_("Save has unknown '%s' tag!"),xml_get(cur));
               continue;
            }
            xmlr_attr_int_def( cur, "slot", n, -1 );
            if ((n<0) || (n >= array_size(ship->outfit_structure))) {
               WARN(_("Outfit slot out of range, not adding to ship."));
               continue;
            }
            player_parseShipSlot( cur, ship, &ship->outfit_structure[n] );
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"outfits_utility")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do { /* load each outfit */
            int n;
            xml_onlyNodes(cur);
            if (!xml_isNode(cur,"outfit")) {
               WARN(_("Save has unknown '%s' tag!"),xml_get(cur));
               continue;
            }
            xmlr_attr_int_def( cur, "slot", n, -1 );
            if ((n<0) || (n >= array_size(ship->outfit_utility))) {
               WARN(_("Outfit slot out of range, not adding."));
               continue;
            }
            player_parseShipSlot( cur, ship, &ship->outfit_utility[n] );
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"outfits_weapon")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do { /* load each outfit */
            int n;
            xml_onlyNodes(cur);
            if (!xml_isNode(cur,"outfit")) {
               WARN(_("Save has unknown '%s' tag!"),xml_get(cur));
               continue;
            }
            xmlr_attr_int_def( cur, "slot", n, -1 );
            if ((n<0) || (n >= array_size(ship->outfit_weapon))) {
               WARN(_("Outfit slot out of range, not adding."));
               continue;
            }
            player_parseShipSlot( cur, ship, &ship->outfit_weapon[n] );
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"outfits_intrinsic")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do { /* load each outfit */
            xml_onlyNodes(cur);
            if (!xml_isNode(cur,"outfit")) {
               WARN(_("Save has unknown '%s' tag!"),xml_get(cur));
               continue;
            }
            const Outfit *o = player_tryGetOutfit( xml_get(cur), 1 );
            if (o!=NULL) {
               if (pilot_hasOutfitLimit( ship, o->limit ))
                  WARN(_("Player ship '%s' has intrinsic outfit '%s' exceeding limits! Removing."),ship->name,o->name);
               else
                  pilot_addOutfitIntrinsic( ship, o );
            }
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node, "commodities")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur, "commodity")) {
               int cid, quantity;

               xmlr_attr_int( cur, "quantity", quantity );
               xmlr_attr_int_def( cur, "id", cid, 0 );

               /* Get the commodity. */
               com = commodity_get(xml_get(cur));
               if (com == NULL) {
                  WARN(_("Unknown commodity '%s' detected, removing."), xml_get(cur));
                  continue;
               }

               /* actually add the cargo with id hack
                * Note that the player's cargo_free is ignored here. */
               if ((quantity==0) && (cid==0))
                  WARN(_("Adding cargo '%s' to ship '%s' that is not a mission cargo with quantity=0!"), com->name, ship->name );
               pilot_cargoAddRaw( ship, com, quantity, cid );
            }
         } while (xml_nextNode(cur));
         continue;
      }
      //WARN(_("Save has unknown '%s' tag!"),xml_get(node));
   } while (xml_nextNode(node));

   /* Update stats. */
   pilot_calcStats( ship );

   /* Test for validity. */
   if (fuel >= 0)
      ship->fuel = MIN(ship->fuel_max, fuel);
   /* ships can now be non-spaceworthy on save
    * str = pilot_isSpaceworthy( ship ); */
   if (!pilot_slotsCheckSafety( ship )) {
      DEBUG(_("Player ship '%s' failed slot validity check , removing all outfits and adding to stock."),
            ship->name );
      /* Remove all outfits. */
      for (int i=0; i<array_size(ship->outfits); i++) {
         const Outfit *o = ship->outfits[i]->outfit;
         int ret = pilot_rmOutfitRaw( ship, ship->outfits[i] );
         if (ret==0)
            player_addOutfit( o, 1 );
      }
      pilot_calcStats( ship );
   }

   /* Sets inrange by default if weapon sets are missing. */
   for (int i=0; i<PILOT_WEAPON_SETS; i++)
      pilot_weapSetInrange( ship, i, WEAPSET_INRANGE_PLAYER_DEF );

   /* Second pass for weapon sets. */
   active_set = 0;
   node = parent->xmlChildrenNode;
   do {
      xmlNodePtr cur;

      if (xml_isNode(node,"vars")) {
         ps.p->shipvar = lvar_load( node );
         continue;
      }
      else if (!xml_isNode(node,"weaponsets"))
         continue;

      /* Check for autoweap. */
      xmlr_attr_int( node, "autoweap", autoweap );

      /* Load the last weaponset the player used on this ship. */
      xmlr_attr_int_def( node, "active_set", active_set, -1 );

      /* Check for aim_lines. */
      xmlr_attr_int( node, "aim_lines", aim_lines );

      /* Parse weapon sets. */
      cur = node->xmlChildrenNode;
      do { /* Load each weapon set. */
         int in_range, manual, weap_type, volley;
         xmlNodePtr ccur;

         xml_onlyNodes(cur);
         if (!xml_isNode(cur,"weaponset")) {
            WARN(_("Player ship '%s' has unknown node '%s' in 'weaponsets' (expected 'weaponset')."),
                  ship->name, cur->name);
            continue;
         }

         /* Get id. */
         xmlr_attr_int_def(cur, "id", id, -1);
         if (id == -1) {
            WARN(_("Player ship '%s' missing 'id' tag for weapon set."),ship->name);
            continue;
         }
         if ((id < 0) || (id >= PILOT_WEAPON_SETS)) {
            WARN(_("Player ship '%s' has invalid weapon set id '%d' [max %d]."),
                  ship->name, id, PILOT_WEAPON_SETS-1 );
            continue;
         }

         /* Clean up weapon set. */
         pilot_weapSetClear( ship, id );

         /* Set inrange mode. */
         xmlr_attr_int( cur, "inrange", in_range );
         if (in_range > 0)
            pilot_weapSetInrange( ship, id, in_range );

         /* Set manual mode. */
         xmlr_attr_int( cur, "manual", manual );
         if (manual > 0)
            pilot_weapSetManual( ship, id, manual );

         /* Set volley mode. */
         xmlr_attr_int( cur, "volley", volley );
         if (volley > 0)
            pilot_weapSetVolley( ship, id, volley );

         if (autoweap) /* Autoweap handles everything except inrange and manual. */
            continue;

         /* Set type mode. */
         xmlr_attr_int_def( cur, "type", weap_type, -1 );
         if (weap_type == -1) {
            WARN(_("Player ship '%s' missing 'type' tag for weapon set."),ship->name);
            continue;
         }
         pilot_weapSetType( ship, id, weap_type );

         /* Parse individual weapons. */
         ccur = cur->xmlChildrenNode;
         do {
            int level, weapid;
            /* Only nodes. */
            xml_onlyNodes(ccur);

            /* Only weapon nodes. */
            if (!xml_isNode(ccur,"weapon")) {
               WARN(_("Player ship '%s' has unknown 'weaponset' child node '%s' (expected 'weapon')."),
                     ship->name, ccur->name );
               continue;
            }

            /* Get level. */
            xmlr_attr_int_def( ccur, "level", level, -1 );
            if (level == -1) {
               WARN(_("Player ship '%s' missing 'level' tag for weapon set weapon."), ship->name);
               continue;
            }
            weapid = xml_getInt(ccur);
            if ((weapid < 0) || (weapid >= array_size(ship->outfits))) {
               WARN(_("Player ship '%s' has invalid weapon id %d [max %d]."),
                     ship->name, weapid, array_size(ship->outfits)-1 );
               continue;
            }

            /* Add the weapon set. */
            pilot_weapSetAdd( ship, id, ship->outfits[weapid], level );

         } while (xml_nextNode(ccur));
      } while (xml_nextNode(cur));
   } while (xml_nextNode(node));

   /* Set up autoweap if necessary. */
   ship->autoweap = autoweap;
   if (autoweap)
      pilot_weaponAuto( ship );
   pilot_weaponSafe( ship );
   if (active_set >= 0 && active_set < PILOT_WEAPON_SETS)
      ship->active_set = active_set;
   else
      pilot_weaponSetDefault( ship );
   /* Copy the weapon set over to the player ship, where we store it. */
   ws_copy( ps.weapon_sets, ship->weapon_sets );

   /* Set aimLines */
   ship->aimLines = aim_lines;

   /* Add it to the stack if it's not what the player is in */
   if (is_player == 0)
      array_push_back( &player_stack, ps );
   else {
      pilot_setPlayer( ship );
      player.ps = ps;
   }

   return 0;
}

/**
 * @brief Input binding for toggling stealth for the player.
 */
void player_stealth (void)
{
   if (player.p == NULL)
      return;

   /* Handle destealth first. */
   if (pilot_isFlag(player.p, PILOT_STEALTH)) {
      pilot_destealth( player.p );
      player_message(_("You have destealthed."));
      return;
   }

   /* Stealth case. */
   if (pilot_stealth( player.p )) {
      player_message( "#g%s", _("You have entered stealth mode.") );
   }
   else {
      /* Stealth failed. */
      if (player.p->lockons > 0)
         player_message( "#r%s", _("Unable to stealth: missiles locked on!") );
      else
         player_message( "#r%s", _("Unable to stealth: other pilots nearby!") );
   }
}
