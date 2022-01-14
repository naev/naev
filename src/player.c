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
#include "camera.h"
#include "claim.h"
#include "comm.h"
#include "conf.h"
#include "dialogue.h"
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
#include "nlua_var.h"
#include "nstring.h"
#include "ntime.h"
#include "nxml.h"
#include "opengl.h"
#include "pause.h"
#include "pilot.h"
#include "player_gui.h"
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
static const Ship* player_ship      = NULL; /**< Temporary ship to hold when naming it */
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
static PlayerShip_t* player_stack      = NULL;  /**< Stack of ships player has. */
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
static void player_parseShipSlot( xmlNodePtr node, Pilot *ship, PilotOutfitSlot *slot );
static int player_parseShip( xmlNodePtr parent, int is_player );
static int player_parseEscorts( xmlNodePtr parent );
static int player_parseMetadata( xmlNodePtr parent );
static void player_addOutfitToPilot( Pilot* pilot, const Outfit* outfit, PilotOutfitSlot *s );
static int player_runUpdaterScript( const char* type, const char* name, int q );
static const Outfit* player_tryGetOutfit( const char* name, int q );
static void player_tryAddLicense( const char* name );
/* Render. */
static void player_renderStealthUnderlay( double dt );
static void player_renderStealthOverlay( double dt );
static void player_renderAimHelper( double dt );
/* Misc. */
static int player_filterSuitableSpob( Spob *p );
static void player_spobOutOfRangeMsg (void);
static int player_outfitCompare( const void *arg1, const void *arg2 );
static int player_thinkMouseFly(void);
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
   memset( &player.ps, 0, sizeof(PlayerShip_t) );
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
   const char *title, *caption;
   char *ret, buf[PATH_MAX];

   const char *speed_opts[] = {
      _("Normal Speed"),
      _("Slow Speed")
   };

   /* Set up new player. */
   player_newSetup();

   /* Some meta-data. */
   player.date_created = time(NULL);

   /* Get the name. */
   player.name = dialogue_input( _("Player Name"), 1, 60,
         _("Please write your name:") );

   /* Player cancelled dialogue. */
   if (player.name == NULL) {
      menu_main();
      return;
   }

   snprintf( buf, sizeof(buf), "saves/%s.ns", player.name);
   if (PHYSFS_exists( buf )) {
      int r = dialogue_YesNo(_("Overwrite"),
            _("You already have a pilot named %s. Overwrite?"),player.name);
      if (r==0) { /* no */
         player_new();
         return;
      }
   }

   /* Set game speed. */
   title = _("Game Speed");
   caption = _("Your game can be set to normal speed or slow speed. Slow speed"
         " causes time in space to pass at half the rate it would in normal"
         " speed, which may be helpful if you have difficulty playing the game"
         " at normal speed. Would you like to use normal speed or slow speed"
         " for this profile? (If unsure, normal speed is probably what you"
         " want.)");

   dialogue_makeChoice( title, caption, 2 );
   dialogue_addChoice( title, caption, speed_opts[0] );
   dialogue_addChoice( title, caption, speed_opts[1] );
   ret = dialogue_runChoice();
   player.dt_mod = 1.;
   if (ret != NULL) {
      if (strcmp(ret, speed_opts[1]) == 0)
         player.dt_mod = 0.5;
      free( ret );
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
   shipname = start_shipname();
   if (ship==NULL) {
      WARN(_("Ship not properly set by module."));
      return -1;
   }
   acquired = start_acquired();
   /* Setting a default name in the XML prevents naming prompt. */
   if (player_newShip( ship, shipname, 0, acquired, (shipname==NULL) ? 0 : 1 ) == NULL) {
      player_new();
      return -1;
   }
   start_position( &x, &y );
   vect_cset( &player.p->solid->pos, x, y );
   vectnull( &player.p->solid->vel );
   player.p->solid->dir = RNGF() * 2.*M_PI;
   space_init( start_system(), 1 );

   /* Set player speed to default 1 */
   player.speed = 1.;

   /* Reset speed (to make sure player.dt_mod is accounted for). */
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
Pilot* player_newShip( const Ship* ship, const char *def_name,
      int trade, const char *acquired, int noname )
{
   char *ship_name, *old_name;
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

   ps = player_newShipMake( ship_name );
   ps->autoweap  = 1;
   ps->favourite = 0;
   ps->shipvar   = array_create( lvar );
   ps->acquired  = (acquired!=NULL) ? strdup( acquired ) : NULL;
   ps->acquired_date = ntime_get();

   /* Player is trading ship in. */
   if (trade) {
      if (player.p == NULL)
         ERR(_("Player ship isn't valid... This shouldn't happen!"));
      old_name = player.p->name;
      player_swapShip( ship_name, 1 ); /* Move to the new ship. */
      player_rmShip( old_name );
   }

   free(ship_name);

   /* Update ship list if landed. */
   if (landed) {
      int w = land_getWid( LAND_WINDOW_EQUIPMENT );
      equipment_regenLists( w, 0, 1 );
   }

   return ps->p;
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
   pilot_setFlagRaw( flags, PILOT_NO_OUTFITS );

   /* in case we're respawning */
   player_rmFlag( PLAYER_CREATING );

   /* create the player */
   if (player.p == NULL) {
      double px, py, dir;
      unsigned int id;
      Vector2d vp, vv;

      /* Set position to defaults. */
      if (player.p != NULL) {
         px    = player.p->solid->pos.x;
         py    = player.p->solid->pos.y;
         dir   = player.p->solid->dir;
      }
      else {
         px    = 0.;
         py    = 0.;
         dir   = 0.;
      }
      vect_cset( &vp, px, py );
      vect_cset( &vv, 0., 0. );

      /* Create the player. */
      id = pilot_create( player_ship, name, faction_get("Player"), "player",
            dir, &vp, &vv, flags, 0, 0 );
      cam_setTargetPilot( id, 0 );
      ps = &player.ps;
   }
   else {
      /* Grow memory. */
      ps = &array_grow( &player_stack );
      memset( ps, 0, sizeof(PlayerShip_t) );
      /* Create the ship. */
      ps->p = pilot_createEmpty( player_ship, name, faction_get("Player"), "player", flags );
   }

   if (player.p == NULL)
      ERR(_("Something seriously wonky went on, newly created player does not exist, bailing!"));

   /* Add GUI. */
   player_guiAdd( player_ship->gui );

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
   Vector2d v;
   double dir;
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

   /* Run onremove hook for all old outfits. */
   for (int j=0; j<array_size(player.p->outfits); j++)
      pilot_outfitLRemove( player.p, player.p->outfits[j] );

   /* Get rid of deployed escorts. */
   escort_clearDeployed( player.p );

   /* Swap information over. */
   ptemp = player.ps;
   player.ps = *ps;
   *ps = ptemp;

   /* Swap player and ship */
   ship = player.ps.p;

   /* move credits over */
   ship->credits = player.p->credits;

   /* move cargo over */
   if (move_cargo)
      pilot_cargoMove( ship, player.p );

   /* Copy target info */
   ship->target = player.p->target;
   ship->nav_spob = player.p->nav_spob;
   ship->nav_hyperspace = player.p->nav_hyperspace;
   ship->nav_anchor = player.p->nav_anchor;
   ship->nav_asteroid = player.p->nav_asteroid;

   /* Store position. */
   v     = player.p->solid->pos;
   dir   = player.p->solid->dir;

   /* extra pass to calculate stats */
   pilot_calcStats( ship );
   pilot_calcStats( player.p );

   /* Run onadd hook for all new outfits. */
   for (int j=0; j<array_size(ship->outfits); j++)
      pilot_outfitLAdd( ship, ship->outfits[j] );

   /* now swap the players */
   player.p    = pilot_replacePlayer( ship );
   player.ps.p = player.p;

   /* Copy position back. */
   player.p->solid->pos = v;
   player.p->solid->dir = dir;

   /* Fill the tank. */
   if (landed)
      land_refuel();

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
   hparam[1].type    = HOOK_PARAM_STRING;
   hparam[1].u.str   = player.p->ship->name;
   hparam[2].type    = HOOK_PARAM_STRING;
   hparam[2].u.str   = ps->p->name;
   hparam[3].type    = HOOK_PARAM_STRING;
   hparam[3].u.str   = ps->p->ship->name;
   hparam[4].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "ship_swap", hparam );

   return;
}

/**
 * @brief Calculates the price of one of the player's ships.
 *
 *    @param shipname Name of the ship.
 *    @return The price of the ship in credits.
 */
credits_t player_shipPrice( const char *shipname )
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

   return pilot_worth( ship );
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
      pilot_free( ps->p );
      lvar_freeArray( ps->shipvar );
      free( ps->acquired );

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
   diff_clear();
   var_cleanup();
   missions_cleanup();
   events_cleanup();
   space_clearKnown();
   land_cleanup();
   map_cleanup();
   factions_clearDynamic();

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
   array_free( player.ps.shipvar );
   memset( &player.ps, 0, sizeof(PlayerShip_t) );

   free(player_message_noland);
   player_message_noland = NULL;

   /* Clean up gui. */
   gui_cleanup();
   player_guiCleanup();
   ovr_setOpen(0);

   /* Clear up info buttons. */
   info_buttonClear();

   /* clean up the stack */
   for (int i=0; i<array_size(player_stack); i++)
      pilot_free(player_stack[i].p);
   array_free(player_stack);
   player_stack = NULL;
   /* nothing left */

   array_free(player_outfits);
   player_outfits  = NULL;

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

   /* just in case purge the pilot stack */
   pilots_cleanAll();

   /* Reset some player stuff. */
   player_creds   = 0;
   player_payback = 0;
   free( player.gui );
   player.gui = NULL;
   free( player.chapter );
   player.chapter = NULL;

   /* Clear omsg. */
   omsg_cleanup();

   /* Stop the sounds. */
   sound_stopAll();

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
void player_warp( const double x, const double y )
{
   vect_cset( &player.p->solid->pos, x, y );
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
   if ((player.p == NULL) || player_isFlag(PLAYER_CREATING) ||
         pilot_isFlag( player.p, PILOT_HIDE))
      return;

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
   pilot_render( player.p, dt );

   /* Render the player's overlay. */
   pilot_renderOverlay( player.p, dt );
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

      gl_gameToScreenCoords( &x, &y, t->solid->pos.x, t->solid->pos.y );
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
   gl_gameToScreenCoords( &x, &y, player.p->solid->pos.x, player.p->solid->pos.y );

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

   a = player.p->solid->dir;
   r = 200.;
   gl_gameToScreenCoords( &x1, &y1, player.p->solid->pos.x, player.p->solid->pos.y );

   b = pilot_aimAngle( player.p, target );

   theta = 22.*M_PI/180.;

   /* The angular error will give the exact colour that is used. */
   d = ABS( angle_diff(a,b) / (2*theta) );
   d = MIN( 1, d );

   c = cInert;
   c.a = 0.3;
   gl_gameToScreenCoords( &x2, &y2, player.p->solid->pos.x + r*cos( a+theta ),
                           player.p->solid->pos.y + r*sin( a+theta ) );
   gl_renderLine( x1, y1, x2, y2, &c );
   gl_gameToScreenCoords( &x2, &y2, player.p->solid->pos.x + r*cos( a-theta ),
                           player.p->solid->pos.y + r*sin( a-theta ) );
   gl_renderLine( x1, y1, x2, y2, &c );

   c.r = d*0.9;
   c.g = d*0.2 + (1.-d)*0.8;
   c.b = (1-d)*0.2;
   c.a = 0.7;
   col_gammaToLinear( &c );
   gl_gameToScreenCoords( &x2, &y2, player.p->solid->pos.x + r*cos( a ),
                           player.p->solid->pos.y + r*sin( a ) );

   gl_renderLine( x1, y1, x2, y2, &c );

   c2 = cWhite;
   c2.a = 0.7;
   glUseProgram(shaders.crosshairs.program);
   glUniform1f(shaders.crosshairs.paramf, 1.);
   gl_renderShader( x2, y2, 7, 7, 0., &shaders.crosshairs, &c2, 1 );

   gl_gameToScreenCoords( &x2, &y2, player.p->solid->pos.x + r*cos( b ),
                           player.p->solid->pos.y + r*sin( b ) );

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
   double turn;
   int facing, fired;

   /* last i heard, the dead don't think */
   if (pilot_isFlag(pplayer,PILOT_DEAD)) {
      /* no sense in accelerating or turning */
      pilot_setThrust( pplayer, 0. );
      pilot_setTurn( pplayer, 0. );
      return;
   }

   /* We always have to run ai_think in the case the player has escorts so that
    * they properly form formations. */
   ai_think( pplayer, dt );

   /* Under manual control is special. */
   if (pilot_isFlag( pplayer, PILOT_MANUAL_CONTROL )) {
      return;
   }

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
      facing = player_thinkMouseFly();

   /* turning taken over by PLAYER_FACE */
   if (!facing && player_isFlag(PLAYER_FACE)) {
      /* Try to face pilot target. */
      if (player.p->target != PLAYER_ID) {
         target = pilot_getTarget( player.p );
         if (target != NULL) {
            pilot_face( pplayer,
                  vect_angle( &player.p->solid->pos, &target->solid->pos ));

            /* Disable turning. */
            facing = 1;
         }
      }
      /* Try to face asteroid. */
      else if (player.p->nav_asteroid != -1) {
         AsteroidAnchor *field = &cur_system->asteroids[player.p->nav_anchor];
         Asteroid *ast = &field->asteroids[player.p->nav_asteroid];
         pilot_face( pplayer,
               vect_angle( &player.p->solid->pos, &ast->pos ));
         /* Disable turning. */
         facing = 1;
      }
      /* If not try to face spob target. */
      else if ((player.p->nav_spob != -1) && ((preemption == 0) || (player.p->nav_hyperspace == -1))) {
         pilot_face( pplayer,
               vect_angle( &player.p->solid->pos,
                  &cur_system->spobs[ player.p->nav_spob ]->pos ));
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
   if (player_isFlag(PLAYER_REVERSE)) {

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
       * If the player has reverse thrusters, fire those.
       */
      if (player.p->stats.misc_reverse_thrust)
         player_accel( -PILOT_REVERSE_THRUST );
      else if (!facing) {
         pilot_face( pplayer, VANGLE(player.p->solid->vel) + M_PI );
         /* Disable turning. */
         facing = 1;
      }
   }

   /* Normal turning scheme */
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
   fired = 0;

   /* Primary weapon. */
   if (player_isFlag(PLAYER_PRIMARY)) {
      fired |= pilot_shoot( pplayer, 0 );
      player_setFlag(PLAYER_PRIMARY_L);
   }
   else if (player_isFlag(PLAYER_PRIMARY_L)) {
      pilot_shootStop( pplayer, 0 );
      player_rmFlag(PLAYER_PRIMARY_L);
   }
   /* Secondary weapon - we use PLAYER_SECONDARY_L to track last frame. */
   if (player_isFlag(PLAYER_SECONDARY)) { /* needs target */
      fired |= pilot_shoot( pplayer, 1 );
      player_setFlag(PLAYER_SECONDARY_L);
   }
   else if (player_isFlag(PLAYER_SECONDARY_L)) {
      pilot_shootStop( pplayer, 1 );
      player_rmFlag(PLAYER_SECONDARY_L);
   }

   if (fired) {
      player.autonav_timer = MAX( player.autonav_timer, 1. );
      player_autonavResetSpeed();
   }

   pilot_setThrust( pplayer, player_acc );
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
   double pitch = 1., volume = 1.;

   /* Calculate engine sound to use. */
   if (pilot_isFlag(pplayer, PILOT_AFTERBURNER))
      engsound = pplayer->afterburner->outfit->u.afb.sound;
   else if ((pplayer->solid->thrust > 1e-3) || (pplayer->solid->thrust < -1e-3)) {
      /* See if is in hyperspace. */
      if (pilot_isFlag(pplayer, PILOT_HYPERSPACE)) {
         engsound = snd_hypEng;
      }
      else {
         engsound = pplayer->ship->sound;
         pitch = pplayer->ship->engine_pitch;
         volume = conf.engine_vol;
      }
   }
   else
      engsound = -1;
   /* See if sound must change. */
   if (player_lastEngineSound != engsound) {
      sound_stopGroup( player_engine_group );
      if (engsound >= 0) {
         sound_pitchGroup( player_engine_group, pitch );
         sound_volumeGroup( player_engine_group, volume );
         sound_playGroup( player_engine_group, engsound, 0 );
      }
   }
   player_lastEngineSound = engsound;

   /* Sound. */
   /*
    * Sound is now camera-specific and thus not player specific. A bit sad really.
   sound_updateListener( pplayer->solid->dir,
         pplayer->solid->pos.x, pplayer->solid->pos.y,
         pplayer->solid->vel.x, pplayer->solid->vel.y );
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

   if (toolkit_isOpen() && (type>0 || pilot_weapSet(player.p,id)->type != WEAPSET_TYPE_WEAPON))
      return;

   if ((type>0) && (pilot_isFlag(player.p, PILOT_HYP_PREP) ||
         pilot_isFlag(player.p, PILOT_HYPERSPACE) ||
         pilot_isFlag(player.p, PILOT_LANDING) ||
         pilot_isFlag(player.p, PILOT_TAKEOFF)))
      return;

   pilot_weapSetPress( player.p, id, type );
}

/**
 * @brief Resets the player speed stuff.
 */
void player_resetSpeed (void)
{
   double spd = player.speed * player_dt_default();
   pause_setSpeed( spd );
   sound_setSpeed( spd );
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
      if (pilot_isFlag(player.p, PILOT_COOLDOWN)) {
         gui_cooldownEnd();
         pilot_cooldownEnd(player.p, str);
      }
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

   if ((player.autonav == AUTONAV_SPOB_LAND_APPROACH) ||
         (player.autonav == AUTONAV_SPOB_APPROACH) ||
         (player.autonav == AUTONAV_SPOB_LAND_BRAKE))
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

         /* See if the player has the asteroid scanner. */
         if (player.p->stats.misc_asteroid_scan) {
            /* Recover and display some info about the asteroid. */
            AsteroidAnchor *anchor = &cur_system->asteroids[field];
            Asteroid *ast = &anchor->asteroids[id];
            const AsteroidType *at = space_getType( ast->type );

            player_message( _("Asteroid targeted, composition: ") );
            for (int i=0; i<array_size(at->material); i++) {
              Commodity *com = at->material[i];
              player_message( _("%s, quantity: %i"), _(com->name), at->quantity[i] );
            }
            ast->scanned = 1;
         }
         else
            player_message( _("Asteroid targeted") );
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
      takeoff(1);
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

   /* Find new target. */
   if (player.p->nav_spob == -1) { /* get nearest spob target */
      double td = -1.; /* temporary distance */
      int tp = -1; /* temporary spob */
      for (int i=0; i<array_size(cur_system->spobs); i++) {
         spob = cur_system->spobs[i];
         double d = vect_dist(&player.p->solid->pos,&spob->pos);
         if (pilot_inRangeSpob( player.p, i ) &&
               spob_hasService(spob,SPOB_SERVICE_LAND) &&
               ((tp==-1) || ((td == -1) || (td > d)))) {
            tp = i;
            td = d;
         }
      }
      player_targetSpobSet( tp );
      player_hyperspacePreempt(0);

      /* no landable spob */
      if (player.p->nav_spob < 0)
         return PLAYER_LAND_DENIED;

      silent = 1; /* Suppress further targeting noises. */
   }
   /* Check if spob is in range when not uninhabited. */
   else if (!spob_isFlag(cur_system->spobs[ player.p->nav_spob ], SPOB_UNINHABITED) && !pilot_inRangeSpob( player.p, player.p->nav_spob )) {
      player_spobOutOfRangeMsg();
      return PLAYER_LAND_AGAIN;
   }

   if (player_isFlag(PLAYER_NOLAND)) {
      player_message( "#r%s", player_message_noland );
      return PLAYER_LAND_DENIED;
   }
   else if (pilot_isFlag( player.p, PILOT_NOLAND)) {
      player_message( _("#rDocking stabilizers malfunctioning, cannot land.") );
      return PLAYER_LAND_DENIED;
   }

   /* attempt to land at selected spob */
   spob = cur_system->spobs[player.p->nav_spob];
   if ((spob->lua_can_land==LUA_NOREF) && !spob_hasService(spob, SPOB_SERVICE_LAND)) {
      player_messageRaw( _("#rYou can't land here.") );
      return PLAYER_LAND_DENIED;
   }
   else if (!player_isFlag(PLAYER_LANDACK)) { /* no landing authorization */
      if ((spob->lua_can_land!=LUA_NOREF) || spob_hasService(spob,SPOB_SERVICE_INHABITED)) { /* Basic services */
         if (spob->can_land || (spob->land_override > 0))
            player_message( "#%c%s>#0 %s", spob_getColourChar(spob),
                  spob_name(spob), spob->land_msg );
         else if (spob->bribed && (spob->land_override >= 0))
            player_message( "#%c%s>#0 %s", spob_getColourChar(spob),
                  spob_name(spob), spob->bribe_ack_msg );
         else { /* Hostile */
            player_message( "#%c%s>#0 %s", spob_getColourChar(spob),
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
   else if (vect_dist2(&player.p->solid->pos,&spob->pos) > pow2(spob->radius)) {
      if (loud)
         player_message(_("#rYou are too far away to land on %s."), spob_name(spob));
      return PLAYER_LAND_AGAIN;
   }
   else if (vect_odist2( &player.p->solid->vel ) > pow2(MAX_HYPERSPACE_VEL)) {
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
      if (nlua_pcall( spob->lua_env, 1, 0 )) {
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
   pilot_setThrust( player.p, 0. );
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
   if (p->can_land || (p->land_override > 0) || p->bribed)
      return;

   player_rmFlag(PLAYER_LANDACK);
   player_message( _("#%c%s>#0 Landing permission revoked."),
         spob_getColourChar(p), spob_name(p) );
}

/**
 * @brief Checks whether the player's ship is able to takeoff.
 */
int player_canTakeoff(void)
{
   return !pilot_checkSpaceworthy(player.p);
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
 */
void player_board (void)
{
   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return;

   /* Try to grab target if not available. */
   if (player.p->target==PLAYER_ID) {
      Pilot *p;
      /* We don't try to find far away targets, only nearest and see if it matches.
       * However, perhaps looking for first boardable target within a certain range
       * could be more interesting. */
      player_targetNearest();
      p = pilot_getTarget( player.p );
      if ((!pilot_isDisabled(p) && !pilot_isFlag(p,PILOT_BOARDABLE)) ||
            pilot_isFlag(p,PILOT_NOBOARD)) {
         player_targetClear();
         player_message( _("#rYou need a target to board first!") );
         return;
      }
   }

   player_autonavBoard( player.p->target );
}

/**
 * @brief Sets the player's hyperspace target.
 *
 *    @param id ID of the hyperspace target.
 */
void player_targetHyperspaceSet( int id )
{
   int old;

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

   if ((player.autonav == AUTONAV_JUMP_APPROACH) ||
         (player.autonav == AUTONAV_JUMP_BRAKE))
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

   player_targetHyperspaceSet( id );

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
 * @brief Returns the player's total default time delta based on dt_mod and ship's dt_default.
 *
 *    @return The default/minimum time delta
 */
double player_dt_default (void)
{
   if (player.p != NULL && player.p->ship != NULL)
      return player.p->stats.time_mod * player.p->ship->dt_default * player.dt_mod;

   return player.dt_mod;
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
   player_autonavResetSpeed();
   player.autonav_timer = MAX( player.autonav_timer, 10. );
}

/**
 * @brief Actually attempts to jump in hyperspace.
 *
 *    @return 1 if actually started a jump, 0 otherwise.
 */
int player_jump (void)
{
   int h;
   double mindist;

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
      mindist  = INFINITY;
      for (int i=0; i<array_size(cur_system->jumps); i++) {
         double dist = vect_dist2( &player.p->solid->pos, &cur_system->jumps[i].pos );
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
      player_message(_("#rAborting hyperspace sequence."));
      return 0;
   }

   /* Try to hyperspace. */
   h = space_hyperspace(player.p);
   if (h == -1)
      player_message(_("#rYou are too far from a jump point to initiate hyperspace."));
   else if (h == -2)
      player_message(_("#rHyperspace drive is offline."));
   else if (h == -3)
      player_message(_("#rYou do not have enough fuel to hyperspace jump."));
   else {
      player_message(_("#oPreparing for hyperspace."));
      /* Stop acceleration noise. */
      player_accelOver();
      /* Stop possible shooting. */
      pilot_shootStop( player.p, 0 );
      pilot_shootStop( player.p, 1 );

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
   StarSystem *sys, *destsys;
   JumpPoint *jp;
   Pilot *const* pilot_stack;
   int map_npath;

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
   space_calcJumpInPos( cur_system, sys, &player.p->solid->pos, &player.p->solid->vel, &player.p->solid->dir, player.p );
   cam_setTargetPilot( player.p->id, 0 );

   /* reduce fuel */
   player.p->fuel -= player.p->fuel_consumption;

   /* stop hyperspace */
   pilot_rmFlag( player.p, PILOT_HYPERSPACE );
   pilot_rmFlag( player.p, PILOT_HYP_BEGIN );
   pilot_rmFlag( player.p, PILOT_HYP_BRAKE );
   pilot_rmFlag( player.p, PILOT_HYP_PREP );

   /* Set the ptimer. */
   player.p->ptimer = HYPERSPACE_FADEIN;

   /* GIve some non-targetable time. */
   player.p->itimer = PILOT_PLAYER_NONTARGETABLE_JUMPIN_DELAY;
   pilot_setFlag( player.p, PILOT_NONTARGETABLE );

   /* Update the map */
   map_jump();

   /* Add persisted pilots */
   pilot_stack = pilot_getAll();
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];
      if (pilot_isFlag(p, PILOT_PERSIST) || pilot_isFlag(p, PILOT_PLAYER)) {
         pilot_clearHooks(p);
         ai_cleartasks(p);
         if (p != player.p)
            space_calcJumpInPos( cur_system, sys, &p->solid->pos, &p->solid->vel, &p->solid->dir, player.p );
         /* Run Lua stuff for all persistant pilots. */
         pilot_outfitLInitAll( p );
      }
   }

   /* Disable autonavigation if arrived. */
   if (player_isFlag(PLAYER_AUTONAV)) {
      if (player.p->nav_hyperspace == -1) {
         player_message( _("#oAutonav arrived at the %s system."), _(cur_system->name) );
         player_autonavEnd();
      }
      else {
         destsys = map_getDestination( &map_npath );
         player_message( n_(
                  "#oAutonav continuing until %s (%d jump left).",
                  "#oAutonav continuing until %s (%d jumps left).",
                  map_npath),
               (sys_isKnown(destsys) ? _(destsys->name) : _("Unknown")),
               map_npath );
      }
   }

   /* Update lua stuff. */
   pilot_outfitLInitAll( player.p );

   /* Safe since this is run in the player hook section. */
   pilot_outfitLOnjumpin( player.p );
   hooks_run( "jumpin" );
   hooks_run( "enter" );
   events_trigger( EVENT_TRIGGER_ENTER );
   missions_run( MIS_AVAIL_SPACE, -1, NULL, NULL );

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

   /* The player should not continue following if the target pilot has been changed. */
   if (player_isFlag(PLAYER_AUTONAV) && player.autonav == AUTONAV_PLT_FOLLOW)
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
   if ((player.p->target == PLAYER_ID) && (player.p->nav_spob < 0)
         && (preemption == 1 || player.p->nav_spob == -1)
         && !pilot_isFlag(player.p, PILOT_HYP_PREP)) {
      player.p->nav_hyperspace = -1;
      player_hyperspacePreempt(0);
      map_clear();
   }
   else if (player.p->target == PLAYER_ID)
      player_targetSpobSet( -1 );
   else
      player_targetSet( PLAYER_ID );
   gui_setNav();
}

/**
 * @brief Clears all player targets: hyperspace, spob, asteroid, etc...
 */
void player_targetClearAll (void)
{
   player_targetSpobSet( -1 );
   player_targetHyperspaceSet( -1 );
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
   double d;

   d = pilot_getNearestPos( player.p, &dt, player.p->solid->pos.x,
         player.p->solid->pos.y, 1 );
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
      Pilot *p = pilot_stack[i];

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
   Spob *spob = cur_system->spobs[player.p->nav_spob];
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
      player_message(_("#rNo target selected to hail."));

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
      player_message(_("#rNo target selected to hail."));
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
      Pilot *p = pilot_stack[i];

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

   player_message(_("#rYou haven't been hailed by any pilots."));
}

/**
 * @brief Toggles mouse flying.
 */
void player_toggleMouseFly(void)
{
   if (!conf.mouse_fly)
      return;

   if (!player_isFlag(PLAYER_MFLY)) {
      input_mouseShow();
      player_message(_("#oMouse flying enabled."));
      player_setFlag(PLAYER_MFLY);
   }
   else {
      input_mouseHide();
      player_rmFlag(PLAYER_MFLY);
      player_message(_("#rMouse flying disabled."));

      if (conf.mouse_thrust)
         player_accelOver();
   }
}

/**
 * @brief Starts braking or active cooldown.
 */
void player_brake(void)
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
 *    @return 1 if cursor is outside the dead zone, 0 if it isn't.
 */
static int player_thinkMouseFly(void)
{
   double px, py, r, x, y;

   px = player.p->solid->pos.x;
   py = player.p->solid->pos.y;
   gl_screenToGameCoords( &x, &y, player.mousex, player.mousey );
   r = sqrt(pow2(x-px) + pow2(y-py));
   if (r > 50.) { /* Ignore mouse input within a 50 px radius of the centre. */
      pilot_face(player.p, atan2( y - py, x - px));
      if (conf.mouse_thrust) { /* Only alter thrust if option is enabled. */
         double acc = CLAMP(0., 1., (r - 100.) / 200.);
         acc = 3. * pow2(acc) - 2. * pow(acc, 3.);
         /* Only accelerate when within 180 degrees of the intended direction. */
         if (ABS(angle_diff(atan2( y - py, x - px), player.p->solid->dir)) < M_PI_2 )
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

   /* Get prices. */
   p1 = pilot_worth( ps1->p );
   p2 = pilot_worth( ps2->p );

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
int player_getOutfitsFiltered( const Outfit **outfits,
      int(*filter)( const Outfit *o ), const char *name )
{
   if (array_size(player_outfits) == 0)
      return 0;

   /* We'll sort. */
   qsort( player_outfits, array_size(player_outfits),
         sizeof(PlayerOutfit_t), player_outfitCompare );

   for (int i=0; i<array_size(player_outfits); i++)
      outfits[i] = (Outfit*)player_outfits[i].o;

   return outfits_filter( outfits, array_size(player_outfits), filter, name );
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
   /* Make sure not already done. */
   if (player_eventAlreadyDone(id))
      return;

   /* Mark as done. */
   if (events_done == NULL)
      events_done = array_create( int );
   array_push_back( &events_done, id );

   qsort( events_done, array_size(events_done), sizeof(int), cmp_int );
}

/**
 * @brief Checks to see if player has already completed a event.
 *
 *    @param id ID of the event to see if player has completed.
 *    @return 1 if player has completed the event, 0 otherwise.
 */
int player_eventAlreadyDone( int id )
{
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
      player_brokeHyperspace();
      player_rmFlag( PLAYER_HOOK_HYPER );
   }
   if (player_isFlag( PLAYER_HOOK_JUMPIN)) {
      pilot_outfitLOnjumpin( player.p );
      hooks_run( "jumpin" );
      hooks_run( "enter" );
      events_trigger( EVENT_TRIGGER_ENTER );
      missions_run( MIS_AVAIL_SPACE, -1, NULL, NULL );
      player_rmFlag( PLAYER_HOOK_JUMPIN );
   }
   if (player_isFlag( PLAYER_HOOK_LAND )) {
      land( cur_system->spobs[ player.p->nav_spob ], 0 );
      player_rmFlag( PLAYER_HOOK_LAND );
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

   for (int i=0; i<array_size(player.p->escorts); i++) {
      double a;
      Vector2d v;
      unsigned int e;
      int dockslot = -1;

      if (!player.p->escorts[i].persist) {
         escort_rmListIndex(player.p, i);
         i--;
         continue;
      }

      a = RNGF() * 2 * M_PI;
      vect_cset( &v, player.p->solid->pos.x + 50.*cos(a),
            player.p->solid->pos.y + 50.*sin(a) );

      /* Update outfit if needed. */
      if (player.p->escorts[i].type != ESCORT_TYPE_BAY)
         continue;

      for (int j=0; j<array_size(player.p->outfits); j++) {
         int q;
         const PilotOutfitSlot *po = player.p->outfits[j];

         /* Must have outfit. */
         if (po->outfit == NULL)
            continue;

         /* Must be fighter bay. */
         if (!outfit_isFighterBay(po->outfit))
            continue;

         /* Must not have all deployed. */
         q = po->u.ammo.deployed + po->u.ammo.quantity;
         if (q >= outfit_amount(po->outfit))
            continue;

         dockslot = j;
         break;
      }

      if (dockslot == -1)
         DEBUG(_("Escort is undeployed"));

      /* Create escort. */
      e = escort_create( player.p, player.p->escorts[i].ship,
            &v, &player.p->solid->vel, player.p->solid->dir,
            player.p->escorts[i].type, 0, dockslot );
      player.p->escorts[i].id = e; /* Important to update ID. */
   }

   return 0;
}

/**
 * @brief Saves the player's escorts.
 */
static int player_saveEscorts( xmlTextWriterPtr writer )
{
   for (int i=0; i<array_size(player.p->escorts); i++) {
      if (!player.p->escorts[i].persist)
         continue;
      xmlw_startElem(writer, "escort");
      xmlw_attr(writer,"type","bay"); /**< @todo other types. */
      xmlw_str(writer, "%s", player.p->escorts[i].ship);
      xmlw_endElem(writer); /* "escort" */
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
   char **guis;
   int cycles, periods, seconds;
   double rem;

   xmlw_startElem(writer,"player");

   /* Standard player details. */
   xmlw_attr(writer,"name","%s",player.name);
   xmlw_elem(writer,"dt_mod","%f",player.dt_mod);
   xmlw_elem(writer,"credits","%"CREDITS_PRI,player.p->credits);
   xmlw_elem(writer,"chapter","%s",player.chapter);
   if (player.gui != NULL)
      xmlw_elem(writer,"gui","%s",player.gui);
   xmlw_elem(writer,"guiOverride","%d",player.guiOverride);
   xmlw_elem(writer,"mapOverlay","%d",ovr_isOpen());
   gui_radarGetRes( &player.radar_res );
   xmlw_elem(writer,"radar_res","%f",player.radar_res);
   xmlw_elem(writer,"eq_outfitMode","%d",player.eq_outfitMode);
   xmlw_elem(writer,"map_minimal","%d",player.map_minimal);

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
      /* Remove cargo with id and no mission. */
      if (ship->commodities[i].id > 0) {
         int found = 0;
         for (int j=0; j<MISSION_MAX; j++) {
            /* Only check active missions. */
            if (player_missions[j]->id > 0) {
               /* Now check if it's in the cargo list. */
               for (int k=0; k<array_size(player_missions[j]->cargo); k++) {
                  /* See if it matches a cargo. */
                  if (player_missions[j]->cargo[k] == ship->commodities[i].id) {
                     found = 1;
                     break;
                  }
               }
            }
            if (found)
               break;
         }

         if (!found) {
            WARN(_("Found mission cargo without associated mission."));
            WARN(_("Please reload save game to remove the dead cargo."));
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

   xmlw_startElem(writer, "weaponsets");
   xmlw_attr(writer, "autoweap", "%d", ship->autoweap);
   xmlw_attr(writer, "active_set", "%d", ship->active_set);
   xmlw_attr(writer, "aim_lines", "%d", ship->aimLines);
   for (int i=0; i<PILOT_WEAPON_SETS; i++) {
      PilotWeaponSetOutfit *weaps = pilot_weapSetList( ship, i );
      xmlw_startElem(writer,"weaponset");
      /* Inrange isn't handled by autoweap for the player. */
      xmlw_attr(writer,"inrange","%d",pilot_weapSetInrangeCheck(ship,i));
      xmlw_attr(writer,"id","%d",i);
      if (!ship->autoweap) {
         const char *name = pilot_weapSetName(ship,i);
         if (name != NULL)
            xmlw_attr(writer,"name","%s",name);
         xmlw_attr(writer,"type","%d",pilot_weapSetTypeCheck(ship,i));
         for (int j=0; j<array_size(weaps); j++) {
            xmlw_startElem(writer,"weapon");
            xmlw_attr(writer,"level","%d",weaps[j].level);
            xmlw_str(writer,"%d",weaps[j].slot->id);
            xmlw_endElem(writer); /* "weapon" */
         }
      }
      xmlw_endElem(writer); /* "weaponset" */
   }
   xmlw_endElem(writer); /* "weaponsets" */

   /* Ship variables. */
   xmlw_startElem(writer, "vars");
   lvar_save( pship->shipvar, writer );
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
   player.speed = 1.;
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
      player_updater_env = nlua_newEnv(1);
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
   const char *spob;
   unsigned int services;
   Spob *pnt;
   xmlNodePtr node, cur;
   int map_overlay_enabled;
   StarSystem *sys;
   double a, r;
   Pilot *old_ship;
   PilotFlags flags;
   int time_set;

   xmlr_attr_strd(parent, "name", player.name);

   /* Make sure player.p is NULL. */
   player.p = NULL;
   pnt = NULL;

   /* Safe defaults. */
   spob        = NULL;
   time_set    = 0;
   map_overlay_enabled = 0;
   player_ran_updater = 0;

   player.dt_mod = 1.; /* For old saves. */
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
      xmlr_float(node, "dt_mod", player.dt_mod);
      xmlr_ulong(node, "credits", player_creds);
      xmlr_strd(node, "gui", player.gui);
      xmlr_int(node, "guiOverride", player.guiOverride);
      xmlr_int(node, "mapOverlay", map_overlay_enabled);
      xmlr_float(node, "radar_res", player.radar_res);
      xmlr_int(node, "eq_outfitMode", player.eq_outfitMode);
      xmlr_int(node, "map_minimal", player.map_minimal);

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

   } while (xml_nextNode(node));

   /* Handle cases where ship is missing. */
   if (player.p == NULL) {
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
         old_ship = player_stack[array_size(player_stack)-1].p;
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
   player.speed = 1.;

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
         services = SPOB_SERVICE_LAND | SPOB_SERVICE_INHABITED | SPOB_SERVICE_REFUEL;

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
   sys = system_get( spob_getSystem( pnt->name ) );
   space_gfxLoad( sys );
   a = RNGF() * 2.*M_PI;
   r = RNGF() * pnt->radius * 0.8;
   player_warp( pnt->pos.x + r*cos(a), pnt->pos.y + r*sin(a) );
   player.p->solid->dir = RNG(0,359) * M_PI/180.;

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
      if (xml_isNode(node,"done")) {
         int id = mission_getID( xml_get(node) );
         if (id < 0)
            DEBUG(_("Mission '%s' doesn't seem to exist anymore, removing from save."),
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
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"done")) {
         int id = event_dataID( xml_get(node) );
         if (id < 0)
            DEBUG(_("Event '%s' doesn't seem to exist anymore, removing from save."),
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
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      if ( xml_isNode( node, "license" ) ) {
         char *name = xml_get( node );
         if (name == NULL) {
            WARN( _( "License node is missing name attribute." ) );
            continue;
         }
         player_tryAddLicense( name );
      }
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
      if (xml_isNode(node,"escort")) {
         char *buf, *ship;
         EscortType_t type;

         xmlr_attr_strd( node, "type", buf );
         if (strcmp(buf,"bay")==0)
            type = ESCORT_TYPE_BAY;
         else {
            WARN(_("Escort has invalid type '%s'."), buf);
            type = ESCORT_TYPE_NULL;
         }
         free(buf);

         ship = xml_get(node);

         /* Add escort to the list. */
         escort_addList( player.p, ship, type, 0, 1 );
      }
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

   char *name = xml_get(node);
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
   int n, id;
   int fuel;
   const Ship *ship_parsed;
   Pilot* ship;
   xmlNodePtr node;
   int quantity;
   Commodity *com;
   PilotFlags flags;
   int autoweap, level, weapid, active_set, aim_lines, in_range, weap_type;
   PlayerShip_t ps;

   memset( &ps, 0, sizeof(PlayerShip_t) );

   /* Parse attributes. */
   xmlr_attr_strd( parent, "name", name );
   xmlr_attr_strd( parent, "model", model );
   xmlr_attr_int_def( parent, "favourite", ps.favourite, 0 );

   /* Safe defaults. */
   pilot_clearFlagsRaw( flags );
   pilot_setFlagRaw( flags, PILOT_PLAYER );
   pilot_setFlagRaw( flags, PILOT_NO_OUTFITS );

   /* Get the ship. */
   ship_parsed = ship_get(model);
   if (ship_parsed == NULL) {
      WARN(_("Player ship '%s' not found!"), model);

      /* Clean up. */
      free(name);
      free(model);

      return -1;
   }

   /* Add GUI if applicable. */
   player_guiAdd( ship_parsed->gui );

   /* Player is currently on this ship */
   if (is_player != 0) {
      unsigned int pid = pilot_create( ship_parsed, name, faction_get("Player"), "player", 0., NULL, NULL, flags, 0, 0 );
      ship = player.p;
      cam_setTargetPilot( pid, 0 );
   }
   else
      ship = pilot_createEmpty( ship_parsed, name, faction_get("Player"), "player", flags );
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
            if (o!=NULL)
               pilot_addOutfitIntrinsic( ship, o );
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node, "commodities")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur, "commodity")) {
               int cid;

               xmlr_attr_int( cur, "quantity", quantity );
               xmlr_attr_int( cur, "id", cid );

               /* Get the commodity. */
               com = commodity_get(xml_get(cur));
               if (com == NULL) {
                  WARN(_("Unknown commodity '%s' detected, removing."), xml_get(cur));
                  continue;
               }

               /* actually add the cargo with id hack
                * Note that the player's cargo_free is ignored here.
                */
               pilot_cargoAddRaw( ship, com, quantity, 0 );
               if (cid != 0)
                  array_back(ship->commodities).id = cid;
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
    * str = pilot_checkSpaceworthy( ship ); */
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
         ps.shipvar = lvar_load( node );
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

         /* Set inrange mode. */
         xmlr_attr_int( cur, "inrange", in_range );
         if (in_range > 0)
            pilot_weapSetInrange( ship, id, in_range );

         if (autoweap) /* Autoweap handles everything except inrange. */
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

   /* Set aimLines */
   ship->aimLines = aim_lines;

   /* Add it to the stack if it's not what the player is in */
   if (is_player == 0)
      array_push_back( &player_stack, ps );
   else
      player.ps = ps;

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
      player_message(_("#gYou have entered stealth mode."));
   }
   else {
      /* Stealth failed. */
      if (player.p->lockons > 0)
         player_message(_("#rUnable to stealth: missiles locked on!"));
      else
         player_message(_("#rUnable to stealth: other pilots nearby!"));
   }
}
