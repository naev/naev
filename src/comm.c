/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file comm.c
 *
 * @brief For communicating with spobs/pilots.
 */

/** @cond */
#include "naev.h"
/** @endcond */

#include "comm.h"

#include "ai.h"
#include "array.h"
#include "commodity.h"
#include "dialogue.h"
#include "escort.h"
#include "hook.h"
#include "log.h"
#include "ndata.h"
#include "nlua.h"
#include "opengl.h"
#include "pilot.h"
#include "player.h"
#include "rng.h"
#include "toolkit.h"

#define BUTTON_WIDTH    80 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */

#define GRAPHIC_WIDTH  256 /**< Width of graphic. */
#define GRAPHIC_HEIGHT 256 /**< Height of graphic. */

static Spob *comm_spob         = NULL; /**< Spob currently talking to. */
static glTexture *comm_graphic = NULL; /**< Pilot's graphic. */
static int comm_commClose      = 0; /**< Close comm when done. */
static nlua_env comm_env       = LUA_NOREF; /**< Comm Lua env. */

/*
 * Prototypes.
 */
/* Static. */
static unsigned int comm_open( glTexture *gfx, int faction,
      int override, int bribed, const char *name );
static void comm_close( unsigned int wid, const char *unused );
static void comm_bribeSpob( unsigned int wid, const char *unused );
static const char* comm_getString( const Pilot *p, const char *str );

/**
 * @brief Checks to see if comm is open.
 *
 *    @return 1 if comm is open.
 */
int comm_isOpen (void)
{
   return window_exists( "wdwComm" );
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
   Pilot *p;
   Pilot *const* pltstk;

   /* Get the pilot. */
   p  = pilot_get( pilot );
   c  = pilot_getFactionColourChar( p );

   /* Make sure pilot exists. */
   if (p == NULL)
      return -1;

   /* Make sure pilot in range. */
   if (!pilot_isFlag(p, PILOT_HAILING) &&
         pilot_inRangePilot( player.p, p, NULL ) <= 0) {
      player_message(_("#rTarget is out of communications range"));
      return -1;
   }

   /* Destroy the window if it's already present. */
   wid = window_get( "wdwComm" );
   if (wid > 0) {
      window_destroy( wid );
      return 0;
   }

   /* Must not be jumping. */
   if (pilot_isFlag(p, PILOT_HYPERSPACE)) {
      player_message(_("#%c%s#r is jumping and can't respond"), c, p->name);
      return 0;
   }

   /* Must not be disabled. */
   if (pilot_isFlag(p, PILOT_DISABLED)) {
      player_message(_("#%c%s#r does not respond"), c, p->name);
      return 0;
   }

   /* Check for player faction (escorts). */
   if (p->faction == FACTION_PLAYER) {
      escort_playerCommand( p );
      return 0;
   }

   /* Set up for the comm_get* functions. */
   ai_setPilot( p );

   /* Have pilot stop hailing. */
   pilot_rmFlag( p, PILOT_HAILING );

   /* Don't close automatically. */
   comm_commClose = 0;

   /* Run specific hail hooks on hailing pilot. */
   HookParam hparam[] = {
      { .type = HOOK_PARAM_PILOT,
         .u = { .lp = p->id } },
      { .type = HOOK_PARAM_SENTINEL } };
   if (pilot_canTarget( p )) {
      hooks_runParam( "hail", hparam );
      pilot_runHook( p, PILOT_HOOK_HAIL );
   }

   /* Check to see if pilot wants to communicate. */
   msg = comm_getString( p, "comm_no" );
   if (msg != NULL) {
      if (comm_commClose==0)
         player_messageRaw( msg );
      return 0;
   }

   /* Run generic hail hooks on all pilots. */
   pltstk = pilot_getAll();
   for (int i=0; i<array_size(pltstk); i++)
      ai_hail( pltstk[i] );

   /* Close window if necessary. */
   if (comm_commClose) {
      p  = NULL;
      comm_spob = NULL;
      comm_commClose = 0;
      return 0;
   }

   /* Set up environment first time. */
   if (comm_env == LUA_NOREF) {
      comm_env = nlua_newEnv(1);
      nlua_loadStandard( comm_env );

      size_t bufsize;
      char *buf = ndata_read( COMM_PATH, &bufsize );
      if (nlua_dobufenv(comm_env, buf, bufsize, COMM_PATH) != 0) {
         WARN( _("Error loading file: %s\n"
             "%s\n"
             "Most likely Lua file has improper syntax, please check"),
               COMM_PATH, lua_tostring(naevL,-1));
         free(buf);
         return -1;
      }
      free(buf);
   }

   /* Run Lua. */
   nlua_getenv(naevL, comm_env,"comm");
   lua_pushpilot(naevL, p->id);
   if (nlua_pcall(comm_env, 1, 0)) { /* error has occurred */
      WARN( _("Comm: '%s'"), lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }

   return 0;
}

/**
 * @brief Opens a communication dialogue with a spob.
 *
 *    @param spob Spob to communicate with.
 *    @return 0 on success.
 */
int comm_openSpob( Spob *spob )
{
   unsigned int wid;

   /* Destroy the window if it's already present. */
   wid = window_get( "wdwComm" );
   if (wid > 0) {
      window_destroy( wid );
      return 0;
   }

   /* Must not be disabled. */
   if (!spob_hasService(spob, SPOB_SERVICE_INHABITED)) {
      player_message(_("%s does not respond."), spob_name(spob));
      return 0;
   }

   comm_spob = spob;

   /* Create the generic comm window. */
   wid = comm_open( gl_dupTexture( comm_spob->gfx_space ),
         comm_spob->presence.faction, 0, spob->bribed, spob_name(comm_spob) );

   /* Add special buttons. */
   if (!spob->can_land && !spob->bribed && (spob->bribe_msg != NULL))
      window_addButtonKey( wid, -20, 20 + BUTTON_HEIGHT + 20,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnBribe", _("Bribe"), comm_bribeSpob, SDLK_b );

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
      int override, int bribed, const char *name )
{
   int namex, standx, logox, y;
   int namew, standw, logow, logoh, width;
   const glTexture *logo;
   const char *stand;
   unsigned int wid;
   const glColour *c;
   glFont *font;
   int gw, gh;
   double aspect;

   /* Clean up. */
   gl_freeTexture(comm_graphic);
   comm_graphic = NULL;

   /* Get faction details. */
   comm_graphic = gfx;
   logo = NULL;
   if (faction_isKnown(faction))
      logo = faction_logo(faction);

   /* Get standing colour / text. */
   stand = faction_getStandingBroad( faction, bribed, override );
   if (bribed)
      c = &cNeutral;
   else if (override < 0)
      c = &cHostile;
   else if (override > 0)
      c = &cFriend;
   else
      c = faction_getColour( faction );

   namew  = gl_printWidthRaw( NULL, name );
   standw = gl_printWidthRaw( NULL, stand );
   width  = MAX(namew, standw);

   logow = logo == NULL ? 0 : logo->w * (double)FACTION_LOGO_SM / MAX( logo->w, logo->h );
   logoh = logo == NULL ? 0 : logo->h * (double)FACTION_LOGO_SM / MAX( logo->w, logo->h );

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
      y  = MAX( font->h*2 + 15, logoh );
      logox = GRAPHIC_WIDTH/2 - logow/2 - width/2 - 2;
   }
   else {
      logox = 0;
      y = font->h*2 + 15;
   }

   /* Create the window. */
   wid = window_create( "wdwComm", _("Communication Channel"), -1, -1,
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

   /* Faction logo. */
   if (logo != NULL) {
      window_addImage( wid, 19 + logox, -30 - GRAPHIC_HEIGHT - 4,
            logow, logoh, "imgFaction", logo, 0 );
      y -= (logoh - (gl_defFont.h*2 + 15)) / 2;
   }

   /* Name. */
   window_addText( wid, 19 + namex, -30 - GRAPHIC_HEIGHT - y + font->h*2 + 10,
         GRAPHIC_WIDTH - namex - logow, 20, 0, "txtName", font, &cWhite, name );

   /* Standing. */
   window_addText( wid, 19 + standx, -30 - GRAPHIC_HEIGHT - y + font->h + 5,
         GRAPHIC_WIDTH - standx - logow, 20, 0, "txtStanding", font, c, stand );

   /* Buttons. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", _("Close"), comm_close );

   return wid;
}

/**
 * @brief Closes the comm window.
 *
 *    @param wid ID of window calling the function.
 *    @param unused Unused.
 */
static void comm_close( unsigned int wid, const char *unused )
{
   gl_freeTexture(comm_graphic);
   comm_graphic = NULL;
   comm_spob = NULL;
   /* Close the window. */
   window_close( wid, unused );
}

/**
 * @brief Tries to bribe the spob
 *
 *    @param wid ID of window calling the function.
 *    @param unused Unused.
 */
static void comm_bribeSpob( unsigned int wid, const char *unused )
{
   (void) unused;
   int answer;
   credits_t price;

   /* Get price. */
   price = comm_spob->bribe_price;

   /* No bribing. */
   if (comm_spob->bribe_price <= 0.) {
      dialogue_msgRaw( _("Bribe Starport"), comm_spob->bribe_msg );
      return;
   }

   /* Yes/No input. */
   answer = dialogue_YesNoRaw( _("Bribe Starport"), comm_spob->bribe_msg );

   /* Said no. */
   if (answer == 0) {
      dialogue_msg(_("Bribe Starport"), _("You decide not to pay."));
      return;
   }

   /* Check if has the money. */
   if (!player_hasCredits( price )) {
      dialogue_msg(_("Bribe Starport"), _("You don't have enough credits for the bribery."));
      return;
   }

   /* Pay the money. */
   player_modCredits( -price );
   dialogue_msg(_("Bribe Starport"), _("You have permission to dock."));

   /* Mark as bribed and don't allow bribing again. */
   comm_spob->bribed = 1;

   /* Reopen window. */
   window_destroy( wid );
   comm_openSpob( comm_spob );
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
 *    @param p Pilot to get string from.
 *    @param str String to get.
 *    @return String matching str.
 */
static const char* comm_getString( const Pilot *p, const char *str )
{
   const char *ret;

   if (p->ai == NULL)
      return NULL;

   /* Get memory table. */
   nlua_getenv(naevL, p->ai->env, "mem");

   /* Get str message. */
   lua_getfield(naevL, -1, str );
   if (!lua_isstring(naevL, -1))
      ret = NULL;
   else
      ret = lua_tostring(naevL, -1);
   lua_pop(naevL, 2);

   return ret;
}
