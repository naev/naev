/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file pilot_weapon.c
 *
 * @brief Handles pilot weapon sets which server to manage and interface with
 * active outfits..
 *
 * The basic approach is a flag and sweep operation. Outfits are flagged based
 * on what weapon sets they belong to when weapon sets change or weapons are
 * fired. Afterwards, the weapons will be updated every iteration to fire or
 * change states as necessary.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "array.h"
#include "escort.h"
#include "log.h"
#include "nlua_pilotoutfit.h"
#include "pilot.h"
#include "pilot_ship.h"
#include "player.h"
#include "player_autonav.h"
#include "sound.h"
#include "spfx.h"
#include "weapon.h"

/*
 * Prototypes.
 */
static void pilot_weapSetUpdateOutfits( Pilot *p, PilotWeaponSet *ws );
static int  pilot_shootWeaponSetOutfit( Pilot *p, const Outfit *o,
                                        const Target *target, double time,
                                        int aim );
static void pilot_weapSetUpdateRange( const Pilot *p, PilotWeaponSet *ws );

/**
 * @brief Gets a weapon set from id.

 *    @param p Pilot to get weapon set from.
 *    @param id ID of the weapon set.
 *    @return The weapon set matching id.
 */
PilotWeaponSet *pilot_weapSet( Pilot *p, int id )
{
   return &p->weapon_sets[id];
}

/**
 * @brief Useful function for AI, clears activeness of all weapon sets.
 */
void pilot_weapSetAIClear( Pilot *p )
{
   for ( int i = 0; i < PILOT_WEAPON_SETS; i++ ) {
      PilotWeaponSet *ws = &p->weapon_sets[i];
      ws->active         = 0;
   }
}

static int pilot_weapSetPressToggle( Pilot *p, PilotWeaponSet *ws )
{
   int ret = 0;
   /* Toggle state. */
   for ( int j = 0; j < array_size( ws->slots ); j++ ) {
      PilotOutfitSlot *pos = p->outfits[ws->slots[j].slotid];
      if ( pos->outfit == NULL )
         continue;
      if ( !( pos->flags & PILOTOUTFIT_TOGGLEABLE ) )
         continue;
      /* Holding has priority over toggling. */
      if ( pos->flags & PILOTOUTFIT_ISON_HOLD )
         continue;

      /* Flip state to ON/OFF. */
      if ( pos->flags & PILOTOUTFIT_ISON ) {
         pos->flags &= ~PILOTOUTFIT_ISON_TOGGLE;
      } else {
         pos->flags |= PILOTOUTFIT_ISON_TOGGLE;
         if ( ws->volley )
            pos->flags |= PILOTOUTFIT_VOLLEY;
         if ( ws->inrange )
            pos->flags |= PILOTOUTFIT_INRANGE;
         if ( ws->manual )
            pos->flags |= PILOTOUTFIT_MANUAL;
      }
      ret = 1;
   }
   return ret;
}

/**
 * @brief Handles a weapon set press.
 *
 *    @param p Pilot the weapon set belongs to.
 *    @param id ID of the weapon set.
 *    @param type Is +1 if it's a press or -1 if it's a release.
 *    @return Whether or not something changed.
 */
int pilot_weapSetPress( Pilot *p, int id, int type )
{
   PilotWeaponSet *ws  = pilot_weapSet( p, id );
   int             ret = 0;
   WeaponSetType   t;

   /* Case no outfits. */
   if ( ws->slots == NULL )
      return 0;

   /* Must not be disabled or cooling down. */
   if ( ( pilot_isDisabled( p ) ) || ( pilot_isFlag( p, PILOT_COOLDOWN ) ) )
      return 0;

   /* If not advanced we'll override the types. */
   if ( p->advweap )
      t = ws->type;
   else {
      if ( id < 2 )
         t = WEAPSET_TYPE_HOLD;
      else
         t = WEAPSET_TYPE_DEFAULT;
   }

   /* Handle fire groups. */
   switch ( t ) {
   case WEAPSET_TYPE_DEFAULT:
      /* Tap is toggle, hold is hold. */
      if ( type < 0 ) {
         ret        = ( ws->active != 0 );
         ws->active = 0;
      } else if ( type > 1 ) {
         ret        = ( ws->active != 1 );
         ws->active = 1;
      } else {
         ret        = ( ws->active != 0 );
         ws->active = 0;
         ret |= pilot_weapSetPressToggle( p, ws );
      }
      break;

   case WEAPSET_TYPE_TOGGLE:
      /* This just toggles an outfit on until it turns off. If it is on, it
       * toggles it off instead. */
      /* Only care about presses. */
      if ( type < 0 )
         break;
      ret = pilot_weapSetPressToggle( p, ws );
      break;

   case WEAPSET_TYPE_HOLD:
      /* Activation philosophy here is to turn on while pressed and off
       * when it's not held anymore. */

      /* Clear change variables. */
      if ( type > 0 ) {
         ret        = ( ws->active != 1 );
         ws->active = 1;
      } else if ( type < 0 ) {
         ret        = ( ws->active != 0 );
         ws->active = 0;
      }
      break;
   }
   return ret;
}

/**
 * @brief Updates the local state of all the pilot's outfits based on the weapon
 * sets.
 */
void pilot_weapSetUpdateOutfitState( Pilot *p )
{
   int non, noff;
   int breakstealth;

   /* First pass to remove all hold flags, as we recompute it. */
   for ( int i = 0; i < array_size( p->outfits ); i++ ) {
      PilotOutfitSlot *pos = p->outfits[i];
      pos->flags &= ~PILOTOUTFIT_ISON_HOLD;
   }

   /* Now mark all the outfits as on or off. */
   for ( int i = 0; i < PILOT_WEAPON_SETS; i++ ) {
      PilotWeaponSet *ws = &p->weapon_sets[i];
      if ( ws->slots == NULL )
         continue;

      /* Only care if on. */
      if ( !ws->active )
         continue;

      /* Only care about HOLD sets. */
      if ( p->advweap && ( ws->type == WEAPSET_TYPE_TOGGLE ) )
         continue;

      /* Time to mark as on. */
      for ( int j = 0; j < array_size( ws->slots ); j++ ) {
         PilotOutfitSlot *pos = p->outfits[ws->slots[j].slotid];
         if ( pos->outfit == NULL )
            continue;
         /* If held, we clear the toggle. */
         pos->flags &= ~PILOTOUTFIT_ISON_TOGGLE;
         pos->flags |= PILOTOUTFIT_ISON_HOLD;
         if ( ws->volley )
            pos->flags |= PILOTOUTFIT_VOLLEY;
         if ( ws->inrange )
            pos->flags |= PILOTOUTFIT_INRANGE;
         if ( ws->manual )
            pos->flags |= PILOTOUTFIT_MANUAL;
      }
   }

   /* Last pass figures out what to do. */
   breakstealth = 0;
   non = noff           = 0;
   pilotoutfit_modified = 0;
   for ( int i = 0; i < array_size( p->outfits ); i++ ) {
      PilotOutfitSlot *pos = p->outfits[i];
      const Outfit    *o   = pos->outfit;
      if ( o == NULL )
         continue;
      if ( !( pos->flags & PILOTOUTFIT_TOGGLEABLE ) )
         continue;

      if ( pos->flags & ( PILOTOUTFIT_ISON_TOGGLE | PILOTOUTFIT_ISON_HOLD ) )
         pos->flags |= PILOTOUTFIT_ISON;
      else
         pos->flags &= ~( PILOTOUTFIT_ISON | PILOTOUTFIT_DYNAMIC_FLAGS );

      /* Se whether to turn on or off. */
      if ( pos->flags & PILOTOUTFIT_ISON ) {
         /* Weapons are handled separately. */
         if ( outfit_isWeapon( o ) && ( outfit_luaOntoggle( o ) == LUA_NOREF ) )
            continue;

         if ( pos->state == PILOT_OUTFIT_OFF ) {
            int n = pilot_outfitOn( p, pos );
            if ( ( n > 0 ) &&
                 !outfit_isProp( pos->outfit, OUTFIT_PROP_STEALTH_ON ) )
               breakstealth = 1;
            else
               pos->flags &= ~( PILOTOUTFIT_ISON | PILOTOUTFIT_DYNAMIC_FLAGS );
            non += n;
         }
      } else {
         if ( pos->state == PILOT_OUTFIT_ON )
            noff += pilot_outfitOff( p, pos, 1 );
      }
   }

   /* Now update stats and shit as necessary. */
   if ( ( non + noff > 0 ) || pilotoutfit_modified ) {
      /* pilot_destealth should run calcStats already. */
      if ( pilot_isFlag( p, PILOT_STEALTH ) && breakstealth )
         pilot_destealth( p );
      else
         pilot_calcStats( p );
   }
}

/**
 * @brief Updates the pilot's weapon sets.
 *
 *    @param p Pilot to update.
 */
void pilot_weapSetUpdate( Pilot *p )
{
   int    n, shotweap, target_set, breakstealth;
   double time;
   Target wt;

   if ( pilot_isFlag( p, PILOT_HYP_BEGIN ) )
      return;

   breakstealth         = 0;
   shotweap             = 0;
   n                    = 0;
   target_set           = 0;
   pilotoutfit_modified = 0;
   for ( int i = 0; i < array_size( p->outfits ); i++ ) {
      PilotOutfitSlot *pos = p->outfits[i];
      const Outfit    *o   = pos->outfit;
      int              volley;
      int              shot = 0;
      if ( o == NULL )
         continue;
      if ( !( pos->flags & PILOTOUTFIT_TOGGLEABLE ) )
         continue;
      if ( !( pos->flags & PILOTOUTFIT_ISON ) )
         continue;
      if ( !outfit_isWeapon( o ) )
         continue;

      /* @TODO Make beams not fire all at once. */
      volley = ( ( pos->flags & PILOTOUTFIT_VOLLEY ) || outfit_isBeam( o ) );

      /* For non-volley mode we want to run once per outfit type. */
      if ( !volley ) {
         int s = 0;
         for ( int j = 0; j < i; j++ ) {
            const PilotOutfitSlot *posj = p->outfits[j];
            if ( posj->flags != PILOTOUTFIT_ISON )
               continue;
            /* Found a match. */
            if ( posj->outfit == o ) {
               s = 1;
               break;
            }
         }
         if ( s != 0 )
            continue;
      }

      /* Only "locked on" outfits. */
      if ( outfit_isSeeker( o ) && ( pos->u.ammo.lockon_timer > 0. ) )
         continue;

      /* Lazy target setting. */
      if ( !target_set ) {
         pilot_weaponTarget( p, &wt );
         target_set = 1;
      }
      time = weapon_targetFlyTime( o, p, &wt );

      /* Only "inrange" outfits.
       * XXX for simplicity we are using pilot position / velocity instead of
       * mount point, which might be a bit off. */
      if ( ( pos->flags & PILOTOUTFIT_INRANGE ) && !outfit_isFighterBay( o ) ) {
         /* Check range for different types. */
         if ( time <= 0. ) /* Beam will have a fly time of INFINITY in range, or
                              -1. otherwise. */
            continue;
         else if ( outfit_isBolt( o ) ) {
            if ( pilot_outfitRange( p, o ) / outfit_speed( o ) < time )
               continue;
         } else if ( outfit_isLauncher( o ) ) {
            if ( outfit_launcherDuration( o ) * p->stats.launch_range *
                    p->stats.weapon_range <
                 time )
               continue;
         }

         /* Must be in aiming arc if applicable. */
         if ( !weapon_inArc( o, p, &wt, &p->solid.pos, &p->solid.vel,
                             p->solid.dir, time ) )
            continue;
      }

      /* Shoot the weapon of the weaponset. */
      if ( volley )
         shot = pilot_shootWeapon( p, pos, &wt, time,
                                   !( pos->flags & PILOTOUTFIT_MANUAL ) );
      else
         shot = pilot_shootWeaponSetOutfit(
            p, o, &wt, time, !( pos->flags & PILOTOUTFIT_MANUAL ) );
      shotweap += shot;
      n++;

      if ( shot && !outfit_isProp( pos->outfit, OUTFIT_PROP_STEALTH_ON ) )
         breakstealth = 1;
   }

   /* Now update stats and shit as necessary. */
   if ( ( n > 0 ) || pilotoutfit_modified ) {
      /* pilot_destealth should run calcStats already. */
      if ( pilot_isFlag( p, PILOT_STEALTH ) && breakstealth )
         pilot_destealth( p );
      else
         pilot_calcStats( p );

      /* Firing stuff aborts active cooldown. */
      if ( pilot_isFlag( p, PILOT_COOLDOWN ) && ( shotweap > 0 ) )
         pilot_cooldownEnd( p, NULL );

      /* Trigger onshoot after stealth gets broken. */
      if ( shotweap > 0 ) {
         pilot_shipLOnshootany( p );
         pilot_outfitLOnshootany( p );
      }
   }
}

/**
 * @brief Updates the outfits with their current weapon set level.
 */
static void pilot_weapSetUpdateOutfits( Pilot *p, PilotWeaponSet *ws )
{
   /* Have to update slots potentially. */
   for ( int i = 0; i < array_size( p->outfits ); i++ ) {
      PilotOutfitSlot *o = p->outfits[i];
      o->weapset         = -1;
      for ( int j = 0; j < PILOT_WEAPON_SETS; j++ ) {
         if ( pilot_weapSetCheck( p, j, o ) != -1 ) {
            o->weapset = j;
            break;
         }
      }
   }

   /* Update range. */
   pilot_weapSetUpdateRange( p, ws );
}

/**
 * @brief Checks the current weapon set type.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set to check.
 *    @return The type of the weapon set.
 */
int pilot_weapSetTypeCheck( Pilot *p, int id )
{
   const PilotWeaponSet *ws = pilot_weapSet( p, id );
   return ws->type;
}

/**
 * @brief Changes the weapon sets mode.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param type The mode (a WEAPSET_TYPE constant).
 */
void pilot_weapSetType( Pilot *p, int id, WeaponSetType type )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   ws->type           = type;
   ws->active         = 0; /* Disable no matter what. */
   // p->autoweap        = 0;
   pilot_weapSetUpdateOutfits( p, ws );
}

/**
 * @brief Checks the current weapon set inrange property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set to check.
 *    @return The inrange mode of the weapon set.
 */
int pilot_weapSetInrangeCheck( Pilot *p, int id )
{
   const PilotWeaponSet *ws = pilot_weapSet( p, id );
   return ws->inrange;
}

/**
 * @brief Changes the weapon set inrange property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param inrange Whether or not to only fire at stuff in range.
 */
void pilot_weapSetInrange( Pilot *p, int id, int inrange )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   ws->inrange        = inrange;
   // p->autoweap        = 0;
}

/**
 * @brief Checks the current weapon set manual property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set to check.
 *    @return The manual mode of the weapon set.
 */
int pilot_weapSetManualCheck( Pilot *p, int id )
{
   const PilotWeaponSet *ws = pilot_weapSet( p, id );
   return ws->manual;
}

/**
 * @brief Changes the weapon set manual property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param manual Whether or not to have manual aiming.
 */
void pilot_weapSetManual( Pilot *p, int id, int manual )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   ws->manual         = manual;
   // p->autoweap        = 0;
}

/**
 * @brief Checks the current weapon set volley property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set to check.
 *    @return The volley mode of the weapon set.
 */
int pilot_weapSetVolleyCheck( Pilot *p, int id )
{
   const PilotWeaponSet *ws = pilot_weapSet( p, id );
   return ws->volley;
}

/**
 * @brief Changes the weapon set volley property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param volley Whether or not to have volley aiming.
 */
void pilot_weapSetVolley( Pilot *p, int id, int volley )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   ws->volley         = volley;
   // p->autoweap        = 0;
}

/**
 * @brief Gets the name of a weapon set.
 */
const char *pilot_weapSetName( Pilot *p, int id )
{
   static char     setname[STRMAX_SHORT];
   const char     *base, *type;
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   type = base = NULL;

   switch ( ws->type ) {
   case WEAPSET_TYPE_DEFAULT:
      type = p_( "weapset", "Default" );
      break;
   case WEAPSET_TYPE_TOGGLE:
      type = p_( "weapset", "Toggle" );
      break;
   case WEAPSET_TYPE_HOLD:
      type = p_( "weapset", "Hold" );
      break;
   default:
      type = p_( "weapset", "Unknown" );
      break;
   }

   if ( array_size( ws->slots ) == 0 )
      base = _( "Empty" );
   else {
      const Outfit *o        = NULL;
      int           not_same = 0;
      int           has_weap = 0;
      int           has_util = 0;
      int           has_stru = 0;
      for ( int i = 0; i < array_size( ws->slots ); i++ ) {
         const PilotOutfitSlot *pos = p->outfits[ws->slots[i].slotid];
         if ( pos->outfit == NULL ) /* Ignore empty slots. */
            continue;
         if ( !pilot_slotIsToggleable( pos ) ) /* Ignore non-active. */
            continue;
         if ( o == NULL )
            o = pos->outfit;
         else if ( o != pos->outfit )
            not_same = 1;
         switch ( pos->sslot->slot.type ) {
         case OUTFIT_SLOT_STRUCTURE:
            has_stru++;
            break;
         case OUTFIT_SLOT_UTILITY:
            has_util++;
            break;
         case OUTFIT_SLOT_WEAPON:
            has_weap++;
            break;
         default:
            break;
         }
      }
      if ( o == NULL )
         base = _( "Empty" );
      else if ( not_same == 0 )
         base = outfit_name( o );
      else if ( has_weap && !has_util && !has_stru )
         base = p_( "weapset", "Weapons" );
      else if ( !has_weap && has_util && !has_stru )
         base = p_( "weapset", "Utilities" );
      else if ( !has_weap && !has_util && has_stru )
         base = p_( "weapset", "Structurals" );
      else
         base = p_( "weapset", "Mixed" );
   }

   if ( p->advweap )
      snprintf( setname, sizeof( setname ), p_( "weapset", "%s - %s" ), type,
                base );
   else
      snprintf( setname, sizeof( setname ), "%s", base );
   return setname;
}

/**
 * @brief Adds an outfit to a weapon set.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param o Outfit to add.
 */
void pilot_weapSetAdd( Pilot *p, int id, const PilotOutfitSlot *o )
{
   PilotWeaponSetOutfit *slot;
   PilotWeaponSet       *ws = pilot_weapSet( p, id );

   /* Create if needed. */
   if ( ws->slots == NULL )
      ws->slots = array_create( PilotWeaponSetOutfit );

   /* Check if already there. */
   for ( int i = 0; i < array_size( ws->slots ); i++ ) {
      if ( ws->slots[i].slotid != o->id )
         continue;
      return;
   }

   /* Add it. */
   slot         = &array_grow( &ws->slots );
   slot->slotid = o->id;
   if ( o->outfit != NULL )
      slot->range2 = pow2( pilot_outfitRange( p, o->outfit ) );
   else
      slot->range2 = 0.;
   pilot_weapSetUpdateOutfits( p, ws );
}

/**
 * @brief Removes a slot from a weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set.
 *    @param o Outfit to remove.
 */
void pilot_weapSetRm( Pilot *p, int id, const PilotOutfitSlot *o )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   for ( int i = 0; i < array_size( ws->slots ); i++ ) {
      if ( ws->slots[i].slotid != o->id )
         continue;

      array_erase( &ws->slots, &ws->slots[i], &ws->slots[i + 1] );
      pilot_weapSetUpdateOutfits( p, ws );
      return;
   }
}

/**
 * @brief Clears a weapon set.
 *
 *    @param p Pilot to clear weapon set.
 *    @param id Weapon set to clear.
 */
void pilot_weapSetClear( Pilot *p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   ws->type           = WEAPSET_TYPE_TOGGLE;
   array_free( ws->slots );
   ws->slots = NULL;
   // p->autoweap = 0;

   /* Update if needed. */
   pilot_weapSetUpdateOutfits( p, ws );
}

/**
 * @brief Checks to see if a slot is in a weapon set.
 */
int pilot_weapSetInSet( Pilot *p, int id, const PilotOutfitSlot *o )
{
   const PilotWeaponSet *ws = pilot_weapSet( p, id );
   for ( int i = 0; i < array_size( ws->slots ); i++ ) {
      /* Must match the current weapon. */
      if ( ws->slots[i].slotid != o->id )
         continue;
      return 0;
   }
   /* Not found. */
   return -1;
}

/**
 * @brief Checks to see if a slot is in a weapon set and usable.
 *
 *    @param p Pilot to check.
 *    @param id ID of the weapon set.
 *    @param o Outfit slot to check.
 *    @return The level to which it belongs (or -1 if it isn't set).
 */
int pilot_weapSetCheck( Pilot *p, int id, const PilotOutfitSlot *o )
{
   const PilotWeaponSet *ws = pilot_weapSet( p, id );
   for ( int i = 0; i < array_size( ws->slots ); i++ ) {
      /* Must match the current weapon. */
      if ( ws->slots[i].slotid != o->id )
         continue;
      return 0;
   }
   /* Not found. */
   return -1;
}

/**
 * @brief Update the weapon sets given pilot stat changes.
 *
 *    @param p Pilot to update.
 */
void pilot_weapSetUpdateStats( Pilot *p )
{
   for ( int i = 0; i < PILOT_WEAPON_SETS; i++ )
      pilot_weapSetUpdateRange( p, &p->weapon_sets[i] );
}

/**
 * @brief Updates the weapon range for a pilot weapon set.
 *
 *    @param p Pilot whose weapon set is being updated.
 *    @param ws Weapon Set to update range for.
 */
static void pilot_weapSetUpdateRange( const Pilot *p, PilotWeaponSet *ws )
{
   double range, speed;
   double range_accum;
   int    range_num;
   double speed_accum;
   int    speed_num;

   /* Calculate ranges. */
   range_accum   = 0.;
   range_num     = 0;
   speed_accum   = 0.;
   speed_num     = 0;
   ws->range_min = INFINITY;

   for ( int i = 0; i < array_size( ws->slots ); i++ ) {
      PilotOutfitSlot *pos = p->outfits[ws->slots[i].slotid];
      if ( pos->outfit == NULL )
         continue;

      /* Empty Launchers aren't valid */
      if ( outfit_isLauncher( pos->outfit ) && ( pos->u.ammo.quantity <= 0 ) )
         continue;

      /* Get range. */
      range = pilot_outfitRange( p, pos->outfit );
      if ( range >= 0. ) {
         /* Calculate. */
         range_accum += range;
         range_num++;

         /* Add minimum. */
         ws->range_min = MIN( range, ws->range_min );
      }

      /* Get speed. */
      speed = outfit_speed( pos->outfit );
      if ( speed >= 0. ) {
         /* Calculate. */
         speed_accum += speed;
         speed_num++;
      }
   }

   /* Postprocess range. */
   if ( range_num == 0 )
      ws->range = 0;
   else
      ws->range = range_accum / (double)range_num;

   /* Postprocess speed. */
   if ( speed_num == 0 )
      ws->speed = 0;
   else
      ws->speed = speed_accum / (double)speed_num;
}

/**
 * @brief Gets the minimum range of the current pilot weapon set.
 *
 *    @param p Pilot to get the minimum range of.
 *    @param id ID of weapon set to get the minimum range of.
 */
double pilot_weapSetRangeMin( Pilot *p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   return ws->range_min;
}

/**
 * @brief Gets the range of the current pilot weapon set.
 *
 *    @param p Pilot to get the range of.
 *    @param id ID of weapon set to get the range of.
 */
double pilot_weapSetRange( Pilot *p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   return ws->range;
}

/**
 * @brief Gets the speed of the current pilot weapon set.
 *
 *    @param p Pilot to get the speed of.
 *    @param id ID of weapon set to get the speed of.
 */
double pilot_weapSetSpeed( Pilot *p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );
   return ws->speed;
}

/**
 * @brief Gets the ammo of the current pilot weapon set.
 *
 *    @param p Pilot to get the speed of.
 *    @param id ID of weapon set to get the speed of.
 */
double pilot_weapSetAmmo( Pilot *p, int id )
{
   PilotWeaponSet *ws    = pilot_weapSet( p, id );
   double          ammo  = 0.;
   int             nammo = 0;
   for ( int i = 0; i < array_size( ws->slots ); i++ ) {
      int              amount;
      PilotOutfitSlot *s = p->outfits[ws->slots[i].slotid];
      amount             = pilot_maxAmmoO( p, s->outfit );
      if ( amount > 0 ) {
         ammo += (double)s->u.ammo.quantity / (double)amount;
         nammo++;
      }
   }
   return ( nammo == 0 ) ? 1. : ammo / (double)nammo;
}

/**
 * @brief Cleans up a weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set to clean up.
 */
void pilot_weapSetCleanup( Pilot *p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet( p, id );

   array_free( ws->slots );
   ws->slots = NULL;

   /* Update range. */
   pilot_weapSetUpdateRange( p, ws );
}

/**
 * @brief Frees a pilot's weapon sets.
 */
void pilot_weapSetFree( Pilot *p )
{
   for ( int i = 0; i < PILOT_WEAPON_SETS; i++ )
      pilot_weapSetCleanup( p, i );
}

/**
 * @brief Lists the items in a pilot weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set.
 *    @return The array (array.h) of pilot weaponset outfits.
 */
PilotWeaponSetOutfit *pilot_weapSetList( Pilot *p, int id )
{
   return pilot_weapSet( p, id )->slots;
}

/**
 * @brief Stops a beam outfit and sets delay as appropriate.
 *
 *    @param p Pilot that is firing.
 *    @param w Pilot's beam outfit.
 */
void pilot_stopBeam( const Pilot *p, PilotOutfitSlot *w )
{
   double rate_mod, energy_mod, used;

   /* There's nothing to do if the beam isn't active. */
   if ( w->u.beamid == 0 )
      return;

   /* Safeguard against a nasty race condition. */
   if ( w->outfit == NULL ) {
      w->u.beamid = 0;
      return;
   }

   /* Lua test to stop beam. */
   /*
      if ((outfit_luaOnshoot(w->outfit)!= LUA_NOREF) &&
      !pilot_outfitLOnshoot( p, w, 0 ))
      return;
      */

   /* Calculate rate modifier. */
   pilot_getRateMod( &rate_mod, &energy_mod, p, w->outfit );

   /* Beam duration used. Compensate for the fact its duration might have
    * been shortened by heat. */
   used = outfit_duration( w->outfit ) - w->timer;

   w->timer    = rate_mod * MAX( outfit_beamMinDelay( w->outfit ),
                                 ( used / outfit_duration( w->outfit ) ) *
                                    outfit_delay( w->outfit ) );
   w->u.beamid = 0;
   w->state    = PILOT_OUTFIT_OFF;
}

/**
 * @brief Computes an estimation of ammo flying time
 *
 *    @param o the weapon to shoot.
 *    @param parent Parent of the weapon.
 *    @param pos Target's position.
 *    @param vel Target's velocity.
 */
double pilot_weapFlyTime( const Outfit *o, const Pilot *parent, const vec2 *pos,
                          const vec2 *vel )
{
   vec2   approach_vector, relative_location, orthoradial_vector;
   double speed, radial_speed, orthoradial_speed, dist, t;

   dist = vec2_dist( &parent->solid.pos, pos );

   /* Beam weapons */
   if ( outfit_isBeam( o ) ) {
      if ( dist <= pilot_outfitRange( parent, o ) )
         return INFINITY;
      return -1.; /* Impossible. */
   }

   /* A bay doesn't have range issues */
   if ( outfit_isFighterBay( o ) )
      return -1.;

   /* Missiles use absolute velocity while bolts and unguided rockets use
    * relative vel */
   if ( outfit_isLauncher( o ) && outfit_launcherAI( o ) != AMMO_AI_UNGUIDED )
      vec2_cset( &approach_vector, -vel->x, -vel->y );
   else
      vec2_cset( &approach_vector, VX( parent->solid.vel ) - vel->x,
                 VY( parent->solid.vel ) - vel->y );

   /* Modify speed. */
   speed = outfit_speed( o ) * parent->stats.weapon_speed;
   if ( outfit_isLauncher( o ) )
      speed *= parent->stats.launch_speed;
   else if ( outfit_isForward( o ) )
      speed *= parent->stats.fwd_speed;
   else if ( outfit_isTurret( o ) )
      speed *= parent->stats.tur_speed;

   /* Get the vector : shooter -> target */
   vec2_cset( &relative_location, pos->x - VX( parent->solid.pos ),
              pos->y - VY( parent->solid.pos ) );

   /* Get the orthogonal vector */
   vec2_cset( &orthoradial_vector, VY( parent->solid.pos ) - pos->y,
              pos->x - VX( parent->solid.pos ) );

   radial_speed = vec2_dot( &approach_vector, &relative_location );
   radial_speed = radial_speed / VMOD( relative_location );

   orthoradial_speed = vec2_dot( &approach_vector, &orthoradial_vector );
   orthoradial_speed = orthoradial_speed / VMOD( relative_location );

   if ( ( ( speed * speed -
            VMOD( approach_vector ) * VMOD( approach_vector ) ) != 0 ) &&
        ( speed * speed - orthoradial_speed * orthoradial_speed ) > 0 )
      t = dist *
          ( sqrt( speed * speed - orthoradial_speed * orthoradial_speed ) -
            radial_speed ) /
          ( speed * speed - VMOD( approach_vector ) * VMOD( approach_vector ) );
   else
      return INFINITY;

   /* if t < 0, try the other solution */
   if ( t < 0 )
      t = -dist *
          ( sqrt( speed * speed - orthoradial_speed * orthoradial_speed ) +
            radial_speed ) /
          ( speed * speed - VMOD( approach_vector ) * VMOD( approach_vector ) );

   /* if t still < 0, no solution */
   if ( t < 0 )
      return INFINITY;

   return t;
}

/**
 * @brief Gets the weapon target of a pilot.
 *
 *    @param p Pilot to get weapon target of.
 *    @param[out] wt Weapon target structure set up.
 *    @return The pilot pointer if applicable.
 */
Pilot *pilot_weaponTarget( Pilot *p, Target *wt )
{
   Pilot *pt = pilot_getTarget( p );
   if ( pt != NULL ) {
      wt->type = TARGET_PILOT;
      wt->u.id = pt->id;
      return pt;
   } else if ( p->nav_asteroid != -1 ) {
      wt->type           = TARGET_ASTEROID;
      wt->u.ast.anchor   = p->nav_anchor;
      wt->u.ast.asteroid = p->nav_asteroid;
      return NULL;
   }
   wt->type = TARGET_NONE;
   return NULL;
}

/**
 * @brief Calculates and shoots the appropriate weapons in a weapon set matching
 * an outfit.
 */
static int pilot_shootWeaponSetOutfit( Pilot *p, const Outfit *o,
                                       const Target *target, double time,
                                       int aim )
{
   int    is_launcher, is_bay;
   double rate_mod, energy_mod;
   int    maxp, minh;
   double q, maxt;

   /* Stores if it is a launcher or bay. */
   is_launcher = outfit_isLauncher( o );
   is_bay      = outfit_isFighterBay( o );

   /* Calculate rate modifier. */
   pilot_getRateMod( &rate_mod, &energy_mod, p, o );

   /* Find optimal outfit, coolest that can fire. */
   minh = -1;
   maxt = 0.;
   maxp = -1;
   q    = 0.;
   for ( int i = 0; i < array_size( p->outfits ); i++ ) {
      const PilotOutfitSlot *pos = p->outfits[i];

      /* Only matching outfits. */
      if ( pos->outfit != o )
         continue;

      /* Must be on. */
      if ( !( pos->flags & PILOTOUTFIT_ISON ) )
         continue;

      /* Launcher only counts with ammo. */
      if ( ( is_launcher || is_bay ) && ( pos->u.ammo.quantity <= 0 ) )
         continue;

      /* Get coolest that can fire. */
      if ( pos->timer <= 0. ) {
         if ( is_launcher ) {
            if ( ( minh < 0 ) ||
                 ( p->outfits[minh]->u.ammo.quantity < pos->u.ammo.quantity ) )
               minh = i;
         } else {
            // TODO some other criteria?
            // if ( ( minh < 0 ) || ( p->outfits[minh]->heat_T > pos->heat_T ) )
            minh = i;
         }
      }

      /* Save some stuff. */
      if ( ( maxp < 0 ) || ( pos->timer > maxt ) ) {
         maxp = i;
         maxt = pos->timer;
      }
      q += 1.;
   }

   /* No weapon can fire. */
   if ( minh < 0 )
      return 0;

   /* Only fire if the last weapon to fire fired more than (q-1)/q ago. */
   if ( maxt > rate_mod * outfit_delay( o ) * ( ( q - 1. ) / q ) )
      return 0;

   /* Shoot the weapon. */
   return pilot_shootWeapon( p, p->outfits[minh], target, time, aim );
}

/**
 * @brief Actually handles the shooting, how often the player.p can shoot and
 * such.
 *
 *    @param p Pilot that is shooting.
 *    @param w Pilot's outfit to shoot.
 *    @param target Target shooting at.
 *    @param time Expected flight time.
 *    @param aim Whether or not to aim, if negative, indicates that it is
 * automatically shot.
 *    @return 0 if nothing was shot and 1 if something was shot.
 */
int pilot_shootWeapon( Pilot *p, PilotOutfitSlot *w, const Target *target,
                       double time, int aim )
{
   vec2   vp, vv;
   double rate_mod, energy_mod;
   double energy;

   /* Make sure weapon has outfit. */
   if ( w->outfit == NULL )
      return 0;

   /* check to see if weapon is ready */
   if ( w->timer > 0. )
      return 0;

   /* Calculate rate modifier. */
   pilot_getRateMod( &rate_mod, &energy_mod, p, w->outfit );

   /* Get weapon mount position. */
   pilot_getMount( p, w, &vp );

   /* Modify velocity to take into account the rotation. */
   vec2_cset( &vv, p->solid.vel.x - vp.y * p->solid.dir_vel,
              p->solid.vel.y + vp.x * p->solid.dir_vel );

   /* Get absolute weapon mount position. */
   vp.x += p->solid.pos.x;
   vp.y += p->solid.pos.y;

   /* Regular bolt weapons. */
   if ( outfit_isBolt( w->outfit ) ) {
      /* enough energy? */
      if ( outfit_energy( w->outfit ) * energy_mod > p->energy )
         return 0;

      /* Lua test. */
      if ( ( aim >= 0 ) && ( outfit_luaOnshoot( w->outfit ) != LUA_NOREF ) &&
           !pilot_outfitLOnshoot( p, w ) )
         return 0;

      energy = outfit_energy( w->outfit ) * energy_mod;
      p->energy -= energy;
      if ( !outfit_isProp( w->outfit, OUTFIT_PROP_SHOOT_DRY ) ) {
         for ( int i = 0; i < outfit_shots( w->outfit ); i++ ) {
            weapon_add( w, NULL, p->solid.dir, &vp, &vv, p, target, time, aim );
         }
      }
   }

   /*
    * Beam weapons.
    */
   else if ( outfit_isBeam( w->outfit ) ) {
      /* Don't fire if the existing beam hasn't been destroyed yet. */
      if ( w->u.beamid > 0 )
         return 0;

      /* Check if enough energy to last a second. */
      if ( outfit_energy( w->outfit ) * energy_mod > p->energy )
         return 0;

      /* Lua test. */
      if ( ( aim >= 0 ) && ( outfit_luaOnshoot( w->outfit ) != LUA_NOREF ) &&
           !pilot_outfitLOnshoot( p, w ) )
         return 0;

      /** @todo Handle warmup stage. */
      w->state = PILOT_OUTFIT_ON;
      if ( !outfit_isProp( w->outfit, OUTFIT_PROP_SHOOT_DRY ) ) {
         w->u.beamid =
            beam_start( w, p->solid.dir, &vp, &p->solid.vel, p, target, aim );
      }

      w->timer = outfit_duration( w->outfit );
      if ( pilot_isPlayer( p ) &&
           !outfit_isProp( w->outfit, OUTFIT_PROP_WEAP_POINTDEFENSE ) )
         player_autonavReset( 1. );

      return 1; /* Return early due to custom timer logic. */
   }

   /*
    * Missile launchers
    */
   else if ( outfit_isLauncher( w->outfit ) ) {
      Target wt;
      /* Must have ammo left. */
      if ( w->u.ammo.quantity <= 0 )
         return 0;

      /* enough energy? */
      if ( outfit_energy( w->outfit ) * energy_mod > p->energy )
         return 0;

      /* Shooter can't be the target - safety check for the player.p */
      if ( target == NULL ) {
         pilot_weaponTarget( p, &wt );
         target = &wt;
      }
      // if ((w->outfit->u.lau.ai != AMMO_AI_UNGUIDED) &&
      // !((target->type==TARGET_PILOT) || (target->type==TARGET_ASTEROID)))
      if ( ( outfit_launcherAI( w->outfit ) != AMMO_AI_UNGUIDED ) &&
           ( target->type != TARGET_PILOT ) )
         return 0;

      /* Lua test. */
      if ( ( aim >= 0 ) && ( outfit_luaOnshoot( w->outfit ) != LUA_NOREF ) &&
           !pilot_outfitLOnshoot( p, w ) )
         return 0;

      energy = outfit_energy( w->outfit ) * energy_mod;
      p->energy -= energy;
      if ( !outfit_isProp( w->outfit, OUTFIT_PROP_SHOOT_DRY ) ) {
         int n = outfit_shots( w->outfit );
         for ( int i = 0; i < n; i++ )
            weapon_add( w, NULL, p->solid.dir, &vp, &vv, p, target, time, aim );
      }

      pilot_rmAmmo( p, w, 1 );

      /* Make the AI aware a seeker has been shot */
      if ( outfit_isSeeker( w->outfit ) )
         p->shoot_indicator = 1;

      /* If last ammo was shot, update the range */
      if ( w->u.ammo.quantity <= 0 ) {
         for ( int j = 0; j < PILOT_WEAPON_SETS; j++ )
            pilot_weapSetUpdateRange( p, &p->weapon_sets[j] );
      }
   }

   /*
    * Fighter bays.
    */
   else if ( outfit_isFighterBay( w->outfit ) ) {
      int dockslot = -1;

      /* Must have ammo left. */
      if ( w->u.ammo.quantity <= 0 )
         return 0;

      /* Lua test. */
      if ( ( aim >= 0 ) && ( outfit_luaOnshoot( w->outfit ) != LUA_NOREF ) &&
           !pilot_outfitLOnshoot( p, w ) )
         return 0;

      /* Get index of outfit slot */
      for ( int j = 0; j < array_size( p->outfits ); j++ ) {
         if ( p->outfits[j] == w )
            dockslot = j;
      }

      /* Create the escort. */
      if ( !outfit_isProp( w->outfit, OUTFIT_PROP_SHOOT_DRY ) )
         escort_create( p, outfit_bayShip( w->outfit ), &vp, &p->solid.vel,
                        p->solid.dir, ESCORT_TYPE_BAY, 1, dockslot );

      w->u.ammo.quantity -= 1; /* we just shot it */
      p->mass_outfit -= outfit_massAmmo( w->outfit );
      pilot_updateMass( p );
   } else
      WARN( _( "Shooting unknown weapon type: %s" ), outfit_name( w->outfit ) );

   /* Reset timer. */
   w->timer += rate_mod * outfit_delay( w->outfit );

   /* Reset autonav if is player. */
   if ( pilot_isPlayer( p ) &&
        !outfit_isProp( w->outfit, OUTFIT_PROP_WEAP_POINTDEFENSE ) )
      player_autonavReset( 1. );

   return 1;
}

/**
 * @brief Gets applicable fire rate and energy modifications for a pilot's
 * weapon.
 *
 *    @param[out] rate_mod Fire rate multiplier.
 *    @param[out] energy_mod Energy use multiplier.
 *    @param p Pilot who owns the outfit.
 *    @param o Pilot's outfit.
 */
void pilot_getRateMod( double *rate_mod, double *energy_mod, const Pilot *p,
                       const Outfit *o )
{
   double rate;
   switch ( outfit_type( o ) ) {
   case OUTFIT_TYPE_BOLT:
   case OUTFIT_TYPE_BEAM:
      rate        = p->stats.fwd_firerate * p->stats.weapon_firerate;
      *energy_mod = p->stats.fwd_energy * p->stats.weapon_energy;
      break;
   case OUTFIT_TYPE_TURRET_BOLT:
   case OUTFIT_TYPE_TURRET_BEAM:
      rate        = p->stats.tur_firerate * p->stats.weapon_firerate;
      *energy_mod = p->stats.tur_energy * p->stats.weapon_energy;
      break;

   case OUTFIT_TYPE_LAUNCHER:
   case OUTFIT_TYPE_TURRET_LAUNCHER:
      rate        = p->stats.launch_rate * p->stats.weapon_firerate;
      *energy_mod = p->stats.launch_energy * p->stats.weapon_energy;
      break;

   case OUTFIT_TYPE_FIGHTER_BAY:
      rate        = p->stats.fbay_rate;
      *energy_mod = 1.;
      break;

   default:
      rate        = 1.;
      *energy_mod = 1.;
      break;
   }

   /* Compute rate in the case of negative values. */
   if ( rate <= 0. )
      *rate_mod = INFINITY;
   else
      *rate_mod = 1. / rate;
}

/**
 * @brief Clears the pilots weapon settings.
 *
 *    @param p Pilot whose weapons we're clearing.
 */
void pilot_weaponClear( Pilot *p )
{
   for ( int i = 0; i < PILOT_WEAPON_SETS; i++ ) {
      PilotWeaponSet *ws = pilot_weapSet( p, i );
      array_erase( &ws->slots, array_begin( ws->slots ),
                   array_end( ws->slots ) );
   }
}

/**
 * @brief Tries to automatically set and create the pilot's weapon set.
 *
 * Weapon set 0 is for all weapons. <br />
 * Weapon set 1 is for forward weapons. Ammo using weapons are secondaries. <br
 * /> Weapon set 2 is for turret weapons. Ammo using weapons are secondaries.
 * <br /> Weapon set 3 is for all weapons. Forwards are primaries and turrets
 * are secondaries. <br /> Weapon set 4 is for seeking weapons. High payload
 * variants are secondaries. <br /> Weapon set 5 is for fighter bays. <br />
 *
 *    @param p Pilot to automagically generate weapon lists.
 */
void pilot_weaponAuto( Pilot *p )
{
   int idnext   = 2;
   int hasfb    = 0;
   int haspd    = 0;
   int isplayer = pilot_isPlayer( p );

   /* Clear weapons. */
   pilot_weaponClear( p );

   /* Set modes. */
   pilot_weapSetType( p, 0, WEAPSET_TYPE_HOLD ); /* Primary. */
   pilot_weapSetType( p, 1, WEAPSET_TYPE_HOLD ); /* Secondary. */

   if ( isplayer ) {
      for ( int i = 2; i < PILOT_WEAPON_SETS; i++ )
         pilot_weapSetType( p, i, WEAPSET_TYPE_DEFAULT );

      /* See if fighter bays or point defense. */
      for ( int i = 0; i < array_size( p->outfits ); i++ ) {
         PilotOutfitSlot *slot = p->outfits[i];
         const Outfit    *o    = slot->outfit;

         if ( o == NULL )
            continue;
         if ( !pilot_slotIsToggleable( slot ) ) /* Ignore non-active. */
            continue;

         if ( outfit_isFighterBay( o ) )
            hasfb = 1;
         else if ( outfit_isProp( o, OUTFIT_PROP_WEAP_POINTDEFENSE ) )
            haspd = 1;
      }

      /* Determine weapon ids so they get together. */
      if ( haspd ) {
         haspd = 2; /* 0 weapset. */
         idnext++;
      }
      if ( hasfb ) {
         hasfb = 2 + !!haspd; /* 0 or 1 weapset. */
         idnext++;
      }
   } else {
      /* Set weapon sets. */
      for ( int i = 2; i < PILOT_WEAPON_SETS; i++ )
         pilot_weapSetType( p, i, WEAPSET_TYPE_TOGGLE );
   }

   /* Iterate through all the outfits. */
   for ( int i = 0; i < array_size( p->outfits ); i++ ) {
      PilotOutfitSlot *slot = p->outfits[i];
      const Outfit    *o    = slot->outfit;
      int              id;

      if ( o == NULL )
         continue;
      if ( !pilot_slotIsToggleable( slot ) ) /* Ignore non-active. */
         continue;

      if ( isplayer ) {
         if ( outfit_isSecondary( o ) )
            id = 1; /* Secondary override. */
         /* Bolts and beams. */
         else if ( !outfit_isProp( o, OUTFIT_PROP_WEAP_POINTDEFENSE ) &&
                   ( outfit_isBolt( o ) || outfit_isBeam( o ) ||
                     ( outfit_isLauncher( o ) && !outfit_isSeeker( o ) ) ) )
            id = 0; /* Primary. */
         /* Seekers. */
         else if ( outfit_isLauncher( o ) && outfit_isSeeker( o ) )
            id = 1; /* Secondary. */
         /* Point defense. */
         else if ( outfit_isProp( o, OUTFIT_PROP_WEAP_POINTDEFENSE ) )
            id = haspd;
         /* Fighter bays. */
         else if ( outfit_isFighterBay( o ) )
            id = hasfb;
         /* Rest just incrcement. */
         else {
            id = idnext++;
            /* Ran out of space. */
            if ( id >= PILOT_WEAPON_SETS )
               break;
         }
      } else {
         if ( outfit_isSecondary( o ) )
            id = 1; /* Secondary override. */
         /* Bolts and beams. */
         else if ( outfit_isBolt( o ) || outfit_isBeam( o ) ||
                   ( outfit_isLauncher( o ) && !outfit_isSeeker( o ) ) )
            id = 0; /* Primary. */
         /* Point defense. */
         else if ( outfit_isProp( o, OUTFIT_PROP_WEAP_POINTDEFENSE ) )
            id = 2;
         /* Fighter bays. */
         else if ( outfit_isFighterBay( o ) )
            id = 3;
         else
            continue;
      }

      /* Add to its base group. */
      pilot_weapSetAdd( p, id, slot );

      /* Add to additional special groups if not player. */
      if ( !isplayer ) {
         if ( outfit_isTurret( o ) )
            pilot_weapSetAdd( p, 4, slot );
         if ( outfit_isLauncher( o ) && outfit_isSeeker( o ) ) {
            pilot_weapSetAdd( p, 5, slot );
            if ( outfit_isTurret( o ) )
               pilot_weapSetAdd( p, 6, slot );
         }
      }
   }

   /* All should be inrange. */
   for ( int i = 0; i < PILOT_WEAPON_SETS; i++ ) {
      pilot_weapSetInrange( p, i, 1 );
      /* Update range and speed (at 0)*/
      pilot_weapSetUpdateRange( p, &p->weapon_sets[i] );
   }

   /* Update all outfits. */
   pilot_weaponSafe( p );
}

/**
 * @brief Sets the weapon set as safe.
 *
 *    @param p Pilot to set weapons as safe.
 */
void pilot_weaponSafe( Pilot *p )
{
   for ( int j = 0; j < PILOT_WEAPON_SETS; j++ ) {
      PilotWeaponSet *ws = &p->weapon_sets[j];

      /* Update range. */
      pilot_weapSetUpdateRange( p, ws );
   }
}

/**
 * @brief Disables a given active outfit.
 *
 * @param p Pilot whose outfit we are disabling.
 * @param o Outfit to disable.
 * @param natural Whether a result of natural behaviour, or something automatic.
 * @return Whether the outfit was actually disabled.
 */
int pilot_outfitOff( Pilot *p, PilotOutfitSlot *o, int natural )
{
   /* Must be equipped, not disabled, not cooling down. */
   if ( o->outfit == NULL || ( pilot_isDisabled( p ) ) ||
        ( pilot_isFlag( p, PILOT_COOLDOWN ) ) )
      return 0;

   if ( outfit_isAfterburner( o->outfit ) ) { /* Afterburners */
      if ( ( outfit_luaOntoggle( o->outfit ) != LUA_NOREF ) &&
           !pilot_outfitLOntoggle( p, o, 0, natural ) )
         return 0;
      o->flags &= ~PILOTOUTFIT_DYNAMIC_FLAGS;
      pilot_afterburnOver( p );

   } else if ( outfit_isBeam( o->outfit ) ) {
      if ( ( outfit_luaOntoggle( o->outfit ) != LUA_NOREF ) &&
           !pilot_outfitLOntoggle( p, o, 0, natural ) )
         return 0;
      o->flags &= ~PILOTOUTFIT_DYNAMIC_FLAGS;

      /* Beams use stimer to represent minimum time until shutdown. */
      if ( o->u.beamid > 0 ) {
         beam_end( o->u.beamid );
         pilot_stopBeam( p, o ); /* Sets the state. */
      } else
         o->state = PILOT_OUTFIT_OFF;
   } else if ( !( o->flags & PILOTOUTFIT_TOGGLEABLE ) )
      /* Case of a mod we can't toggle. */
      return 0;
   else if ( outfit_luaOntoggle( o->outfit ) != LUA_NOREF ) {
      int ret = pilot_outfitLOntoggle( p, o, 0, natural );
      if ( ret ) {
         if ( outfit_isWeapon( o->outfit ) )
            o->state = PILOT_OUTFIT_OFF;
         o->flags &= ~( PILOTOUTFIT_DYNAMIC_FLAGS | PILOTOUTFIT_ISON );
      }
      return ret;
   } else {
      o->stimer = outfit_cooldown( o->outfit );
      if ( o->stimer < 0. )
         o->state = PILOT_OUTFIT_OFF;
      else
         o->state = PILOT_OUTFIT_COOLDOWN;
      o->flags &= ~PILOTOUTFIT_DYNAMIC_FLAGS;
   }

   return 1;
}

/**
 * @brief Enable a given active outfit.
 *
 * @param p Pilot whose outfit we are enabling.
 * @param pos Outfit to enable.
 * @return Whether the outfit was actually enabled.
 */
int pilot_outfitOn( Pilot *p, PilotOutfitSlot *pos )
{
   if ( pos->outfit == NULL )
      return 0;
   if ( outfit_isAfterburner( pos->outfit ) ) {
      int ret = pilot_afterburn( p );
      if ( ret )
         pos->state = PILOT_OUTFIT_ON;
      return ret;
   } else if ( outfit_luaOntoggle( pos->outfit ) != LUA_NOREF ) {
      int ret = pilot_outfitLOntoggle( p, pos, 1, 1 );
      if ( ret && outfit_isWeapon( pos->outfit ) )
         pos->state = PILOT_OUTFIT_ON;
      return ret;
   } else {
      pos->state  = PILOT_OUTFIT_ON;
      pos->stimer = outfit_duration( pos->outfit ) * p->stats.cooldown_mod;
   }

   return 1;
}

/**
 * @brief Disables all active outfits for a pilot.
 *
 * @param p Pilot whose outfits we are disabling.
 * @return Whether any outfits were actually disabled.
 */
int pilot_outfitOffAll( Pilot *p )
{
   int nchg = 0;
   for ( int i = 0; i < array_size( p->outfits ); i++ ) {
      PilotOutfitSlot *pos = p->outfits[i];
      /* Picky about our outfits. */
      if ( pos->outfit == NULL )
         continue;
      if ( pos->state == PILOT_OUTFIT_ON )
         nchg += pilot_outfitOff( p, pos, 0 );
   }
   return ( nchg > 0 );
}

/**
 * @brief Disables all active outfits for a pilot.
 *
 * @param p Pilot whose outfits we are disabling.
 * @return Whether any outfits were actually disabled.
 */
int pilot_outfitOffAllStealth( Pilot *p )
{
   int nchg = 0;
   for ( int i = 0; i < array_size( p->outfits ); i++ ) {
      PilotOutfitSlot *o = p->outfits[i];
      /* Picky about our outfits. */
      if ( o->outfit == NULL )
         continue;
      if ( outfit_isProp( o->outfit, OUTFIT_PROP_STEALTH_ON ) )
         continue;
      if ( o->state == PILOT_OUTFIT_ON )
         nchg += pilot_outfitOff( p, o, 0 );
   }
   return ( nchg > 0 );
}

/**
 * @brief Activate the afterburner.
 */
int pilot_afterburn( Pilot *p )
{
   if ( p == NULL )
      return 0;

   if ( pilot_isFlag( p, PILOT_HYP_PREP ) ||
        pilot_isFlag( p, PILOT_HYPERSPACE ) ||
        pilot_isFlag( p, PILOT_LANDING ) || pilot_isFlag( p, PILOT_TAKEOFF ) ||
        pilot_isDisabled( p ) || pilot_isFlag( p, PILOT_COOLDOWN ) )
      return 0;

   /* Not under manual control if is player. */
   if ( pilot_isFlag( p, PILOT_MANUAL_CONTROL ) && pilot_isPlayer( p ) )
      return 0;

   /** @todo fancy effect? */
   if ( p->afterburner == NULL )
      return 0;

   /* Needs at least enough energy to afterburn fo 0.5 seconds. */
   if ( p->energy < outfit_energy( p->afterburner->outfit ) * 0.5 )
      return 0;

   /* Turn it on. */
   if ( p->afterburner->state == PILOT_OUTFIT_OFF ) {
      p->afterburner->state  = PILOT_OUTFIT_ON;
      p->afterburner->stimer = outfit_duration( p->afterburner->outfit );
      pilot_setFlag( p, PILOT_AFTERBURNER );
      if ( !outfit_isProp( p->afterburner->outfit, OUTFIT_PROP_STEALTH_ON ) )
         pilot_destealth( p ); /* No afterburning stealth. */
      pilot_calcStats( p );

      /* @todo Make this part of a more dynamic activated outfit sound system.
       */
      sound_playPos( outfit_afterburnerSoundOn( p->afterburner->outfit ),
                     p->solid.pos.x, p->solid.pos.y, p->solid.vel.x,
                     p->solid.vel.y );
   }

   if ( pilot_isPlayer( p ) ) {
      double afb_mod = MIN( 1., pilot_massFactor( player.p ) );
      spfx_shake( afb_mod *
                  outfit_afterburnerRumble( player.p->afterburner->outfit ) );
   }
   return 1;
}

/**
 * @brief Deactivates the afterburner.
 */
void pilot_afterburnOver( Pilot *p )
{
   if ( p == NULL )
      return;
   if ( p->afterburner == NULL )
      return;

   if ( p->afterburner->state == PILOT_OUTFIT_ON ) {
      p->afterburner->state = PILOT_OUTFIT_OFF;
      pilot_rmFlag( p, PILOT_AFTERBURNER );
      pilot_calcStats( p );

      /* @todo Make this part of a more dynamic activated outfit sound system.
       */
      sound_playPos( outfit_afterburnerSoundOff( p->afterburner->outfit ),
                     p->solid.pos.x, p->solid.pos.y, p->solid.vel.x,
                     p->solid.vel.y );
   }
}

/**
 * @brief Copies a weapon set over.
 */
void ws_copy( PilotWeaponSet       dest[PILOT_WEAPON_SETS],
              const PilotWeaponSet src[PILOT_WEAPON_SETS] )
{
   ws_free( dest );
   memcpy( dest, src, sizeof( PilotWeaponSet ) * PILOT_WEAPON_SETS );
   for ( int i = 0; i < PILOT_WEAPON_SETS; i++ )
      dest[i].slots = array_copy( PilotWeaponSetOutfit, src[i].slots );
}

/**
 * @brief Frees a weapon set.
 */
void ws_free( PilotWeaponSet ws[PILOT_WEAPON_SETS] )
{
   for ( int i = 0; i < PILOT_WEAPON_SETS; i++ ) {
      array_free( ws[i].slots );
      ws[i].slots = NULL;
   }
}
