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

#include <malloc.h>
#include <string.h>

#include "log.h"
#include "naev.h"
#include "xml.h"


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
extern int misn_run( Mission *misn, char *func );
/* intern */
static int hook_run( Hook *hook );
static void hook_free( Hook *h );
static int hook_needSave( Hook *h );
static int hook_parse( xmlNodePtr base );
/* externed */
int hook_save( xmlTextWriterPtr writer );
int hook_load( xmlNodePtr parent );


/**
 * @fn static int hook_run( Hook *hook )
 *
 * @brief Runs a hook.
 *
 *    @param hook Hook to run.
 *    @return 0 on success.
 */
static int hook_run( Hook *hook )
{
   int i;
   Mission* misn;

   if (hook->delete) return 0; /* hook should be deleted not run */

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
 * @fn unsigned int hook_add( unsigned int parent, char *func, char *stack )
 *
 * @brief Adds a new hook.
 *
 *    @param parent Hook mission parent.
 *    @param func Function to run when hook is triggered.
 *    @param stack Stack hook belongs to.
 *    @return The new hooks identifier.
 */
unsigned int hook_add( unsigned int parent, char *func, char *stack )
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
 * @fn void hook_rm( unsigned int id )
 *
 * @brief Removes a hook.
 *
 *    @param id Identifier of the hook to remove.
 */
void hook_rm( unsigned int id )
{
   int l,m,h;

   l = 0;
   h = hook_nstack-1;
   while (l <= h) {
      m = (l+h)/2;
      if (hook_stack[m].id > id) h = m-1;
      else if (hook_stack[m].id < id) l = m+1;
      else break;
   }

   /* mark to delete, but do not delete yet, hooks are running */
   if (hook_runningstack) {
      hook_stack[m].delete = 1;
      return;
   }

   /* Free the hook */
   hook_free( &hook_stack[m] );

   /* last hook, just clip the stack */
   if (m == (hook_nstack-1)) {
      hook_nstack--;
      return;
   }

   /* move it! */
   memmove( &hook_stack[m], &hook_stack[m+1], sizeof(Hook) * (hook_nstack-m-1) );
   hook_nstack--;
}


/**
 * @fn void hook_rmParent( unsigned int parent )
 *
 * @brief Removes all hooks belonging to parent.
 *
 *    @param parent Parent id to remove all hooks belonging to.
 */
void hook_rmParent( unsigned int parent )
{
   int i;

   for (i=0; i<hook_nstack; i++)
      if (parent == hook_stack[i].parent) {
         hook_rm( hook_stack[i].id );
         if (!hook_runningstack) i--;
      }
}


/**
 * @fn int hooks_run( char* stack )
 *
 * @brief Runs all the hooks of stack.
 *
 *    @param stack Stack to run.
 *    @return 0 on success.
 */
int hooks_run( char* stack )
{
   int i;

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
 * @fn void hook_runID( unsigned int id )
 *
 * @brief Runs a single hook by id.
 *
 *    @param id Identifier of the hook to run.
 */
void hook_runID( unsigned int id )
{
   int i;

   for (i=0; i<hook_nstack; i++)
      if (hook_stack[i].id == id) {
         hook_run( &hook_stack[i] );
         return;
      }
   DEBUG("Attempting to run hook of id '%d' which is not in the stack", id);
}


/**
 * @fn static void hook_free( Hook *h )
 *
 * @brief Frees a hook.
 *
 *    @param h Hook to free.
 */
static void hook_free( Hook *h )
{
   if (h->func != NULL) free(h->func);
   if (h->stack != NULL) free(h->stack);
}


/**
 * @fn void hook_cleanup (void)
 *
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
 * @fn static int hook_needSave( Hook *h )
 *
 * @brief Checks if a hook needs to be saved.
 *
 *    @param h Hook to check if it should be saved.
 *    @return 1 if hook should be saved.
 */
static int hook_needSave( Hook *h )
{
   int i;
   char *nosave[] = {
         "death", "board", "disable", /* pilot hooks */
         "end" };
  
   for (i=0; strcmp(nosave[i],"end") != 0; i++)
      if (strcmp(nosave[i],h->stack)==0) return 0;

   return 1;
}


/**
 * @Fn int hook_save( xmlTextWriterPtr writer )
 *
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
 * @fn int hook_load( xmlNodePtr parent )
 *
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
 * @fn static int hook_parse( xmlNodePtr base )
 *
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


