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
#include "hook.h"
#include "nstring.h"


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
static int escort_disabled( void *data );
static int escort_command( Pilot *parent, int cmd, int param );
/* Extern */
extern void ai_setPilot( Pilot *p ); /**< from ai.c */


/**
 * @brief Adds an escort to the escort list of a pilot.
 *
 *    @param p Pilot to add escort to.
 *    @param ship Ship of the escort.
 *    @param type Type of the escort.
 *    @param id ID of the pilot representing the escort.
 *    @return 0 on success.
 */
int escort_addList( Pilot *p, char *ship, EscortType_t type, unsigned int id )
{
   p->nescorts++;
   if (p->nescorts == 1)
      p->escorts = malloc(sizeof(Escort_t) * ESCORT_PREALLOC);
   else if (p->nescorts > ESCORT_PREALLOC)
      p->escorts = realloc( p->escorts, sizeof(Escort_t) * p->nescorts );
   p->escorts[p->nescorts-1].ship = strdup(ship);
   p->escorts[p->nescorts-1].type = type;
   p->escorts[p->nescorts-1].id   = id;

   return 0;
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
 *    @return The ID of the escort on success.
 */
unsigned int escort_create( Pilot *p, char *ship,
      Vector2d *pos, Vector2d *vel, double dir,
      EscortType_t type, int add )
{
   Ship *s;
   Pilot *pe;
   char buf[16];
   unsigned int e;
   PilotFlags f;
   unsigned int hook;
   unsigned int parent;

   /* Get important stuff. */
   parent = p->id;
   s = ship_get(ship);
   nsnprintf(buf, 16, "escort*%u", parent);

   /* Set flags. */
   pilot_clearFlagsRaw( f );
   pilot_setFlagRaw( f, PILOT_ESCORT );
   if (type == ESCORT_TYPE_BAY)
      pilot_setFlagRaw( f, PILOT_CARRIED );

   /* Create the pilot. */
   e = pilot_create( s, NULL, p->faction, buf, dir, pos, vel, f, -1 );
   pe = pilot_get(e);
   pe->parent = parent;

   /* Add to escort list. */
   if (add != 0)
      escort_addList( p, ship, type, e );

   /* Hook the disable to lose the count. */
   hook = hook_addFunc( escort_disabled, pe, "disable" );
   pilot_addHook( pe, PILOT_HOOK_DISABLE, hook );

   return e;
}


/**
 * @brief Hook to decrement count when pilot is disabled.
 *
 *    @param data Disabled pilot (should still be valid).
 */
static int escort_disabled( void *data )
{
   int i;
   Pilot *pe, *p;
   Outfit *o;

   /* Get escort that got disabled. */
   pe = (Pilot*) data;

   /* Get parent. */
   p = pilot_get( pe->parent );
   if (p == NULL) /* Parent is no longer with us. */
      return 0;

   /* Remove from escorts list. */
   for (i=0; i<p->nescorts; i++) {
      if (p->escorts[i].id == pe->id) {
         p->nescorts--;
         memmove( &p->escorts[i], &p->escorts[i+1],
               sizeof(Escort_t)*(p->nescorts-i) );
         break;
      }
   }

   /* Remove from deployed list. */
   for (i=0; i<p->noutfits; i++) {
      o = p->outfits[i]->outfit;
      if (o==NULL)
         continue;
      if (outfit_isFighterBay(o)) {
         o = outfit_ammo(o);
         if (outfit_isFighter(o) &&
               (strcmp(pe->ship->name,o->u.fig.ship)==0)) {
            p->outfits[i]->u.ammo.deployed -= 1;
            break;
         }
      }
   }

   return 0;
}


/**
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
      e = pilot_get( parent->escorts[i].id );
      if (e == NULL) /* Most likely died. */
         continue;

      /* Check if command makes sense. */
      if ((cmd == ESCORT_RETURN) && !pilot_isFlag(e, PILOT_CARRIED))
         continue;

      n++; /* Amount of escorts left. */

      /* Prepare AI. */
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

         default:
            WARN("Invalid escort command '%d'.", cmd);
            return -1;
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
   if (parent->target != parent->id)
      ret = escort_command( parent, ESCORT_ATTACK, parent->target );
   if ((ret == 0) && (parent == player.p))
      player_message("\egEscorts: \e0Attacking %s.", t->name);
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
   ret = escort_command( parent, ESCORT_HOLD, -1 );
   if ((ret == 0) && (parent == player.p))
         player_message("\egEscorts: \e0Holding position.");
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
   ret = escort_command( parent, ESCORT_RETURN, -1 );
   if ((ret == 0) && (parent == player.p))
      player_message("\egEscorts: \e0Returning to ship.");
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
   ret = escort_command( parent, ESCORT_CLEAR, -1);
   if ((ret == 0) && (parent == player.p))
      player_message("\egEscorts: \e0Clearing orders.");
   return ret;
}


