/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file shipstats.c
 *
 * @brief Handles the ship statistics.
 */


#include "shipstats.h"

#include "naev.h"

#include "log.h"


typedef struct ShipStatsLookup_ {
   ShipStatsType type;
   const char *name;
   size_t offset;
   int data;
} ShipStatsLookup;


#define ELEM( t, n, d ) \
   { .type=t, .name=#n, .offset=offsetof( ShipStats, n ), .data=d }
#define NELEM( t ) \
   { .type=t, .name=NULL, .offset=0 }

static const ShipStatsLookup ss_lookup[] = {
   NELEM( SS_TYPE_NIL ),

   ELEM( SS_TYPE_D_JUMP_DELAY,      jump_delay, 0 ),
   ELEM( SS_TYPE_D_JUMP_RANGE,      jump_range, 0 ),
   ELEM( SS_TYPE_D_CARGO_INERTIA,   cargo_inertia, 0 ),

   ELEM( SS_TYPE_D_EW_HIDE,         ew_hide, 0 ),
   ELEM( SS_TYPE_D_EW_DETECT,       ew_detect, 0 ),

   ELEM( SS_TYPE_D_LAUNCH_RATE,     launch_rate, 0 ),
   ELEM( SS_TYPE_D_LAUNCH_RANGE,    launch_range, 0 ),
   ELEM( SS_TYPE_D_AMMO_CAPACITY,   ammo_capacity, 0 ),

   ELEM( SS_TYPE_D_FORWARD_HEAT,    fwd_heat, 0 ),
   ELEM( SS_TYPE_D_FORWARD_DAMAGE,  fwd_damage, 0 ),
   ELEM( SS_TYPE_D_FORWARD_FIRERATE, fwd_firerate, 0 ),
   ELEM( SS_TYPE_D_FORWARD_ENERGY,  fwd_energy, 0 ),

   ELEM( SS_TYPE_D_TURRET_HEAT,     tur_heat, 0 ),
   ELEM( SS_TYPE_D_TURRET_DAMAGE,   tur_damage, 0 ),
   ELEM( SS_TYPE_D_TURRET_FIRERATE, tur_firerate, 0 ),
   ELEM( SS_TYPE_D_TURRET_ENERGY,   tur_energy, 0 ),

   ELEM( SS_TYPE_D_NEBULA_DMG_SHIELD, nebula_dmg_shield, 0 ),
   ELEM( SS_TYPE_D_NEBULA_DMG_ARMOUR, nebula_dmg_armour, 0 ),

   ELEM( SS_TYPE_D_HEAT_DISSIPATION, heat_dissipation, 0 ),

   NELEM( SS_TYPE_D_SENTINAL ),

   NELEM( SS_TYPE_B_SENTINAL ),

   /* Sentinal. */
   NELEM( SS_TYPE_SENTINAL )
};

#undef NELEM
#undef ELEM


/*
 * Prototypes.
 */


/**
 * @brief Creatse a shipstat list element from an xml node.
 *
 *    @param node Node to create element from.
 *    @return Liste lement created from node.
 */
ShipStatList* ss_listFromXML( xmlNodePtr node )
{
   const ShipStatsLookup *sl;
   ShipStatList *ll;
   ShipStatsType type;

   /* Try to get type. */
   type = ss_typeFromName( (char*) node->name );
   if (type == SS_TYPE_NIL)
      return NULL;

   /* Allocate. */
   ll = malloc( sizeof(ShipStatList) );
   ll->next    = NULL;
   ll->target  = 0;
   ll->type    = type;

   /* Set the data. */
   sl = &ss_lookup[ type ];
   if (sl->data == 0)
      ll->d.d     = xml_getFloat(node) / 100.;
   DEBUG("%s : %f", sl->name, ll->d.d);

   return ll;
}


/**
 * @brief Checks for sanity.
 */
int ss_check (void)
{
   ShipStatsType i;

   for (i=0; i<=SS_TYPE_SENTINAL; i++) {
      if (ss_lookup[i].type != i) {
         WARN("ss_lookup: %s should have id %d but has %d",
               ss_lookup[i].name, i, ss_lookup[i].type );
         return -1;
      }
   }

   return 0;
}


/**
 * @brief Initializes a stat structure.
 */
int ss_statsInit( ShipStats *stats )
{
   int i;
   char *ptr;
   double *dbl;
   const ShipStatsLookup *sl;

#if DEBUGGING
   memset( stats, 0, sizeof(ShipStats) );
#endif /* DEBUGGING */

   ptr = (char*) stats;
   for (i=0; i<SS_TYPE_SENTINAL; i++) {
      sl = &ss_lookup[ i ];

      /* Only want valid names. */
      if (sl->name == NULL)
         continue;

      /* Handle doubles. */
      if (sl->data == 0) {
         dbl   = (double*) &ptr[ sl->offset ];
         *dbl  = 1.0;
      }
   }

   return 0;
}


/**
 * @brief Modifies a stat structure using a single element.
 *
 *    @param stats Stat structure to modify.
 *    @param list Single element to apply.
 *    @return 0 on success.
 */
int ss_statsModSingle( ShipStats *stats, const ShipStatList* list )
{
   char *ptr;
   double *dbl;
   const ShipStatsLookup *sl = &ss_lookup[ list->type ];

   ptr = (char*) stats;
   if (sl->data == 0) {
      dbl   = (double*) &ptr[ sl->offset ];
      *dbl *= (1. + list->d.d);
   }

   return 0;
}


/**
 * @brief Updates a stat structure from a stat list.
 *
 *    @param stats Stats to update.
 *    @param list List to update from.
 */
int ss_statsModFromList( ShipStats *stats, const ShipStatList* list )
{
   int ret;
   const ShipStatList *ll;

   ret = 0;
   for (ll = list; ll != NULL; ll = ll->next)
      ret |= ss_statsModSingle( stats, ll );

   return ret;
}


/**
 * @brief Gets the name from type.
 *
 * O(1) look up.
 *
 *    @param type Type to get name of.
 *    @return Name of the type.
 */
const char* ss_nameFromType( ShipStatsType type )
{
   return ss_lookup[ type ].name;
}


/**
 * @brief Gets the type from the name.
 *
 *    @param name Name to get type of.
 *    @return Type matching the name.
 */
ShipStatsType ss_typeFromName( const char *name )
{
   int i;
   for (i=0; i<SS_TYPE_SENTINAL; i++)
      if ((ss_lookup[i].name != NULL) && (strcmp(name,ss_lookup[i].name)==0))
         return ss_lookup[i].type;
   return SS_TYPE_NIL;
}


int ss_statsListDesc( const ShipStatList *ll, char *buf, int len, int newline, int pilot )
{
   int i;
   const ShipStatsLookup *sl;
   i = 0;
   for ( ; ll != NULL; ll=ll->next) {
      sl = &ss_lookup[ ll->type ];

      i += snprintf( &buf[i], len-i, \
            "%s%+.0f%% %s", (!newline&&(i==0)) ? "" : "\n", \
            ll->d.d*100., sl->name );
   }
   return i;
}


/**
 * @brief Writes the ship statistics description.
 *
 *    @param s Ship stats to use.
 *    @param buf Buffer to write to.
 *    @param len Space left in the buffer.
 *    @param newline Add a newline at start.
 *    @param pilot Stats come from a pilot.
 *    @return Number of characters written.
 */
int ss_statsDesc( const ShipStats *s, char *buf, int len, int newline, int pilot )
{
   int i;

   /* Set stat text. */
   i = 0;
#define DESC_ADD(x, s) \
   if ((pilot && (x!=1.)) || (!pilot && (x!=0.))) \
      i += snprintf( &buf[i], len-i, \
            "%s%+.0f%% "s, (!newline&&(i==0)) ? "" : "\n", \
            (pilot) ? (x-1.)*100. : x );
   /* Freighter Stuff. */
   DESC_ADD(s->jump_delay,"Jump Time");
   DESC_ADD(s->jump_range,"Jump Range");
   DESC_ADD(s->cargo_inertia,"Cargo Inertia");
   /* Scout Stuff. */
   DESC_ADD(s->ew_detect,"Detection");
   DESC_ADD(s->ew_hide,"Cloaking");
   /* Military Stuff. */
   DESC_ADD(s->heat_dissipation,"Heat Dissipation");
   /* Bomber Stuff. */
   DESC_ADD(s->launch_rate,"Launch Rate");
   DESC_ADD(s->launch_range,"Launch Range");
   DESC_ADD(s->ammo_capacity,"Ammo Capacity");
   /* Fighter Stuff. */
   DESC_ADD(s->fwd_heat,"Heat (Cannon)");
   DESC_ADD(s->fwd_damage,"Damage (Cannon)");
   DESC_ADD(s->fwd_firerate,"Fire Rate (Cannon)");
   DESC_ADD(s->fwd_energy,"Energy Usage (Cannon)");
   /* Cruiser Stuff. */
   DESC_ADD(s->tur_heat,"Heat (Turret)");
   DESC_ADD(s->tur_damage,"Damage (Turret)");
   DESC_ADD(s->tur_firerate,"Fire Rate (Turret)");
   DESC_ADD(s->tur_energy,"Energy Usage (Turret)");
   /* Misc. */
   DESC_ADD(s->nebula_dmg_shield,"Nebula Damage (Shield)");
   DESC_ADD(s->nebula_dmg_armour,"Nebula Damage (Armour)");
#undef DESC_ADD

   return i;
}


