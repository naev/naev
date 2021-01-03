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

#include "dialogue.h"
#include "hook.h"
#include "log.h"
#include "nlua.h"
#include "nluadef.h"
#include "nstring.h"
#include "player.h"


#define ESCORT_PREALLOC    8 /**< Number of escorts to automatically allocate first. */



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
int escort_addList( Pilot *p, char *ship, EscortType_t type,
      unsigned int id, int persist )
{
   p->nescorts++;
   p->escorts = realloc( p->escorts, sizeof(Escort_t) * p->nescorts );
   p->escorts[p->nescorts-1].ship = strdup(ship);
   p->escorts[p->nescorts-1].type = type;
   p->escorts[p->nescorts-1].id   = id;
   p->escorts[p->nescorts-1].persist = persist;

   return 0;
}


/**
 * @brief Remove all escorts from a pilot.
 *
 *    @param p Pilot to remove escorts from.

 */
void escort_freeList( Pilot *p ) {
   int i;

   for (i=0; i<p->nescorts; i++)
      free(p->escorts[i].ship);
   free(p->escorts);
   p->escorts = NULL;
   p->nescorts = 0;
}


/**
 * @brief Remove from escorts list.
 *
 *    @param p Pilot to remove escort from.
 *    @param i index of the pilot representing the escort.

 */
void escort_rmListIndex( Pilot *p, int i ) {
   p->nescorts--;
   free(p->escorts[i].ship);
   memmove( &p->escorts[i], &p->escorts[i+1],
         sizeof(Escort_t)*(p->nescorts-i) );
}


/**
 * @brief Remove from escorts list.
 *
 *    @param p Pilot to remove escort from.
 *    @param id ID of the pilot representing the escort.

 */
void escort_rmList( Pilot *p, unsigned int id ) {
   int i;

   for (i=0; i<p->nescorts; i++) {
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
unsigned int escort_create( Pilot *p, char *ship,
      Vector2d *pos, Vector2d *vel, double dir,
      EscortType_t type, int add, int dockslot )
{
   Ship *s;
   Pilot *pe;
   unsigned int e;
   PilotFlags f;
   unsigned int parent;

   /* Get important stuff. */
   parent = p->id;
   s = ship_get(ship);

   /* Set flags. */
   pilot_clearFlagsRaw( f );
   pilot_setFlagRaw( f, PILOT_NOJUMP );
   pilot_setFlagRaw( f, PILOT_PERSIST );
   if (type == ESCORT_TYPE_BAY)
      pilot_setFlagRaw( f, PILOT_CARRIED );

   /* Create the pilot. */
   e = pilot_create( s, NULL, p->faction, "escort", dir, pos, vel, f, parent, dockslot );
   pe = pilot_get(e);
   pe->parent = parent;

   /* Add to escort list. */
   if (add != 0)
      escort_addList( p, ship, type, e, 1 );

   return e;
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
   int i;
   Pilot *e;

   if (parent->nescorts == 0)
      return 1;

   for (i=0; i<parent->nescorts; i++) {
      e = pilot_get( parent->escorts[i].id );
      if (e == NULL) /* Most likely died. */
         continue;

      pilot_msg(parent, e, cmd, idx);
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
   t = pilot_get(parent->target);
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
   if ((ret == 0) && (parent == player.p))
      player_message(_("#gEscorts: #0Attacking %s."), t->name);
   return ret;
}
/**
 * @brief Have a pilot order its escorts to hold position.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_hold( Pilot *parent )
{
   int ret;
   ret = escort_command( parent, "e_hold", 0 );
   if ((ret == 0) && (parent == player.p))
         player_message(_("#gEscorts: #0Holding position."));
   return ret;
}
/**
 * @brief Have a pilot order its escorts to dock.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_return( Pilot *parent )
{
   int ret;
   ret = escort_command( parent, "e_return", 0 );
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
   int ret;
   ret = escort_command( parent, "e_clear", 0 );
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
   int i;
   const char *title, *caption, *ret;

   /* "Attack My Target" order is omitted deliberately since e is your
    * target, making "Attack My Target" a useless command. */
   const char *opts[] = {
      _("Hold Position"),
      _("Return To Ship"),
      _("Clear Orders"),
      _("Cancel")
   };
   int nopts = 4;

   /* Must not be NULL */
   if (e == NULL)
      return 1;

   title = _("Game Speed");
   caption = _("Select the order to give to this escort.");

   dialogue_makeChoice( title, caption, nopts );
   for (i=0; i<nopts; i++) {
      dialogue_addChoice( title, caption, opts[i] );
   }

   ret = dialogue_runChoice();
   if (ret != NULL) {
      if (strcmp(ret, opts[0]) == 0) { /* Hold position */
         pilot_msg( player.p, e, "e_hold", 0 );
         return 0;
      }
      else if (strcmp(ret, opts[1]) == 0) { /* Return to ship */
         pilot_msg( player.p, e, "e_return", 0 );
         return 0;
      }
      else if (strcmp(ret, opts[2]) == 0) { /* Clear orders */
         pilot_msg( player.p, e, "e_clear", 0 );
         return 0;
      }
   }
   return 1;
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
