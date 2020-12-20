/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file claim.c
 *
 * @brief Handles claiming of systems.
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
struct Claim_s {
   int active; /**< Have we, in fact, claimed these contents?. */
   int *ids; /**< System ids. */
   char **strs; /**< Strings. */
};

static char **claimed_strs = NULL; /**< Global claimed strings. */


/**
 * @brief Creates a system claim.
 *
 *    @return Newly created system claim or NULL on error.
 */
Claim_t *claim_create (void)
{
   Claim_t *claim;

   claim         = malloc( sizeof(Claim_t) );
   claim->active = 0;
   claim->ids    = NULL;
   claim->strs   = NULL;

   return claim;
}


/**
 * @brief Adds a string claim to a claim.
 *
 *    @param claim Claim to add system to.
 *    @param str String to claim.
 */
int claim_addStr( Claim_t *claim, const char *str )
{
   char **s;

   assert( !claim->active );
   /* Allocate if necessary. */
   if (claim->strs == NULL)
      claim->strs = array_create( char* );

   /* New ID. */
   s = &array_grow( &claim->strs );
   *s = strdup( str );
   return 0;
}


/**
 * @brief Adds a claim to a system claim.
 *
 *    @param claim Claim to add system to.
 *    @param ss_id Id of the system to add to the claim.
 */
int claim_addSys( Claim_t *claim, int ss_id )
{
   int *id;

   assert( !claim->active );
   /* Allocate if necessary. */
   if (claim->ids == NULL)
      claim->ids = array_create( int );

   /* New ID. */
   id  = &array_grow( &claim->ids );
   *id = ss_id;
   return 0;
}


/**
 * @brief Tests to see if a system claim would have collisions.
 *
 *    @param claim System to test.
 *    @return 0 if no collision found, 1 if a collision was found.
 */
int claim_test( Claim_t *claim )
{
   int claimed, i, j;

   /* Must actually have a claim. */
   if (claim == NULL)
      return 0;

   /* See if the system is claimed. */
   if (claim->ids != NULL) {
      for (i=0; i<array_size(claim->ids); i++) {
         claimed = sys_isFlag( system_getIndex(claim->ids[i]), SYSTEM_CLAIMED );
         if (claimed)
            return 1;
      }
   }

   /* Check strings. */
   if ((claim->strs != NULL) && (claimed_strs != NULL)) {
      for (i=0; i<array_size(claim->strs); i++) {
         for (j=0; j<array_size(claimed_strs); j++) {
            if (strcmp( claim->strs[i], claimed_strs[j] )==0)
               return 1;
         }
      }
   }

   return 0;
}


/**
 * @brief Tests to see if a system is claimed by a system claim.
 *
 *    @param claim Claim to test.
 *    @param str Stringto see if is claimed by the claim.
 *    @return 0 if no collision is found, 1 otherwise.
 */
int claim_testStr( Claim_t *claim, const char *str )
{
   int i;

   /* Must actually have a claim. */
   if (claim == NULL)
      return 0;

   /* Make sure something to activate. */
   if (claim->strs == NULL)
      return 0;

   /* Check strings. */
   for (i=0; i<array_size(claim->strs); i++) {
      if (strcmp( claim->strs[i], str )==0)
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
int claim_testSys( Claim_t *claim, int sys )
{
   int i;

   /* Must actually have a claim. */
   if (claim == NULL)
      return 0;

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
void claim_destroy( Claim_t *claim )
{
   int i;

   if (claim->ids != NULL) {
      if (claim->active)
         for (i=0; i<array_size(claim->ids); i++)
            sys_rmFlag( system_getIndex(claim->ids[i]), SYSTEM_CLAIMED );
      array_free( claim->ids );
   }

   if (claim->strs != NULL) {
      if (claim->active)
         for (i=0; i<array_size(claim->strs); i++)
            free( claim->strs[i] );
      array_free( claim->strs );
   }
   free(claim);
}


/**
 * @brief Clears the claims on all systems.
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

   if (claimed_strs != NULL) {
      for (i=0; i<array_size(claimed_strs); i++)
         free(claimed_strs[i]);
      array_free(claimed_strs);
      claimed_strs = NULL;
   }
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
void claim_activate( Claim_t *claim )
{
   int i;
   char **s;

   /* Add flags. */
   if (claim->ids != NULL) {
      for (i=0; i<array_size(claim->ids); i++)
         sys_setFlag( system_getIndex(claim->ids[i]), SYSTEM_CLAIMED );
   }

   /* Add strings. */
   if (claim->strs != NULL) {
      if (claimed_strs == NULL)
         claimed_strs = array_create( char* );
      for (i=0; i<array_size(claim->strs); i++) {
         s = &array_grow( &claimed_strs );
         *s = strdup( claim->strs[i] );
      }
   }
   claim->active = 1;
}


/**
 * @brief Saves all the systems in a claim in XML.
 *
 * Use between xmlw_startElem and xmlw_endElem.
 *
 *    @param writer XML Writer to use.
 *    @param claim Claim to save.
 */
int claim_xmlSave( xmlTextWriterPtr writer, Claim_t *claim )
{
   int i;
   StarSystem *sys;

   if (claim == NULL)
      return 0;

   if (claim->ids != NULL) {
      for (i=0; i<array_size(claim->ids); i++) {
         sys = system_getIndex( claim->ids[i] );
         if (sys != NULL) {
            xmlw_elem( writer, "sys", "%s", sys->name );
         }
         else
            WARN(_("System Claim has inexistent system"));
      }
   }

   if (claim->strs != NULL) {
      for (i=0; i<array_size(claim->strs); i++)
         xmlw_elem( writer, "str", "%s", claim->strs[i] );
   }

   return 0;
}


/**
 * @brief Loads a claim.
 *
 *    @param parent Parent node containing the claim data.
 *    @return The system claim.
 */
Claim_t *claim_xmlLoad( xmlNodePtr parent )
{
   Claim_t *claim;
   xmlNodePtr node;
   StarSystem *sys;
   char *str;

   /* Create the claim. */
   claim = claim_create();

   /* Load the nodes. */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"sys")) {
         sys = system_get( xml_get(node) );
         if (sys != NULL)
            claim_addSys( claim, system_index(sys) );
         else
            WARN(_("System Claim trying to load system '%s' which doesn't exist"), xml_get(node));
      }
      else if (xml_isNode(node,"str")) {
         str = xml_get(node);
         claim_addStr( claim, str );
      }
   } while (xml_nextNode(node));

   /* Activate the claim. */
   claim_activate( claim );

   return claim;
}



