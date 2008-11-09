/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file escort.c
 *
 * @brief Handles the player's escorts.
 */

#include "escort.h"

#include "naev.h"
#include "log.h"
#include "player.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_space.h"


#define ESCORT_PREALLOC    8 /**< Number of escorts to automatically allocate first. */


/*
 * Different functions.
 */
#define ESCORT_ATTACK      1 /**< Attack order. */
#define ESCORT_HOLD        2 /**< Hold order. */
#define ESCORT_RETURN      3 /**< Return to ship order. */
#define ESCORT_CLEAR       4 /**< Clear orders. */


/*
 * Prototypes.
 */
/* Static */
static int escort_command( Pilot *parent, int cmd, int param );
/* Extern */
extern void ai_setPilot( Pilot *p );


/**
 * @fn int escort_create( unsigned int parent, char *ship,
 *       Vector2d *pos, Vector2d *vel, int carried )
 *
 * @brief Creates an escort.
 *
 *    @param parent Parent of the escort (who he's guarding).
 *    @param ship Name of the ship escort should have.
 *    @param pos Position to create escort at.
 *    @param vel Velocity to create escort with.
 *    @param carried Does escort come out of the parent?
 */
int escort_create( unsigned int parent, char *ship,
      Vector2d *pos, Vector2d *vel, int carried )
{
   Ship *s;
   Pilot *p, *pe;
   char buf[16];
   unsigned int e, f;
   double dir;

   /* Get important stuff. */
   p = pilot_get(parent);
   s = ship_get(ship);
   snprintf(buf, 16, "tpl/escort*%u", parent);

   /* Set flags. */
   f = PILOT_ESCORT;
   if (carried) f |= PILOT_CARRIED;

   /* Get the direction. */
   if (carried) dir = p->solid->dir;
   else dir = 0.;

   /* Create the pilot. */
   e = pilot_create( s, NULL, p->faction, buf, dir, pos, vel, f );
   pe = pilot_get(e);
   pe->parent = parent;

   /* Add to escort list. */
   p->nescorts++;
   if (p->nescorts == 1)
      p->escorts = malloc(sizeof(unsigned int) * ESCORT_PREALLOC);
   else if (p->nescorts > ESCORT_PREALLOC)
      p->escorts = realloc( p->escorts, sizeof(unsigned int) * p->nescorts );
   p->escorts[p->nescorts-1] = e;

   return 0;
}


/**
 * @fn static int escort_command( Pilot *parent, int cmd, int param )
 *
 * @brief Runs an escort command on all of a pilot's escorts.
 *
 *    @param parent Pilot who is giving orders.
 *    @param cmd Order to give.
 *    @param param Parameter for order.
 *    @return 0 on success, 1 if no orders given.
 */
static int escort_command( Pilot *parent, int cmd, int param )
{
   int i, n;
   lua_State *L;
   Pilot *e;
   char *buf;

   if (parent->nescorts == 0)
      return 1;

   n = 0;
   for (i=0; i<parent->nescorts; i++) {
      e = pilot_get( parent->escorts[i] );
      if (e == NULL) /* Most likely died. */
         continue;

      /* Check if command makes sense. */
      if ((cmd == ESCORT_RETURN) && !pilot_isFlag(e, PILOT_CARRIED))
         continue;

      n++; /* Amount of escorts left. */

      /* Prepare ai. */
      ai_setPilot( e );

      /* Set up stack. */
      L = e->ai->L;
      switch (cmd) {
         case ESCORT_ATTACK:
            buf = "e_attack";
            break;
         case ESCORT_HOLD:
            buf = "e_hold";
            break;
         case ESCORT_RETURN:
            buf = "e_return";
            break;
         case ESCORT_CLEAR:
            buf = "e_clear";
            break;
      }
      lua_getglobal(L, buf);
      if (param >= 0)
         lua_pushnumber(L, param);

      /* Run command. */
      if (lua_pcall(L, (param >= 0) ? 1 : 0, 0, 0))
         WARN("Pilot '%s' ai -> '%s': %s", e->name,
               buf, lua_tostring(L,-1));
   }

   return !n;
}


/**
 * @fn int escorts_attack( Pilot *parent )
 *
 * @brief Have a pilot order it's escorts to attack it's target.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_attack( Pilot *parent )
{
   int ret;
   Pilot *t;

   ret = 1;
   if (parent->target != parent->id)
      ret = escort_command( parent, ESCORT_ATTACK, parent->target );
   if ((ret == 0) && (parent == player)) {
      t = pilot_get(parent->target);
      if (t != NULL)
         player_message("Escorts: Attacking %s.", t->name);
   }
   return ret;
}
/**
 * @fn int escorts_hold( Pilot *parent )
 *
 * @brief Have a pilot order it's escorts to hold position.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_hold( Pilot *parent )
{
   int ret;
   ret = escort_command( parent, ESCORT_HOLD, -1 );
   if ((ret == 0) && (parent == player))
         player_message("Escorts: Holding position.");
   return ret;
}
/**
 * @fn int escorts_return( Pilot *parent )
 *
 * @brief Have a pilot order it's escorts to dock.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_return( Pilot *parent )
{
   int ret;
   ret = escort_command( parent, ESCORT_RETURN, -1 );
   if ((ret == 0) && (parent == player))
      player_message("Escorts: Returning to ship.");
   return ret;
}
/**
 * @fn int escorts_clear( Pilot *parent )
 *
 * @brief Have a pilot order it's escorts to clear orders.
 *
 *    @param parent Pilot giving the order.
 */
int escorts_clear( Pilot *parent )
{
   int ret;
   ret = escort_command( parent, ESCORT_CLEAR, -1);
   if ((ret == 0) && (parent == player))
      player_message("Escorts: Clearing orders.");
   return ret;
}


