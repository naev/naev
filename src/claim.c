/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file claim.c
 *
 * @brief Handles claiming systems.
 */

#include "claim.h"

#include "naev.h"

#include "log.h"
#include "space.h"
#include "array.h"
#include "event.h"
#include "mission.h"


/**
 * @brief The claim structure.
 */
struct SysClaim_s {
   int *ids; /**< System ids. */
};


/**
 * @brief Creates a system claim.
 *
 *    @return Newly created system claim or NULL on error.
 */
SysClaim_t *claim_create (void)
{
   SysClaim_t *claim;

   claim = malloc( sizeof(SysClaim_t) );
   claim->ids = NULL;

   return claim;
}


/**
 * @brief Adds a claim to a system claim.
 *
 *    @param claim Claim to add system to.
 *    @param ss_id Id of the system to add to the claim.
 */
int claim_add( SysClaim_t *claim, int ss_id )
{
   int *id;

   /* Allocate if necessary. */
   if (claim->ids == NULL)
      claim->ids = array_create( int );

   /* New ID. */
   id  = &array_grow( &claim->ids );
   *id = ss_id;
   return 0;
}


/**
 * @brief Lists the systems a claim has.
 *
 *    @param claim Claim to list systems of.
 *    @param n Number of claims.
 *    @return The list of claims.
 */
int* claim_list( SysClaim_t *claim, int *n )
{
   if (claim->ids == NULL) {
      *n = 0;
      return NULL;
   }

   *n = array_size(claim->ids);
   return claim->ids;
}


/**
 * @brief Tests to see if a system claim would have collisions.
 *
 *    @param claim System to test.
 *    @return 0 if no collision found, 1 if a collision was found.
 */
int claim_test( SysClaim_t *claim )
{
   int claimed;
   int i;

   /* Make sure something to activate. */
   if (claim->ids == NULL)
      return 0;

   /* Check flags. */
   for (i=0; i<array_size(claim->ids); i++) {
      claimed = sys_isFlag( system_getIndex(claim->ids[i]), SYSTEM_CLAIMED );
      if (claimed)
         return 1;
   }

   return 0;
}


/**
 * @brief Tests to see if a system is claimed by a system claim.
 *
 *    @param claim System claim to test.
 *    @param sys System to see if is claimed by the system claim.
 *    @return 0 if no collision is found, 1 otherwise.
 */
int claim_testSys( SysClaim_t *claim, int sys )
{
   int i;

   /* Make sure something to activate. */
   if (claim->ids == NULL)
      return 0;

   /* See if the system is claimed. */
   for (i=0; i<array_size(claim->ids); i++)
      if (claim->ids[i] == sys)
         return 1;

   return 0;
}


/**
 * @brief Destroys a system claim.
 *
 *    @param claim System claim to destroy.
 */
void claim_destroy( SysClaim_t *claim )
{
   if (claim->ids != NULL)
      array_free( claim->ids );
   free(claim);
}


/**
 * @brief Clears the claimings on all systems.
 */
void claim_clear (void)
{
   StarSystem *sys;
   int nsys;
   int i;

   /* Clears all the flags. */
   sys = system_getAll( &nsys );
   for (i=0; i<nsys; i++)
      sys_rmFlag( &sys[i], SYSTEM_CLAIMED );
}


/**
 * @brief Activates all the claims.
 */
void claim_activateAll (void)
{
   claim_clear();
   event_activateClaims();
   missions_activateClaims();
}


/**
 * @brief Activates a claim on a system.
 *
 *    @param claim Claim to activate.
 */
void claim_activate( SysClaim_t *claim )
{
   int i;

   /* Make sure something to activate. */
   if (claim->ids == NULL)
      return;

   /* Add flags. */
   for (i=0; i<array_size(claim->ids); i++)
      sys_setFlag( system_getIndex(claim->ids[i]), SYSTEM_CLAIMED );
}


/**
 * @brief Saves all the systems in a claim in XML.
 *
 * Use between xmlw_startElem and xmlw_endElem.
 *
 *    @param writer XML Writer to use.
 *    @param claim Claim to save.
 */
int claim_xmlSave( xmlTextWriterPtr writer, SysClaim_t *claim )
{
   int i;
   StarSystem *sys;

   if ((claim == NULL) || (claim->ids == NULL))
      return 0;

   for (i=0; i<array_size(claim->ids); i++) {
      sys = system_getIndex( claim->ids[i] );
      if (sys != NULL) {
         xmlw_elem( writer, "sys", "%s", sys->name );
      }
      else
         WARN("System Claim has inexistent system.");
   }

   return 0;
}


/**
 * @brief Loads a claim.
 *
 *    @param parent Parent node containing the claim data.
 *    @return The system claim.
 */
SysClaim_t *claim_xmlLoad( xmlNodePtr parent )
{
   SysClaim_t *claim;
   xmlNodePtr node;
   StarSystem *sys;

   /* Create the claim. */
   claim = claim_create();

   /* Load the nodes. */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"sys")) {
         sys = system_get( xml_get(node) );
         if (sys != NULL)
            claim_add( claim, system_index(sys) );
         else
            WARN("System Claim trying to load system '%s' which doesn't exist.", xml_get(node));
      }
   } while (xml_nextNode(node));

   /* Activate the claim. */
   claim_activate( claim );

   return claim;
}



