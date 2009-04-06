/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file hook.c
 *
 * @brief Handles hooks.
 *
 * Currently only used in the mission system.
 */


#include "hook.h"

#include "naev.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "nxml.h"
#include "player.h"


#define HOOK_CHUNK   32 /**< Size to grow by when out of space */


/**
 * @struct Hook
 *
 * @brief Internal representation of a hook.
 */
typedef struct Hook_ {
   unsigned int id; /**< unique id */
   unsigned int parent; /**< mission it's connected to */
   char *func; /**< function it runs */
   char *stack; /**< stack it's a part of */

   int delete; /**< indicates it should be deleted when possible */
} Hook;


/* 
 * the stack
 */
static unsigned int hook_id = 0; /**< Unique hook id generator. */
static Hook* hook_stack = NULL; /**< Stack of hooks. */
static int hook_mstack = 0; /**< Size of hook memory. */
static int hook_nstack = 0; /**< Number of hooks currently used. */
static int hook_runningstack = 0; /**< Check if stack is running. */


/*
 * prototypes
 */
/* extern */
extern int misn_run( Mission *misn, const char *func );
/* intern */
static int hook_run( Hook *hook );
static void hook_free( Hook *h );
static int hook_needSave( Hook *h );
static int hook_parse( xmlNodePtr base );
/* externed */
int hook_save( xmlTextWriterPtr writer );
int hook_load( xmlNodePtr parent );


/**
 * @brief Runs a hook.
 *
 *    @param hook Hook to run.
 *    @return 0 on success.
 */
static int hook_run( Hook *hook )
{
   int i;
   Mission* misn;

   if (hook->delete)
      return 0; /* hook should be deleted not run */

   /* locate the mission */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].id == hook->parent)
         break;
   if (i>=MISSION_MAX) {
      WARN("Trying to run hook with parent not in player mission stack: deleting");
      hook->delete = 1; /* so we delete it */
      return -1;
   }
   misn = &player_missions[i];

   if (misn_run( misn, hook->func ) < 0) /* error has occured */
      WARN("Hook [%s] '%d' -> '%s' failed", hook->stack,
            hook->id, hook->func);

   return 0;
}


/**
 * @brief Adds a new hook.
 *
 *    @param parent Hook mission parent.
 *    @param func Function to run when hook is triggered.
 *    @param stack Stack hook belongs to.
 *    @return The new hooks identifier.
 */
unsigned int hook_add( unsigned int parent, const char *func, const char *stack )
{
   Hook *new_hook;

   /* if memory must grow */
   if (hook_nstack+1 > hook_mstack) {
      hook_mstack += HOOK_CHUNK;
      hook_stack = realloc(hook_stack, hook_mstack*sizeof(Hook));
   }

   /* create the new hook */
   new_hook = &hook_stack[hook_nstack];
   new_hook->id = ++hook_id;
   new_hook->parent = parent;
   new_hook->func = strdup(func);
   new_hook->stack = strdup(stack);
   new_hook->delete = 0;

   hook_nstack++;

   return new_hook->id;
}


/**
 * @brief Removes a hook.
 *
 *    @param id Identifier of the hook to remove.
 *    @return 1 if hook was removed, 2 if hook was scheduled for removal and
 *            0 if it wasn't removed.
 */
int hook_rm( unsigned int id )
{
   int l,m,h,f;

   /* Binary search. */
   f = 0;
   l = 0;
   h = hook_nstack-1;
   while (l <= h) {
      m = (l+h)/2;
      if (hook_stack[m].id > id) h = m-1;
      else if (hook_stack[m].id < id) l = m+1;
      else {
         f = 1;
         break;
      }
   }

   /* Check if hook was found. */
   if (f == 0)
      return 0;

   /* Mark to delete, but do not delete yet, hooks are running. */
   if (hook_runningstack) {
      hook_stack[m].delete = 1;
      return 2;
   }

   /* Free the hook. */
   hook_free( &hook_stack[m] );

   /* Last hook, just clip the stack. */
   if (m == (hook_nstack-1)) {
      hook_nstack--;
      return 1;
   }

   /* Move it! */
   memmove( &hook_stack[m], &hook_stack[m+1], sizeof(Hook) * (hook_nstack-m-1) );
   hook_nstack--;
   return 1;
}


/**
 * @brief Removes all hooks belonging to parent.
 *
 *    @param parent Parent id to remove all hooks belonging to.
 */
void hook_rmParent( unsigned int parent )
{
   int i;

   for (i=0; i<hook_nstack; i++)
      if (parent == hook_stack[i].parent) {
         /* Only decrement if hook was actually removed. */
         if (hook_rm( hook_stack[i].id ) == 1)
            i--;
      }
}


/**
 * @brief Runs all the hooks of stack.
 *
 *    @param stack Stack to run.
 *    @return 0 on success.
 */
int hooks_run( const char* stack )
{
   int i;

   /* Don't update if player is dead. */
   if ((player==NULL) || player_isFlag(PLAYER_DESTROYED))
      return 0;

   hook_runningstack = 1; /* running hooks */
   for (i=0; i<hook_nstack; i++)
      if ((strcmp(stack, hook_stack[i].stack)==0) && !hook_stack[i].delete) {
         hook_run( &hook_stack[i] );
      }
   hook_runningstack = 0; /* not running hooks anymore */

   for (i=0; i<hook_nstack; i++)
      if (hook_stack[i].delete) { /* Delete any that need deleting */
         hook_rm( hook_stack[i].id );
         i--;
      }
   
   return 0;
}


/**
 * @brief Runs a single hook by id.
 *
 *    @param id Identifier of the hook to run.
 *    @return The ID of the hook or 0 if it got deleted.
 */
void hook_runID( unsigned int id )
{
   Hook *h;
   int i, ret;

   /* Don't update if player is dead. */
   if ((player==NULL) || player_isFlag(PLAYER_DESTROYED))
      return;

   /* Try to find the hook and run it. */
   ret = 0;
   for (i=0; i<hook_nstack; i++)
      if (hook_stack[i].id == id) {
         h = &hook_stack[i];
         hook_run( h );
         ret = 1;
         break;
      }

   /* Hook not found. */
   if (ret == 0)
      DEBUG("Attempting to run hook of id '%d' which is not in the stack", id);
}


/**
 * @brief Frees a hook.
 *
 *    @param h Hook to free.
 */
static void hook_free( Hook *h )
{
   if (h->func != NULL)
      free(h->func);
   if (h->stack != NULL)
      free(h->stack);
}


/**
 * @brief Gets rid of all current hooks.
 */
void hook_cleanup (void)
{
   int i;

   for (i=0; i<hook_nstack; i++)
      hook_free( &hook_stack[i] );
   free( hook_stack );
   /* sane defaults just in case */
   hook_stack = NULL;
   hook_nstack = 0;
   hook_mstack = 0;
}


/**
 * @brief Checks if a hook needs to be saved.
 *
 *    @param h Hook to check if it should be saved.
 *    @return 1 if hook should be saved.
 */
static int hook_needSave( Hook *h )
{
   int i;
   char *nosave[] = {
         "death", "board", "disable", "jump", /* pilot hooks */
         "end" };
  
   for (i=0; strcmp(nosave[i],"end") != 0; i++)
      if (strcmp(nosave[i],h->stack)==0) return 0;

   return 1;
}


/**
 * @brief Saves all the hooks.
 *
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int hook_save( xmlTextWriterPtr writer )
{
   int i;
   Hook *h;

   xmlw_startElem(writer,"hooks");
   for (i=0; i<hook_nstack; i++) {
      h = &hook_stack[i];

      if (!hook_needSave(h)) continue; /* no need to save it */

      xmlw_startElem(writer,"hook");

      /* xmlw_attr(writer,"id","%u",h->id); I don't think it's needed */
      xmlw_elem(writer,"parent","%u",h->parent);
      xmlw_elem(writer,"func","%s",h->func);
      xmlw_elem(writer,"stack","%s",h->stack);

      xmlw_endElem(writer); /* "hook" */
   }
   xmlw_endElem(writer); /* "hooks" */

   return 0;
}


/**
 * @brief Loads hooks for a player.
 *
 *    @param parent Parent xml node containing the hooks.
 *    @return 0 on success.
 */
int hook_load( xmlNodePtr parent )
{
   xmlNodePtr node;

   hook_cleanup();

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"hooks"))
         hook_parse(node);
   } while (xml_nextNode(node));

   return 0;
}


/**
 * @brief Parses an individual hook.
 *
 *    @param base Parent xml node of the hook.
 *    @return 0 on success.
 */
static int hook_parse( xmlNodePtr base )
{
   xmlNodePtr node, cur;
   char *func, *stack;
   unsigned int parent;

   node = base->xmlChildrenNode;
   do {
      if (xml_isNode(node,"hook")) {
         parent = 0;
         func = NULL;
         stack = NULL;

         cur = node->xmlChildrenNode;
         do {
            xmlr_long(cur,"parent",parent);
            xmlr_str(cur,"func",func);
            xmlr_str(cur,"stack",stack);
         } while (xml_nextNode(cur));

         if ((parent == 0) || (func == NULL) || (stack == NULL)) {
            WARN("Invalid hook.");
            return -1;
         }
         hook_add( parent, func, stack );
      }
   } while (xml_nextNode(node));

   return 0;
}


