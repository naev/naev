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
#include "nstring.h"


/**
 * @brief The data type.
 */
typedef enum StatDataType_ {
   SS_DATA_TYPE_DOUBLE,          /**< Relative [0:inf] value. */
   SS_DATA_TYPE_DOUBLE_ABSOLUTE, /**< Absolute double value. */
   SS_DATA_TYPE_INTEGER,         /**< Absolute integer value. */
   SS_DATA_TYPE_BOOLEAN          /**< Boolean value, defaults 0. */
} StatDataType;


/**
 * @brief Internal look up table for ship stats.
 *
 * Makes it much easier to work with stats at the cost of some minor performance.
 */
typedef struct ShipStatsLookup_ {
   /* Explicitly set. */
   ShipStatsType type;  /**< Type of the stat. */
   const char *name;    /**< Name to look into XML for, must match name in the structure. */
   const char *display; /**< Display name for visibility by player. */
   StatDataType data;   /**< Type of data for the stat. */
   int inverted;        /**< Indicates whether the good value is inverted, by
                             default positive is good, with this set negative
                             is good. */

   /* Self calculated. */
   size_t offset;       /**< Stores the byte offset in the structure. */
} ShipStatsLookup;


/* Flexible do everything macro. */
#define ELEM( t, n, dsp, d , i) \
   { .type=t, .name=#n, .display=dsp, .data=d, .inverted=i, .offset=offsetof( ShipStats, n ) }
/* Standard types. */
#define D__ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, SS_DATA_TYPE_DOUBLE, 0 )
#define A__ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, SS_DATA_TYPE_DOUBLE_ABSOLUTE, 0 )
#define I__ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, SS_DATA_TYPE_INTEGER, 0 )
#define B__ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, SS_DATA_TYPE_BOOLEAN, 0 )
/* Inverted types. */
#define DI_ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, SS_DATA_TYPE_DOUBLE, 1 )
#define AI_ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, SS_DATA_TYPE_DOUBLE_ABSOLUTE, 1 )
#define II_ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, SS_DATA_TYPE_INTEGER, 1 )
#define BI_ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, SS_DATA_TYPE_BOOLEAN, 1 )
/** Nil element. */
#define N__ELEM( t ) \
   { .type=t, .name=NULL, .display=NULL, .inverted=0, .offset=0 }

/**
 * The ultimate look up table for ship stats, everything goes through this.
 */
static const ShipStatsLookup ss_lookup[] = {
   /* Null element. */
   N__ELEM( SS_TYPE_NIL ),

   D__ELEM( SS_TYPE_D_SPEED_MOD,          speed_mod,           gettext_noop("Speed") ),
   D__ELEM( SS_TYPE_D_TURN_MOD,           turn_mod,            gettext_noop("Turn") ),
   D__ELEM( SS_TYPE_D_THRUST_MOD,         thrust_mod,          gettext_noop("Thrust") ),
   D__ELEM( SS_TYPE_D_CARGO_MOD,          cargo_mod,           gettext_noop("Cargo Space") ),
   D__ELEM( SS_TYPE_D_ARMOUR_MOD,         armour_mod,          gettext_noop("Armour Strength") ),
   D__ELEM( SS_TYPE_D_ARMOUR_REGEN_MOD,   armour_regen_mod,    gettext_noop("Armour Regeneration") ),
   D__ELEM( SS_TYPE_D_SHIELD_MOD,         shield_mod,          gettext_noop("Shield Strength") ),
   D__ELEM( SS_TYPE_D_SHIELD_REGEN_MOD,   shield_regen_mod,    gettext_noop("Shield Regeneration") ),
   D__ELEM( SS_TYPE_D_ENERGY_MOD,         energy_mod,          gettext_noop("Energy Capacity") ),
   D__ELEM( SS_TYPE_D_ENERGY_REGEN_MOD,   energy_regen_mod,    gettext_noop("Energy Regeneration") ),
   D__ELEM( SS_TYPE_D_CPU_MOD,            cpu_mod,             gettext_noop("CPU Capacity") ),

   DI_ELEM( SS_TYPE_D_JUMP_DELAY,         jump_delay,          gettext_noop("Jump Time") ),
   DI_ELEM( SS_TYPE_D_CARGO_INERTIA,      cargo_inertia,       gettext_noop("Cargo Inertia") ),

   D__ELEM( SS_TYPE_D_EW_HIDE,            ew_hide,             gettext_noop("Cloaking") ),
   D__ELEM( SS_TYPE_D_EW_DETECT,          ew_detect,           gettext_noop("Detection") ),
   D__ELEM( SS_TYPE_D_EW_JUMPDETECT,      ew_jump_detect,      gettext_noop("Jump Detection") ),

   D__ELEM( SS_TYPE_D_LAUNCH_RATE,        launch_rate,         gettext_noop("Fire Rate (Launcher)") ),
   D__ELEM( SS_TYPE_D_LAUNCH_RANGE,       launch_range,        gettext_noop("Launch Range") ),
   D__ELEM( SS_TYPE_D_LAUNCH_DAMAGE,      launch_damage,       gettext_noop("Damage (Launcher)") ),
   D__ELEM( SS_TYPE_D_AMMO_CAPACITY,      ammo_capacity,       gettext_noop("Ammo Capacity") ),
   D__ELEM( SS_TYPE_D_LAUNCH_LOCKON,      launch_lockon,       gettext_noop("Launch Lock-on") ),

   DI_ELEM( SS_TYPE_D_FORWARD_HEAT,       fwd_heat,            gettext_noop("Heat (Cannon)") ),
   D__ELEM( SS_TYPE_D_FORWARD_DAMAGE,     fwd_damage,          gettext_noop("Damage (Cannon)") ),
   D__ELEM( SS_TYPE_D_FORWARD_FIRERATE,   fwd_firerate,        gettext_noop("Fire Rate (Cannon)") ),
   DI_ELEM( SS_TYPE_D_FORWARD_ENERGY,     fwd_energy,          gettext_noop("Energy Usage (Cannon)") ),

   DI_ELEM( SS_TYPE_D_TURRET_HEAT,        tur_heat,            gettext_noop("Heat (Turret)") ),
   D__ELEM( SS_TYPE_D_TURRET_DAMAGE,      tur_damage,          gettext_noop("Damage (Turret)") ),
   D__ELEM( SS_TYPE_D_TURRET_TRACKING,    tur_tracking,        gettext_noop("Tracking (Turret)") ),
   D__ELEM( SS_TYPE_D_TURRET_FIRERATE,    tur_firerate,        gettext_noop("Fire Rate (Turret)") ),
   DI_ELEM( SS_TYPE_D_TURRET_ENERGY,      tur_energy,          gettext_noop("Energy Usage (Turret)") ),

   D__ELEM( SS_TYPE_D_NEBULA_ABSORB_SHIELD,  nebu_absorb_shield,   gettext_noop("Nebula Resistance (Shield)") ),
   D__ELEM( SS_TYPE_D_NEBULA_ABSORB_ARMOUR,  nebu_absorb_armour,   gettext_noop("Nebula Resistance (Armour)") ),

   D__ELEM( SS_TYPE_D_HEAT_DISSIPATION,   heat_dissipation,    gettext_noop("Heat Dissipation") ),
   D__ELEM( SS_TYPE_D_STRESS_DISSIPATION, stress_dissipation,  gettext_noop("Stress Dissipation") ),
   D__ELEM( SS_TYPE_D_CREW,               crew_mod,            gettext_noop("Crew") ),
   DI_ELEM( SS_TYPE_D_MASS,               mass_mod,            gettext_noop("Ship Mass") ),
   D__ELEM( SS_TYPE_D_ENGINE_LIMIT_REL,   engine_limit_rel,    gettext_noop("Engine Mass Limit") ),

   A__ELEM( SS_TYPE_A_ENERGY_FLAT,        energy_flat,         gettext_noop("Energy Capacity") ),
   AI_ELEM( SS_TYPE_A_ENERGY_REGEN_FLAT,  energy_usage,        gettext_noop("Energy Usage") ),
   A__ELEM( SS_TYPE_A_SHIELD_FLAT,        shield_flat,         gettext_noop("Shield Capacity") ),
   AI_ELEM( SS_TYPE_A_SHIELD_REGEN_FLAT,  shield_usage,        gettext_noop("Shield Usage") ),
   A__ELEM( SS_TYPE_A_ARMOUR_FLAT,        armour_flat,         gettext_noop("Armour") ),
   AI_ELEM( SS_TYPE_A_ARMOUR_REGEN_FLAT,  armour_damage,       gettext_noop("Armour Damage") ),
   A__ELEM( SS_TYPE_A_CPU_MAX,            cpu_max,             gettext_noop("CPU Capacity") ),

   A__ELEM( SS_TYPE_A_ENGINE_LIMIT,       engine_limit,        gettext_noop("Engine Mass Limit") ),

   I__ELEM( SS_TYPE_I_HIDDEN_JUMP_DETECT, misc_hidden_jump_detect, gettext_noop("Hidden Jump Detection") ),

   B__ELEM( SS_TYPE_B_INSTANT_JUMP,       misc_instant_jump,   gettext_noop("Instant Jump") ),
   B__ELEM( SS_TYPE_B_REVERSE_THRUST,     misc_reverse_thrust, gettext_noop("Reverse Thrusters") ),
   B__ELEM( SS_TYPE_B_ASTEROID_SCAN,      misc_asteroid_scan,  gettext_noop("Asteroid Scanner") ),

   /* Sentinel. */
   N__ELEM( SS_TYPE_SENTINEL )
};


/*
 * Prototypes.
 */
static const char* ss_printD_colour( double d, const ShipStatsLookup *sl );
static const char* ss_printI_colour( int i, const ShipStatsLookup *sl );
static int ss_printD( char *buf, int len, int newline, double d, const ShipStatsLookup *sl );
static int ss_printA( char *buf, int len, int newline, double d, const ShipStatsLookup *sl );
static int ss_printI( char *buf, int len, int newline, int i, const ShipStatsLookup *sl );
static int ss_printB( char *buf, int len, int newline, int b, const ShipStatsLookup *sl );


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
   switch (sl->data) {
      case SS_DATA_TYPE_DOUBLE:
         ll->d.d     = xml_getFloat(node) / 100.;
         break;

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         ll->d.d     = xml_getFloat(node);
         break;

      case SS_DATA_TYPE_BOOLEAN:
         ll->d.i     = !!xml_getInt(node);
         break;

      case SS_DATA_TYPE_INTEGER:
         ll->d.i     = xml_getInt(node);
         break;
   }

   return ll;
}


/**
 * @brief Checks for validity.
 */
int ss_check (void)
{
   ShipStatsType i;

   for (i=0; i<=SS_TYPE_SENTINEL; i++) {
      if (ss_lookup[i].type != i) {
         WARN(_("ss_lookup: %s should have id %d but has %d"),
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
   char *fieldptr;
   double *dbl;
   const ShipStatsLookup *sl;

   /* Clear the memory. */
   memset( stats, 0, sizeof(ShipStats) );

   ptr = (char*) stats;
   for (i=0; i<SS_TYPE_SENTINEL; i++) {
      sl = &ss_lookup[ i ];

      /* Only want valid names. */
      if (sl->name == NULL)
         continue;

      /* Handle doubles. */
      switch (sl->data) {
         case SS_DATA_TYPE_DOUBLE:
            fieldptr = &ptr[ sl->offset ];
            memcpy(&dbl, &fieldptr, sizeof(double*));
            *dbl  = 1.0;
            break;

         /* No need to set, memset does the work. */
         case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         case SS_DATA_TYPE_INTEGER:
         case SS_DATA_TYPE_BOOLEAN:
            break;
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
   char *fieldptr;
   double *dbl;
   int *i;
   const ShipStatsLookup *sl = &ss_lookup[ list->type ];

   ptr = (char*) stats;
   switch (sl->data) {
      case SS_DATA_TYPE_DOUBLE:
      case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         fieldptr = &ptr[ sl->offset ];
         memcpy(&dbl, &fieldptr, sizeof(double*));
         *dbl += list->d.d;
         if ((sl->data==SS_DATA_TYPE_DOUBLE) && (*dbl < 0.)) /* Don't let the values go negative. */
            *dbl = 0.;

         /* We'll increment amount. */
         if (amount != NULL) {
            if ((sl->inverted && (list->d.d < 0.)) ||
                  (!sl->inverted && (list->d.d > 0.))) {
               memcpy(&ptr, &amount, sizeof(char*));
               fieldptr = &ptr[ sl->offset ];
               memcpy(&dbl, &fieldptr, sizeof(double*));
               (*dbl)  += 1.0;
            }
         }
         break;

      case SS_DATA_TYPE_INTEGER:
         fieldptr = &ptr[ sl->offset ];
         memcpy(&i, &fieldptr, sizeof(int*));
         *i   += list->d.i;
         if (amount != NULL) {
            if ((sl->inverted && (list->d.i < 0)) ||
                  (!sl->inverted && (list->d.i > 0))) {
               memcpy(&ptr, &amount, sizeof(char*));
               fieldptr = &ptr[ sl->offset ];
               memcpy(&i, &fieldptr, sizeof(int*));
               (*i)    += 1;
            }
         }
         break;

      case SS_DATA_TYPE_BOOLEAN:
         fieldptr = &ptr[ sl->offset ];
         memcpy(&i, &fieldptr, sizeof(int*));
         *i    = 1; /* Can only set to true. */
         break;
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
 * @brief Gets the offset from type.
 *
 *    @param type Type to get offset of.
 *    @return Offset of the type.
 */
size_t ss_offsetFromType( ShipStatsType type )
{
   return ss_lookup[ type ].offset;
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
   for (i=0; i<SS_TYPE_SENTINEL; i++)
      if ((ss_lookup[i].name != NULL) && (strcmp(name,ss_lookup[i].name)==0))
         return ss_lookup[i].type;

   WARN(_("ss_typeFromName: No ship stat matching '%s'"), name);
   return SS_TYPE_NIL;
}


/**
 * @brief Some colour coding for ship stats doubles.
 */
static const char* ss_printD_colour( double d, const ShipStatsLookup *sl )
{
   if (sl->inverted) {
      if (d < 0.)
         return "g";
      return "r";
   }

   if (d > 0.)
      return "g";
   return "r";
}
/**
 * @brief Some colour coding for ship stats integers.
 */
static const char* ss_printI_colour( int i, const ShipStatsLookup *sl )
{
   if (sl->inverted) {
      if (i < 0)
         return "g";
      return "r";
   }

   if (i > 0)
      return "g";
   return "r";
}


/**
 * @brief Helper to print doubles.
 */
static int ss_printD( char *buf, int len, int newline, double d, const ShipStatsLookup *sl )
{
   if (FABS(d) < 1e-10)
      return 0;
   return nsnprintf( buf, len, "%s\a%s%+.0f%% %s\a0",
         (newline) ? "\n" : "",
         ss_printD_colour( d, sl ),
         d*100., _(sl->display) );
}


/**
 * @brief Helper to print absolute doubles.
 */
static int ss_printA( char *buf, int len, int newline, double d, const ShipStatsLookup *sl )
{
   if (FABS(d) < 1e-10)
      return 0;
   return nsnprintf( buf, len, "%s\a%s%+.0f %s\a0",
         (newline) ? "\n" : "",
         ss_printD_colour( d, sl ),
         d, _(sl->display) );
}


/**
 * @brief Helper to print integers.
 */
static int ss_printI( char *buf, int len, int newline, int i, const ShipStatsLookup *sl )
{
   if (i == 0)
      return 0;
   return nsnprintf( buf, len, "%s\a%s%+d %s\a0",
         (newline) ? "\n" : "",
         ss_printI_colour( i, sl ),
         i, _(sl->display) );
}


/**
 * @brief Helper to print booleans.
 */
static int ss_printB( char *buf, int len, int newline, int b, const ShipStatsLookup *sl )
{
   if (!b)
      return 0;
   return nsnprintf( buf, len, "%s\a%s%s\a0",
         (newline) ? "\n" : "",
         ss_printI_colour( b, sl ),
         _(sl->display) );
}




/**
 * @brief Writes the ship statistics description.
 *
 *    @param ll Ship stats to use.
 *    @param buf Buffer to write to.
 *    @param len Space left in the buffer.
 *    @param newline Add a newline at start.
 *    @return Number of characters written.
 */
int ss_statsListDesc( const ShipStatList *ll, char *buf, int len, int newline )
{
   int i, left, newl;
   const ShipStatsLookup *sl;
   i     = 0;
   newl  = newline;
   for ( ; ll != NULL; ll=ll->next) {
      left  = len-i;
      if (left < 0)
         break;
      sl    = &ss_lookup[ ll->type ];

      switch (sl->data) {
         case SS_DATA_TYPE_DOUBLE:
            i += ss_printD( &buf[i], left, newl, ll->d.d, sl );
            break;

         case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
            i += ss_printA( &buf[i], left, newl, ll->d.d, sl );
            break;

         case SS_DATA_TYPE_INTEGER:
            i += ss_printI( &buf[i], left, newl, ll->d.i, sl );
            break;

         case SS_DATA_TYPE_BOOLEAN:
            i += ss_printB( &buf[i], left, newl, ll->d.i, sl );
            break;
      }

      newl = 1;
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
 *    @return Number of characters written.
 */
int ss_statsDesc( const ShipStats *s, char *buf, int len, int newline )
{
   int i, l, left;
   char *ptr;
   char *fieldptr;
   double *dbl;
   int *num;
   const ShipStatsLookup *sl;

   l   = 0;
   ptr = (char*) s;
   for (i=0; i<SS_TYPE_SENTINEL; i++) {
      sl = &ss_lookup[ i ];

      /* Only want valid names. */
      if (sl->name == NULL)
         continue;

      /* Calculate offset left. */
      left = len-l;
      if (left < 0)
         break;

      switch (sl->data) {
         case SS_DATA_TYPE_DOUBLE:
            fieldptr = &ptr[ sl->offset ];
            memcpy(&dbl, &fieldptr, sizeof(double*));
            l    += ss_printD( &buf[l], left, (newline||(l!=0)), ((*dbl)-1.), sl );
            break;

         case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
            fieldptr = &ptr[ sl->offset ];
            memcpy(&dbl, &fieldptr, sizeof(double*));
            l    += ss_printA( &buf[l], left, (newline||(l!=0)), (*dbl), sl );
            break;

         case SS_DATA_TYPE_INTEGER:
            fieldptr = &ptr[ sl->offset ];
            memcpy(&num, &fieldptr, sizeof(int*));
            l    += ss_printI( &buf[l], left, (newline||(l!=0)), (*num), sl );
            break;

         case SS_DATA_TYPE_BOOLEAN:
            fieldptr = &ptr[ sl->offset ];
            memcpy(&num, &fieldptr, sizeof(int*));
            l    += ss_printB( &buf[l], left, (newline||(l!=0)), (*num), sl );
            break;
      }
   }

   return l;
}


/**
 * @brief Generates CSV output for the given ship stats.
 *
 *    @param s Ship stats to use.
 *    @param[out] buf Buffer to write to.
 *    @param len Space left in the buffer.
 *    @return Number of characters written.
 */
int ss_csv( const ShipStats *s, char *buf, int len )
{
   int i, l, left, num;
   double dbl;
   char *ptr;
   const ShipStatsLookup *sl;

   l = 0;
   ptr = (char*) s;
   for (i=0; i<SS_TYPE_SENTINEL; i++) {
      sl = &ss_lookup[ i ];

      /* Only want valid names. */
      if (sl->name == NULL)
         continue;

      /* Calculate offset left. */
      left = len - l;
      if (left < 0) {
         WARN(_("Buffer out of space, CSV output truncated"));
         break;
      }

      if (s == NULL) {
         l += nsnprintf( &buf[l], left, "%s,", sl->name );
         continue;
      }

      switch (sl->data) {
         case SS_DATA_TYPE_DOUBLE:
            memcpy(&dbl, &ptr[ sl->offset ], sizeof(double));
            l  += nsnprintf( &buf[l], left, "%f,", (dbl - 1.) * 100. );
            break;

         case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
            memcpy(&dbl, &ptr[ sl->offset ], sizeof(double));
            l  += nsnprintf( &buf[l], left, "%f,", dbl );
            break;

         case SS_DATA_TYPE_INTEGER:
         case SS_DATA_TYPE_BOOLEAN:
            memcpy(&num, &ptr[ sl->offset ], sizeof(int));
            l  += nsnprintf( &buf[l], left, "%d,", num );
            break;
      }
   }

   buf[l-1] = '\n'; /* Terminate with a newline. */
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



