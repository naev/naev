/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file shipstats.c
 *
 * @brief Handles the ship statistics.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "shipstats.h"

#include "log.h"
#include "nstring.h"

/**
 * @brief The data type.
 */
typedef enum StatDataType_ {
   SS_DATA_TYPE_DOUBLE,          /**< Relative [0:inf] value. */
   SS_DATA_TYPE_DOUBLE_ABSOLUTE, /**< Absolute double value. */
   SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT, /**< Absolute percent double value. */
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
   const char *unit;    /**< Unit of the stat. */
   StatDataType data;   /**< Type of data for the stat. */
   int inverted;        /**< Indicates whether the good value is inverted, by
                             default positive is good, with this set negative
                             is good. */

   /* Self calculated. */
   size_t offset;       /**< Stores the byte offset in the structure. */
} ShipStatsLookup;

/* Flexible do everything macro. */
#define ELEM( t, n, dsp, u, d , i) \
   { .type=t, .name=#n, .display=dsp, .unit=u, .data=d, .inverted=i, .offset=offsetof( ShipStats, n ) }
/* Standard types. */
#define D__ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, N_("%"), SS_DATA_TYPE_DOUBLE, 0 )
#define DI_ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, N_("%"), SS_DATA_TYPE_DOUBLE, 1 )

#define A__ELEM( t, n, dsp, u ) \
   ELEM( t, n, dsp, u, SS_DATA_TYPE_DOUBLE_ABSOLUTE, 0 )
#define AI_ELEM( t, n, dsp, u) \
   ELEM( t, n, dsp, u, SS_DATA_TYPE_DOUBLE_ABSOLUTE, 1 )

#define P__ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, N_("%"), SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT, 0 )
#define PI_ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, N_("%"), SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT, 1 )

#define I__ELEM( t, n, dsp, u ) \
   ELEM( t, n, dsp, u, SS_DATA_TYPE_INTEGER, 0 )
#define II_ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, u, SS_DATA_TYPE_INTEGER, 1 )

#define B__ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, NULL, SS_DATA_TYPE_BOOLEAN, 0 )
#define BI_ELEM( t, n, dsp ) \
   ELEM( t, n, dsp, NULL, SS_DATA_TYPE_BOOLEAN, 1 )

/** Nil element. */
#define N__ELEM( t ) \
   { .type=t, .name=NULL, .display=NULL, .unit=NULL, .inverted=0, .offset=0 }

/**
 * The ultimate look up table for ship stats, everything goes through this.
 */
static const ShipStatsLookup ss_lookup[] = {
   /* Null element. */
   N__ELEM( SS_TYPE_NIL ),

   D__ELEM( SS_TYPE_D_SPEED_MOD,          speed_mod,           N_("Speed") ),
   D__ELEM( SS_TYPE_D_TURN_MOD,           turn_mod,            N_("Turn") ),
   D__ELEM( SS_TYPE_D_ACCEL_MOD,          accel_mod,           N_("Accel") ),
   D__ELEM( SS_TYPE_D_CARGO_MOD,          cargo_mod,           N_("Cargo Space") ),
   D__ELEM( SS_TYPE_D_FUEL_MOD,           fuel_mod,            N_("Fuel Cpacity") ),
   D__ELEM( SS_TYPE_D_FUEL_USAGE_MOD,     fuel_usage_mod,      N_("Fuel Usage") ),
   D__ELEM( SS_TYPE_D_ARMOUR_MOD,         armour_mod,          N_("Armour Strength") ),
   D__ELEM( SS_TYPE_D_ARMOUR_REGEN_MOD,   armour_regen_mod,    N_("Armour Regeneration") ),
   D__ELEM( SS_TYPE_D_SHIELD_MOD,         shield_mod,          N_("Shield Strength") ),
   D__ELEM( SS_TYPE_D_SHIELD_REGEN_MOD,   shield_regen_mod,    N_("Shield Regeneration") ),
   D__ELEM( SS_TYPE_D_ENERGY_MOD,         energy_mod,          N_("Energy Capacity") ),
   D__ELEM( SS_TYPE_D_ENERGY_REGEN_MOD,   energy_regen_mod,    N_("Energy Regeneration") ),
   D__ELEM( SS_TYPE_D_CPU_MOD,            cpu_mod,             N_("CPU Capacity") ),
   DI_ELEM( SS_TYPE_D_COOLDOWN_MOD,       cooldown_mod,        N_("Ability Cooldown") ),

   DI_ELEM( SS_TYPE_D_JUMP_DELAY,         jump_delay,          N_("Jump Time") ),
   DI_ELEM( SS_TYPE_D_LAND_DELAY,         land_delay,          N_("Landing Time") ),
   DI_ELEM( SS_TYPE_D_CARGO_INERTIA,      cargo_inertia,       N_("Cargo Inertia") ),

   DI_ELEM( SS_TYPE_D_EW_HIDE,            ew_hide,             N_("Detected Range") ),
   DI_ELEM( SS_TYPE_D_EW_SIGNATURE,       ew_signature,        N_("Signature Range") ),
   DI_ELEM( SS_TYPE_D_EW_STEALTH,         ew_stealth,          N_("Stealth Range") ),
   D__ELEM( SS_TYPE_D_EW_DETECT,          ew_detect,           N_("Detection") ),
   D__ELEM( SS_TYPE_D_EW_TRACK,           ew_track,            N_("Tracking") ),
   D__ELEM( SS_TYPE_D_EW_JUMPDETECT,      ew_jump_detect,      N_("Jump Detection") ),
   DI_ELEM( SS_TYPE_D_EW_STEALTH_TIMER,   ew_stealth_timer,    N_("Stealth Discovered Speed") ),
   DI_ELEM( SS_TYPE_D_EW_SCANNED_TIME,    ew_scanned_time,     N_("Scanned Speed") ),

   D__ELEM( SS_TYPE_D_LAUNCH_RATE,        launch_rate,         N_("Fire Rate (Launcher)") ),
   D__ELEM( SS_TYPE_D_LAUNCH_RANGE,       launch_range,        N_("Launch Range") ),
   D__ELEM( SS_TYPE_D_LAUNCH_DAMAGE,      launch_damage,       N_("Damage (Launcher)") ),
   D__ELEM( SS_TYPE_D_AMMO_CAPACITY,      ammo_capacity,       N_("Ammo Capacity") ),
   DI_ELEM( SS_TYPE_D_LAUNCH_LOCKON,      launch_lockon,       N_("Launch Lock-on") ),
   DI_ELEM( SS_TYPE_D_LAUNCH_CALIBRATION, launch_calibration,  N_("Launch Calibration") ),
   D__ELEM( SS_TYPE_D_LAUNCH_RELOAD,      launch_reload,       N_("Ammo Reload Rate") ),

   D__ELEM( SS_TYPE_D_FBAY_DAMAGE,        fbay_damage,         N_("Fighter Damage") ),
   D__ELEM( SS_TYPE_D_FBAY_HEALTH,        fbay_health,         N_("Fighter Health") ),
   D__ELEM( SS_TYPE_D_FBAY_MOVEMENT,      fbay_movement,       N_("Fighter Movement") ),
   D__ELEM( SS_TYPE_D_FBAY_CAPACITY,      fbay_capacity,       N_("Fighter Bay Capacity") ),
   D__ELEM( SS_TYPE_D_FBAY_RATE,          fbay_rate,           N_("Fighter Bay Launch Rate") ),
   D__ELEM( SS_TYPE_D_FBAY_RELOAD,        fbay_reload,         N_("Fighter Reload Rate") ),

   DI_ELEM( SS_TYPE_D_FORWARD_HEAT,       fwd_heat,            N_("Heat (Cannon)") ),
   D__ELEM( SS_TYPE_D_FORWARD_DAMAGE,     fwd_damage,          N_("Damage (Cannon)") ),
   D__ELEM( SS_TYPE_D_FORWARD_FIRERATE,   fwd_firerate,        N_("Fire Rate (Cannon)") ),
   DI_ELEM( SS_TYPE_D_FORWARD_ENERGY,     fwd_energy,          N_("Energy Usage (Cannon)") ),
   D__ELEM( SS_TYPE_D_FORWARD_DAMAGE_AS_DISABLE,fwd_dam_as_dis,N_("Damage as Disable (Cannon)") ),

   DI_ELEM( SS_TYPE_D_TURRET_HEAT,        tur_heat,            N_("Heat (Turret)") ),
   D__ELEM( SS_TYPE_D_TURRET_DAMAGE,      tur_damage,          N_("Damage (Turret)") ),
   D__ELEM( SS_TYPE_D_TURRET_TRACKING,    tur_tracking,        N_("Tracking (Turret)") ),
   D__ELEM( SS_TYPE_D_TURRET_FIRERATE,    tur_firerate,        N_("Fire Rate (Turret)") ),
   DI_ELEM( SS_TYPE_D_TURRET_ENERGY,      tur_energy,          N_("Energy Usage (Turret)") ),
   D__ELEM( SS_TYPE_D_TURRET_DAMAGE_AS_DISABLE, tur_dam_as_dis,N_("Damage as Disable (Turret)") ),

   D__ELEM( SS_TYPE_D_HEAT_DISSIPATION,   heat_dissipation,    N_("Heat Dissipation") ),
   D__ELEM( SS_TYPE_D_STRESS_DISSIPATION, stress_dissipation,  N_("Stress Dissipation") ),
   D__ELEM( SS_TYPE_D_CREW,               crew_mod,            N_("Crew") ),
   DI_ELEM( SS_TYPE_D_MASS,               mass_mod,            N_("Ship Mass") ),
   D__ELEM( SS_TYPE_D_ENGINE_LIMIT_REL,   engine_limit_rel,    N_("Engine Mass Limit") ),
   D__ELEM( SS_TYPE_D_LOOT_MOD,           loot_mod,            N_("Boarding Bonus") ),
   DI_ELEM( SS_TYPE_D_TIME_MOD,           time_mod,            N_("Time Constant") ),
   D__ELEM( SS_TYPE_D_TIME_SPEEDUP,       time_speedup,        N_("Action Speed") ),
   DI_ELEM( SS_TYPE_D_COOLDOWN_TIME,      cooldown_time,       N_("Ship Cooldown Time") ),
   D__ELEM( SS_TYPE_D_JUMP_DISTANCE,      jump_distance,       N_("Jump Distance") ),
   DI_ELEM( SS_TYPE_D_JUMP_WARMUP,        jump_warmup,         N_("Jump Warmup") ),
   D__ELEM( SS_TYPE_D_MINING_BONUS,       mining_bonus,        N_("Mining Bonus") ),

   A__ELEM( SS_TYPE_A_ACCEL,              accel,               N_("Accel"),              _UNIT_ACCEL ),
   A__ELEM( SS_TYPE_A_TURN,               turn,                N_("Turn Rate"),           _UNIT_ROTATION ),
   A__ELEM( SS_TYPE_A_SPEED,              speed,               N_("Maximum Speed"),       _UNIT_SPEED ),
   A__ELEM( SS_TYPE_A_ENERGY,             energy,              N_("Energy Capacity"),     _UNIT_ENERGY ),
   A__ELEM( SS_TYPE_A_ENERGY_REGEN,       energy_regen,        N_("Energy Regeneration"), _UNIT_POWER ),
   AI_ELEM( SS_TYPE_A_ENERGY_REGEN_MALUS, energy_regen_malus,  N_("Energy Usage"),        _UNIT_POWER ),
   AI_ELEM( SS_TYPE_A_ENERGY_LOSS,        energy_loss,         N_("Energy Usage"),        _UNIT_POWER ),
   A__ELEM( SS_TYPE_A_SHIELD,             shield,              N_("Shield Capacity"),     _UNIT_ENERGY ),
   A__ELEM( SS_TYPE_A_SHIELD_REGEN,       shield_regen,        N_("Shield Regeneration"), _UNIT_POWER ),
   AI_ELEM( SS_TYPE_A_SHIELD_REGEN_MALUS, shield_regen_malus,  N_("Shield Usage"),        _UNIT_POWER ),
   A__ELEM( SS_TYPE_A_ARMOUR,             armour,              N_("Armour"),              _UNIT_ENERGY ),
   A__ELEM( SS_TYPE_A_ARMOUR_REGEN,       armour_regen,        N_("Armour Regeneration"), _UNIT_POWER ),
   AI_ELEM( SS_TYPE_A_ARMOUR_REGEN_MALUS, armour_regen_malus,  N_("Armour Damage"),       _UNIT_POWER ),
   A__ELEM( SS_TYPE_A_DAMAGE,             damage,              N_("Damage"),              _UNIT_POWER ),
   A__ELEM( SS_TYPE_A_DISABLE,            disable,             N_("Disable"),             _UNIT_POWER ),

   A__ELEM( SS_TYPE_A_CPU_MAX,            cpu_max,             N_("CPU Capacity"),        _UNIT_CPU ),
   A__ELEM( SS_TYPE_A_ENGINE_LIMIT,       engine_limit,        N_("Engine Mass Limit"),   _UNIT_MASS ),
   A__ELEM( SS_TYPE_A_FUEL_REGEN,         fuel_regen,          N_("Fuel Regeneration"),   _UNIT_PER_TIME ),
   A__ELEM( SS_TYPE_A_ASTEROID_SCAN,      asteroid_scan,       N_("Asteroid Scanner Range"), _UNIT_DISTANCE ),
   A__ELEM( SS_TYPE_A_NEBULA_VISIBILITY,  nebu_visibility,     N_("Nebula Visibility"),   _UNIT_DISTANCE ),

   P__ELEM( SS_TYPE_P_ABSORB,             absorb,              N_("Damage Absorption") ),

   P__ELEM( SS_TYPE_P_NEBULA_ABSORB,      nebu_absorb,         N_("Nebula Resistance") ),
   P__ELEM( SS_TYPE_P_JAMMING_CHANCE,     jam_chance,          N_("Missile jamming chance") ),

   I__ELEM( SS_TYPE_I_FUEL,               fuel,                N_("Fuel"),                _UNIT_UNIT ),
   I__ELEM( SS_TYPE_I_CARGO,              cargo,               N_("Cargo"),               _UNIT_MASS ),
   I__ELEM( SS_TYPE_I_CREW,               crew,                N_("Crew"),                _UNIT_UNIT ),

   B__ELEM( SS_TYPE_B_HIDDEN_JUMP_DETECT, misc_hidden_jump_detect, N_("Hidden Jump Detection") ),
   B__ELEM( SS_TYPE_B_INSTANT_JUMP,       misc_instant_jump,   N_("Instant Jump") ),
   B__ELEM( SS_TYPE_B_REVERSE_THRUST,     misc_reverse_thrust, N_("Reverse Thrusters") ),

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
static double ss_statsGetInternal( const ShipStats *s, ShipStatsType type );
static int ss_statsGetLuaInternal( lua_State *L, const ShipStats *s, ShipStatsType type, int internal );
static void ss_adjustDoubleStat( double* statptr, double adjustment, int inverted );

ShipStatList* ss_statsSetList( ShipStatList *head, ShipStatsType type, double value, int overwrite, int raw )
{
   const ShipStatsLookup *sl;
   ShipStatList *ll = NULL;
   ShipStatList *newhead = head;
   int init = overwrite;

   if (type == SS_TYPE_NIL)
      return NULL;
   sl = &ss_lookup[ type ];

   /* See if there is an element to modify or overwrite. */
   if (head != NULL) {
      for (ShipStatList *l=head; l!=NULL; l=l->next) {
         if (type==l->type) {
            ll = l;
            break;
         }
      }
   }

   /* Allocate. */
   if (ll==NULL) {
      newhead = ll = malloc( sizeof(ShipStatList) );
      ll->next    = head;
      ll->target  = 0;
      ll->type    = type;
      init        = 1; /* Force initialization. */
   }

   if (init) {
      switch (sl->data) {
         case SS_DATA_TYPE_DOUBLE:
            ll->d.d = 1.;
            break;
         case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
         case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
            ll->d.d = 0.;
            break;
         case SS_DATA_TYPE_BOOLEAN:
         case SS_DATA_TYPE_INTEGER:
            ll->d.i = 0;
            break;
      }
   }

   /* Set the data. */
   switch (sl->data) {
      case SS_DATA_TYPE_DOUBLE:
         if (raw)
            ll->d.d *= value;
         else
            ll->d.d *= value / 100.;
         break;

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
         if (raw)
            ll->d.d += value;
         else
            ll->d.d += value / 100.;
         break;

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         ll->d.d += value;
         break;

      case SS_DATA_TYPE_BOOLEAN:
         ll->d.i |= (fabs(value) > 1e-5);
         break;

      case SS_DATA_TYPE_INTEGER:
         ll->d.i += round(value);
         break;
   }

   return newhead;
}

/**
 * @brief Creates a shipstat list element from an xml node.
 *
 *    @param node Node to create element from.
 *    @return List element created from node.
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
      case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
         ll->d.d  = xml_getFloat(node) / 100.;
         break;

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         ll->d.d  = xml_getFloat(node);
         break;

      case SS_DATA_TYPE_BOOLEAN:
         ll->d.i  = !!xml_getInt(node);
         break;

      case SS_DATA_TYPE_INTEGER:
         ll->d.i  = xml_getInt(node);
         break;
   }

   /* Sort them. */
   ss_sort( &ll );

   return ll;
}

/**
 * @brief Creatse a shipstat list element from an xml node.
 *
 *    @param writer Writer to use to write the XML data.
 *    @param ll ShipStats to save.
 *    @return 0 on success.
 */
int ss_listToXML( xmlTextWriterPtr writer, const ShipStatList *ll )
{
   for (const ShipStatList *l=ll; l!=NULL; l=l->next) {
      const ShipStatsLookup *sl = &ss_lookup[ l->type ];
      switch (sl->data) {
         case SS_DATA_TYPE_DOUBLE:
         case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
            xmlw_elem( writer, sl->name, "%f", l->d.d * 100 );
            break;

         case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
            xmlw_elem( writer, sl->name, "%f", l->d.d );
            break;

         case SS_DATA_TYPE_BOOLEAN:
         case SS_DATA_TYPE_INTEGER:
            xmlw_elem( writer, sl->name, "%d", l->d.i );
            break;
      }
   }
   return 0;
}

static int shipstat_sort( const void *a, const void *b )
{
   const ShipStatList **la = (const ShipStatList**) a;
   const ShipStatList **lb = (const ShipStatList**) b;
   const ShipStatsLookup *sla = &ss_lookup[ (*la)->type ];
   const ShipStatsLookup *slb = &ss_lookup[ (*lb)->type ];
   return strcmp( sla->name, slb->name );
}

/**
 * @brief Sorts the ship stats, useful if doing saving stuff.
 *
 *    @param ll Ship stat list to sort.
 *    @return 0 on success.
 */
int ss_sort( ShipStatList **ll )
{
   int n, i;
   ShipStatList **arr;

   /* Nothing to do. */
   if (*ll==NULL)
      return 0;

   n = 0;
   for (ShipStatList *l=*ll; l!=NULL; l=l->next)
      n++;

   arr = malloc( sizeof(ShipStatList*) * n );
   i = 0;
   for (ShipStatList *l=*ll; l!=NULL; l=l->next) {
      arr[i] = l;
      i++;
   }
   qsort( arr, n, sizeof(ShipStatList*), shipstat_sort );

   *ll = arr[0];
   for (i=1; i<n; i++)
      arr[i-1]->next = arr[i];
   arr[n-1]->next = NULL;
   free( arr );
   return 0;
}


/**
 * @brief Checks for validity.
 */
int ss_check (void)
{
   for (ShipStatsType i=0; i<=SS_TYPE_SENTINEL; i++) {
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
   char *ptr;

   /* Clear the memory. */
   memset( stats, 0, sizeof(ShipStats) );

   ptr = (char*) stats;
   for (int i=0; i<SS_TYPE_SENTINEL; i++) {
      const ShipStatsLookup *sl = &ss_lookup[ i ];

      /* Only want valid names. */
      if (sl->name == NULL)
         continue;

      /* Handle doubles. */
      switch (sl->data) {
         case SS_DATA_TYPE_DOUBLE:
            {
               double *dbl;
               const char *fieldptr = &ptr[ sl->offset ];
               memcpy(&dbl, &fieldptr, sizeof(double*));
               *dbl  = 1.0;
               break;
            }

            /* No need to set, memset does the work. */
         case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
         case SS_DATA_TYPE_INTEGER:
         case SS_DATA_TYPE_BOOLEAN:
            break;
      }
   }

   return 0;
}

/**
 * @brief Merges two different ship stats.
 *
 *    @param dest Destination ship stats.
 *    @param src Source to be merged with destination.
 */
int ss_statsMerge( ShipStats *dest, const ShipStats *src )
{
   int *destint;
   const int *srcint;
   double *destdbl;
   const double *srcdbl;
   char *destptr;
   const char *srcptr;

   destptr = (char*) dest;
   srcptr = (const char*) src;
   for (int i=0; i<SS_TYPE_SENTINEL; i++) {
      const ShipStatsLookup *sl = &ss_lookup[ i ];

      /* Only want valid names. */
      if (sl->name == NULL)
         continue;

      switch (sl->data) {
         case SS_DATA_TYPE_DOUBLE:
            destdbl = (double*) (void*)&destptr[ sl->offset ];
            srcdbl = (const double*) (const void*)&srcptr[ sl->offset ];
            *destdbl = (*destdbl) * (*srcdbl);
            break;

         case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
            destdbl = (double*) (void*)&destptr[ sl->offset ];
            srcdbl = (const double*) (const void*)&srcptr[ sl->offset ];
            *destdbl = (*destdbl) + (*srcdbl);
            break;

         case SS_DATA_TYPE_INTEGER:
            destint = (int*) &destptr[ sl->offset ];
            srcint = (const int*) &srcptr[ sl->offset ];
            *destint = (*destint) + (*srcint);
            break;

         case SS_DATA_TYPE_BOOLEAN:
            destint = (int*) &destptr[ sl->offset ];
            srcint = (const int*) &srcptr[ sl->offset ];
            *destint = !!((*destint) + (*srcint));
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
 *    @return 0 on success.
 */
int ss_statsMergeSingle( ShipStats *stats, const ShipStatList *list )
{
   return ss_statsMergeSingleScale(stats, list, 1);
}

/**
 * @brief Makes adjustments so the stats are positively additive.
 */
static void ss_adjustDoubleStat(double* statptr, double adjustment, int inverted)
{
   double currstat = *statptr;
   /*
    * For inverted stats, we first invert the stat, apply the adjustment, and then reinvert the stat
    *
    * This is because for inverted stats "infinitely good" = 0. We don't want to be able to get
    * "infinitely good" with a finite improvement. So we first invert the current stat,
    * so like normal stats "infinitely good" is actually infinity. Then we apply the adjustment
    * (we need to subtract the adjustment as the inverting has flipped the direction of "good" and "bad")
    * and then reinvert to get back to the real value.
    *
    * For example, consider a "cargo inertia" penalty of 20%, when our current cargo interia is 125%.
    *
    * We invert 125% to get 80%, subtract 20% to get 60%, and then invert to get 166.67%.
    *
    * The way to think of this is to imagine "cargo inertia" was a "positive stat"
    * called say "cargo goodness". Lets say a "cargo goodness" of 200% made cargo mass only have 50% effect.
    *
    * Then one could assume "cargo goodness" of 80% would make cargo have a 125% effect.
    *
    * Basically you invert the "cargo goodness" to get the "cargo inertia".
    *
    * In this case, lowering the "cargo goodness" by another 20% would take the "cargo goodness" to 60%,
    * making cargo have a 166.67% effect, namely the inverse of 60%.
    *
    * With non-inverted stats, adding 100% makes the stat "twice as good", but you only have to remove 50%
    * to make it "half as good". Removing 100% from a non-inverted stat makes your ship non-functional.
    *
    * Stat inversion is just for readability's sake, because "jump time" or "cargo inertia" is easier to
    * understand than "jump fastness" or "cargo goodness". But it should still work the same.
    * A 100% improvement in jump time should half the jump time, and a 50% loss should double jump time.
    *
    * We don't actually get that holding true without doing this inversion before applying adjustments.
    *
    * In the formula used in the code, we actually do the following rearrangement, which saves one division:
    *
    * 1 / (1 / currstat - adjustment)) = currstat / (1 - currstat * adjustment)
    */
   *statptr = (inverted) ? (currstat / (1. - currstat * adjustment)) : (currstat + adjustment);
}

/**
 * @brief Modifies a stat structure using a single element.
 *
 *    @param stats Stat structure to modify.
 *    @param list Single element to apply.
 *    @param scale Scaling factor.
 *    @return 0 on success.
 */
int ss_statsMergeSingleScale( ShipStats *stats, const ShipStatList *list, double scale )
{
   char *ptr;
   char *fieldptr;
   double *dbl;
   int *i;
   const ShipStatsLookup *sl = &ss_lookup[ list->type ];

   ptr = (char*) stats;
   switch (sl->data) {
      case SS_DATA_TYPE_DOUBLE:
         fieldptr = &ptr[ sl->offset ];
         memcpy(&dbl, &fieldptr, sizeof(double*));
         ss_adjustDoubleStat(dbl, list->d.d * scale, sl->inverted);
         break;
      case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
      case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
         fieldptr = &ptr[ sl->offset ];
         memcpy(&dbl, &fieldptr, sizeof(double*));
         *dbl += list->d.d * scale;
         break;

      case SS_DATA_TYPE_INTEGER:
         fieldptr = &ptr[ sl->offset ];
         memcpy(&i, &fieldptr, sizeof(int*));
         *i   += list->d.i * scale;
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
 *    @return 0 on success.
 */
int ss_statsMergeFromList( ShipStats *stats, const ShipStatList* list )
{
   if (list==NULL)
      return 0;
   int ret = 0;
   for (const ShipStatList *ll = list; ll != NULL; ll = ll->next)
      ret |= ss_statsMergeSingle( stats, ll );
   return ret;
}

/**
 * @brief Updates a stat structure from a stat list.
 *
 *    @param stats Stats to update.
 *    @param list List to update from.
 *    @param scale Scaling factor.
 *    @return 0 on success.
 */
int ss_statsMergeFromListScale( ShipStats *stats, const ShipStatList* list, double scale )
{
   if (list==NULL)
      return 0;
   int ret = 0;
   for (const ShipStatList *ll = list; ll != NULL; ll = ll->next)
      ret |= ss_statsMergeSingleScale( stats, ll, scale );
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
   for (int i=0; i<SS_TYPE_SENTINEL; i++)
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
   if (sl->inverted)
      return (d < 0.) ? "g" : "r";
   return (d > 0.) ? "g" : "r";
}

/**
 * @brief Some colour coding for ship stats integers.
 */
static const char* ss_printI_colour( int i, const ShipStatsLookup *sl )
{
   if (sl->inverted)
         return (i < 0) ? "g" : "r";
   return (i > 0) ? "g" : "r";
}

/**
 * @brief Helper to print doubles.
 */
static int ss_printD( char *buf, int len, int newline, double d, const ShipStatsLookup *sl )
{
   if (FABS(d) < 1e-10)
      return 0;

   return scnprintf( buf, len, p_("shipstats_double","%s#%s%s: %+g %s#0"),
         (newline) ? "\n" : "",
         ss_printD_colour( d, sl ),
         _(sl->display), d*100.,
         _(sl->unit));
}

/**
 * @brief Helper to print absolute doubles.
 */
static int ss_printA( char *buf, int len, int newline, double d, const ShipStatsLookup *sl )
{
   if (FABS(d) < 1e-10)
      return 0;
   return scnprintf( buf, len, p_("shipstats_absolute","%s#%s%s: %+g %s#0"),
         (newline) ? "\n" : "",
         ss_printD_colour( d, sl ),
         _(sl->display), d, /* TODO probably use num2strU here, but we want the sign enforced. */
         _(sl->unit));
}

/**
 * @brief Helper to print integers.
 */
static int ss_printI( char *buf, int len, int newline, int i, const ShipStatsLookup *sl )
{
   if (i == 0)
      return 0 ;
   return scnprintf( buf, len, p_("shipstats_integer","%s#%s%s: %+d %s#0"),
         (newline) ? "\n" : "",
         ss_printI_colour( i, sl ),
         _(sl->display), i,
         _(sl->unit));
}

/**
 * @brief Helper to print booleans.
 */
static int ss_printB( char *buf, int len, int newline, int b, const ShipStatsLookup *sl )
{
   if (!b)
      return 0;
   return scnprintf( buf, len, p_("shipstats_boolean","%s#%s%s#0"),
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
   int i     = 0;
   int newl  = newline;
   for ( ; ll != NULL; ll=ll->next) {
      const ShipStatsLookup *sl;
      int left  = len-i;
      if (left < 0)
         break;
      sl    = &ss_lookup[ ll->type ];

      switch (sl->data) {
         case SS_DATA_TYPE_DOUBLE:
         case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
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
   int l, left;
   char *ptr;
   char *fieldptr;
   double *dbl;
   int *num;

   l   = 0;
   ptr = (char*) s;
   for (int i=0; i<SS_TYPE_SENTINEL; i++) {
      const ShipStatsLookup *sl = &ss_lookup[ i ];

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

         case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
            fieldptr = &ptr[ sl->offset ];
            memcpy(&dbl, &fieldptr, sizeof(double*));
            l    += ss_printD( &buf[l], left, (newline||(l!=0)), (*dbl), sl );
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

/**
 * @brief Sets a ship stat by name.
 */
int ss_statsSet( ShipStats *s, const char *name, double value, int overwrite )
{
   const ShipStatsLookup *sl;
   ShipStatsType type;
   char *ptr;
   double *destdbl;
   int *destint;
   double v;

   type = ss_typeFromName( name );
   if (type == SS_TYPE_NIL) {
      WARN(_("Unknown ship stat type '%s'!"), name );
      return -1;
   }

   sl = &ss_lookup[ type ];
   ptr = (char*) s;
   switch (sl->data) {
      case SS_DATA_TYPE_DOUBLE:
         destdbl = (double*) (void*)&ptr[ sl->offset ];
         v = value / 100.;
         if (overwrite)
            *destdbl = 1.0;
         ss_adjustDoubleStat(destdbl, v, sl->inverted);
         break;

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
         destdbl  = (double*) (void*)&ptr[ sl->offset ];
         if (overwrite)
            *destdbl = value / 100.;
         else
            *destdbl += value / 100.;
         break;

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         destdbl  = (double*) (void*)&ptr[ sl->offset ];
         if (overwrite)
            *destdbl = value;
         else
            *destdbl += value;
         break;

      case SS_DATA_TYPE_BOOLEAN:
         destint  = (int*) &ptr[ sl->offset ];
         if (overwrite)
            *destint = (fabs(value) > 1e-5);
         else
            *destint |= (fabs(value) > 1e-5);
         break;

      case SS_DATA_TYPE_INTEGER:
         destint  = (int*) &ptr[ sl->offset ];
         if (overwrite)
            *destint = round(value);
         else
            *destint += round(value);
         break;
   }

   return 0;
}

static double ss_statsGetInternal( const ShipStats *s, ShipStatsType type )
{
   const ShipStatsLookup *sl;
   const char *ptr;
   const double *destdbl;
   const int *destint;

   sl = &ss_lookup[ type ];
   ptr = (const char*) s;
   switch (sl->data) {
      case SS_DATA_TYPE_DOUBLE:
         destdbl = (const double*) (const void*)&ptr[ sl->offset ];
         return 100.*((*destdbl) - 1.0);

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
         destdbl = (const double*) (const void*)&ptr[ sl->offset ];
         return 100.*(*destdbl);

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         destdbl  = (const double*) (const void*)&ptr[ sl->offset ];
         return *destdbl;

      case SS_DATA_TYPE_BOOLEAN:
      case SS_DATA_TYPE_INTEGER:
         destint  = (const int*) &ptr[ sl->offset ];
         return *destint;
   }
   return 0.;
}

static int ss_statsGetLuaInternal( lua_State *L, const ShipStats *s, ShipStatsType type, int internal )
{
   const ShipStatsLookup *sl;
   const char *ptr;
   const double *destdbl;
   const int *destint;

   sl = &ss_lookup[ type ];
   ptr = (const char*) s;
   switch (sl->data) {
      case SS_DATA_TYPE_DOUBLE:
         destdbl = (const double*) (const void*)&ptr[ sl->offset ];
         if (internal)
            lua_pushnumber(L, *destdbl );
         else
            lua_pushnumber(L, 100.*((*destdbl) - 1.0) );
         return 0;

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE_PERCENT:
         destdbl = (const double*) (const void*)&ptr[ sl->offset ];
         if (internal)
            lua_pushnumber(L, *destdbl );
         else
            lua_pushnumber(L, 100.*(*destdbl));
         return 0;

      case SS_DATA_TYPE_DOUBLE_ABSOLUTE:
         destdbl  = (const double*) (const void*)&ptr[ sl->offset ];
         lua_pushnumber(L, *destdbl);
         return 0;

      case SS_DATA_TYPE_BOOLEAN:
         destint  = (const int*) &ptr[ sl->offset ];
         lua_pushboolean(L, *destint);
         return 0;

      case SS_DATA_TYPE_INTEGER:
         destint  = (const int*) &ptr[ sl->offset ];
         lua_pushinteger(L, *destint);
         return 0;
   }
   lua_pushnil(L);
   return -1;
}

/**
 * @brief Gets a ship stat value by name.
 */
double ss_statsGet( const ShipStats *s, const char *name )
{
   ShipStatsType type = ss_typeFromName( name );
   if (type == SS_TYPE_NIL) {
      WARN(_("Unknown ship stat type '%s'!"), name );
      return 0;
   }

   return ss_statsGetInternal( s, type );
}

/**
 * @brief Gets a ship stat value by name and pushes it to Lua.
 */
int ss_statsGetLua( lua_State *L, const ShipStats *s, const char *name, int internal )
{
   ShipStatsType type;

   if (name==NULL)
      return ss_statsGetLuaTable( L, s, internal );

   type = ss_typeFromName( name );
   if (type == SS_TYPE_NIL) {
      WARN(_("Unknown ship stat type '%s'!"), name );
      return -1;
   }

   return ss_statsGetLuaInternal( L, s, type, internal );
}

/**
 * @brief Converts ship stats to a Lua table, which is pushed on the Lua stack.
 */
int ss_statsGetLuaTable( lua_State *L, const ShipStats *s, int internal )
{
   lua_newtable(L);
   for (int i=0; i<SS_TYPE_SENTINEL; i++) {
      const ShipStatsLookup *sl = &ss_lookup[ i ];

      /* Only want valid names. */
      if (sl->name == NULL)
         continue;

      /* Push name and get value. */
      lua_pushstring(L, sl->name);
      ss_statsGetLuaInternal( L, s, i, internal );
      lua_rawset( L, -3 );
   }
   return 0;
}
