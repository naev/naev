/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file npc.c
 *
 * @brief Handles NPC stuff.
 */


#include "npc.h"

#include "naev.h"

#include <string.h>

#include "log.h"
#include "land.h"
#include "opengl.h"
#include "array.h"


/**
 * @brief NPC types.
 */
typedef enum NPCtype {
   NPC_TYPE_NULL,
   NPC_TYPE_GIVER,
   NPC_TYPE_MISSION,
   NPC_TYPE_EVENT
}

/**
 * @brief Minimum needed NPC data for event/mission.
 */
typedef struct NPCdata_ {
   char *name;
   char *func;
} NPCdata;

/**
 * @brief The bar NPC.
 */
typedef struct NPC_s {
   NPCtype type;
   int priority;
   char *name;
   glTexture *portrait;
   union {
      Mission misn;
      NPCdata data;
   } u;
} NPC_t;

static unsigned int npc_array_idgen = 0; /**< ID generator. */
static NPC_t* npc_array  = NULL; /**< Missions at the spaceport bar. */


/*
 * Prototypes.
 */
static unsigned int npc_add( NPC_t *npc );
static unsigned int npc_add_giver( Mission *misn );
static unsigned int npc_add_me( NPCtype type, char *parent, char *func,
      char *name, int priority, char *portrait );
static int npc_rm( NPC_t *npc );
static void npc_sort (void);
static NPC_t *npc_arrayGet( unsigned int id );
static void npc_free( NPC_t *npc );


/**
 * @brief Adds an NPC to the spaceport bar.
 */
static unsigned int npc_add( NPC_t *npc )
{
   NPC_t *new_npc;

   /* Must be landed. */
   if (!landed)
      return 0;

   /* Create if needed. */
   if (npc_array == NULL)
      npc_array = array_create( NPC_t );

   /* Grow. */
   new_npc = &array_grow( npc_array );

   /* Copy over. */
   memcpy( new_npc, npc, sizeof(NPC_t) );

   /* Set ID. */
   new_npc->id = ++npc_array_idgen;
   return new_npc->id;
}


/**
 * @brief Adds a mission giver NPC to the mission computer.
 */
static unsigned int npc_add_giver( Mission *misn )
{
   NPC_t npc;

   /* Set up the data. */
   npc.type       = NPC_TYPE_GIVER;
   npc.name       = strdup(misn->npc);
   npc.priority   = misn->data->priority;
   npc.portrait   = misn->portrait;
   memcpy( &npc.u.misn, misn, sizeof(Mission) );

   return npc_add( &npc );
}


/**
 * @brief Adds either a mission or event NPC.
 */
static unsigned int npc_add_me( NPCtype type, char *parent, char *func,
      char *name, int priority, char *portrait )
{
   NPC_t npc;

   /* The data. */
   npc.type       = type;
   npc.name       = strdup( parent );
   npc.priority   = priority;
   npc.portrait   = gl_newImage( portrait, 0 );
   npc.u.data.name = misn;
   npc.u.data.func = strdup( func );

   return npc_add( &npc );
}


/**
 * @brief Adds a mission NPC to the mission computer.
 */
unsigned int npc_add_mission( char *misn, char *func, char *name,
      int priority, char *portrait )
{
   return npc_add_me( NPC_TYPE_MISSION, misn, func, name, priority, portrait );
}


/**
 * @brief Adds a event NPC to the mission computer.
 */
unsigned int npc_add_event( char *evt, char *func, char *name,
      int priority, char *portrait )
{
   return npc_add_me( NPC_TYPE_EVENT, evt, func, name, priority, portrait );
}


/**
 * @brief Removes an npc from the spaceport bar.
 */
static int npc_rm( NPC_t *npc )
{
   return 0;
}


/**
 * @brief Gets an NPC by ID.
 */
static NPC_t *npc_arrayGet( unsigned int id )
{
   int i;
   for (i=0; i<array_size( npc_array ); i++)
      if (npc_array[i].id == id)
         return &npc_array[i].id;
   return NULL;
}


/**
 * @brief removes an event NPC.
 */
int npc_rm_event( unsigned int id, const char *evt )
{
   NPC_t *npc;

   /* Get the NPC. */
   npc = npc_arrayGet( id );
   if (npc == NULL)
      return -1;

   /* Doesn't match type. */
   if (npc->type != NPC_TYPE_EVENTN)
      return -1;

   /* Doesn't belong to the mission. */
   if (strcmp(npc->u.name, evt)!=0)
      return -1;

   /* Remove the NPC. */
   return npc_rm( npc );
}


/**
 * @brief removes a mission NPC.
 */
int npc_rm_mission( unsigned int id, const char *misn )
{
   NPC_t *npc;

   /* Get the NPC. */
   npc = npc_arrayGet( id );
   if (npc == NULL)
      return -1;

   /* Doesn't match type. */
   if (npc->type != NPC_TYPE_MISSION)
      return -1;

   /* Doesn't belong to the mission. */
   if (strcmp(npc->u.name, misn)!=0)
      return -1;

   /* Remove the NPC. */
   return npc_rm( npc );
}


/**
 * @brief NPC compare function.
 */
static void npc_compare( const void *arg1, const void *arg2 )
{
   const NPC_t *npc1, *npc2;

   npc1 = (NPC_t*)arg1;
   npc2 = (NPC_t*)arg2;

   /* Compare priority. */
   if (npc1->priority > npc2->priority)
      return +1;
   else if (npc1->priority < npc2->priority)
      return -1;
   
   return 0;
}


/**
 * @brief Sorts the NPCs.
 */
static void npc_sort (void)
{
   qsort( npc_array, array_size(npc_array), sizeof(NPC_t), npc_compare );
}


/**
 * @brief Generates the bar missions.
 */
void npc_generate (void)
{
   int i;
   Mission *missions;
   int nmissions;

   /* Get the missions. */
   missions = missions_genList( &nmissions,
         land_planet->faction, land_planet->name, cur_system->name,
         MIS_AVAIL_BAR );

   /* Add to the bar NPC stack - may be not empty. */
   for (i=0; i<nmissions; i++)
      npc_add_giver( &missions[i] );

   /* Sort NPC. */
   npc_sort();
}


/**
 * @brief Frees a single npc.
 */
static void npc_free( NPC_t *npc )
{
   switch (npc->type) {
      case NPC_TYPE_GIVER:
         free(npc->name);
         break;

      case NPC_TYPE_MISSION:
      case NPC_TYPE_EVENT:
         free(npc->name);
         gl_freeTexture(npc->portrait);
         free(npc->u.data.func);
         break;

      default:
         WARN("Freeing NPC of invalid type.");
         return;
   }
}


/**
 * @brief Cleans up the spaceport bar NPC.
 */
void npc_clear (void)
{
   int i;

   /* First pass to clear the data. */
   for (i=0; i<array_size( npc_array ); i++) {
      npc_free( &npc_array[i] );
   }

   /* Second pass clears array. */
}


/**
 * @brief Frees the NPC stuff.
 */
void npc_freeAll (void)
{
   /* Clear the NPC. */
   npc_clear();

   /* Free the array. */
   array_free( npc_array );
   npc_array = NULL;
}



