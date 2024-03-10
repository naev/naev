/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file claim.c
 *
 * @brief Handles claiming of systems.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "claim.h"

#include "array.h"
#include "event.h"
#include "log.h"
#include "mission.h"
#include "space.h"

/**
 * @brief The claim structure.
 */
struct Claim_s {
   int active;    /**< Have we, in fact, claimed these contents?. */
   int *ids;      /**< System ids. */
   char **strs;   /**< Strings. */
   int exclusive; /**< Whether or not this claim is exclusive. Exclusive claims
      do not allow other claims to work, but non-exclusive do not have this issue,
      so multiple non-exclusive claims can share the same system and block any
      exclusive claims. */
};

static char **claimed_strs = NULL; /**< Global claimed strings. */

/**
 * @brief Creates a system claim.
 *
 *    @param exclusive Whether or not this claim is exclusive.
 *    @return Newly created system claim or NULL on error.
 */
Claim_t *claim_create( int exclusive )
{
   Claim_t *claim= malloc( sizeof(Claim_t) );
   claim->active = 0;
   claim->ids    = NULL;
   claim->strs   = NULL;
   claim->exclusive = exclusive;

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
   assert( !claim->active );
   /* Allocate if necessary. */
   if (claim->strs == NULL)
      claim->strs = array_create( char* );

   /* New ID. */
   array_push_back( &claim->strs, strdup( str ) );
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
   assert( !claim->active );
   /* Allocate if necessary. */
   if (claim->ids == NULL)
      claim->ids = array_create( int );

   /* New ID. */
   array_push_back( &claim->ids, ss_id );
   return 0;
}

/**
 * @brief See if a claim actually contains data.
 *
 *    @param claim to test.
 *    @return 0 if claim contains something, 1 otherwise.
 */
int claim_isNull( const Claim_t *claim )
{
   if (claim == NULL)
      return 1;

   if (array_size(claim->ids) == 0)
      return 1;

   return 0;
}

/**
 * @brief Tests to see if a system claim would have collisions.
 *
 *    @param claim System to test.
 *    @return 0 if no collision found, 1 if a collision was found.
 */
int claim_test( const Claim_t *claim )
{
   int exc;

   /* Must actually have a claim. */
   if (claim == NULL)
      return 0;

   exc = claim->exclusive;

   /* See if the system is claimed. */
   for (int i=0; i<array_size(claim->ids); i++) {
      const StarSystem *sys = system_getIndex( claim->ids[i] );
      int claimed = sys_isFlag( sys, SYSTEM_CLAIMED );
      if (claimed || (exc && (sys->claims_soft>0)))
         return 1;
   }

   /* Check strings. */
   for (int i=0; i<array_size(claim->strs); i++) {
      for (int j=0; j<array_size(claimed_strs); j++) {
         if (strcmp( claim->strs[i], claimed_strs[j] )==0)
            return 1;
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
int claim_testStr( const Claim_t *claim, const char *str )
{
   /* Must actually have a claim. */
   if (claim == NULL)
      return 0;

   /* Check strings. */
   for (int i=0; i<array_size(claim->strs); i++) {
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
int claim_testSys( const Claim_t *claim, int sys )
{
   /* Must actually have a claim. */
   if (claim == NULL)
      return 0;

   /* See if the system is claimed. */
   for (int i=0; i<array_size(claim->ids); i++)
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
   if (claim->active) {
      for (int i=0; i<array_size(claim->ids); i++) {
         StarSystem *sys = system_getIndex(claim->ids[i]);
         if (claim->exclusive)
            sys_rmFlag( sys, SYSTEM_CLAIMED );
         else
            sys->claims_soft--;
      }
   }
   array_free( claim->ids );

   for (int i=0; i<array_size(claim->strs); i++) {
      if (claim->active) {
         for (int j=0; j<array_size(claimed_strs); j++) {
            if (strcmp(claim->strs[i], claimed_strs[j])==0) {
               free( claimed_strs[j] );
               array_erase( &claimed_strs, &claimed_strs[j], &claimed_strs[j+1] );
               break;
            }
         }
      }
      free( claim->strs[i] );
   }
   array_free( claim->strs );
   free(claim);
}

/**
 * @brief Clears the claims on all systems.
 */
void claim_clear (void)
{
   /* Clears all the flags. */
   StarSystem *sys = system_getAll();
   for (int i=0; i<array_size(sys); i++) {
      sys_rmFlag( &sys[i], SYSTEM_CLAIMED );
      sys[i].claims_soft = 0;
   }

   for (int i=0; i<array_size(claimed_strs); i++)
      free(claimed_strs[i]);
   array_free(claimed_strs);
   claimed_strs = NULL;
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
   /* Add flags. */
   for (int i=0; i<array_size(claim->ids); i++) {
      StarSystem *sys = system_getIndex( claim->ids[i] );
      if (claim->exclusive)
         sys_setFlag( sys, SYSTEM_CLAIMED );
      else
         sys->claims_soft++;
   }

   /* Add strings. */
   if ((claimed_strs == NULL) && (array_size(claim->strs) > 0))
      claimed_strs = array_create( char* );
   for (int i=0; i<array_size(claim->strs); i++)
      array_push_back( &claimed_strs, strdup( claim->strs[i] ) );
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
int claim_xmlSave( xmlTextWriterPtr writer, const Claim_t *claim )
{
   if (claim == NULL)
      return 0;

   xmlw_attr( writer, "exclusive", "%d", claim->exclusive );

   for (int i=0; i<array_size(claim->ids); i++) {
      StarSystem *sys = system_getIndex( claim->ids[i] );
      if (sys != NULL)
         xmlw_elem( writer, "sys", "%s", sys->name );
      else
         WARN(_("System Claim has inexistent system"));
   }

   for (int i=0; i<array_size(claim->strs); i++)
      xmlw_elem( writer, "str", "%s", claim->strs[i] );

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
   int exclusive;

   /* Exclusiveness defaults to true due to older versions. */
   xmlr_attr_int_def( parent, "exclusive", exclusive, 1 );

   /* Create the claim. */
   claim = claim_create( exclusive );

   /* Load the nodes. */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"sys")) {
         const StarSystem *sys = system_get( xml_get(node) );
         if (sys != NULL)
            claim_addSys( claim, system_index(sys) );
         else
            WARN(_("System Claim trying to load system '%s' which doesn't exist"), xml_get(node));
      }
      else if (xml_isNode(node,"str")) {
         const char *str = xml_get(node);
         claim_addStr( claim, str );
      }
   } while (xml_nextNode(node));

   /* Activate the claim. */
   claim_activate( claim );

   return claim;
}
