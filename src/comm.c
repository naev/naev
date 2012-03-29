/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file comm.c
 *
 * @brief For communicating with planets/pilots.
 */


#include "comm.h"

#include "naev.h"

#include "log.h"
#include "toolkit.h"
#include "dialogue.h"
#include "pilot.h"
#include "rng.h"
#include "nlua.h"
#include "player.h"
#include "opengl.h"
#include "ai.h"
#include "ai_extra.h"
#include "hook.h"

#define COMM_WDWNAME    "Communication Channel" /**< Map window name. */

#define BUTTON_WIDTH    80 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */

#define GRAPHIC_WIDTH  256 /**< Width of graphic. */
#define GRAPHIC_HEIGHT 256 /**< Height of graphic. */


static Pilot *comm_pilot       = NULL; /**< Pilot currently talking to. */
static Planet *comm_planet     = NULL; /**< Planet currently talking to. */
static glTexture *comm_graphic = NULL; /**< Pilot's graphic. */
static int comm_commClose      = 0; /**< Close comm when done. */


/* We need direct pilot access. */
extern Pilot** pilot_stack;
extern int pilot_nstack;


/*
 * Prototypes.
 */
/* Static. */
static unsigned int comm_open( glTexture *gfx, int faction,
      int override, int bribed, char *name );
static unsigned int comm_openPilotWindow (void);
static void comm_addPilotSpecialButtons( unsigned int wid );
static void comm_close( unsigned int wid, char *unused );
static void comm_bribePilot( unsigned int wid, char *unused );
static void comm_bribePlanet( unsigned int wid, char *unused );
static void comm_requestFuel( unsigned int wid, char *unused );
static int comm_getNumber( double *val, char* str );
static const char* comm_getString( char *str );


/**
 * @brief Checks to see if comm is open.
 *
 *    @return 1 if comm is open.
 */
int comm_isOpen (void)
{
   return window_exists( COMM_WDWNAME );
}


/**
 * @brief Queues a close command when possible.
 */
void comm_queueClose (void)
{
   comm_commClose = 1;
}


/**
 * @brief Opens the communication dialogue with a pilot.
 *
 *    @param pilot Pilot to communicate with.
 *    @return 0 on success.
 */
int comm_openPilot( unsigned int pilot )
{
   const char *msg;
   char c;
   unsigned int wid;
   int run;
   Pilot *p;
   HookParam hparam[2];

   /* Get the pilot. */
   p           = pilot_get( pilot );
   comm_pilot  = p;
   c = pilot_getFactionColourChar( p );

   /* Make sure pilot exists. */
   if (comm_pilot == NULL)
      return -1;

   /* Make sure pilot in range. */
   if (!pilot_isFlag(p, PILOT_HAILING) &&
         pilot_inRangePilot( player.p, comm_pilot ) <= 0) {
      player_message("\erTarget is out of communications range.");
      comm_pilot = NULL;
      return -1;
   }

   /* Destroy the window if it's already present. */
   wid = window_get(COMM_WDWNAME);
   if (wid > 0) {
      window_destroy( wid );
      return 0;
   }

   /* Must not be jumping. */
   if (pilot_isFlag(comm_pilot, PILOT_HYPERSPACE)) {
      player_message("\e%c%s\er is jumping and can't respond.", c, comm_pilot->name);
      return 0;
   }

   /* Must not be disabled. */
   if (pilot_isFlag(comm_pilot, PILOT_DISABLED)) {
      player_message("\e%c%s\er does not respond.", c, comm_pilot->name);
      return 0;
   }

   /* Check to see if pilot wants to communicate. */
   msg = comm_getString( "comm_no" );
   if (msg != NULL) {
      player_message( msg );
      return 0;
   }

   /* Set up for the comm_get* functions. */
   ai_setPilot( comm_pilot );

   /* Have pilot stop hailing. */
   pilot_rmFlag( comm_pilot, PILOT_HAILING );

   /* Don't close automatically. */
   comm_commClose = 0;

   /* Run generic hail hooks. */
   hparam[0].type       = HOOK_PARAM_PILOT;
   hparam[0].u.lp.pilot = p->id;
   hparam[1].type       = HOOK_PARAM_SENTINEL;
   run = 0;
   run += hooks_runParam( "hail", hparam );
   run += pilot_runHook( comm_pilot, PILOT_HOOK_HAIL );

   /* Close window if necessary. */
   if (comm_commClose) {
      comm_pilot  = NULL;
      comm_planet = NULL;
      comm_commClose = 0;
      return 0;
   }

   /* Create the pilot window. */
   wid = comm_openPilotWindow();

   return 0;
}


/**
 * @brief Creates the pilot window.
 */
static unsigned int comm_openPilotWindow (void)
{
   unsigned int wid;

   /* Create the generic comm window. */
   wid = comm_open( ship_loadCommGFX( comm_pilot->ship ),
         comm_pilot->faction,
         pilot_isHostile(comm_pilot) ? -1 : pilot_isFriendly(comm_pilot) ? 1 : 0,
         pilot_isFlag(comm_pilot, PILOT_BRIBED),
         comm_pilot->name );

   /* Add special buttons. */
   comm_addPilotSpecialButtons( wid );

   return wid;
}


/**
 * @brief Adds the pilot special buttons to a window.
 *
 *    @param wid Window to add pilot special buttons to.
 */
static void comm_addPilotSpecialButtons( unsigned int wid )
{
   window_addButton( wid, -20, 20 + BUTTON_HEIGHT + 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnGreet", "Greet", NULL );
   if (!pilot_isFlag(comm_pilot, PILOT_BRIBED) && /* Not already bribed. */
         ((faction_getPlayer( comm_pilot->faction ) < 0) || /* Hostile. */
            pilot_isHostile(comm_pilot)))
      window_addButton( wid, -20, 20 + 2*BUTTON_HEIGHT + 40,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnBribe", "Bribe", comm_bribePilot );
   else
      window_addButton( wid, -20, 20 + 2*BUTTON_HEIGHT + 40,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnRequest",
            "Refuel", comm_requestFuel );
}


/**
 * @brief Opens a communication dialogue with a planet.
 *
 *    @param planet Planet to communicate with.
 *    @return 0 on success.
 */
int comm_openPlanet( Planet *planet )
{
   unsigned int wid;

   /* Destroy the window if it's already present. */
   wid = window_get(COMM_WDWNAME);
   if (wid > 0) {
      window_destroy( wid );
      return 0;
   }

   /* Must not be disabled. */
   if (!planet_hasService(planet, PLANET_SERVICE_INHABITED)) {
      player_message("%s does not respond.", planet->name);
      return 0;
   }

   /* Make sure planet in range. */
   /* Function uses planet index in local system, so I moved this to player.c.
   if ( pilot_inRangePlanet( player.p, planet->id ) <= 0 ) {
      player_message("\erTarget is out of communications range.");
      comm_planet = NULL;
      return 0;
   }
   */

   comm_planet = planet;

   /* Create the generic comm window. */
   wid = comm_open( gl_dupTexture( comm_planet->gfx_space ),
         comm_planet->faction, 0, 0, comm_planet->name );

   /* Add special buttons. */
   if (!planet->can_land && !planet->bribed && (planet->bribe_msg != NULL))
      window_addButton( wid, -20, 20 + BUTTON_HEIGHT + 20,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnBribe", "Bribe", comm_bribePlanet );

   return 0;
}


/**
 * @brief Sets up the comm window.
 *
 *    @param gfx Graphic to use for the comm window (is freed).
 *    @param faction Faction of what you're communicating with.
 *    @param override If positive sets to ally, if negative sets to hostile.
 *    @param bribed Whether or not the target is bribed.
 *    @param name Name of object talking to.
 *    @return The comm window id.
 */
static unsigned int comm_open( glTexture *gfx, int faction,
      int override, int bribed, char *name )
{
   int namex, standx, logox, y;
   int namew, standw, logow, width;
   glTexture *logo;
   char *stand;
   unsigned int wid;
   const glColour *c;
   glFont *font;
   int gw, gh;
   double aspect;

   /* Clean up. */
   if (comm_graphic != NULL) {
      /* First clean up if needed. */
      gl_freeTexture(comm_graphic);
      comm_graphic = NULL;
   }

   /* Get faction details. */
   comm_graphic   = gfx;
   logo           = faction_logoSmall(faction);

   /* Get standing colour / text. */
   if (bribed) {
      stand = "Neutral";
      c     = &cNeutral;
   }
   else if (override < 0) {
      stand = "Hostile";
      c     = &cHostile;
   }
   else if (override > 0) {
      stand = "Friendly";
      c     = &cFriend;
   }
   else {
      stand = faction_getStandingBroad(faction_getPlayer(faction));
      c     = faction_getColour( faction );
   }

   namew  = gl_printWidthRaw( NULL, name );
   standw = gl_printWidthRaw( NULL, stand );
   width  = MAX(namew, standw);

   logow = logo == NULL ? 0 : logo->w;

   if (width + logow > GRAPHIC_WIDTH) {
      font = &gl_smallFont;
      namew  = MIN(gl_printWidthRaw( font, name ), GRAPHIC_WIDTH - logow);
      standw = MIN(gl_printWidthRaw( font, stand ), GRAPHIC_WIDTH - logow);
      width  = MAX(namew, standw);
   }
   else
      font = &gl_defFont;

   namex  = GRAPHIC_WIDTH/2 -  namew/2 + logow/2;
   standx = GRAPHIC_WIDTH/2 - standw/2 + logow/2;

   if (logo != NULL) {
      y  = MAX( font->h*2 + 15, logo->h );
      logox = GRAPHIC_WIDTH/2 - logow/2 - width/2 - 2;
   }
   else {
      logox = 0;
      y = font->h*2 + 15;
   }

   /* Create the window. */
   wid = window_create( COMM_WDWNAME, -1, -1,
         20 + GRAPHIC_WIDTH + 20 + BUTTON_WIDTH + 20,
         30 + GRAPHIC_HEIGHT + y + 5 + 20 );
   window_setCancel( wid, comm_close );

   /* Create the image. */
   window_addRect( wid, 19, -30, GRAPHIC_WIDTH+1, GRAPHIC_HEIGHT + y + 5,
         "rctGFX", &cGrey10, 1 );

   if (comm_graphic != NULL) {
      aspect = comm_graphic->w / comm_graphic->h;
      gw = MIN( GRAPHIC_WIDTH,  comm_graphic->w );
      gh = MIN( GRAPHIC_HEIGHT, comm_graphic->h );
   
      if (comm_graphic->w > GRAPHIC_WIDTH || comm_graphic->h > GRAPHIC_HEIGHT) {
         gh = MIN( GRAPHIC_HEIGHT, GRAPHIC_HEIGHT / aspect );
         gw = MIN( GRAPHIC_WIDTH, GRAPHIC_WIDTH * aspect );
      }

      window_addImage( wid, 20 + (GRAPHIC_WIDTH-gw)/2,
            -30 - (GRAPHIC_HEIGHT-gh)/2,
            gw, gh, "imgGFX", comm_graphic, 0 );
   }
   else
      gh = gw = 0;

   /* Faction logo. */
   if (logo != NULL) {
      window_addImage( wid, 19 + logox, -30 - GRAPHIC_HEIGHT - 4,
            0, 0, "imgFaction", logo, 0 );
      y -= (logo->h - (gl_defFont.h*2 + 15)) / 2;
   }

   /* Name. */
   window_addText( wid, 19 + namex, -30 - GRAPHIC_HEIGHT - y + font->h*2 + 10,
         GRAPHIC_WIDTH - logow, 20, 0, "txtName", font, &cDConsole, name );

   /* Standing. */
   window_addText( wid, 19 + standx, -30 - GRAPHIC_HEIGHT - y + font->h + 5,
         GRAPHIC_WIDTH - logow, 20, 0, "txtStanding", font, c, stand );

   /* Buttons. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", comm_close );

   return wid;
}


/**
 * @brief Closes the comm window.
 *
 *    @param wid ID of window calling the function.
 *    @param unused Unused.
 */
static void comm_close( unsigned int wid, char *unused )
{
   /* Clean up a bit after ourselves. */
   if (comm_graphic != NULL) {
      gl_freeTexture(comm_graphic);
      comm_graphic = NULL;
   }
   comm_pilot  = NULL;
   comm_planet = NULL;
   /* Close the window. */
   window_close( wid, unused );
}


/**
 * @brief Tries to bribe the pilot.
 *
 *    @param wid ID of window calling the function.
 *    @param unused Unused.
 */
static void comm_bribePilot( unsigned int wid, char *unused )
{
   (void) unused;
   int answer;
   double d;
   credits_t price;
   const char *str;
   lua_State *L;

   /* Unbribable. */
   str = comm_getString( "bribe_no" );
   if (str != NULL) {
      dialogue_msg("Bribe Pilot", "%s", str );
      return;
   }

   /* Get amount pilot wants. */
   if (comm_getNumber( &d, "bribe" )) {
      WARN("Pilot '%s' accepts bribes but doesn't give price!", comm_pilot->name );
      d = 0.;
   }
   price = (credits_t) d;

   /* Check to see if already bribed. */
   if (price == 0) {
      dialogue_msg("Bribe Pilot", "\"Money won't save your hide now!\"");
      return;
   }

   /* Bribe message. */
   str = comm_getString( "bribe_prompt" );
   if (str == NULL)
      answer = dialogue_YesNo( "Bribe Pilot", "\"I'm gonna need at least %"CREDITS_PRI" credits to not leave you as a hunk of floating debris.\"\n\nPay %"CREDITS_PRI" credits?", price, price );
   else
      answer = dialogue_YesNo( "Bribe Pilot", "%s\n\nPay %"CREDITS_PRI" credits?", str, price );

   /* Said no. */
   if (answer == 0) {
      dialogue_msg("Bribe Pilot", "You decide not to pay.");
      return;
   }

   /* Check if has the money. */
   if (!player_hasCredits( price )) {
      dialogue_msg("Bribe Pilot", "You don't have enough credits for the bribery.");
      return;
   }

   player_modCredits( -price );
   str = comm_getString( "bribe_paid" );
   if (str == NULL)
      dialogue_msg("Bribe Pilot", "\"Pleasure to do business with you.\"");
   else
      dialogue_msg("Bribe Pilot", "%s", str);

   /* Mark as bribed and don't allow bribing again. */
   pilot_setFlag( comm_pilot, PILOT_BRIBED );
   pilot_rmHostile( comm_pilot );

   /* Stop hyperspace if necessary. */
   pilot_rmFlag( comm_pilot, PILOT_HYP_PREP );
   pilot_rmFlag( comm_pilot, PILOT_HYP_BRAKE );
   pilot_rmFlag( comm_pilot, PILOT_HYP_BEGIN );

   /* Don't allow rebribe. */
   if (comm_pilot->ai != NULL) {
      L = comm_pilot->ai->L;
      lua_getglobal(L, "mem");
      lua_pushnumber(L, 0);
      lua_setfield(L, -2, "bribe");
      lua_pop(L,1);
   }

   /* Reopen window. */
   window_destroy( wid );
   comm_openPilot( comm_pilot->id );
}


/**
 * @brief Tries to bribe the planet
 *
 *    @param wid ID of window calling the function.
 *    @param unused Unused.
 */
static void comm_bribePlanet( unsigned int wid, char *unused )
{
   (void) unused;
   int answer;
   credits_t price;

   /* Get price. */
   price = comm_planet->bribe_price;

   /* No bribing. */
   if (comm_planet->bribe_price <= 0.) {
      dialogue_msg( "Bribe Starport", comm_planet->bribe_msg );
      return;
   }

   /* Yes/No input. */
   answer = dialogue_YesNo( "Bribe Starport", comm_planet->bribe_msg );

   /* Said no. */
   if (answer == 0) {
      dialogue_msg("Bribe Starport", "You decide not to pay.");
      return;
   }

   /* Check if has the money. */
   if (!player_hasCredits( price )) {
      dialogue_msg("Bribe Starport", "You don't have enough credits for the bribery.");
      return;
   }

   /* Pay the money. */
   player_modCredits( -price );
   dialogue_msg("Bribe Starport", "You have permission to dock.");

   /* Mark as bribed and don't allow bribing again. */
   comm_planet->bribed = 1;

   /* Reopen window. */
   window_destroy( wid );
   comm_openPlanet( comm_planet );
}


/**
 * @brief Tries to request help from the pilot.
 *
 *    @param wid ID of window calling the function.
 *    @param unused Unused.
 */
static void comm_requestFuel( unsigned int wid, char *unused )
{
   (void) wid;
   (void) unused;
   double val;
   const char *msg;
   int ret;
   credits_t price;

   /* Check to see if ship has a no refuel message. */
   msg = comm_getString( "refuel_no" );
   if (msg != NULL) {
      dialogue_msg( "Request Fuel", msg );
      return;
   }

   /* Must need refueling. */
   if (player.p->fuel >= player.p->fuel_max) {
      dialogue_msg( "Request Fuel", "Your fuel deposits are already full." );
      return;
   }

   /* See if pilot has enough fuel. */
   if (comm_pilot->fuel < 200.) {
      dialogue_msg( "Request Fuel",
            "\"Sorry, I don't have enough fuel to spare at the moment.\"" );
      return;
   }

   /* See if player can get refueled. */
   ret = comm_getNumber( &val, "refuel" );
   msg = comm_getString( "refuel_msg" );
   if ((ret != 0) || (msg == NULL) || pilot_isFlag(comm_pilot, PILOT_MANUAL_CONTROL)) {
      dialogue_msg( "Request Fuel", "\"Sorry, I'm busy now.\"" );
      return;
   }
   price = (credits_t) val;

   /* Check to see if is already refueling. */
   if (pilot_isFlag(comm_pilot, PILOT_REFUELING)) {
      dialogue_msg( "Request Fuel", "Pilot is already refueling you." );
      return;
   }

   /* See if player really wants to pay. */
   if (price > 0) {
      ret = dialogue_YesNo( "Request Fuel", "%s\n\nPay %"CREDITS_PRI" credits?", msg, price );
      if (ret == 0) {
         dialogue_msg( "Request Fuel", "You decide not to pay." );
         return;
      }
   }
   else
      dialogue_msg( "Request Fuel", "%s", msg );

   /* Check if he has the money. */
   if (!player_hasCredits( price )) {
      dialogue_msg( "Request Fuel", "You need %"CREDITS_PRI" more credits!",
            price - player.p->credits);
      return;
   }

   /* Take money. */
   player_modCredits( -price );
   pilot_modCredits( comm_pilot, price );

   /* Start refueling. */
   pilot_rmFlag(  comm_pilot, PILOT_HYP_PREP);
   pilot_rmFlag(  comm_pilot, PILOT_HYP_BRAKE );
   pilot_rmFlag(  comm_pilot, PILOT_HYP_BEGIN);
   pilot_setFlag( comm_pilot, PILOT_REFUELING);
   ai_refuel(     comm_pilot, player.p->id );

   /* Last message. */
   if (price > 0)
      dialogue_msg( "Request Fuel", "\"On my way.\"" );
}


/**
 * @brief Gets the amount the communicating pilot wants as a bribe.
 *
 * Valid targets for now are:
 *    - "bribe": amount pilot wants to be paid.
 *    - "refuel": amount pilot wants to be paid for refueling the player.
 *
 *    @param[out] val Value of the number gotten.
 *    @param str Name of number to get.
 *    @return 0 for success, 1 on error (including not found).
 */
static int comm_getNumber( double *val, char* str )
{
   int ret;
   lua_State *L;

   if (comm_pilot->ai == NULL)
      return 1;

   /* Set up the state. */
   L = comm_pilot->ai->L;
   lua_getglobal( L, "mem" );

   /* Get number amount. */
   lua_getfield( L, -1, str );
   /* Check to see if it's a number. */
   if (!lua_isnumber(L, -1))
      ret = 1;
   else {
      *val = lua_tonumber(L, -1);
      ret = 0;
   }
   /* Clean up. */
   lua_pop(L, 2);
   return ret;
}


/**
 * @brief Gets a string from the pilot's memory.
 *
 * Valid targets are:
 *    - comm_no: message of communication failure.
 *    - bribe_no: unbribe message
 *    - bribe_prompt: bribe prompt
 *    - bribe_paid: paid message
 *
 *    @param str String to get.
 *    @return String matching str.
 */
static const char* comm_getString( char *str )
{
   lua_State *L;
   const char *ret;

   if (comm_pilot->ai == NULL)
      return NULL;

   /* Get memory table. */
   L = comm_pilot->ai->L;
   lua_getglobal( L, "mem" );

   /* Get str message. */
   lua_getfield(L, -1, str );
   if (!lua_isstring(L, -1))
      ret = NULL;
   else
      ret = lua_tostring(L, -1);
   lua_pop(L, 2);

   return ret;
}

