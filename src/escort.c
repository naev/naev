/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file escort.c
 *
 * @brief Handles the player's escorts.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "escort.h"

#include "array.h"
#include "dialogue.h"
#include "hook.h"
#include "log.h"
#include "nlua.h"
#include "nluadef.h"
#include "nstring.h"
#include "player.h"

/*
 * Prototypes.
 */
/* Static */
static int escort_command( Pilot *parent, const char *cmd, unsigned int index );

/**
 * @brief Adds an escort to the escort list of a pilot.
 *
 *    @param p Pilot to add escort to.
 *    @param ship Ship of the escort.
 *    @param type Type of the escort.
 *    @param id ID of the pilot representing the escort.
 *    @param persist True if escort should respawn on takeoff/landing.
 *    @return 0 on success.
 */
int escort_addList( Pilot *p, const Ship *ship, EscortType_t type,
      unsigned int id, int persist )
{
   Escort_t *escort;
   if (p->escorts == NULL)
      p->escorts = array_create( Escort_t );
   escort = &array_grow( &p->escorts );
   escort->ship = ship;
   escort->type = type;
   escort->id   = id;
   escort->persist = persist;

   return 0;
}

/**
 * @brief Remove all escorts from a pilot.
 *
 *    @param p Pilot to remove escorts from.
 */
void escort_freeList( Pilot *p )
{
   array_free(p->escorts);
   p->escorts = NULL;
}

/**
 * @brief Remove from escorts list.
 *
 *    @param p Pilot to remove escort from.
 *    @param i index of the pilot representing the escort.

 */
void escort_rmListIndex( Pilot *p, int i )
{
   array_erase( &p->escorts, &p->escorts[i], &p->escorts[i+1] );
}

/**
 * @brief Remove from escorts list.
 *
 *    @param p Pilot to remove escort from.
 *    @param id ID of the pilot representing the escort.

 */
void escort_rmList( Pilot *p, unsigned int id )
{
   for (int i=0; i<array_size(p->escorts); i++) {
      if (p->escorts[i].id == id) {
         escort_rmListIndex( p, i );
         break;
      }
   }
}

/**
 * @brief Creates an escort.
 *
 *    @param p Parent of the escort (who he's guarding).
 *    @param ship Name of the ship escort should have.
 *    @param pos Position to create escort at.
 *    @param vel Velocity to create escort with.
 *    @param dir Direction to face.
 *    @param type Type of escort.
 *    @param add Whether or not to add it to the escort list.
 *    @param dockslot The outfit slot which launched the escort (-1 if N/A)
 *    @return The ID of the escort on success.
 */
unsigned int escort_create( Pilot *p, const Ship *ship,
      const vec2 *pos, const vec2 *vel, double dir,
      EscortType_t type, int add, int dockslot )
{
   Pilot *pe;
   PilotFlags f;
   unsigned int parent;

   /* Get important stuff. */
   parent = p->id;

   /* Set flags. */
   pilot_clearFlagsRaw( f );
   //pilot_setFlagRaw( f, PILOT_NOJUMP );
   if (p->faction == FACTION_PLAYER) {
      pilot_setFlagRaw( f, PILOT_PERSIST );
      pilot_setFlagRaw( f, PILOT_NOCLEAR );
   }
   if (type == ESCORT_TYPE_BAY)
      pilot_setFlagRaw( f, PILOT_CARRIED );

   /* Create the pilot. */
   pe = pilot_create( ship, NULL, p->faction, "escort", dir, pos, vel, f, parent, dockslot );
   pe->parent = parent;

   /* Make invincible to player. */
   if (pe->parent == PLAYER_ID)
      pilot_setFlag( pe, PILOT_INVINC_PLAYER );

   /* Set some flags for consistent behaviour. */
   if (pilot_isFlag(p, PILOT_HOSTILE))
      pilot_setFlag( pe, PILOT_HOSTILE );
   if (pilot_isFlag(p, PILOT_FRIENDLY))
      pilot_setFlag( pe, PILOT_FRIENDLY );

   /* Compute fighter bay bonuses. */
   if (pilot_isFlagRaw( f, PILOT_CARRIED )) {
      /* Damage. */
      if (p->stats.fbay_damage != 1.) {
         pe->intrinsic_stats = ss_statsSetList( pe->intrinsic_stats, SS_TYPE_D_LAUNCH_DAMAGE, p->stats.fbay_damage, 0, 1 );
         pe->intrinsic_stats = ss_statsSetList( pe->intrinsic_stats, SS_TYPE_D_FORWARD_DAMAGE, p->stats.fbay_damage, 0, 1 );
         pe->intrinsic_stats = ss_statsSetList( pe->intrinsic_stats, SS_TYPE_D_TURRET_DAMAGE, p->stats.fbay_damage, 0, 1 );
      }
      /* Health. */
      if (p->stats.fbay_health != 1.) {;
         pe->intrinsic_stats = ss_statsSetList( pe->intrinsic_stats, SS_TYPE_D_ARMOUR_MOD, p->stats.fbay_health, 0, 1 );
         pe->intrinsic_stats = ss_statsSetList( pe->intrinsic_stats, SS_TYPE_D_SHIELD_MOD, p->stats.fbay_health, 0, 1 );
      }
      /* Movement. */
      if (p->stats.fbay_movement != 1.) {;
         pe->intrinsic_stats = ss_statsSetList( pe->intrinsic_stats, SS_TYPE_D_SPEED_MOD, p->stats.fbay_movement, 0, 1 );
         pe->intrinsic_stats = ss_statsSetList( pe->intrinsic_stats, SS_TYPE_D_TURN_MOD, p->stats.fbay_movement, 0, 1 );
         pe->intrinsic_stats = ss_statsSetList( pe->intrinsic_stats, SS_TYPE_D_ACCEL_MOD, p->stats.fbay_movement, 0, 1 );
      }
      /* Update stats. */
      pilot_calcStats( pe );
   }

   /* Add to escort list. */
   if (add != 0)
      escort_addList( p, ship, type, pe->id, 1 );

   return pe->id;
}

/**
 * @brief Creates an escort from a reference.
 *
 *    @param p Parent of the escort (who he's guarding).
 *    @param pe Reference for the escort.
 *    @param pos Position to create escort at.
 *    @param vel Velocity to create escort with.
 *    @param dir Direction to face.
 *    @param type Type of escort.
 *    @param add Whether or not to add it to the escort list.
 *    @param dockslot The outfit slot which launched the escort (-1 if N/A)
 *    @return The ID of the escort on success.
 */
unsigned int escort_createRef( Pilot *p, Pilot *pe,
      const vec2 *pos, const vec2 *vel, double dir,
      EscortType_t type, int add, int dockslot )
{
   if (pilot_get( pe->id ) == NULL) /* Not on stack yet. */
      pilot_addStack( pe ); /* Sets the ID, and resets internals. */
   else
      pilot_reset( pe ); /* Reset internals. */
   pe->parent = p->id;

   /* Make invincible to player. */
   if (pe->parent == PLAYER_ID)
      pilot_setFlag( pe, PILOT_INVINC_PLAYER );

   /* Copy stuff over if necessary. */
   if (pos != NULL)
      memcpy( &pe->solid.pos, pos, sizeof(vec2) );
   if (vel != NULL)
      memcpy( &pe->solid.vel, vel, sizeof(vec2) );
   pe->solid.dir = dir;

   /* Set some flags for consistent behaviour. */
   if (p->faction == FACTION_PLAYER) {
      pilot_setFlag( pe, PILOT_PERSIST );
      pilot_setFlag( pe, PILOT_NOCLEAR );
   }
   if (pilot_isFlag(p, PILOT_HOSTILE))
      pilot_setFlag( pe, PILOT_HOSTILE );
   if (pilot_isFlag(p, PILOT_FRIENDLY))
      pilot_setFlag( pe, PILOT_FRIENDLY );

   /* Add to escort list. */
   if (add != 0)
      escort_addList( p, pe->ship, type, pe->id, 1 );
   pe->dockslot = dockslot;

   return pe->id;
}

/**
 * @brief Clears deployed escorts of a pilot.
 */
int escort_clearDeployed( Pilot *p )
{
   int q = 0;
   /* Iterate backwards so we don't have to care about indices. */
   for (int j=array_size(p->escorts)-1; j>=0; j--) {
      Pilot *pe;
      Escort_t *e = &p->escorts[j];

      /* Only try to dock fighters. */
      if (e->type != ESCORT_TYPE_BAY)
         continue;

      pe = pilot_get( e->id );
      if (pe==NULL)
         continue;
      /* Hack so it can dock. */
      memcpy( &pe->solid.pos, &p->solid.pos, sizeof(vec2) );
      memcpy( &pe->solid.vel, &p->solid.vel, sizeof(vec2) );
      if (pilot_dock( pe, p ))
         WARN(_("Pilot '%s' has escort '%s' docking error!"), p->name, pe->name);
      else
         q++;
   }
   return q;
}

/**
 * @brief Runs an escort command on all of a pilot's escorts.
 *
 *    @param parent Pilot who is giving orders.
 *    @param cmd Order to give.
 *    @param idx Lua index of argument or 0.
 *    @return 0 on success, 1 if no orders given.
 */
static int escort_command( Pilot *parent, const char *cmd, unsigned int idx )
{
   if (array_size(parent->escorts) == 0)
      return 1;

   for (int i=0; i<array_size(parent->escorts); i++) {
      Pilot *e = pilot_get( parent->escorts[i].id );
      if (e == NULL) /* Most likely died. */
         continue;

      pilot_msg( parent, e, cmd, idx );
   }

   return 0;
}

/**
 * @brief Have a pilot order its escorts to attack its target.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_attack( Pilot *parent )
{
   int ret;
   Pilot *t;

   /* Avoid killing self. */
   t = pilot_getTarget( parent );
   if (t == NULL)
      return 1;
   if (t->faction == parent->faction)
      return 1;

   /* Send command. */
   ret = 1;
   if (parent->target != parent->id) {
      lua_pushpilot(naevL, parent->target);
      ret = escort_command( parent, "e_attack", -1 );
      lua_pop(naevL, 1);
   }
   if ((ret == 0) && (parent == player.p)) {
      const char *pltname;
      if (pilot_inRangePilot( parent, t, NULL ) > 0)
         pltname = t->name;
      else
         pltname = _("Unknown");

      if (pilot_isFlag(t, PILOT_DISABLED))
         player_message(_("#gEscorts: #0Destroying %s."), pltname);
      else
         player_message(_("#gEscorts: #0Engaging %s."), pltname);
   }
   return ret;
}

/**
 * @brief Have a pilot order its escorts to hold position.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_hold( Pilot *parent )
{
   int ret = escort_command( parent, "e_hold", 0 );
   if ((ret == 0) && (parent == player.p))
         player_message(_("#gEscorts: #0Holding formation."));
   return ret;
}

/**
 * @brief Have a pilot order its escorts to dock.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_return( Pilot *parent )
{
   int ret = escort_command( parent, "e_return", 0 );
   if ((ret == 0) && (parent == player.p))
      player_message(_("#gEscorts: #0Returning to ship."));
   return ret;
}

/**
 * @brief Have a pilot order its escorts to clear orders.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_clear( Pilot *parent )
{
   int ret = escort_command( parent, "e_clear", 0 );
   if ((ret == 0) && (parent == player.p))
      player_message(_("#gEscorts: #0Clearing orders."));
   return ret;
}

/**
 * @brief Open a dialog for the player to issue a command to an escort.
 *
 *    @param e The pilot for the player to issue an order to.
 *    @return 0 on success, 1 if no orders given.
 */
int escort_playerCommand( Pilot *e )
{
   const char *title, *caption;
   char *choice;
   int ret = 1;

   /* "Attack My Target" order is omitted deliberately since e is your
    * target, making "Attack My Target" a useless command. */
   const char *opts[] = {
      _("Hold Position"),
      _("Return To Ship"),
      _("Clear Orders"),
      _("Cancel")
   };
   const int nopts = 4;

   /* Must not be NULL */
   if (e == NULL)
      return 1;

   title = _("Escort Orders");
   caption = _("Select the order to give to this escort.");

   dialogue_makeChoice( title, caption, nopts );
   for (int i=0; i<nopts; i++)
      dialogue_addChoice( title, caption, opts[i] );

   choice = dialogue_runChoice();
   if (choice != NULL) {
      if (strcmp(choice, opts[0]) == 0) { /* Hold position */
         pilot_msg( player.p, e, "e_hold", 0 );
         ret = 0;
      }
      else if (strcmp(choice, opts[1]) == 0) { /* Return to ship */
         pilot_msg( player.p, e, "e_return", 0 );
         ret = 0;
      }
      else if (strcmp(choice, opts[2]) == 0) { /* Clear orders */
         pilot_msg( player.p, e, "e_clear", 0 );
         ret = 0;
      }
   }
   free( choice );
   return ret;
}

/**
 * @brief Have a pilot order its escorts to jump.
 *
 *    @param parent Pilot giving the order.
 *    @param jp Where to jump.
 */
int escorts_jump( Pilot *parent, JumpPoint *jp )
{
   int ret;
   LuaJump lj;

   lj.destid = jp->targetid;
   lj.srcid = cur_system->id;

   lua_pushjump( naevL, lj );
   ret = escort_command( parent, "hyperspace", -1 );
   lua_pop(naevL, 1);

   if ((ret == 0) && (parent == player.p))
      player_message(_("#gEscorts: #0Jumping."));
   return ret;
}
