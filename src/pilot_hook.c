/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file pilot_hook.c
 *
 * @brief Handles the pilot hook stuff.
 */
/** @cond */
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "pilot_hook.h"

#include "array.h"
#include "hook.h"
#include "log.h"
#include "nstring.h"
#include "nxml.h"

static PilotHook *pilot_globalHooks = NULL; /**< Global hooks that affect all pilots. */
static int pilot_hookCleanup = 0; /**< Are hooks being removed from a pilot? */

/**
 * @brief Tries to run a pilot hook if he has it.
 *
 *    @param p Pilot to run the hook.
 *    @param hook_type Type of hook to run.
 *    @param param Parameters to pass.
 *    @param nparam Number of parameters to pass.
 *    @return The number of hooks run.
 */
int pilot_runHookParam( Pilot* p, int hook_type, const HookParam* param, int nparam )
{
   int n, run;
   HookParam hstaparam[5], *hdynparam, *hparam;

   /* Set up hook parameters. */
   if (nparam <= 3) {
      hstaparam[0].type = HOOK_PARAM_PILOT;
      hstaparam[0].u.lp = p->id;
      n  = 1;
      if (nparam != 0)
         memcpy( &hstaparam[n], param, sizeof(HookParam)*nparam );
      n += nparam;
      hstaparam[n].type = HOOK_PARAM_SENTINEL;
      hdynparam         = NULL;
      hparam            = hstaparam;
   }
   else {
      hdynparam   = malloc( sizeof(HookParam) * (nparam+2) );
      hdynparam[0].type       = HOOK_PARAM_PILOT;
      hdynparam[0].u.lp       = p->id;
      memcpy( &hdynparam[1], param, sizeof(HookParam)*nparam );
      hdynparam[nparam+1].type = HOOK_PARAM_SENTINEL;
      hparam                  = hdynparam;
   }

   /* Run pilot specific hooks. */
   run = 0;
   for (int i=0; i<array_size(p->hooks); i++) {
      int ret;
      if (p->hooks[i].type != hook_type)
         continue;

      ret = hook_runIDparam( p->hooks[i].id, hparam );
      if (ret)
         WARN(_("Pilot '%s' failed to run hook type %d"), p->name, hook_type);
      else
         run++;
   }

   /* Run global hooks. */
   for (int i=0; i<array_size(pilot_globalHooks); i++) {
      int ret;
      if (pilot_globalHooks[i].type != hook_type)
         continue;

      ret = hook_runIDparam( pilot_globalHooks[i].id, hparam );
      if (ret)
         WARN(_("Pilot '%s' failed to run hook type %d"), p->name, hook_type);
      else
         run++;
   }

   /* Clean up. */
   free( hdynparam );

   if (run > 0)
      claim_activateAll(); /* Reset claims. */

   return run;
}

/**
 * @brief Tries to run a pilot hook if he has it.
 *
 *    @param p Pilot to run the hook.
 *    @param hook_type Type of hook to run.
 *    @return The number of hooks run.
 */
int pilot_runHook( Pilot* p, int hook_type )
{
   return pilot_runHookParam( p, hook_type, NULL, 0 );
}

/**
 * @brief Adds a hook to the pilot.
 *
 *    @param pilot Pilot to add the hook to.
 *    @param type Type of the hook to add.
 *    @param hook ID of the hook to add.
 */
void pilot_addHook( Pilot *pilot, int type, unsigned int hook )
{
   PilotHook *phook;

   /* Allocate memory. */
   if (pilot->hooks == NULL)
      pilot->hooks = array_create( PilotHook );

   /* Create the new hook. */
   phook        = &array_grow( &pilot->hooks );
   phook->type  = type;
   phook->id    = hook;
}

/**
 * @brief Adds a pilot global hook.
 */
void pilots_addGlobalHook( int type, unsigned int hook )
{
   PilotHook phook;

   /* Allocate memory. */
   if (pilot_globalHooks == NULL)
      pilot_globalHooks = array_create( PilotHook );

   /* Create the new hook. */
   phook.type = type;
   phook.id   = hook;
   array_push_back( &pilot_globalHooks, phook );
}

/**
 * @brief Removes a pilot global hook.
 */
void pilots_rmGlobalHook( unsigned int hook )
{
   for (int i=0; i<array_size(pilot_globalHooks); i++) {
      if (pilot_globalHooks[i].id != hook)
         continue;
      array_erase( &pilot_globalHooks, &pilot_globalHooks[i], &pilot_globalHooks[i+1] );
      return;
   }
}

/**
 * @brief Removes all the pilot global hooks.
 */
void pilots_clearGlobalHooks (void)
{
   array_erase( &pilot_globalHooks, array_begin(pilot_globalHooks), array_end(pilot_globalHooks) );
}

/**
 * @brief Removes a hook from all the pilots.
 *
 *    @param hook Hook to remove.
 */
void pilots_rmHook( unsigned int hook )
{
   Pilot *const*plist;

   /* Cleaning up a pilot's hooks. */
   if (pilot_hookCleanup)
      return;

   /* Remove global hook first. */
   pilots_rmGlobalHook( hook );

   plist = pilot_getAll();
   for (int i=0; i<array_size(plist); i++) {
      Pilot *p = plist[i];

      for (int j=0; j<array_size(p->hooks); j++) {
         /* Hook not found. */
         if (p->hooks[j].id != hook)
            continue;

         array_erase( &p->hooks, &p->hooks[j], &p->hooks[j+1] );
         j--; /* Dun like it but we have to keep iterator safe. */
      }
   }
}

/**
 * @brief Clears the pilots hooks.
 *
 *    @param p Pilot whose hooks we're clearing
 */
void pilot_clearHooks( Pilot *p )
{
   /* Remove the hooks. */
   pilot_hookCleanup = 1;
   for (int i=0; i<array_size(p->hooks); i++)
      hook_rm( p->hooks[i].id );
   pilot_hookCleanup = 0;

   array_free(p->hooks);
   p->hooks  = NULL;
}

/**
 * @brief Clears global pilot hooks.
 */
void pilot_freeGlobalHooks (void)
{
   /* Clear global hooks. */
   if (pilot_globalHooks != NULL) {
      pilots_clearGlobalHooks();
      array_free( pilot_globalHooks );
      pilot_globalHooks = NULL;
   }
}
