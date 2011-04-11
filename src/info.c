/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file menu.h
 *
 * @brief Handles the important game menus.
 */


#include "info.h"

#include "naev.h"

#include <string.h>

#include "menu.h"
#include "toolkit.h"
#include "dialogue.h"
#include "log.h"
#include "pilot.h"
#include "space.h"
#include "player.h"
#include "mission.h"
#include "ntime.h"
#include "map.h"
#include "land.h"
#include "equipment.h"
#include "tk/toolkit_priv.h"


#define BUTTON_WIDTH    90 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */

#define menu_Open(f)    (menu_open |= (f)) /**< Marks a menu as opened. */
#define menu_Close(f)   (menu_open &= ~(f)) /**< Marks a menu as closed. */

#define INFO_WINDOWS      6 /**< Amount of windows in the tab. */

#define INFO_WIN_MAIN      0
#define INFO_WIN_SHIP      1
#define INFO_WIN_WEAP      2
#define INFO_WIN_CARGO     3
#define INFO_WIN_MISN      4
#define INFO_WIN_STAND     5
static const char *info_names[INFO_WINDOWS] = {
   "Main",
   "Ship",
   "Weapons",
   "Cargo",
   "Missions",
   "Standings"
}; /**< Name of the tab windows. */


static unsigned int info_wid = 0;
static unsigned int *info_windows = NULL;

static CstSlotWidget info_eq;
static CstSlotWidget info_eq_weaps;
static int *info_factions;


/*
 * prototypes
 */
/* information menu */
static void info_close( unsigned int wid, char* str );
static void info_openMain( unsigned int wid );
static void info_openShip( unsigned int wid );
static void info_openWeapons( unsigned int wid );
static void info_openCargo( unsigned int wid );
static void info_openMissions( unsigned int wid );
static void info_getDim( unsigned int wid, int *w, int *h, int *lw );
static void standings_close( unsigned int wid, char *str );
static void ship_update( unsigned int wid );
static void weapons_genList( unsigned int wid );
static void weapons_update( unsigned int wid, char *str );
static void weapons_rename( unsigned int wid, char *str );
static void weapons_autoweap( unsigned int wid, char *str );
static void weapons_fire( unsigned int wid, char *str );
static void weapons_inrange( unsigned int wid, char *str );
static void weapons_renderLegend( double bx, double by, double bw, double bh, void* data );
static void info_openStandings( unsigned int wid );
static void standings_update( unsigned int wid, char* str );
static void cargo_genList( unsigned int wid );
static void cargo_update( unsigned int wid, char* str );
static void cargo_jettison( unsigned int wid, char* str );
static void mission_menu_abort( unsigned int wid, char* str );
static void mission_menu_genList( unsigned int wid, int first );
static void mission_menu_update( unsigned int wid, char* str );


/**
 * @brief Opens the information menu.
 */
void menu_info( int window )
{
   int w, h;

   /* Not under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   /* Open closes when previously opened. */
   if (menu_isOpen(MENU_INFO) || dialogue_isOpen()) {
      info_close( 0, NULL );
      return;
   }

   /* Dimensions. */
   w = 600;
   h = 500;

   /* Create the window. */
   info_wid = window_create( "Info", -1, -1, w, h );
   window_setCancel( info_wid, info_close );

   /* Create tabbewd window. */
   info_windows = window_addTabbedWindow( info_wid, -1, -1, -1, -1, "tabInfo",
         INFO_WINDOWS, info_names );

   /* Open the subwindows. */
   info_openMain(       info_windows[ INFO_WIN_MAIN ] );
   info_openShip(       info_windows[ INFO_WIN_SHIP ] );
   info_openWeapons(    info_windows[ INFO_WIN_WEAP ] );
   info_openCargo(      info_windows[ INFO_WIN_CARGO ] );
   info_openMissions(   info_windows[ INFO_WIN_MISN ] );
   info_openStandings(  info_windows[ INFO_WIN_STAND ] );

   menu_Open(MENU_INFO);

   /* Set active window. */
   window_tabWinSetActive( info_wid, "tabInfo", CLAMP( 0, 5, window ) );
}
/**
 * @brief Closes the information menu.
 *    @param str Unused.
 */
static void info_close( unsigned int wid, char* str )
{
   (void) wid;
   if (info_wid > 0) {
      window_close( info_wid, str );
      info_wid = 0;
      menu_Close(MENU_INFO);
   }
}


/**
 * @brief Updates the info windows.
 */
void info_update (void)
{
   weapons_update( info_windows[ INFO_WIN_WEAP ], NULL );
}


/**
 * @brief Opens the main info window.
 */
static void info_openMain( unsigned int wid )
{
   char str[128], **buf, creds[ECON_CRED_STRLEN];
   char **licenses;
   int nlicenses;
   int i;
   char *nt;
   int w, h;

   /* Get the dimensions. */
   window_dimWindow( wid, &w, &h );

   /* pilot generics */
   nt = ntime_pretty( ntime_get(), 2 );
   window_addText( wid, 40, 20, 120, h-80,
         0, "txtDPilot", &gl_smallFont, &cDConsole,
         "Pilot:\n"
         "Date:\n"
         "Combat Rating:\n"
         "\n"
         "Money:\n"
         "Ship:\n"
         "Fuel:"
         );
   credits2str( creds, player.p->credits, 2 );
   snprintf( str, 128,
         "%s\n"
         "%s\n"
         "%s\n"
         "\n"
         "%s Credits\n"
         "%s\n"
         "%.0f (%d Jumps)",
         player.name,
         nt,
         player_rating(),
         creds,
         player.p->name,
         player.p->fuel, pilot_getJumps(player.p) );
   window_addText( wid, 140, 20,
         200, h-80,
         0, "txtPilot", &gl_smallFont, &cBlack, str );
   free(nt);

   /* menu */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", info_close );

   /* List. */
   buf = player_getLicenses( &nlicenses );
   licenses = malloc(sizeof(char*)*nlicenses);
   for (i=0; i<nlicenses; i++)
      licenses[i] = strdup(buf[i]);
   window_addText( wid, -20, -40, w-80-200-40, 20, 1, "txtList",
         NULL, &cDConsole, "Licenses" );
   window_addList( wid, -20, -70, w-80-200-40, h-110-BUTTON_HEIGHT,
         "lstLicenses", licenses, nlicenses, 0, NULL );
}


/**
 * @brief Shows the player what outfits he has.
 *
 *    @param str Unused.
 */
static void info_openShip( unsigned int wid )
{
   int w, h;

   /* Get the dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "closeOutfits", "Close", info_close );

   /* Text. */
   window_addText( wid, 40, -60, 100, h-60, 0, "txtSDesc", &gl_smallFont,
         &cDConsole,
         "Name:\n"
         "Model:\n"
         "Class:\n"
         "Crew:\n"
         "\n"
         "Total CPU:\n"
         "Mass:\n"
         "Jump Time:\n"
         "Thrust:\n"
         "Speed:\n"
         "Turn:\n"
         "\n"
         "Shield:\n"
         "Armour:\n"
         "Energy:\n"
         "Cargo Space:\n"
         "Fuel:\n"
         );
   window_addText( wid, 140, -60, w-300., h-60, 0, "txtDDesc", &gl_smallFont,
         &cBlack, NULL );

   /* Custom widget. */
   equipment_slotWidget( wid, -20, -40, 180, h-60, &info_eq );
   info_eq.selected  = player.p;
   info_eq.canmodify = 0;

   /* Update ship. */
   ship_update( wid );
}


/**
 * @brief Updates the ship stuff.
 */
static void ship_update( unsigned int wid )
{
   char buf[1024];
   int cargo;

   cargo = pilot_cargoUsed( player.p ) + pilot_cargoFree( player.p );
   snprintf( buf, sizeof(buf),
         "%s\n"
         "%s\n"
         "%s\n"
         "%d\n"
         "\n"
         "%.0f teraflops\n"
         "%.0f tonnes\n"
         "%s average\n"
         "%.0f kN/tonne\n"
         "%.0f m/s (max %.0f m/s)\n"
         "%.0f deg/s\n"
         "\n"
         "%.0f / %.0f MJ (%.1f MW)\n" /* Shield */
         "%.0f / %.0f MJ (%.1f MW)\n" /* Armour */
         "%.0f / %.0f MJ (%.1f MW)\n" /* Energy */
         "%d / %d tonnes\n"
         "%.0f / %.0f units (%d jumps)",
         /* Generic */
         player.p->name,
         player.p->ship->name,
         ship_class(player.p->ship),
         (int)floor(player.p->crew),
         player.p->cpu_max,
         /* Movement. */
         player.p->solid->mass,
         ntime_pretty( pilot_hyperspaceDelay( player.p ), 2 ),
         player.p->thrust / player.p->solid->mass,
         player.p->speed, solid_maxspeed( player.p->solid, player.p->speed, player.p->thrust ),
         player.p->turn*180./M_PI,
         /* Health. */
         player.p->shield, player.p->shield_max, player.p->shield_regen,
         player.p->armour, player.p->armour_max, player.p->armour_regen,
         player.p->energy, player.p->energy_max, player.p->energy_regen,
         pilot_cargoUsed( player.p ), cargo,
         player.p->fuel, player.p->fuel_max, pilot_getJumps(player.p));
   window_modifyText( wid, "txtDDesc", buf );
}


/**
 * @brief Opens the weapons window.
 */
static void info_openWeapons( unsigned int wid )
{
   int w, h;

   /* Get the dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Buttons */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "closeCargo", "Close", info_close );
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20), BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnRename", "Rename", weapons_rename );

   /* Checkboxes. */
   window_addCheckbox( wid, 220, 20+2*(BUTTON_HEIGHT+20)-40, 250, BUTTON_HEIGHT,
         "chkAutoweap", "Automatically handle weapons", weapons_autoweap, player.p->autoweap );
   window_addCheckbox( wid, 220, 20+2*(BUTTON_HEIGHT+20)-10, 300, BUTTON_HEIGHT,
         "chkFire", "Enable fire mode (fires when activated)", weapons_fire,
         pilot_weapSetModeCheck( player.p, info_eq_weaps.weapons ) );
   window_addCheckbox( wid, 220, 20+2*(BUTTON_HEIGHT+20)+20, 300, BUTTON_HEIGHT,
         "chkInrange", "Only shoot weapons that are in range", weapons_inrange,
         pilot_weapSetInrangeCheck( player.p, info_eq_weaps.weapons ) );

   /* Custom widget. */
   equipment_slotWidget( wid, 20, -40, 180, h-60, &info_eq_weaps );
   info_eq_weaps.selected  = player.p;
   info_eq_weaps.canmodify = 0;

   /* Custom widget for legend. */
   window_addCust( wid, 220, -220, w-200-60, 100, "cstLegend", 0,
         weapons_renderLegend, NULL, NULL );

   /* List. */
   weapons_genList( wid );
}


/**
 * @brief Generates the weapons list.
 */
static void weapons_genList( unsigned int wid )
{
   const char *str;
   char **buf;
   int i;
   int w, h;

   /* Get the dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Destroy widget if needed. */
   if (widget_exists( wid, "lstWeapSets" ))
      window_destroyWidget( wid, "lstWeapSets" );

   /* List */
   buf = malloc( sizeof(char*) * PILOT_WEAPON_SETS );
   for (i=0; i<PILOT_WEAPON_SETS; i++) {
      str = pilot_weapSetName( info_eq_weaps.selected, i );
      if (str == NULL) {
         buf[i] = malloc( sizeof(char) * PATH_MAX );
         snprintf( buf[i], PATH_MAX, "Weapon Set %d", (i+1)%10 );
      }
      else {
         buf[i] = strdup( str );
      }
   }
   window_addList( wid, 20+180+20, -40,
         w - (20+180+20+20), 160,
         "lstWeapSets", buf, PILOT_WEAPON_SETS,
         0, weapons_update );

   /* Update. */
   weapons_update( wid, NULL );
}


/**
 * @brief Updates the weapon sets.
 */
static void weapons_update( unsigned int wid, char *str )
{
   (void) str;
   int pos;

   /* Update the position. */
   pos = toolkit_getListPos( wid, "lstWeapSets" );
   info_eq_weaps.weapons = pos;

   /* Update fire mode. */
   window_checkboxSet( wid, "chkFire",
         pilot_weapSetModeCheck( player.p, pos ) );

   /* Update inrange. */
   window_checkboxSet( wid, "chkInrange",
         pilot_weapSetInrangeCheck( player.p, pos ) );

   /* Update autoweap. */
   window_checkboxSet( wid, "chkAutoweap", player.p->autoweap );
}


/**
 * @brief Renames a weapon set.
 */
static void weapons_rename( unsigned int wid, char *str )
{
   (void) str;
   char *name;

   /* Prompt for new name. */
   name = dialogue_input( "Rename Weapon Set", 3, 30,
         "What do you want to rename the weapon set '%s'?",
         pilot_weapSetName( player.p, info_eq_weaps.weapons ) );

   /* Cancelled. */
   if (name == NULL)
      return;

   /* Change name. */
   pilot_weapSetNameSet( player.p, info_eq_weaps.weapons, name );
   free(name);

   /* Disable autoweap. */
   player.p->autoweap = 0;

   /* Regenerate list. */
   weapons_genList( wid );
}


/**
 * @brief Toggles autoweap for the sihp.
 */
static void weapons_autoweap( unsigned int wid, char *str )
{
   int state, sure;

   /* Set state. */
   state = window_checkboxState( wid, str );

   /* Run autoweapons if needed. */
   if (state) {
      sure = dialogue_YesNoRaw( "Enable autoweapons?",
            "Are you sure you want to enable automatic weapon groups for the "
            "ship?\n\nThis will overwrite all manually-tweaked weapons groups." );
      if (!sure) {
         window_checkboxSet( wid, str, 0 );
         return;
      }
      player.p->autoweap = 1;
      pilot_weaponAuto( player.p );
      weapons_genList( wid );
   }
   else
      player.p->autoweap = 0;
}


/**
 * @brief Sets the fire mode.
 */
static void weapons_fire( unsigned int wid, char *str )
{
   int i, state;

   /* Set state. */
   state = window_checkboxState( wid, str );
   pilot_weapSetMode( player.p, info_eq_weaps.weapons, state );

   /* Check to see if they are all fire groups. */
   for (i=0; i<PILOT_WEAPON_SETS; i++)
      if (!pilot_weapSetModeCheck( player.p, i ))
         break;

   /* Not able to set them all to fire groups. */
   if (i >= PILOT_WEAPON_SETS) {
      dialogue_alert( "You can not set all your weapon sets to fire groups!" );
      pilot_weapSetMode( player.p, info_eq_weaps.weapons, 0 );
      window_checkboxSet( wid, str, 0 );
   }

   /* Set default if needs updating. */
   pilot_weaponSetDefault( player.p );
}


/**
 * @brief Sets the inrange property.
 */
static void weapons_inrange( unsigned int wid, char *str )
{
   int state;

   /* Set state. */
   state = window_checkboxState( wid, str );
   pilot_weapSetInrange( player.p, info_eq_weaps.weapons, state );
}


/**
 * @brief Renders the legend.
 */
static void weapons_renderLegend( double bx, double by, double bw, double bh, void* data )
{
   (void) data;
   (void) bw;
   (void) bh;
   double y;

   y = by+bh-20;
   gl_print( &gl_defFont, bx, y, &cBlack, "Legend" );

   y -= 20.;
   toolkit_drawRect( bx, y, 10, 10, &cFontYellow, NULL );
   gl_print( &gl_smallFont, bx+20, y, &cBlack, "Secondary Weapon (Right click toggles)" );

   y -= 15.;
   toolkit_drawRect( bx, y, 10, 10, &cFontRed, NULL );
   gl_print( &gl_smallFont, bx+20, y, &cBlack, "Primary Weapon (Left click toggles)" );
}


/**
 * @brief Shows the player his cargo.
 *
 *    @param str Unused.
 */
static void info_openCargo( unsigned int wid )
{
   int w, h;

   /* Get the dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Buttons */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "closeCargo", "Close", info_close );
   window_addButton( wid, -40 - BUTTON_WIDTH, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnJettisonCargo", "Jettison",
         cargo_jettison );
   window_disableButton( wid, "btnJettisonCargo" );

   /* Generate the list. */
   cargo_genList( wid );
}
/**
 * @brief Generates the cargo list.
 */
static void cargo_genList( unsigned int wid )
{
   char **buf;
   int nbuf;
   int i;
   int w, h;

   /* Get the dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Destroy widget if needed. */
   if (widget_exists( wid, "lstCargo" ))
      window_destroyWidget( wid, "lstCargo" );

   /* List */
   if (player.p->ncommodities==0) {
      /* No cargo */
      buf = malloc(sizeof(char*));
      buf[0] = strdup("None");
      nbuf = 1;
   }
   else {
      /* List the player's cargo */
      buf = malloc(sizeof(char*)*player.p->ncommodities);
      for (i=0; i<player.p->ncommodities; i++) {
         buf[i] = malloc(sizeof(char)*128);
         snprintf(buf[i],128, "%s%s %d",
               player.p->commodities[i].commodity->name,
               (player.p->commodities[i].id != 0) ? "*" : "",
               player.p->commodities[i].quantity);
      }
      nbuf = player.p->ncommodities;
   }
   window_addList( wid, 20, -40,
         w - 40, h - BUTTON_HEIGHT - 80,
         "lstCargo", buf, nbuf, 0, cargo_update );

   cargo_update(wid, NULL);
}
/**
 * @brief Updates the player's cargo in the cargo menu.
 *    @param str Unused.
 */
static void cargo_update( unsigned int wid, char* str )
{
   (void)str;
   int pos;

   if (player.p->ncommodities==0)
      return; /* No cargo */

   pos = toolkit_getListPos( wid, "lstCargo" );

   /* Can jettison all but mission cargo when not landed*/
   if (landed)
      window_disableButton( wid, "btnJettisonCargo" );
   else
      window_enableButton( wid, "btnJettisonCargo" );
}
/**
 * @brief Makes the player jettison the currently selected cargo.
 *    @param str Unused.
 */
static void cargo_jettison( unsigned int wid, char* str )
{
   (void)str;
   int i, j, f, pos, ret;
   Mission *misn;

   if (player.p->ncommodities==0)
      return; /* No cargo, redundant check */

   pos = toolkit_getListPos( wid, "lstCargo" );

   /* Special case mission cargo. */
   if (player.p->commodities[pos].id != 0) {
      if (!dialogue_YesNo( "Abort Mission",
               "Are you sure you want to abort this mission?" ))
         return;

      /* Get the mission. */
      f = 0;
      for (i=0; i<MISSION_MAX; i++) {
         for (j=0; j<player_missions[i].ncargo; j++) {
            if (player_missions[i].cargo[j] == player.p->commodities[pos].id) {
               f = 1;
               break;
            }
         }
         if (f==1)
            break;
      }
      if (!f) {
         WARN("Cargo '%d' does not belong to any active mission.",
               player.p->commodities[pos].id);
         return;
      }
      misn = &player_missions[i];

      /* We run the "abort" function if it's found. */
      ret = misn_tryRun( misn, "abort" );

      /* Now clean up mission. */
      if (ret != 2) {
         mission_cleanup( misn );
         memmove( misn, &player_missions[i+1],
               sizeof(Mission) * (MISSION_MAX-i-1) );
         memset( &player_missions[MISSION_MAX-1], 0, sizeof(Mission) );
      }

      /* Reset markers. */
      mission_sysMark();

      /* Regenerate list. */
      mission_menu_genList( info_windows[ INFO_WIN_MISN ] ,0);
   }
   else {
      /* Remove the cargo */
      commodity_Jettison( player.p->id, player.p->commodities[pos].commodity,
            player.p->commodities[pos].quantity );
      pilot_cargoRm( player.p, player.p->commodities[pos].commodity,
            player.p->commodities[pos].quantity );
   }

   /* We reopen the menu to recreate the list now. */
   ship_update( info_windows[ INFO_WIN_SHIP ] );
   cargo_genList( wid );
}


/**
 * @brief Gets the window standings window dimensions.
 */
static void info_getDim( unsigned int wid, int *w, int *h, int *lw )
{
   /* Get the dimensions. */
   window_dimWindow( wid, w, h );
   *lw = *w-60-BUTTON_WIDTH-120;
}


/**
 * @brief Closes the faction stuff.
 */
static void standings_close( unsigned int wid, char *str )
{
   (void) wid;
   (void) str;
   if (info_factions != NULL)
      free(info_factions);
   info_factions = NULL;
}


/**
 * @brief Displays the player's standings.
 */
static void info_openStandings( unsigned int wid )
{
   int i;
   int n, m;
   char **str;
   int w, h, lw;

   /* Get dimensions. */
   info_getDim( wid, &w, &h, &lw );

   /* On close. */
   window_onClose( wid, standings_close );

   /* Buttons */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "closeMissions", "Close", info_close );

   /* Graphics. */
   window_addImage( wid, 0, 0, 0, 0, "imgLogo", NULL, 0 );

   /* Text. */
   window_addText( wid, lw+40, 0, (w-(lw+60)), 20, 1, "txtName",
         &gl_defFont, &cDConsole, NULL );
   window_addText( wid, lw+40, 0, (w-(lw+60)), 20, 1, "txtStanding",
         &gl_smallFont, &cBlack, NULL );

   /* Gets the faction standings. */
   info_factions  = faction_getAll( &n );
   str            = malloc( sizeof(char*) * n );

   /* Create list. */
   for (i=0; i<n; i++) {
      str[i] = malloc( 256 );
      m = round( faction_getPlayer( info_factions[i] ) );
      snprintf( str[i], 256, "%s   [ %+d%% ]",
            faction_name( info_factions[i] ), m );
   }

   /* Display list. */
   window_addList( wid, 20, -40, lw, h-60,
         "lstStandings", str, n, 0, standings_update );

   standings_update( wid , NULL );
}


/**
 * @brief Updates the standings menu.
 */
static void standings_update( unsigned int wid, char* str )
{
   (void) str;
   int p, y;
   glTexture *t;
   int w, h, lw;
   char buf[128];
   int m;

   /* Get dimensions. */
   info_getDim( wid, &w, &h, &lw );

   /* Get faction. */
   p = toolkit_getListPos( wid, "lstStandings" );
   y = 0;

   /* Render logo. */
   t = faction_logoSmall( info_factions[p] );
   if (t != NULL) {
      window_modifyImage( wid, "imgLogo", t, 0, 0 );
      y  = -40;
      window_moveWidget( wid, "imgLogo", lw+40 + (w-(lw+60)-t->w)/2, y );
      y -= t->h;
   }
   else {
      window_modifyImage( wid, "imgLogo", NULL, 0, 0 );
      y = -20;
   }

   /* Modify text. */
   y -= 20;
   window_modifyText( wid, "txtName", faction_longname( info_factions[p] ) );
   window_moveWidget( wid, "txtName", lw+40, y );
   y -= 40;
   m = round( faction_getPlayer( info_factions[p] ) );
   snprintf( buf, sizeof(buf), "%+d%%   [ %s ]", m, faction_getStanding( m ) );
   window_modifyText( wid, "txtStanding", buf );
   window_moveWidget( wid, "txtStanding", lw+40, y );
}


/**
 * @brief Shows the player's active missions.
 *
 *    @param parent Unused.
 *    @param str Unused.
 */
static void info_openMissions( unsigned int wid )
{
   int w, h;

   /* Get the dimensions. */
   window_dimWindow( wid, &w, &h );

   /* buttons */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "closeMissions", "Close", info_close );
   window_addButton( wid, -20, 40 + BUTTON_HEIGHT,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnAbortMission", "Abort",
         mission_menu_abort );

   /* text */
   window_addText( wid, 300+40, -60,
         200, 40, 0, "txtSReward",
         &gl_smallFont, &cDConsole, "Reward:" );
   window_addText( wid, 300+100, -60,
         140, 40, 0, "txtReward", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, 300+40, -100,
         w - (300+40+40), h - BUTTON_HEIGHT - 120, 0,
         "txtDesc", &gl_smallFont, &cBlack, NULL );

   /* Put a map. */
   map_show( wid, 20, 20, 300, 260, 0.75 );

   /* list */
   mission_menu_genList(wid ,1);
}
/**
 * @brief Creates the current mission list for the mission menu.
 *    @param first 1 if it's the first time run.
 */
static void mission_menu_genList( unsigned int wid, int first )
{
   int i,j;
   char** misn_names;
   int w, h;

   if (!first)
      window_destroyWidget( wid, "lstMission" );

   /* Get the dimensions. */
   window_dimWindow( wid, &w, &h );

   /* list */
   misn_names = malloc(sizeof(char*) * MISSION_MAX);
   j = 0;
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].id != 0)
         misn_names[j++] = (player_missions[i].title!=NULL) ? strdup(player_missions[i].title) : NULL;
   if (j==0) { /* no missions */
      misn_names[0] = strdup("No Missions");
      j = 1;
   }
   window_addList( wid, 20, -40,
         300, h-340,
         "lstMission", misn_names, j, 0, mission_menu_update );

   mission_menu_update(wid ,NULL);
}
/**
 * @brief Updates the mission menu mission information based on what's selected.
 *    @param str Unusued.
 */
static void mission_menu_update( unsigned int wid, char* str )
{
   (void)str;
   char *active_misn;
   Mission* misn;

   active_misn = toolkit_getList( wid, "lstMission" );
   if ((active_misn==NULL) || (strcmp(active_misn,"No Missions")==0)) {
      window_modifyText( wid, "txtReward", "None" );
      window_modifyText( wid, "txtDesc",
            "You currently have no active missions." );
      window_disableButton( wid, "btnAbortMission" );
      return;
   }

   /* Modify the text. */
   misn = &player_missions[ toolkit_getListPos(wid, "lstMission" ) ];
   window_modifyText( wid, "txtReward", misn->reward );
   window_modifyText( wid, "txtDesc", misn->desc );
   window_enableButton( wid, "btnAbortMission" );

   /* Select the system. */
   if (misn->markers != NULL)
      map_center( system_getIndex( misn->markers[0].sys )->name );
}
/**
 * @brief Aborts a mission in the mission menu.
 *    @param str Unused.
 */
static void mission_menu_abort( unsigned int wid, char* str )
{
   (void)str;
   char *selected_misn;
   int pos;
   Mission* misn;
   int ret;

   selected_misn = toolkit_getList( wid, "lstMission" );

   if (dialogue_YesNo( "Abort Mission",
            "Are you sure you want to abort this mission?" )) {

      /* Get the mission. */
      pos = toolkit_getListPos(wid, "lstMission" );
      misn = &player_missions[pos];

      /* We run the "abort" function if it's found. */
      ret = misn_tryRun( misn, "abort" );

      /* Now clean up mission. */
      if (ret != 2) {
         mission_cleanup( misn );
         memmove( misn, &player_missions[pos+1],
               sizeof(Mission) * (MISSION_MAX-pos-1) );
         memset( &player_missions[MISSION_MAX-1], 0, sizeof(Mission) );
      }

      /* Reset markers. */
      mission_sysMark();

      /* Regenerate list. */
      mission_menu_genList(wid ,0);
   }
}

