/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_pilotoutfit.c
 *
 * @brief Handles the Lua pilot outfit (equipped) bindings.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_pilotoutfit.h"

#include "array.h"
#include "log.h"
#include "nlua_asteroid.h"
#include "nlua_outfit.h"
#include "nlua_pilot.h"
#include "nlua_munition.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "slots.h"
#include "weapon.h"

int pilotoutfit_modified = 0;

/* Pilot outfit metatable methods. */
static int poL_slot( lua_State *L );
static int poL_outfit( lua_State *L );
static int poL_state( lua_State *L );
static int poL_progress( lua_State *L );
static int poL_set( lua_State *L );
static int poL_clear( lua_State *L );
static int poL_munition( lua_State *L );
static int poL_shoot( lua_State *L );
static int poL_heat( lua_State *L );
static int poL_heatup( lua_State *L );
static const luaL_Reg poL_methods[] = {
   { "slot", poL_slot },
   { "outfit", poL_outfit },
   { "state", poL_state },
   { "progress", poL_progress },
   { "set", poL_set },
   { "clear", poL_clear },
   { "munition", poL_munition },
   { "shoot", poL_shoot },
   { "heat", poL_heat },
   { "heatup", poL_heatup },
   {0,0}
}; /**< Pilot outfit metatable methods. */

/**
 * @brief Loads the pilot outfit library.
 *
 *    @param env Environment to load pilot outfit library into.
 *    @return 0 on success.
 */
int nlua_loadPilotOutfit( nlua_env env )
{
   nlua_register(env, PILOTOUTFIT_METATABLE, poL_methods, 1);
   return 0;
}

/**
 * @brief Lua bindings to interact with pilot outfits.
 *
 * @luamod pilotoutfit
 */
/**
 * @brief Gets outfit at index.
 *
 *    @param L Lua state to get outfit from.
 *    @param ind Index position to find the outfit.
 *    @return Outfit found at the index in the state.
 */
PilotOutfitSlot* lua_topilotoutfit( lua_State *L, int ind )
{
   return *((PilotOutfitSlot**) lua_touserdata(L,ind));
}
/**
 * @brief Gets outfit at index or raises error if there is no outfit at index.
 *
 *    @param L Lua state to get outfit from.
 *    @param ind Index position to find outfit.
 *    @return Outfit found at the index in the state.
 */
PilotOutfitSlot* luaL_checkpilotoutfit( lua_State *L, int ind )
{
   if (lua_ispilotoutfit(L,ind))
      return lua_topilotoutfit(L,ind);
   luaL_typerror(L, ind, PILOTOUTFIT_METATABLE);
   return NULL;
}
/**
 * @brief Makes sure the outfit is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the outfit to validate.
 *    @return The outfit (doesn't return if fails - raises Lua error ).
 */
PilotOutfitSlot* luaL_validpilotoutfit( lua_State *L, int ind )
{
   PilotOutfitSlot *o;

   if (lua_ispilotoutfit(L, ind))
      o  = luaL_checkpilotoutfit(L,ind);
   else {
      luaL_typerror(L, ind, PILOTOUTFIT_METATABLE);
      return NULL;
   }

   if (o == NULL)
      NLUA_ERROR(L, _("Pilot Outfit is invalid."));

   return o;
}
/**
 * @brief Pushes a pilot outfit on the stack.
 *
 *    @param L Lua state to push outfit into.
 *    @param po Pilot outfit to push.
 *    @return Newly pushed pilotoutfit.
 */
PilotOutfitSlot** lua_pushpilotoutfit( lua_State *L, PilotOutfitSlot *po )
{
   PilotOutfitSlot **lpo = (PilotOutfitSlot**) lua_newuserdata(L, sizeof(PilotOutfitSlot*));
   *lpo = po;
   luaL_getmetatable(L, PILOTOUTFIT_METATABLE);
   lua_setmetatable(L, -2);
   return lpo;
}
/**
 * @brief Checks to see if ind is a pilot outfit.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a pilot outfit.
 */
int lua_ispilotoutfit( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, PILOTOUTFIT_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Gets the properties of the outfit slot.
 *
 *    @luatparam PilotOutfit po Pilot outfit to get the outfit of.
 *    @luareturn A table with slot properties string "size", string "type", string "property", boolean "required", boolean "exclusive", and boolean "locked".
 *               (Strings are English.)
 * @luafunc slot
 */
static int poL_slot( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit(L,1);
   const ShipOutfitSlot *sslot = po->sslot;
   const OutfitSlot *slot = &sslot->slot;

   /* make the slot table and put it in */
   lua_newtable(L);

   lua_pushstring(L, "type"); /* key */
   lua_pushstring(L, slotName( slot->type )); /* value */
   lua_rawset(L, -3); /* table[key = value ]*/

   lua_pushstring(L, "size"); /* key */
   lua_pushstring(L, slotSize(slot->size) );
   lua_rawset(L, -3); /* table[key] = value */

   lua_pushstring(L, "property"); /* key */
   lua_pushstring( L, sp_display(slot->spid)); /* value */
   lua_rawset(L, -3); /* table[key] = value */

   lua_pushstring(L, "required"); /* key */
   lua_pushboolean( L, sslot->required); /* value */
   lua_rawset(L, -3); /* table[key] = value */

   lua_pushstring(L, "exclusive"); /* key */
   lua_pushboolean( L, sslot->exclusive); /* value */
   lua_rawset(L, -3); /* table[key] = value */

   lua_pushstring(L, "locked"); /* key */
   lua_pushboolean( L, sslot->locked); /* value */
   lua_rawset(L, -3); /* table[key] = value */

   return 1;
}

/**
 * @brief Gets the outfit of the PilotOutfit.
 *
 *    @luatparam PilotOutfit po Pilot outfit to get the outfit of.
 *    @luatparam Outfit Outfit corresponding to the Pilot outfit.
 * @luafunc outfit
 */
static int poL_outfit( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit(L,1);
   lua_pushoutfit(L, po->outfit );
   return 1;
}

/**
 * @brief Sets the state of the PilotOutfit.
 *
 *    @luatparam PilotOutfit po Pilot outfit to set the state of.
 *    @luatparam string state State to set the pilot outfit to. Can be either "off", "warmup", "on", or "cooldown".
 * @luafunc state
 */
static int poL_state( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit(L,1);
   const char *state = luaL_optstring(L,2,NULL);
   PilotOutfitState pos = po->state;

   if (!outfit_isMod( po->outfit ))
      return NLUA_ERROR( L, _("'pilotoutfit.%s' only works with modifier outfits!"), "state");

   if (state==NULL || strcmp(state,"off")==0)
      po->state = PILOT_OUTFIT_OFF;
   else if (strcmp(state,"warmup")==0)
      po->state = PILOT_OUTFIT_WARMUP;
   else if (strcmp(state,"on")==0)
      po->state = PILOT_OUTFIT_ON;
   else if (strcmp(state,"cooldown")==0)
      po->state = PILOT_OUTFIT_COOLDOWN;
   else
      return NLUA_ERROR( L, _("Unknown PilotOutfit state '%s'!"), state );

   /* Mark as modified if state changed. */
   if (pos != po->state)
      pilotoutfit_modified = 1;

   return 0;
}

/**
 * @brief Sets the state progress of the PilotOutfit.
 *
 *    @luatparam PilotOutfit po Pilot outfit to set the state of.
 *    @luatparam number progress Progress of the current state with 1 being started and 0 being done.
 * @luafunc progress
 */
static int poL_progress( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit(L,1);

   if (!outfit_isMod( po->outfit ))
      return NLUA_ERROR( L, _("'pilotoutfit.%s' only works with modifier outfits!"), "progress");

   po->progress = CLAMP( 0., 1., luaL_checknumber(L,2) );
   return 0;
}

/**
 * @brief Sets a temporary ship stat modifier of the pilot outfit.
 *
 *    @luatparam PilotOutfit po Pilot outfit to set the modifier of.
 *    @luatparam string mod Modifier name to set to. Same as in the XML definition.
 *    @luatparam number val Value to set the modifier to. In the case of booleans, 0 indicates false, while 1 indicates true.
 * @luafunc set
 */
static int poL_set( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit(L,1);
   const char *name = luaL_checkstring(L,2);
   double value = luaL_checknumber(L,3);
   po->lua_stats = ss_statsSetList( po->lua_stats, ss_typeFromName(name), value, 1, 0 );
   pilotoutfit_modified = 1;
   return 0;
}

/**
 * @brief Clears all the temporary ship stat modifiers of the pilot outfit.
 *
 *    @luatparam PilotOutfit po Pilot outfit to clear temporary modifiers from.
 * @luafunc clear
 */
static int poL_clear( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit(L,1);
   ss_free( po->lua_stats );
   po->lua_stats = NULL;
   pilotoutfit_modified = 1;
   return 0;
}

static Target lua_totarget( lua_State *L, int idx )
{
   Target t;
   /* Handle target. */
   if (lua_isnoneornil(L,idx))
      t.type = TARGET_NONE;
   if (lua_ispilot(L,idx)) {
      t.type = TARGET_PILOT;
      t.u.id = lua_topilot(L,idx);
   }
   else if (lua_isasteroid(L,idx)) {
      const LuaAsteroid_t *ast = lua_toasteroid(L,idx);
      t.type = TARGET_ASTEROID;
      t.u.ast.anchor = ast->parent;
      t.u.ast.asteroid = ast->id;
   }
   else if (lua_ismunition(L,idx)) {
      const LuaMunition *lm = lua_tomunition(L,idx);
      t.type = TARGET_WEAPON;
      t.u.id = lm->id;
   }
   return t;
}

/**
 * @brief Creates a munition.
 *
 *    @luatparam PilotOutfit po Pilot outfit originating the munition.
 *    @luatparam Pilot p Pilot generating the munition, used for faction and damaging purposes.
 *    @luatparam[opt=po:outfit()] Outfit o Outfit to be used as a reference for the munition.
 *    @luatparam[opt=nil] Pilot|Munition|Asteroid|nil t Target pilot to shoot at.
 *    @luatparam[opt=p:dir()] number dir Direction the munition should face.
 *    @luatparam[opt=p:pos()] Vec2 pos Position to create the munition at.
 *    @luatparam[opt=p:vel()] Vec2 vel Initial velocity of the munition. The munition's base velocity gets added to this.
 *    @luatparam[opt=false] boolean noaim Whether or not to disable the tracking and aiming framework when shooting.
 *    @luatreturn Munition The newly created munition.
 * @luafunc munition
 */
static int poL_munition( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit( L, 1 );
   Pilot *p    = luaL_validpilot( L, 2 );
   const Outfit *o = luaL_optoutfit( L, 3, po->outfit );
   Target t = lua_totarget( L, 4 );
   double dir  = luaL_optnumber( L, 5, p->solid.dir );
   vec2 *vp    = luaL_optvector( L, 6, &p->solid.pos );
   vec2 *vv    = luaL_optvector( L, 7, &p->solid.vel );
   int noaim   = lua_toboolean( L, 8 );
   Weapon *w = weapon_add( po, o, dir, vp, vv, p, &t, 0., !noaim );
   lua_pushmunition( L, w );
   return 1;
}

/**
 * @brief Shoots at an object.
 *
 *    @luatparam PilotOutfit po Pilot outfit originating the munition.
 *    @luatparam Pilot p Pilot shooting, used for faction and damaging purposes.
 *    @luatparam[opt=nil] Pilot|Munition|Asteroid|nil t Target pilot to shoot at.
 *    @luatparam[opt=false] boolean stagger Whether or not to stagger similar outfits.
 *    @luatreturn boolean true if was able to shoot, false otherwise.
 * @luafunc shoot
 */
static int poL_shoot( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit( L, 1 );
   Pilot *p    = luaL_validpilot( L, 2 );
   Target t    = lua_totarget( L, 3 );
   int stagger = lua_toboolean( L, 4 );
   double time;
   int ret;
   int has_ammo;

   /* The particular weapon can't fire, so ignore. */
   if (po->timer > 0.) {
      lua_pushboolean(L,0);
      return 1;
   }

   /* Out of ammo. */
   has_ammo = outfit_isLauncher(po->outfit) || outfit_isFighterBay(po->outfit);
   if (has_ammo && (po->u.ammo.quantity <= 0)) {
      lua_pushboolean(L,0);
      return 1;
   }

   /* See if we should stagger the outfits. */
   if (stagger) {
      double q, maxt, rate_mod, energy_mod;
      int maxp, minh;

      /* Calculate rate modifier. */
      pilot_getRateMod( &rate_mod, &energy_mod, p, po->outfit );

      /* Find optimal outfit, coolest that can fire. */
      minh  = -1;
      maxt  = 0.;
      maxp  = -1;
      q     = 0.;
      for (int i=0; i<array_size(p->outfits); i++) {
         PilotOutfitSlot *pos = p->outfits[i];
         if (pos->outfit != po->outfit)
            continue;
         /* Launcher only counts with ammo. */
         if (has_ammo && (pos->u.ammo.quantity <= 0))
            continue;
         /* Get coolest that can fire. */
         if (pos->timer <= 0.) {
            if (has_ammo) {
               if ((minh < 0) || (p->outfits[ minh ]->u.ammo.quantity < pos->u.ammo.quantity))
                  minh = i;
            }
            else {
               if ((minh < 0) || (p->outfits[ minh ]->heat_T > pos->heat_T))
                  minh = i;
            }
         }

         /* Save some stuff. */
         if ((maxp < 0) || (pos->timer > maxt)) {
            maxp = i;
            maxt = pos->timer;
         }
         q += 1.;
      }

      /* Only fire if the last weapon to fire fired more than (q-1)/q ago. */
      if ((minh < 0) || (maxt > rate_mod * outfit_delay(po->outfit) * ((q-1.) / q))) {
         lua_pushboolean(L,0);
         return 1;
      }
   }

   time = weapon_targetFlyTime( po->outfit, p, &t );
   ret = pilot_shootWeapon( p, po, &t, time, -1 );

   lua_pushboolean( L, ret );
   return 1;
}

/**
 * @brief Gets the heat status of the pilot outfit.
 *
 *    @luatparam PilotOutfit po Pilot outfit to get heat of.
 *    @luatparam Boolean absolute If true returns the value in kelvin, otherwise it returns how overheated it is with 1. being normal and 0. being overheated.
 *    @luatreturn Number heat of the pilot outfit in kelvin or closeness to 800 kelvin.
 * @luafunc heat
 */
static int poL_heat( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit( L, 1 );
   if (lua_isboolean(L,2))
      lua_pushnumber( L, po->heat_T );
   else
      lua_pushnumber( L, pilot_heatEfficiencyMod(po->heat_T, po->outfit->overheat_min, po->outfit->overheat_max) );
   return 1;
}

/**
 * @brief Heats up a pilot outfit.
 *
 * @code
 * local heat = po:outfit():heatFor( 5 ) -- 5 pulses should heat up fully
 * ...
 * po:heatup( heat ) -- one pulse
 * @endcode
 *
 *    @luatparam PilotOutfit po Pilot outfit to heat up.
 * @luafunc heatup
 * @see heatFor
 */
static int poL_heatup( lua_State *L )
{
   PilotOutfitSlot *po = luaL_validpilotoutfit( L, 1 );
   double heat = luaL_checknumber( L, 2 );
   po->heat_T += heat / po->heat_C;
   /* Enforce a minimum value as a safety measure. */
   po->heat_T = MAX( po->heat_T, CONST_SPACE_STAR_TEMP );
   return 0;
}
