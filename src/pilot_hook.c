/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_hook.c
 *
 * @brief Handles the pilot hook stuff.
 */


#include "pilot_hook.h"

#include "naev.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "nxml.h"

#include "log.h"
#include "hook.h"
#include "array.h"


static PilotHook *pilot_globalHooks = NULL; /**< Global hooks that affect all pilots. */
static int pilot_hookCleanup = 0; /**< Are hooks being removed from a pilot? */


/**
 * @brief Tries to run a pilot hook if he has it.
 *
 *    @param p Pilot to run the hook.
 *    @param hook_type Type of hook to run.
 *    @return The number of hooks run.
 */
int pilot_runHookParam( Pilot* p, int hook_type, HookParam* param, int nparam )
{
   int n, i, run, ret;
   HookParam hparam[3], *hdynparam;

   /* Set up hook parameters. */
   if (nparam <= 1) {
      hparam[0].type       = HOOK_PARAM_PILOT;
      hparam[0].u.lp.pilot = p->id;
      n  = 1;
      if (nparam == 1) {
         memcpy( &hparam[n], param, sizeof(HookParam) );
         n++;
      }
      hparam[n].type    = HOOK_PARAM_SENTINAL;
      hdynparam         = NULL;
   }
   else {
      hdynparam   = malloc( sizeof(HookParam) * (nparam+2) );
      hdynparam[0].type       = HOOK_PARAM_PILOT;
      hdynparam[0].u.lp.pilot = p->id;
      memcpy( &hdynparam[1], param, sizeof(HookParam)*nparam );
      hdynparam[nparam].type  = HOOK_PARAM_SENTINAL;
   }

   /* Run pilot specific hooks. */
   run = 0;
   for (i=0; i<p->nhooks; i++) {
      if (p->hooks[i].type != hook_type)
         continue;

      ret = hook_runIDparam( p->hooks[i].id, hparam );
      if (ret)
         WARN("Pilot '%s' failed to run hook type %d", p->name, hook_type);
      else
         run++;
   }

   /* Run global hooks. */
   if (pilot_globalHooks != NULL) {
      for (i=0; i<array_size(pilot_globalHooks); i++) {
         if (pilot_globalHooks[i].type != hook_type)
            continue;

         ret = hook_runIDparam( pilot_globalHooks[i].id, hparam );
         if (ret)
            WARN("Pilot '%s' failed to run hook type %d", p->name, hook_type);
         else
            run++;
      }
   }

   /* Clean up. */
   if (hdynparam != NULL)
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
   pilot->nhooks++;
   pilot->hooks = realloc( pilot->hooks, sizeof(PilotHook) * pilot->nhooks );
   pilot->hooks[pilot->nhooks-1].type  = type;
   pilot->hooks[pilot->nhooks-1].id    = hook;
}


/**
 * @brief Adds a pilot global hook.
 */
void pilots_addGlobalHook( int type, unsigned int hook )
{
   PilotHook *phook;

   /* Allocate memory. */
   if (pilot_globalHooks == NULL) {
      pilot_globalHooks = array_create( PilotHook );
   }

   /* Create the new hook. */
   phook       = &array_grow( &pilot_globalHooks );
   phook->type = type;
   phook->id   = hook;
}


/**
 * @brief Removes a pilot global hook.
 */
void pilots_rmGlobalHook( unsigned int hook )
{
   int i;

   /* Must exist pilot hook.s */
   if (pilot_globalHooks == NULL )
      return;

   for (i=0; i<array_size(pilot_globalHooks); i++) {
      if (pilot_globalHooks[i].id == hook) {
         array_erase( &pilot_globalHooks, &pilot_globalHooks[i], &pilot_globalHooks[i+1] );
         return;
      }
   }
}


/**
 * @brief Removes all the pilot global hooks.
 */
void pilots_clearGlobalHooks (void)
{
   /* Must exist pilot hook.s */
   if (pilot_globalHooks == NULL )
      return;

   array_erase( &pilot_globalHooks, pilot_globalHooks, &pilot_globalHooks[ array_size(pilot_globalHooks)-1 ] );
}


/**
 * @brief Removes a hook from all the pilots.
 *
 *    @param hook Hook to remove.
 */
void pilots_rmHook( unsigned int hook )
{
   int i, j;
   Pilot *p, **plist;
   int n;

   /* Cleaning up a pilot's hooks. */
   if (pilot_hookCleanup)
      return;

   /* Remove global hook first. */
   pilots_rmGlobalHook( hook );

   plist = pilot_getAll( &n );
   for (i=0; i<n; i++) {
      p = plist[i];

      /* Must have hooks. */
      if (p->nhooks <= 0)
         continue;

      for (j=0; j<p->nhooks; j++) {

         /* Hook not found. */
         if (p->hooks[j].id != hook)
            continue;

         p->nhooks--;
         memmove( &p->hooks[j], &p->hooks[j+1], sizeof(PilotHook) * (p->nhooks-j) );
         j--; /* Dun like it but we have to keep iterator sane. */
      }
   }
}


/**
 * @brief Clears the pilots hooks.
 *
 *    @param p Pilot to clear his hooks.
 */
void pilot_clearHooks( Pilot *p )
{
   int i;

   /* No hooks to remove. */
   if (p->nhooks <= 0)
      return;

   /* Remove the hookss. */
   pilot_hookCleanup = 1;
   for (i=0; i<p->nhooks; i++)
      hook_rm( p->hooks[i].id );
   pilot_hookCleanup = 0;

   /* Clear the hooks. */
   free(p->hooks);
   p->hooks  = NULL;
   p->nhooks = 0;
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


