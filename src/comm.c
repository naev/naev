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


#define BUTTON_WIDTH    80 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */


static Pilot *comm_pilot       = NULL; /**< Pilot currently talking to. */
static glTexture *comm_graphic = NULL; /**< Pilot's graphic. */


/*
 * Prototypes.
 */
/* Static. */
static void comm_close( unsigned int wid, char *unused );
static void comm_bribe( unsigned int wid, char *unused );
static void comm_requestFuel( unsigned int wid, char *unused );
static int comm_getNumber( double *val, char* str );
static const char* comm_getString( char *str );
/* Extern. */
extern void ai_refuel( Pilot* refueler, unsigned int target ); /**< ai.c */
extern void ai_setPilot( Pilot *p ); /**< from ai.c */


/**
 * @brief Opens the communication dialogue with a pilot.
 *
 *    @param pilot Pilot to communicate with.
 *    @return 0 on success.
 */
int comm_open( unsigned int pilot )
{
   int x,y, w;
   glTexture *logo;
   char *name, *stand;
   const char *msg;
   unsigned int wid;
   glColour *c;

   /* Get the pilot. */
   comm_pilot = pilot_get( pilot );
   if (comm_pilot == NULL)
      return -1;
  
   /* Must not be disabled. */
   if (pilot_isFlag(comm_pilot, PILOT_DISABLED)) {
      player_message("%s does not respond.", comm_pilot->name);
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

   /* Get graphics and text. */
   if (comm_graphic != NULL) {
      /* First clean up if needed. */
      gl_freeTexture(comm_graphic);
      comm_graphic = NULL;
   }
   comm_graphic = ship_loadCommGFX( comm_pilot->ship );
   logo = faction_logoSmall(comm_pilot->faction);
   name = comm_pilot->name;
   /* Get standing colour / text. */
   if (pilot_isHostile(comm_pilot)) {
      stand = "Hostile";
      c = &cHostile;
   }
   else {
      stand = faction_getStandingBroad(faction_getPlayer( comm_pilot->faction ));
      c = faction_getColour( comm_pilot->faction );
   }
   w = MAX(gl_printWidth( NULL, name ), gl_printWidth( NULL, stand ));
   y = gl_defFont.h*2 + 15;
   if (logo != NULL) {
      w += logo->w;
      y = MAX( y, logo->w );
   }
   x = (comm_graphic->w - w) / 2;

   /* Create the window. */
   wid = window_create( "Communication Channel", -1, -1,
         20 + comm_graphic->w + 20 + BUTTON_WIDTH + 20,
         30 + comm_graphic->h + y + 5 + 20 );

   /* Create the ship image. */
   window_addRect( wid, 20, -30, comm_graphic->w, comm_graphic->h + y + 5,
         "rctShip", &cGrey10, 1 );
   window_addImage( wid, 20, -30, "imgShip", comm_graphic, 0 );

   /* Faction logo. */
   if (logo != NULL) {
      window_addImage( wid, x, -30 - comm_graphic->h - 5,
            "imgFaction", logo, 0 );
      x += logo->w + 10;
      y -= (logo->w - (gl_defFont.h*2 + 15)) / 2;
   }
   
   /* Name. */
   window_addText( wid, x, -30 - comm_graphic->h - y + gl_defFont.h*2 + 10,
         comm_graphic->w - x, 20, 0, "txtName",
         NULL, &cDConsole, name );

   /* Standing. */
   window_addText( wid, x, -30 - comm_graphic->h - y + gl_defFont.h + 5,
         comm_graphic->w - x, 20, 0, "txtStanding", NULL, c, stand );

   /* Buttons. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", comm_close );
   window_addButton( wid, -20, 20 + BUTTON_HEIGHT + 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnGreet", "Greet", NULL );
   if (!pilot_isFlag(comm_pilot, PILOT_BRIBED) && /* Not already bribed. */
         ((faction_getPlayer( comm_pilot->faction ) < 0) || /* Hostile. */
            pilot_isHostile(comm_pilot)))
      window_addButton( wid, -20, 20 + 2*BUTTON_HEIGHT + 40,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnBribe", "Bribe", comm_bribe );
   else
      window_addButton( wid, -20, 20 + 2*BUTTON_HEIGHT + 40,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnRequest",
            "Refuel", comm_requestFuel );

   return 0;
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
   comm_pilot = NULL;
   /* Close the window. */
   window_close( wid, unused );
}


/**
 * @brief Tries to bribe the pilot.
 *
 *    @param wid ID of window calling the function.
 *    @param unused Unused.
 */
static void comm_bribe( unsigned int wid, char *unused )
{
   (void) unused;
   int answer;
   double d;
   unsigned int price;
   const char *str;
   lua_State *L;

   /* Unbribeable. */
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
   price = (unsigned int) d;

   /* Check to see if already bribed. */
   if (price == 0) {
      dialogue_msg("Bribe Pilot", "\"Money won't save your hide now!\"");
      return;
   }

   /* Bribe message. */
   str = comm_getString( "bribe_prompt" );
   if (str == NULL) {
      answer = dialogue_YesNo( "Bribe Pilot", "\"I'm gonna need at least %d credits to not leave you as a hunk of floating debris.\"\n\nPay %d credits?", price, price );
   }
   else
      answer = dialogue_YesNo( "Bribe Pilot", "%s\n\nPay %d credits?", str, price );

   /* Said no. */
   if (answer == 0) {
      dialogue_msg("Bribe Pilot", "You decide not to pay.");
      return;
   }

   /* Check if has the money. */
   if (player->credits < price) {
      dialogue_msg("Bribe Pilot", "You don't have enough credits for the bribery.");
   }
   else {
      player->credits -= price;
      str = comm_getString( "bribe_paid" );
      if (str == NULL)
         dialogue_msg("Bribe Pilot", "\"Pleasure to do business with you.\"");
      else
         dialogue_msg("Bribe Pilot", "%s", str);

      /* Mark as bribed and don't allow bribing again. */
      pilot_setFlag( comm_pilot, PILOT_BRIBED );
      pilot_rmHostile( comm_pilot );
      L = comm_pilot->ai->L;
      lua_getglobal(L, "mem");
      lua_pushnumber(L, 0);
      lua_setfield(L, -2, "bribe");
      lua_pop(L,1);

      /* Reopen window. */
      window_destroy( wid );
      comm_open( comm_pilot->id );
   }
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
   unsigned int price;

   /* Must need refueling. */
   if (player->fuel >= player->fuel_max) {
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
   if ((ret != 0) || (msg == NULL)) {
      dialogue_msg( "Request Fuel", "\"Sorry, I'm busy now.\"" );
      return;
   }
   price = (int) val;

   /* Check to see if is already refueling. */
   if (pilot_isFlag(comm_pilot, PILOT_REFUELING)) {
      dialogue_msg( "Request Fuel", "Pilot is already refueling you." );
      return;
   }

   /* See if player really wants to pay. */
   ret = dialogue_YesNo( "Request Fuel", "%s\n\nPay %d credits?", msg, price );
   if (ret == 0) {
      dialogue_msg( "Request Fuel", "You decide not to pay." );
      return;
   }

   /* Check if he has the money. */
   if (player->credits < price) {
      dialogue_msg( "Request Fuel", "You need %d more credits!",
            player->credits - price);
      return;
   }

   /* Take money. */
   player->credits      -= price;
   comm_pilot->credits  += price;

   /* Start refueling. */
   pilot_setFlag(comm_pilot, PILOT_REFUELING);
   ai_refuel( comm_pilot, player->id );

   /* Last message. */
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

