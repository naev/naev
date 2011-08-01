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


/**
 * @brief Internal look up table for ship stats.
 *
 * Makes it much easier to work with stats at the cost of some minor performance.
 */
typedef struct ShipStatsLookup_ {
   ShipStatsType type;  /**< Type of the stat. */
   const char *name;    /**< Name to look into XML for, must match name in the structure. */
   const char *display; /**< Display name for visibility by player. */
   size_t offset;       /**< Stores the byte offset in the structure. */
   int data;            /**< Type of data for the stat. */
} ShipStatsLookup;


#define ELEM( t, n, dsp, d ) \
   { .type=t, .name=#n, .display=dsp, .offset=offsetof( ShipStats, n ), .data=d }
#define NELEM( t ) \
   { .type=t, .name=NULL, .display=NULL, .offset=0 }

static const ShipStatsLookup ss_lookup[] = {
   NELEM( SS_TYPE_NIL ),

   ELEM( SS_TYPE_D_JUMP_DELAY,      jump_delay, "Jump Time", 0 ),
   ELEM( SS_TYPE_D_JUMP_RANGE,      jump_range, "Jump Range", 0 ),
   ELEM( SS_TYPE_D_CARGO_INERTIA,   cargo_inertia, "Cargo Inertia", 0 ),

   ELEM( SS_TYPE_D_EW_HIDE,         ew_hide, "Detection", 0 ),
   ELEM( SS_TYPE_D_EW_DETECT,       ew_detect, "Cloaking", 0 ),

   ELEM( SS_TYPE_D_LAUNCH_RATE,     launch_rate, "Launch Rate", 0 ),
   ELEM( SS_TYPE_D_LAUNCH_RANGE,    launch_range, "Launch Range", 0 ),
   ELEM( SS_TYPE_D_AMMO_CAPACITY,   ammo_capacity, "Ammo Capacity", 0 ),
   ELEM( SS_TYPE_D_LAUNCH_LOCKON,   launch_lockon, "Launch Lockon", 0 ),

   ELEM( SS_TYPE_D_FORWARD_HEAT,    fwd_heat, "Heat (Cannon)", 0 ),
   ELEM( SS_TYPE_D_FORWARD_DAMAGE,  fwd_damage, "Damage (Cannon)", 0 ),
   ELEM( SS_TYPE_D_FORWARD_FIRERATE, fwd_firerate, "Fire Rate (Cannon)", 0 ),
   ELEM( SS_TYPE_D_FORWARD_ENERGY,  fwd_energy, "Energy Usage (Cannon)", 0 ),

   ELEM( SS_TYPE_D_TURRET_HEAT,     tur_heat, "Heat (Turret)", 0 ),
   ELEM( SS_TYPE_D_TURRET_DAMAGE,   tur_damage, "Damage (Turret)", 0 ),
   ELEM( SS_TYPE_D_TURRET_FIRERATE, tur_firerate, "Fire Rate (Turret)", 0 ),
   ELEM( SS_TYPE_D_TURRET_ENERGY,   tur_energy, "Energy Usage (Turret)", 0 ),

   ELEM( SS_TYPE_D_NEBULA_DMG_SHIELD, nebula_dmg_shield, "Nebula Damage (Shield)", 0 ),
   ELEM( SS_TYPE_D_NEBULA_DMG_ARMOUR, nebula_dmg_armour, "Nebula Damage (Armour)", 0 ),

   ELEM( SS_TYPE_D_HEAT_DISSIPATION, heat_dissipation, "Heat Dissipation", 0 ),

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
 *    @param amount If non nil stores the number found in amount.
 *    @return 0 on success.
 */
int ss_statsModSingle( ShipStats *stats, const ShipStatList* list, const ShipStats *amount )
{
   char *ptr;
   double *dbl;
   const ShipStatsLookup *sl = &ss_lookup[ list->type ];

   ptr = (char*) stats;
   if (sl->data == 0) {
      dbl   = (double*) &ptr[ sl->offset ];
      *dbl += list->d.d;
      if (*dbl < 0.) /* Don't let the values go negative. */
         *dbl = 0.;

      /* We'll increment amount. */
      if (amount != NULL) {
         if (list->d.d > 0.) {
            ptr      = (char*) amount;
            dbl      = (double*) &ptr[ sl->offset ];
            (*dbl)  += 1.0;
         }
      }
   }

   return 0;
}


/**
 * @brief Updates a stat structure from a stat list.
 *
 *    @param stats Stats to update.
 *    @param list List to update from.
 *    @param amount If non nil stores the number found in amount.
 */
int ss_statsModFromList( ShipStats *stats, const ShipStatList* list, const ShipStats *amount )
{
   int ret;
   const ShipStatList *ll;

   ret = 0;
   for (ll = list; ll != NULL; ll = ll->next)
      ret |= ss_statsModSingle( stats, ll, amount );

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


/**
 * @brief Writes the ship statistics description.
 *
 *    @param s Ship stats to use.
 *    @param buf Buffer to write to.
 *    @param len Space left in the buffer.
 *    @param newline Add a newline at start.
 *    @return Number of characters written.
 */
int ss_statsListDesc( const ShipStatList *ll, char *buf, int len, int newline )
{
   int i;
   const ShipStatsLookup *sl;
   i = 0;
   for ( ; ll != NULL; ll=ll->next) {
      sl = &ss_lookup[ ll->type ];

      i += snprintf( &buf[i], (len-i),
            "%s%+.0f%% %s", (!newline&&(i==0)) ? "" : "\n",
            ll->d.d*100., sl->display );
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
int ss_statsDesc( const ShipStats *s, char *buf, int len, int newline )
{
   int i, l;
   char *ptr;
   double *dbl;
   const ShipStatsLookup *sl;

   l   = 0;
   ptr = (char*) s;
   for (i=0; i<SS_TYPE_SENTINAL; i++) {
      sl = &ss_lookup[ i ];

      /* Only want valid names. */
      if (sl->name == NULL)
         continue;

      /* Handle doubles. */
      if (sl->data == 0) {
         dbl   = (double*) &ptr[ sl->offset ];
         if (fabs((*dbl)-1.) > 1e-10)
            l += snprintf( &buf[l], (len-l),
                  "%s%+.0f%% %s", (!newline && (l==0)) ? "" : "\n",
                  ((*dbl)-1.)*100., sl->display );
      }
   }

   return l;
}


/**
 * @brief Frees a list of ship stats.
 *
 *    @param ll List to free.
 */
void ss_free( ShipStatList *ll )
{
   while (ll != NULL) {
      ShipStatList *tmp = ll;
      ll = ll->next;
      free(tmp);
   }
}



