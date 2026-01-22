/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file unidiff.c
 *
 * @brief Handles the application and removal of 'diffs' to the universe.
 *
 * Diffs allow changing spobs, factions, etc... in the universe.
 *  These are meant to be applied after the player triggers them, mostly
 *  through missions.
 */
/** @cond */
#include <stdlib.h>
/** @endcond */

#include "unidiff.h"

#include "array.h"
#include "conf.h"
#include "economy.h"
#include "log.h"
#include "map_overlay.h"
#include "ndata.h"
#include "nxml.h"
#include "player.h"
#include "safelanes.h"
#include "space.h"

static UniDiffData_t *diff_available = NULL; /**< Available diffs. */

/**
 * @struct UniDiff_t
 *
 * @brief Represents each Universe Diff.
 */
typedef struct UniDiff_ {
   UniDiffData_t *data;    /**< Bas e data from which diff is being applied. */
   UniHunk_t     *applied; /**< Applied hunks. */
   UniHunk_t     *failed;  /**< Failed hunks. */
} UniDiff_t;

/*
 * Diff stack.
 */
static UniDiff_t *diff_stack = NULL; /**< Currently applied universe diffs. */

/* Useful variables. */
static int diff_universe_changed =
   0; /**< Whether or not the universe changed. */
static int         diff_universe_defer = 0; /**< Defers changes to later. */
static const char *diff_nav_spob =
   NULL; /**< Stores the player's spob target if necessary. */
static const char *diff_nav_hyperspace =
   NULL; /**< Stores the player's hyperspace target if necessary. */

typedef struct HunkProperties {
   const char   *name;    /**< Name of the hunk type. For display purposes. */
   const char   *tag;     /**< Tag of the hunk. Used for parsing XML. */
   UniHunkType_t reverse; /**< What is the hunk type to reverse the changes. */
   const char *const *attrs; /**< Attributes we are interested in for the hunk.
                           NULL terminated array.  */
} HunkProperties;
const char *const           hunk_attr_label[] = { "label", NULL };
static const HunkProperties hunk_prop[HUNK_TYPE_SENTINAL + 1] = {
   [HUNK_TYPE_NONE] =
      {
         .name    = N_( "none" ),
         .tag     = "none",
         .reverse = HUNK_TYPE_NONE,
      },
   /* HUNK_TARGET_SYSTEM. */
   [HUNK_TYPE_SPOB_ADD]        = { .name    = N_( "spob add" ),
                                   .tag     = "spob_add",
                                   .reverse = HUNK_TYPE_SPOB_REMOVE },
   [HUNK_TYPE_SPOB_REMOVE]     = { .name    = N_( "spob remove" ),
                                   .tag     = "spob_remove",
                                   .reverse = HUNK_TYPE_SPOB_ADD },
   [HUNK_TYPE_VSPOB_ADD]       = { .name    = N_( "virtual spob add" ),
                                   .tag     = "spob_virtual_add",
                                   .reverse = HUNK_TYPE_VSPOB_REMOVE },
   [HUNK_TYPE_VSPOB_REMOVE]    = { .name    = N_( "virtual spob remove" ),
                                   .tag     = "spob_virtual_remove",
                                   .reverse = HUNK_TYPE_VSPOB_ADD },
   [HUNK_TYPE_JUMP_ADD]        = { .name    = N_( "jump add" ),
                                   .tag     = "jump_add",
                                   .reverse = HUNK_TYPE_JUMP_REMOVE },
   [HUNK_TYPE_JUMP_REMOVE]     = { .name    = N_( "jump remove" ),
                                   .tag     = "jump_remove",
                                   .reverse = HUNK_TYPE_JUMP_ADD },
   [HUNK_TYPE_SSYS_BACKGROUND] = { .name = N_( "ssys background" ),
                                   .tag  = "background",
                                   .reverse =
                                      HUNK_TYPE_SSYS_BACKGROUND_REVERT },
   [HUNK_TYPE_SSYS_BACKGROUND_REVERT] =
      {
         .name    = N_( "ssys background revert" ),
         .tag     = NULL,
         .reverse = HUNK_TYPE_NONE,
      },
   [HUNK_TYPE_SSYS_FEATURES]           = { .name    = N_( "ssys features" ),
                                           .tag     = "features",
                                           .reverse = HUNK_TYPE_SSYS_FEATURES_REVERT },
   [HUNK_TYPE_SSYS_FEATURES_REVERT]    = { .name    = N_( "ssys features revert" ),
                                           .tag     = NULL,
                                           .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_POS_X]              = { .name    = N_( "ssys pos x" ),
                                           .tag     = "pos_x",
                                           .reverse = HUNK_TYPE_SSYS_POS_X_REVERT },
   [HUNK_TYPE_SSYS_POS_X_REVERT]       = { .name    = N_( "ssys pos x revert" ),
                                           .tag     = NULL,
                                           .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_POS_Y]              = { .name    = N_( "ssys pos y" ),
                                           .tag     = "pos_y",
                                           .reverse = HUNK_TYPE_SSYS_POS_Y_REVERT },
   [HUNK_TYPE_SSYS_POS_Y_REVERT]       = { .name    = N_( "ssys pos x revert" ),
                                           .tag     = NULL,
                                           .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_DISPLAYNAME]        = { .name = N_( "ssys displayname" ),
                                           .tag  = "displayname",
                                           .reverse =
                                              HUNK_TYPE_SSYS_DISPLAYNAME_REVERT },
   [HUNK_TYPE_SSYS_DISPLAYNAME_REVERT] = { .name =
                                              N_( "ssys displayname revert" ),
                                           .tag     = NULL,
                                           .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_DUST]               = { .name    = N_( "ssys dust" ),
                                           .tag     = "dust",
                                           .reverse = HUNK_TYPE_SSYS_DUST_REVERT },
   [HUNK_TYPE_SSYS_DUST_REVERT]        = { .name    = N_( "ssys dust revert" ),
                                           .tag     = NULL,
                                           .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_INTERFERENCE]       = { .name = N_( "ssys interference" ),
                                           .tag  = "interference",
                                           .reverse =
                                              HUNK_TYPE_SSYS_INTERFERENCE_REVERT },
   [HUNK_TYPE_SSYS_INTERFERENCE_REVERT] = { .name =
                                               N_( "ssys interference revert" ),
                                            .tag     = NULL,
                                            .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_NEBU_DENSITY]        = { .name = N_( "ssys nebula density" ),
                                            .tag  = "nebu_density",
                                            .reverse =
                                               HUNK_TYPE_SSYS_NEBU_DENSITY_REVERT },
   [HUNK_TYPE_SSYS_NEBU_DENSITY_REVERT] = { .name = N_(
                                               "ssys nebula density revert" ),
                                            .tag     = NULL,
                                            .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_NEBU_VOLATILITY] =
      { .name    = N_( "ssys nebula volatility" ),
        .tag     = "nebu_volatility",
        .reverse = HUNK_TYPE_SSYS_NEBU_VOLATILITY_REVERT },
   [HUNK_TYPE_SSYS_NEBU_VOLATILITY_REVERT] =
      {
         .name    = N_( "ssys nebula volatility revert" ),
         .tag     = NULL,
         .reverse = HUNK_TYPE_NONE,
      },
   [HUNK_TYPE_SSYS_NEBU_HUE]        = { .name    = N_( "ssys nebula hue" ),
                                        .tag     = "nebu_hue",
                                        .reverse = HUNK_TYPE_SSYS_NEBU_HUE_REVERT },
   [HUNK_TYPE_SSYS_NEBU_HUE_REVERT] = { .name = N_( "ssys nebula hue revert" ),
                                        .tag  = NULL,
                                        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_NOLANES_ADD]     = { .name    = N_( "ssys nolanes add" ),
                                        .tag     = "nolanes_add",
                                        .reverse = HUNK_TYPE_SSYS_NOLANES_REMOVE },
   [HUNK_TYPE_SSYS_NOLANES_REMOVE]  = { .name    = N_( "ssys nolanes remove" ),
                                        .tag     = "nolanes_remove",
                                        .reverse = HUNK_TYPE_SSYS_NOLANES_ADD },
   [HUNK_TYPE_SSYS_TAG_ADD]         = { .name    = N_( "ssys tag = NULL add" ),
                                        .tag     = "tag_add",
                                        .reverse = HUNK_TYPE_SSYS_TAG_REMOVE },
   [HUNK_TYPE_SSYS_TAG_REMOVE]      = { .name    = N_( "ssys tag = NULL remove" ),
                                        .tag     = "tag_remove",
                                        .reverse = HUNK_TYPE_SSYS_TAG_ADD },
   /* HUNK_TARGET_SYSTEM with label. */
   [HUNK_TYPE_SSYS_ASTEROIDS_ADD]        = { .name = N_( "ssys asteroids add" ),
                                             .tag  = "asteroids_add",
                                             .reverse =
                                                HUNK_TYPE_SSYS_ASTEROIDS_ADD_REVERT },
   [HUNK_TYPE_SSYS_ASTEROIDS_ADD_REVERT] = { .name = N_(
                                                "ssys asteroids add revert" ),
                                             .tag     = NULL,
                                             .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_ASTEROIDS_REMOVE] =
      { .name    = N_( "ssys asteroids remove" ),
        .tag     = "asteroids_remove",
        .reverse = HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_REVERT },
   [HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_REVERT] =
      { .name    = N_( "ssys asteroids remove revert" ),
        .tag     = NULL,
        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_ASTEROIDS_POS_X] =
      { .name    = N_( "ssys asteroids pos x" ),
        .tag     = "asteroids_pos_x",
        .reverse = HUNK_TYPE_SSYS_ASTEROIDS_POS_X_REVERT,
        .attrs   = hunk_attr_label },
   [HUNK_TYPE_SSYS_ASTEROIDS_POS_X_REVERT] =
      { .name    = N_( "ssys asteroids pos x revert" ),
        .tag     = NULL,
        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_ASTEROIDS_POS_Y] =
      { .name    = N_( "ssys asteroids pos y" ),
        .tag     = "asteroids_pos_y",
        .reverse = HUNK_TYPE_SSYS_ASTEROIDS_POS_Y_REVERT,
        .attrs   = hunk_attr_label },
   [HUNK_TYPE_SSYS_ASTEROIDS_POS_Y_REVERT] =
      { .name    = N_( "ssys asteroids pos y revert" ),
        .tag     = NULL,
        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_ASTEROIDS_DENSITY] =
      { .name    = N_( "ssys asteroids density" ),
        .tag     = "asteroids_density",
        .reverse = HUNK_TYPE_SSYS_ASTEROIDS_DENSITY_REVERT,
        .attrs   = hunk_attr_label },
   [HUNK_TYPE_SSYS_ASTEROIDS_DENSITY_REVERT] =
      { .name    = N_( "ssys asteroids density revert" ),
        .tag     = NULL,
        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_ASTEROIDS_RADIUS] =
      { .name    = N_( "ssys asteroids radius" ),
        .tag     = "asteroids_radius",
        .reverse = HUNK_TYPE_SSYS_ASTEROIDS_RADIUS_REVERT,
        .attrs   = hunk_attr_label },
   [HUNK_TYPE_SSYS_ASTEROIDS_RADIUS_REVERT] =
      { .name    = N_( "ssys asteroids radius revert" ),
        .tag     = NULL,
        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_ASTEROIDS_MAXSPEED] =
      { .name    = N_( "ssys asteroids maxspeed" ),
        .tag     = "asteroids_maxspeed",
        .reverse = HUNK_TYPE_SSYS_ASTEROIDS_MAXSPEED_REVERT,
        .attrs   = hunk_attr_label },
   [HUNK_TYPE_SSYS_ASTEROIDS_MAXSPEED_REVERT] =
      { .name    = N_( "ssys asteroids maxspeed revert" ),
        .tag     = NULL,
        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_ASTEROIDS_ACCEL] =
      { .name    = N_( "ssys asteroids accel" ),
        .tag     = "asteroids_accel",
        .reverse = HUNK_TYPE_SSYS_ASTEROIDS_ACCEL_REVERT,
        .attrs   = hunk_attr_label },
   [HUNK_TYPE_SSYS_ASTEROIDS_ACCEL_REVERT] =
      { .name    = N_( "ssys asteroids accel revert" ),
        .tag     = NULL,
        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SSYS_ASTEROIDS_ADD_TYPE] =
      { .name    = N_( "ssys asteroids add type" ),
        .tag     = "asteroids_add_type",
        .reverse = HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_TYPE,
        .attrs   = hunk_attr_label },
   [HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_TYPE] =
      { .name    = N_( "ssys asteroids remove type" ),
        .tag     = "asteroids_remove_type",
        .reverse = HUNK_TYPE_SSYS_ASTEROIDS_ADD_TYPE,
        .attrs   = hunk_attr_label },
   /* HUNK_TARGET_TECH. */
   [HUNK_TYPE_TECH_ADD]    = { .name    = N_( "tech add" ),
                               .tag     = "item_add",
                               .reverse = HUNK_TYPE_TECH_REMOVE },
   [HUNK_TYPE_TECH_REMOVE] = { .name    = N_( "tech remove" ),
                               .tag     = "item_remove",
                               .reverse = HUNK_TYPE_TECH_ADD },
   /* HUNK_TARGET_SPOB. */
   [HUNK_TYPE_SPOB_POS_X]          = { .name    = N_( "spob pos x" ),
                                       .tag     = "pos_x",
                                       .reverse = HUNK_TYPE_SPOB_POS_X_REVERT },
   [HUNK_TYPE_SPOB_POS_X_REVERT]   = { .name    = N_( "spob pos x revert" ),
                                       .tag     = NULL,
                                       .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_POS_Y]          = { .name    = N_( "spob pos y" ),
                                       .tag     = "pos_y",
                                       .reverse = HUNK_TYPE_SPOB_POS_Y_REVERT },
   [HUNK_TYPE_SPOB_POS_Y_REVERT]   = { .name    = N_( "spob pos y revert" ),
                                       .tag     = NULL,
                                       .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_CLASS]          = { .name    = N_( "spob class" ),
                                       .tag     = "class",
                                       .reverse = HUNK_TYPE_SPOB_CLASS_REVERT },
   [HUNK_TYPE_SPOB_CLASS_REVERT]   = { .name    = N_( "spob class revert" ),
                                       .tag     = NULL,
                                       .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_FACTION]        = { .name    = N_( "spob faction" ),
                                       .tag     = "faction",
                                       .reverse = HUNK_TYPE_SPOB_FACTION_REVERT },
   [HUNK_TYPE_SPOB_FACTION_REVERT] = { .name    = N_( "spob faction revert" ),
                                       .tag     = NULL,
                                       .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_PRESENCE_BASE]  = { .name = N_( "spob presence base" ),
                                       .tag  = "presence_base",
                                       .reverse =
                                          HUNK_TYPE_SPOB_PRESENCE_BASE_REVERT },
   [HUNK_TYPE_SPOB_PRESENCE_BASE_REVERT] = { .name = N_(
                                                "spob presence base revert" ),
                                             .tag     = NULL,
                                             .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_PRESENCE_BONUS] =
      { .name    = N_( "spob presence bonus" ),
        .tag     = "presence_bonus",
        .reverse = HUNK_TYPE_SPOB_PRESENCE_BONUS_REVERT },
   [HUNK_TYPE_SPOB_PRESENCE_BONUS_REVERT] = { .name = N_(
                                                 "spob presence bonus revert" ),
                                              .tag     = NULL,
                                              .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_PRESENCE_RANGE] =
      { .name    = N_( "spob presence range" ),
        .tag     = "presence_range",
        .reverse = HUNK_TYPE_SPOB_PRESENCE_RANGE_REVERT },
   [HUNK_TYPE_SPOB_PRESENCE_RANGE_REVERT] = { .name = N_(
                                                 "spob presence range revert" ),
                                              .tag     = NULL,
                                              .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_HIDE]                  = { .name    = N_( "spob hide" ),
                                              .tag     = "hide",
                                              .reverse = HUNK_TYPE_SPOB_HIDE_REVERT },
   [HUNK_TYPE_SPOB_HIDE_REVERT]           = { .name    = N_( "spob hide revert" ),
                                              .tag     = NULL,
                                              .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_POPULATION]            = { .name = N_( "spob population" ),
                                              .tag  = "population",
                                              .reverse =
                                                 HUNK_TYPE_SPOB_POPULATION_REVERT },
   [HUNK_TYPE_SPOB_POPULATION_REVERT]     = { .name =
                                                 N_( "spob population revert" ),
                                              .tag     = NULL,
                                              .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_DISPLAYNAME]           = { .name = N_( "spob displayname" ),
                                              .tag  = "displayname",
                                              .reverse =
                                                 HUNK_TYPE_SPOB_DISPLAYNAME_REVERT },
   [HUNK_TYPE_SPOB_DISPLAYNAME_REVERT]    = { .name =
                                                 N_( "spob displayname revert" ),
                                              .tag     = NULL,
                                              .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_DESCRIPTION]           = { .name = N_( "spob description" ),
                                              .tag  = "description",
                                              .reverse =
                                                 HUNK_TYPE_SPOB_DESCRIPTION_REVERT },
   [HUNK_TYPE_SPOB_DESCRIPTION_REVERT]    = { .name =
                                                 N_( "spob description revert" ),
                                              .tag     = NULL,
                                              .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_BAR]                   = { .name    = N_( "spob bar" ),
                                              .tag     = "bar",
                                              .reverse = HUNK_TYPE_SPOB_BAR_REVERT },
   [HUNK_TYPE_SPOB_BAR_REVERT]            = { .name    = N_( "spob bar revert" ),
                                              .tag     = NULL,
                                              .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_SPACE]                 = { .name    = N_( "spob space" ),
                                              .tag     = "gfx_space",
                                              .reverse = HUNK_TYPE_SPOB_SPACE_REVERT },
   [HUNK_TYPE_SPOB_SPACE_REVERT]          = { .name    = N_( "spob space revert" ),
                                              .tag     = NULL,
                                              .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_EXTERIOR]              = { .name    = N_( "spob exterior" ),
                                              .tag     = "gfx_exterior",
                                              .reverse = HUNK_TYPE_SPOB_EXTERIOR_REVERT },
   [HUNK_TYPE_SPOB_EXTERIOR_REVERT] = { .name    = N_( "spob exterior revert" ),
                                        .tag     = NULL,
                                        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_LUA]             = { .name    = N_( "spob lua" ),
                                        .tag     = "lua",
                                        .reverse = HUNK_TYPE_SPOB_LUA_REVERT },
   [HUNK_TYPE_SPOB_LUA_REVERT]      = { .name    = N_( "spob lua revert" ),
                                        .tag     = NULL,
                                        .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SPOB_SERVICE_ADD]     = { .name    = N_( "spob service add" ),
                                        .tag     = "service_add",
                                        .reverse = HUNK_TYPE_SPOB_SERVICE_REMOVE },
   [HUNK_TYPE_SPOB_SERVICE_REMOVE]  = { .name    = N_( "spob service remove" ),
                                        .tag     = "service_remove",
                                        .reverse = HUNK_TYPE_SPOB_SERVICE_ADD },
   [HUNK_TYPE_SPOB_NOMISNSPAWN_ADD] = { .name = N_( "spob nomissionspawn add" ),
                                        .tag  = "nomissionspawn_add",
                                        .reverse =
                                           HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE },
   [HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE] = { .name = N_(
                                              "spob nomissionspawn remove" ),
                                           .tag = "nomissionspawn_remove",
                                           .reverse =
                                              HUNK_TYPE_SPOB_NOMISNSPAWN_ADD },
   [HUNK_TYPE_SPOB_TECH_ADD]           = { .name    = N_( "spob tech add" ),
                                           .tag     = "tech_add",
                                           .reverse = HUNK_TYPE_SPOB_TECH_REMOVE },
   [HUNK_TYPE_SPOB_TECH_REMOVE]        = { .name    = N_( "spob tech remove" ),
                                           .tag     = "tech_remove",
                                           .reverse = HUNK_TYPE_SPOB_TECH_ADD },
   [HUNK_TYPE_SPOB_TAG_ADD]            = { .name    = N_( "spob tag = NULL add" ),
                                           .tag     = "tag_add",
                                           .reverse = HUNK_TYPE_SPOB_TAG_REMOVE },
   [HUNK_TYPE_SPOB_TAG_REMOVE] = { .name    = N_( "spob tag = NULL remove" ),
                                   .tag     = "tag_remove",
                                   .reverse = HUNK_TYPE_SPOB_TAG_ADD },
   /* HUNK_TARGET_FACTION. */
   [HUNK_TYPE_FACTION_VISIBLE]   = { .name    = N_( "faction visible" ),
                                     .tag     = "visible",
                                     .reverse = HUNK_TYPE_FACTION_INVISIBLE },
   [HUNK_TYPE_FACTION_INVISIBLE] = { .name    = N_( "faction invisible" ),
                                     .tag     = "invisible",
                                     .reverse = HUNK_TYPE_FACTION_INVISIBLE },
   [HUNK_TYPE_FACTION_ALLY]      = { .name    = N_( "faction set ally" ),
                                     .tag     = "ally",
                                     .reverse = HUNK_TYPE_FACTION_REALIGN },
   [HUNK_TYPE_FACTION_ENEMY]     = { .name    = N_( "faction set enemy" ),
                                     .tag     = "enemy",
                                     .reverse = HUNK_TYPE_FACTION_REALIGN },
   [HUNK_TYPE_FACTION_NEUTRAL]   = { .name    = N_( "faction set neutral" ),
                                     .tag     = "neutral",
                                     .reverse = HUNK_TYPE_FACTION_REALIGN },
   [HUNK_TYPE_FACTION_REALIGN]   = { .name    = N_( "faction alignment reset" ),
                                     .tag     = NULL,
                                     .reverse = HUNK_TYPE_NONE },
   [HUNK_TYPE_SENTINAL]          = { .name    = N_( "sentinal" ),
                                     .tag     = NULL,
                                     .reverse = HUNK_TYPE_NONE },
};

#define HUNK_CUST( TYPE, DTYPE, FUNC )                                         \
   /* should be possible to do the static_assert with C23's constexpr. */      \
   /* static_assert( hunk_prop[TYPE].tag != NULL, "" ); */                     \
   if ( xml_isNode( cur, hunk_prop[TYPE].tag ) ) {                             \
      memset( &hunk, 0, sizeof( hunk ) );                                      \
      hunk.target.type   = base.target.type;                                   \
      hunk.target.u.name = strdup( base.target.u.name );                       \
      hunk.type          = TYPE;                                               \
      hunk.dtype         = DTYPE;                                              \
      diff_parseAttr( &hunk, cur );                                            \
      FUNC array_push_back( &diff->hunks, hunk );                              \
      continue;                                                                \
   }
#define HUNK_NONE( TYPE )                                                      \
   HUNK_CUST( TYPE, HUNK_DATA_NONE, hunk.u.name = NULL; );
#define HUNK_STRD( TYPE )                                                      \
   HUNK_CUST( TYPE, HUNK_DATA_STRING, hunk.u.name = xml_getStrd( cur ); );
#define HUNK_INT( TYPE )                                                       \
   HUNK_CUST( TYPE, HUNK_DATA_INT, hunk.u.data = xml_getUInt( cur ); );
#define HUNK_FLOAT( TYPE )                                                     \
   HUNK_CUST( TYPE, HUNK_DATA_FLOAT, hunk.u.fdata = xml_getFloat( cur ); );

/*
 * Prototypes.
 */
static int diff_applyInternal( const char *name, int oneshot, int warn );
NONNULL( 1 ) static UniDiff_t *diff_get( const char *name );
static UniDiff_t  *diff_newDiff( void );
static int         diff_removeDiff( UniDiff_t *diff );
static const char *diff_getAttr( UniHunk_t *hunk, const char *name );
static void        diff_parseAttr( UniHunk_t *hunk, xmlNodePtr node );
static int         diff_parseDoc( UniDiffData_t *diff, xmlDocPtr doc );
static int         diff_parseSystem( UniDiffData_t *diff, xmlNodePtr node );
static int         diff_parseTech( UniDiffData_t *diff, xmlNodePtr node );
static int         diff_parseSpob( UniDiffData_t *diff, xmlNodePtr node );
static int         diff_parseFaction( UniDiffData_t *diff, xmlNodePtr node );
static void        diff_hunkFailed( UniDiff_t *diff, const UniHunk_t *hunk );
static void        diff_hunkSuccess( UniDiff_t *diff, const UniHunk_t *hunk );
static void        diff_cleanup( UniDiff_t *diff );
/* Misc. */
static int diff_checkUpdateUniverse( void );
/* Externed. */
int diff_save( xmlTextWriterPtr writer ); /**< Used in save.c */
int diff_load( xmlNodePtr parent );       /**< Used in save.c */

/**
 * @brief Simple comparison for UniDiffData_t based on name.
 */
static int diff_cmp( const void *p1, const void *p2 )
{
   const UniDiffData_t *d1, *d2;
   d1 = (const UniDiffData_t *)p1;
   d2 = (const UniDiffData_t *)p2;
   return strcmp( d1->name, d2->name );
}

/**
 * @brief Loads available universe diffs.
 *
 *    @return 0 on success.
 */
int diff_init( void )
{
#if DEBUGGING
   Uint32 time = SDL_GetTicks();

   for ( int i = 0; i < HUNK_TYPE_SENTINAL; i++ ) {
      if ( hunk_prop[i].name == NULL )
         WARN( "HUNK_TYPE '%d' missing name!", i );
      if ( hunk_prop[i].reverse == HUNK_TYPE_NONE ) {
         /* It's possible that this is an internal usage only reverse one, so we
          * have to see if something points to it instead. */
         int found = 0;
         for ( int j = 0; j < HUNK_TYPE_SENTINAL; j++ ) {
            if ( hunk_prop[j].reverse == (UniHunkType_t)i ) {
               found = 1;
               break;
            }
         }
         /* If not found, that means that the type is not referred to by anyone.
          */
         if ( !found ) {
            if ( hunk_prop[i].name == NULL )
               WARN( "HUNK_TYPE '%d' missing reverse!", i );
            else
               WARN( "HUNK_TYPE '%s' missing reverse!", hunk_prop[i].name );
         }
      }
   }
#endif /* DEBUGGING */

   char **diff_files = ndata_listRecursive( UNIDIFF_DATA_PATH );
   diff_available =
      array_create_size( UniDiffData_t, array_size( diff_files ) );
   for ( int i = 0; i < array_size( diff_files ); i++ ) {
      UniDiffData_t diff;

      memset( &diff, 0, sizeof( diff ) );
      if ( diff_parsePhysFS( &diff, diff_files[i] ) == 0 )
         array_push_back( &diff_available, diff );
      // xmlr_attr_strd( node, "name", diff.name );
   }
   array_free( diff_files );
   array_shrink( &diff_available );

   /* Sort and warn about duplicates. */
   qsort( diff_available, array_size( diff_available ), sizeof( UniDiffData_t ),
          diff_cmp );
   for ( int i = 0; i < array_size( diff_available ) - 1; i++ ) {
      UniDiffData_t       *d  = &diff_available[i];
      const UniDiffData_t *dn = &diff_available[i + 1];
      if ( strcmp( d->name, dn->name ) == 0 )
         WARN( _( "Two unidiff have the same name '%s'!" ), d->name );
   }

#if DEBUGGING
   if ( conf.devmode ) {
      DEBUG( n_( "Loaded %d UniDiff in %.3f s", "Loaded %d UniDiffs in %.3f s",
                 array_size( diff_available ) ),
             array_size( diff_available ), ( SDL_GetTicks() - time ) / 1000. );
   } else
      DEBUG( n_( "Loaded %d UniDiff", "Loaded %d UniDiffs",
                 array_size( diff_available ) ),
             array_size( diff_available ) );
#endif /* DEBUGGING */

   return 0;
}

void diff_freeData( UniDiffData_t *diff )
{
   for ( int j = 0; j < array_size( diff->hunks ); j++ )
      diff_cleanupHunk( &diff->hunks[j] );
   array_free( diff->hunks );
   free( diff->name );
   free( diff->filename );
}

/**
 * @brief Clean up after diffs.
 */
void diff_exit( void )
{
   diff_clear();
   for ( int i = 0; i < array_size( diff_available ); i++ ) {
      UniDiffData_t *d = &diff_available[i];
      diff_freeData( d );
   }
   array_free( diff_available );
   diff_available = NULL;
}

int diff_parse( UniDiffData_t *diff, char *filename )
{
   xmlDocPtr doc = xmlParseFile( filename );
   memset( diff, 0, sizeof( UniDiffData_t ) );
   diff->filename = filename;
   if ( doc == NULL )
      return -1;
   return diff_parseDoc( diff, doc );
}

int diff_parsePhysFS( UniDiffData_t *diff, char *filename )
{
   xmlDocPtr doc = xml_parsePhysFS( filename );
   memset( diff, 0, sizeof( UniDiffData_t ) );
   diff->filename = filename;
   if ( doc == NULL )
      return -1;
   return diff_parseDoc( diff, doc );
}

static int diff_parseDoc( UniDiffData_t *diff, xmlDocPtr doc )
{
   xmlNodePtr parent = doc->xmlChildrenNode;
   xmlNodePtr node;
   if ( strcmp( (char *)parent->name, "unidiff" ) ) {
      WARN( _( "Malformed unidiff file: missing root element 'unidiff'" ) );
      return -1;
   }

   xmlr_attr_strd( parent, "name", diff->name );
   diff->hunks = array_create( UniHunk_t );

   /* Start parsing. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes( node );

      if ( xml_isNode( node, "system" ) )
         diff_parseSystem( diff, node );
      else if ( xml_isNode( node, "tech" ) )
         diff_parseTech( diff, node );
      else if ( xml_isNode( node, "spob" ) )
         diff_parseSpob( diff, node );
      else if ( xml_isNode( node, "faction" ) )
         diff_parseFaction( diff, node );
      else
         WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name,
               node->name );
   } while ( xml_nextNode( node ) );

   xmlFreeDoc( doc );
   return 0;
}

/**
 * @brief Checks if a diff is currently applied.
 *
 *    @param name Diff to check.
 *    @return 0 if it's not applied, 1 if it is.
 */
int diff_isApplied( const char *name )
{
   if ( diff_get( name ) != NULL )
      return 1;
   return 0;
}

/**
 * @brief Gets a diff by name.
 *
 *    @param name Name of the diff to get.
 *    @return The diff if found or NULL if not found.
 */
static UniDiff_t *diff_get( const char *name )
{
   for ( int i = 0; i < array_size( diff_stack ); i++ )
      if ( strcmp( diff_stack[i].data->name, name ) == 0 )
         return &diff_stack[i];
   return NULL;
}

/**
 * @brief Applies a diff to the universe.
 *
 *    @param name Diff to apply.
 *    @return 0 on success.
 */
int diff_apply( const char *name )
{
   diff_nav_hyperspace = NULL;
   diff_nav_spob       = NULL;
   if ( player.p ) {
      if ( player.p->nav_hyperspace >= 0 )
         diff_nav_hyperspace =
            cur_system->jumps[player.p->nav_hyperspace].target->name;
      if ( player.p->nav_spob >= 0 )
         diff_nav_spob = cur_system->spobs[player.p->nav_spob]->name;
   }
   return diff_applyInternal( name, 1, 1 );
}

/**
 * @brief Applies a diff to the universe.
 *
 *    @param name Diff to apply.
 *    @param oneshot Whether or not this diff should be applied as a single
 * one-shot diff.
 *    @param warn Whether or not to warn on failure.
 *    @return 0 on success.
 */
static int diff_applyInternal( const char *name, int oneshot, int warn )
{
   UniDiffData_t *d;
   UniDiff_t     *diff;
   int            nfailed;

   /* Check if already applied. */
   if ( diff_isApplied( name ) )
      return 0;

   /* Reset change variable. */
   if ( oneshot && !diff_universe_defer )
      diff_universe_changed = 0;

   const UniDiffData_t q = { .name = (char *)name };
   d = bsearch( &q, diff_available, array_size( diff_available ),
                sizeof( UniDiffData_t ), diff_cmp );
   if ( d == NULL ) {
      if ( warn )
         WARN( _( "UniDiff '%s' not found in %s!" ), name, UNIDIFF_DATA_PATH );
      return -1;
   }

   /* Prepare it. */
   diff = diff_newDiff();
   memset( diff, 0, sizeof( UniDiff_t ) );
   diff->data = d;

   /* Time to apply. */
   for ( int i = 0; i < array_size( d->hunks ); i++ ) {
      UniHunk_t *hi = &d->hunks[i];
      UniHunk_t  h  = *hi;

      /* Create a copy of the hunk. */
      h.target.u.name = strdup( hi->target.u.name );
      if ( hi->dtype == HUNK_DATA_STRING )
         h.u.name = strdup( hi->u.name );
      if ( h.attr != NULL ) {
         h.attr = array_create( UniAttribute_t );
         for ( int j = 0; j < array_size( hi->attr ); j++ ) {
            UniAttribute_t attr;
            attr.name  = strdup( hi->attr[j].name );
            attr.value = strdup( hi->attr[j].value );
            array_push_back( &h.attr, attr );
         }
      }

      /* Patch. */
      if ( diff_patchHunk( &h ) < 0 )
         diff_hunkFailed( diff, &h );
      else
         diff_hunkSuccess( diff, &h );
   }

   /* Warn about failures. */
   nfailed = array_size( diff->failed );
   if ( nfailed > 0 ) {
      WARN( n_( "Unidiff '%s' failed to apply %d hunk.",
                "Unidiff '%s' failed to apply %d hunks.", nfailed ),
            d->name, nfailed );
      for ( int i = 0; i < nfailed; i++ ) {
         UniHunk_t  *fail   = &diff->failed[i];
         char       *target = fail->target.u.name;
         const char *hname;
         if ( ( fail->type < 0 ) || ( fail->type >= HUNK_TYPE_SENTINAL ) ||
              ( hunk_prop[fail->type].name == NULL ) ) {
            WARN( _( "Unknown unidiff hunk '%d'!" ), fail->type );
            hname = N_( "unknown hunk" );
         } else
            hname = hunk_prop[fail->type].name;

         /* Have to handle all possible data cases. */
         switch ( fail->dtype ) {
         case HUNK_DATA_NONE:
            WARN( p_( "unidiff", "   [%s] %s" ), target, _( hname ) );
            break;
         case HUNK_DATA_STRING:
            WARN( p_( "unidiff", "   [%s] %s: %s" ), target, _( hname ),
                  fail->u.name );
            break;
         case HUNK_DATA_INT:
            WARN( p_( "unidiff", "   [%s] %s: %d" ), target, _( hname ),
                  fail->u.data );
            break;
         case HUNK_DATA_FLOAT:
            WARN( p_( "unidiff", "   [%s] %s: %f" ), target, _( hname ),
                  fail->u.fdata );
            break;

         default:
            WARN( p_( "unidiff", "   [%s] %s: UNKNOWN DATA" ), target,
                  _( hname ) );
         }
      }
   }

   /* Update overlay map just in case. */
   ovr_refresh();

   /* Update universe. */
   if ( oneshot )
      diff_checkUpdateUniverse();

   return 0;
}

/**
 * @brief Starts applying a set of diffs.
 */
void diff_start( void )
{
   diff_universe_changed = 0;
}

/**
 * @brief Cleans up after applying a set of diffs.
 */
void diff_end( void )
{
   diff_checkUpdateUniverse();
}

/**
 * @brief Parses the attributes.
 */
static void diff_parseAttr( UniHunk_t *hunk, xmlNodePtr node )
{
   const char *const *attrs     = hunk_prop[hunk->type].attrs;
   xmlAttr           *attribute = node->properties;
   while ( attribute && attribute->name && attribute->children ) {
      UniAttribute_t attr;
      int            found = 0;

      /* Check if we are interested in the type. */
      if ( attrs != NULL ) {
         for ( int i = 0; attrs[i] != 0; i++ ) {
            if ( strcmp( (const char *)attribute->name, attrs[i] ) == 0 ) {
               found = 1;
               break;
            }
         }
      }
      if ( !found ) {
         WARN( _( "Unidiff hunk '%s' has unkown attribute '%s'" ),
               hunk_prop[hunk->type].name, attribute->name );
         attribute = attribute->next;
         continue;
      }

      attr.name = strdup( (const char *)attribute->name );
      attr.value =
         (char *)xmlNodeListGetString( node->doc, attribute->children, 1 );
      if ( hunk->attr == NULL )
         hunk->attr = array_create( UniAttribute_t );
      array_push_back( &hunk->attr, attr );
      attribute = attribute->next;
   }
}

static const char *diff_getAttr( UniHunk_t *hunk, const char *name )
{
   for ( int i = 0; i < array_size( hunk->attr ); i++ )
      if ( strcmp( hunk->attr[i].name, name ) == 0 )
         return hunk->attr[i].value;
   return NULL;
}

static AsteroidAnchor *diff_getAsteroidsLabel( StarSystem *ssys,
                                               const char *label )
{
   for ( int i = 0; i < array_size( ssys->asteroids ); i++ ) {
      AsteroidAnchor *ast = &ssys->asteroids[i];
      if ( ast->label && strcmp( ast->label, label ) == 0 )
         return ast;
   }
   return NULL;
}

static AsteroidAnchor *diff_getAsteroids( StarSystem *ssys, UniHunk_t *hunk )
{
   const char *label = diff_getAttr( hunk, "label" );
   if ( label == NULL ) {
      WARN( _( "Hunk '%s' does not have a label attribute!" ),
            hunk_prop[hunk->type].name );
      return NULL;
   }
   AsteroidAnchor *ast = diff_getAsteroidsLabel( ssys, label );
   if ( ast != NULL )
      return ast;
   WARN( _( "Hunk '%s' can not find an asteroid field with label '%s' in "
            "system '%s'!" ),
         hunk_prop[hunk->type].name, label, ssys->name );
   return NULL;
}

/**
 * @brief Patches a system.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the system.
 *    @return 0 on success.
 */
static int diff_parseSystem( UniDiffData_t *diff, xmlNodePtr node )
{
   UniHunk_t  base, hunk;
   xmlNodePtr cur;

   /* Set the target. */
   memset( &base, 0, sizeof( UniHunk_t ) );
   base.target.type = HUNK_TARGET_SYSTEM;
   xmlr_attr_strd( node, "name", base.target.u.name );
   if ( base.target.u.name == NULL ) {
      WARN( _( "Unidiff '%s' has a system node without a 'name' tag, not "
               "applying." ),
            diff->name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );

      HUNK_STRD( HUNK_TYPE_SPOB_ADD );
      HUNK_STRD( HUNK_TYPE_SPOB_REMOVE );
      HUNK_STRD( HUNK_TYPE_VSPOB_ADD );
      HUNK_STRD( HUNK_TYPE_VSPOB_REMOVE );
      HUNK_STRD( HUNK_TYPE_JUMP_ADD );
      HUNK_STRD( HUNK_TYPE_JUMP_REMOVE );
      HUNK_STRD( HUNK_TYPE_SSYS_BACKGROUND );
      HUNK_STRD( HUNK_TYPE_SSYS_FEATURES );
      HUNK_FLOAT( HUNK_TYPE_SSYS_POS_X );
      HUNK_FLOAT( HUNK_TYPE_SSYS_POS_Y );
      HUNK_STRD( HUNK_TYPE_SSYS_DISPLAYNAME );
      HUNK_INT( HUNK_TYPE_SSYS_DUST );
      HUNK_FLOAT( HUNK_TYPE_SSYS_INTERFERENCE );
      HUNK_FLOAT( HUNK_TYPE_SSYS_NEBU_DENSITY );
      HUNK_FLOAT( HUNK_TYPE_SSYS_NEBU_VOLATILITY );
      HUNK_FLOAT( HUNK_TYPE_SSYS_NEBU_HUE );
      HUNK_NONE( HUNK_TYPE_SSYS_NOLANES_ADD );
      HUNK_NONE( HUNK_TYPE_SSYS_NOLANES_REMOVE );
      HUNK_STRD( HUNK_TYPE_SSYS_TAG_ADD );
      HUNK_STRD( HUNK_TYPE_SSYS_TAG_REMOVE );
      /* These below use labels to indicate the asteroid field. */
      HUNK_STRD( HUNK_TYPE_SSYS_ASTEROIDS_ADD );
      HUNK_STRD( HUNK_TYPE_SSYS_ASTEROIDS_REMOVE );
      HUNK_FLOAT( HUNK_TYPE_SSYS_ASTEROIDS_POS_X );
      HUNK_FLOAT( HUNK_TYPE_SSYS_ASTEROIDS_POS_Y );
      HUNK_FLOAT( HUNK_TYPE_SSYS_ASTEROIDS_DENSITY );
      HUNK_FLOAT( HUNK_TYPE_SSYS_ASTEROIDS_RADIUS );
      HUNK_FLOAT( HUNK_TYPE_SSYS_ASTEROIDS_MAXSPEED );
      HUNK_FLOAT( HUNK_TYPE_SSYS_ASTEROIDS_ACCEL );
      HUNK_STRD( HUNK_TYPE_SSYS_ASTEROIDS_ADD_TYPE );
      HUNK_STRD( HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_TYPE );

      WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name, cur->name );
   } while ( xml_nextNode( cur ) );

   /* Clean up some stuff. */
   free( base.target.u.name );
   base.target.u.name = NULL;

   return 0;
}

/**
 * @brief Patches a tech.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the tech.
 *    @return 0 on success.
 */
static int diff_parseTech( UniDiffData_t *diff, xmlNodePtr node )
{
   UniHunk_t  base, hunk;
   xmlNodePtr cur;

   /* Set the target. */
   memset( &base, 0, sizeof( UniHunk_t ) );
   base.target.type = HUNK_TARGET_TECH;
   xmlr_attr_strd( node, "name", base.target.u.name );
   if ( base.target.u.name == NULL ) {
      WARN( _( "Unidiff '%s' has an target node without a 'name' tag" ),
            diff->name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );

      HUNK_STRD( HUNK_TYPE_TECH_ADD );
      HUNK_STRD( HUNK_TYPE_TECH_REMOVE );

      WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name, cur->name );
   } while ( xml_nextNode( cur ) );

   /* Clean up some stuff. */
   free( base.target.u.name );
   base.target.u.name = NULL;

   return 0;
}

/**
 * @brief Patches a spob.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the spob.
 *    @return 0 on success.
 */
static int diff_parseSpob( UniDiffData_t *diff, xmlNodePtr node )
{
   UniHunk_t  base, hunk;
   xmlNodePtr cur;

   /* Set the target. */
   memset( &base, 0, sizeof( UniHunk_t ) );
   base.target.type = HUNK_TARGET_SPOB;
   xmlr_attr_strd( node, "name", base.target.u.name );
   if ( base.target.u.name == NULL ) {
      WARN( _( "Unidiff '%s' has an target node without a 'name' tag" ),
            diff->name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );

      HUNK_FLOAT( HUNK_TYPE_SPOB_POS_X );
      HUNK_FLOAT( HUNK_TYPE_SPOB_POS_Y );
      HUNK_STRD( HUNK_TYPE_SPOB_CLASS );
      HUNK_STRD( HUNK_TYPE_SPOB_FACTION );
      HUNK_FLOAT( HUNK_TYPE_SPOB_PRESENCE_BASE );
      HUNK_FLOAT( HUNK_TYPE_SPOB_PRESENCE_BONUS );
      HUNK_INT( HUNK_TYPE_SPOB_PRESENCE_RANGE );
      HUNK_FLOAT( HUNK_TYPE_SPOB_HIDE );
      HUNK_INT( HUNK_TYPE_SPOB_POPULATION );
      HUNK_STRD( HUNK_TYPE_SPOB_DISPLAYNAME );
      HUNK_STRD( HUNK_TYPE_SPOB_DESCRIPTION );
      HUNK_STRD( HUNK_TYPE_SPOB_BAR );
      HUNK_STRD( HUNK_TYPE_SPOB_SERVICE_ADD );
      HUNK_STRD( HUNK_TYPE_SPOB_SERVICE_REMOVE );
      HUNK_NONE( HUNK_TYPE_SPOB_NOMISNSPAWN_ADD );
      HUNK_NONE( HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE );
      HUNK_STRD( HUNK_TYPE_SPOB_TECH_ADD );
      HUNK_STRD( HUNK_TYPE_SPOB_TECH_REMOVE );
      HUNK_STRD( HUNK_TYPE_SPOB_TAG_ADD );
      HUNK_STRD( HUNK_TYPE_SPOB_TAG_REMOVE );
      HUNK_CUST( HUNK_TYPE_SPOB_SPACE, HUNK_DATA_STRING, char str[PATH_MAX];
                 snprintf( str, sizeof( str ), SPOB_GFX_SPACE_PATH "%s",
                           xml_get( cur ) );
                 hunk.u.name = strdup( str ); );
      HUNK_CUST( HUNK_TYPE_SPOB_EXTERIOR, HUNK_DATA_STRING, char str[PATH_MAX];
                 snprintf( str, sizeof( str ), SPOB_GFX_EXTERIOR_PATH "%s",
                           xml_get( cur ) );
                 hunk.u.name = strdup( str ); );
      HUNK_STRD( HUNK_TYPE_SPOB_LUA );

      // cppcheck-suppress nullPointerRedundantCheck
      WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name, cur->name );
   } while ( xml_nextNode( cur ) );

   /* Clean up some stuff. */
   free( base.target.u.name );
   base.target.u.name = NULL;

   return 0;
}

/**
 * @brief Patches a faction.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the spob.
 *    @return 0 on success.
 */
static int diff_parseFaction( UniDiffData_t *diff, xmlNodePtr node )
{
   UniHunk_t  base, hunk;
   xmlNodePtr cur;

   /* Set the target. */
   memset( &base, 0, sizeof( UniHunk_t ) );
   base.target.type = HUNK_TARGET_FACTION;
   xmlr_attr_strd( node, "name", base.target.u.name );
   if ( base.target.u.name == NULL ) {
      WARN( _( "Unidiff '%s' has an target node without a 'name' tag" ),
            diff->name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );

      HUNK_NONE( HUNK_TYPE_FACTION_VISIBLE );
      HUNK_NONE( HUNK_TYPE_FACTION_INVISIBLE );
      HUNK_STRD( HUNK_TYPE_FACTION_ALLY );
      HUNK_STRD( HUNK_TYPE_FACTION_ENEMY );
      HUNK_STRD( HUNK_TYPE_FACTION_NEUTRAL );

      WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name, cur->name );
   } while ( xml_nextNode( cur ) );

   /* Clean up some stuff. */
   free( base.target.u.name );
   base.target.u.name = NULL;

   return 0;
}

/**
 * @brief Reverts a hunk.
 *
 *    @param hunk Hunk to revert.
 *    @return 0 on success.
 */
int diff_revertHunk( const UniHunk_t *hunk )
{
   UniHunk_t rhunk = *hunk;
   rhunk.type      = hunk_prop[hunk->type].reverse;
   return diff_patchHunk( &rhunk );
}

/**
 * @brief Applies a hunk.
 *
 *    @param hunk Hunk to apply.
 *    @return 0 on success.
 */
int diff_patchHunk( UniHunk_t *hunk )
{
   Spob       *p     = NULL;
   StarSystem *ssys  = NULL;
   StarSystem *ssys2 = NULL;
   int         a, b;
   int         f = -1;

   /* Common loading target bit to simplify code below. */
   switch ( hunk->target.type ) {
   case HUNK_TARGET_SYSTEM:
      ssys = system_get( hunk->target.u.name );
      if ( ssys == NULL )
         return -1;
      break;
   case HUNK_TARGET_SPOB:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      break;
   case HUNK_TARGET_FACTION:
      f = faction_get( hunk->target.u.name );
      if ( f < 0 )
         return -1;
      break;
   case HUNK_TARGET_NONE:
   case HUNK_TARGET_TECH:
      break;
   }

   switch ( hunk->type ) {
   /* Adding an spob. */
   case HUNK_TYPE_SPOB_ADD:
      p = spob_get( hunk->u.name );
      if ( p == NULL )
         return -1;
      spob_luaInit( p );
      diff_universe_changed = 1;
      return system_addSpob( ssys, hunk->u.name );
   /* Removing an spob. */
   case HUNK_TYPE_SPOB_REMOVE:
      diff_universe_changed = 1;
      return system_rmSpob( ssys, hunk->u.name );

   /* Adding a virtual spob. */
   case HUNK_TYPE_VSPOB_ADD:
      diff_universe_changed = 1;
      return system_addVirtualSpob( ssys, hunk->u.name );
   /* Removing a virtual spob. */
   case HUNK_TYPE_VSPOB_REMOVE:
      diff_universe_changed = 1;
      return system_rmVirtualSpob( ssys, hunk->u.name );

   /* Adding a jump. */
   case HUNK_TYPE_JUMP_ADD:
      ssys2 = system_get( hunk->u.name );
      if ( ssys2 == NULL )
         return -1;
      diff_universe_changed = 1;
      if ( system_addJump( ssys, ssys2 ) )
         return -1;
      if ( system_addJump( ssys2, ssys ) )
         return -1;
      return 0;
   /* Removing a jump. */
   case HUNK_TYPE_JUMP_REMOVE:
      ssys2 = system_get( hunk->u.name );
      if ( ssys2 == NULL )
         return -1;
      diff_universe_changed = 1;
      if ( system_rmJump( ssys, ssys2 ) )
         return -1;
      if ( system_rmJump( ssys2, ssys ) )
         return -1;
      return 0;

   /* Changing system background. */
   case HUNK_TYPE_SSYS_BACKGROUND:
      hunk->o.name     = ssys->background;
      ssys->background = hunk->u.name;
      return 0;
   case HUNK_TYPE_SSYS_BACKGROUND_REVERT:
      ssys->background = (char *)hunk->o.name;
      return 0;

   /* Changing system features designation. */
   case HUNK_TYPE_SSYS_FEATURES:
      hunk->o.name   = ssys->features;
      ssys->features = hunk->u.name;
      return 0;
   case HUNK_TYPE_SSYS_FEATURES_REVERT:
      ssys->features = (char *)hunk->o.name;
      return 0;

   /* Position changes. */
   case HUNK_TYPE_SSYS_POS_X:
      hunk->o.fdata = ssys->pos.x;
      ssys->pos.x   = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SSYS_POS_X_REVERT:
      ssys->pos.x = hunk->o.fdata;
      return 0;
   case HUNK_TYPE_SSYS_POS_Y:
      hunk->o.fdata = ssys->pos.y;
      ssys->pos.y   = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SSYS_POS_Y_REVERT:
      ssys->pos.y = hunk->o.fdata;
      return 0;

   /* Displayname. */
   case HUNK_TYPE_SSYS_DISPLAYNAME:
      hunk->o.name  = ssys->display;
      ssys->display = hunk->u.name;
      return 0;
   case HUNK_TYPE_SSYS_DISPLAYNAME_REVERT:
      ssys->display = (char *)hunk->o.name;
      return 0;

   /* Dust. */
   case HUNK_TYPE_SSYS_DUST:
      hunk->o.data    = ssys->spacedust;
      ssys->spacedust = hunk->u.data;
      return 0;
   case HUNK_TYPE_SSYS_DUST_REVERT:
      ssys->spacedust = hunk->o.data;
      return 0;

   /* Interefrence. */
   case HUNK_TYPE_SSYS_INTERFERENCE:
      hunk->o.data       = ssys->interference;
      ssys->interference = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SSYS_INTERFERENCE_REVERT:
      ssys->interference = hunk->o.fdata;
      return 0;

   /* Nebula density. */
   case HUNK_TYPE_SSYS_NEBU_DENSITY:
      hunk->o.data       = ssys->nebu_density;
      ssys->nebu_density = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SSYS_NEBU_DENSITY_REVERT:
      ssys->nebu_density = hunk->o.fdata;
      return 0;

   /* Nebula volatility. */
   case HUNK_TYPE_SSYS_NEBU_VOLATILITY:
      hunk->o.data          = ssys->nebu_volatility;
      ssys->nebu_volatility = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SSYS_NEBU_VOLATILITY_REVERT:
      ssys->nebu_volatility = hunk->o.fdata;
      return 0;

   /* Nebula hue. */
   case HUNK_TYPE_SSYS_NEBU_HUE:
      hunk->o.data   = ssys->nebu_hue;
      ssys->nebu_hue = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SSYS_NEBU_HUE_REVERT:
      ssys->nebu_hue = hunk->o.fdata;
      return 0;

   /* Toggle nolanes flag. */
   case HUNK_TYPE_SSYS_NOLANES_ADD:
      if ( sys_isFlag( ssys, SYSTEM_NOLANES ) )
         return -1;
      sys_setFlag( ssys, SYSTEM_NOLANES );
      return 0;
   case HUNK_TYPE_SSYS_NOLANES_REMOVE:
      if ( !sys_isFlag( ssys, SYSTEM_NOLANES ) )
         return -1;
      sys_rmFlag( ssys, SYSTEM_NOLANES );
      return 0;

      /* Modifying tag stuff. */
   case HUNK_TYPE_SSYS_TAG_ADD:
      if ( ssys->tags == NULL )
         ssys->tags = array_create( char * );
      array_push_back( &ssys->tags, strdup( hunk->u.name ) );
      return 0;
   case HUNK_TYPE_SSYS_TAG_REMOVE:
      a = -1;
      for ( int i = 0; i < array_size( ssys->tags ); i++ ) {
         if ( strcmp( ssys->tags[i], hunk->u.name ) == 0 ) {
            a = i;
            break;
         }
      }
      if ( a < 0 )
         return -1; /* Didn't find tag! */
      free( ssys->tags[a] );
      array_erase( &ssys->tags, &ssys->tags[a], &ssys->tags[a + 1] );
      return 0;

   /* Adding an asteroid field. */
   case HUNK_TYPE_SSYS_ASTEROIDS_ADD: {
      AsteroidAnchor ast;
      asteroid_initAnchor( &ast );
      ast.label = strdup( hunk->u.name );
      asteroids_computeInternals( &ast );
      array_push_back( &ssys->asteroids, ast );
   }
      return 0;
   case HUNK_TYPE_SSYS_ASTEROIDS_ADD_REVERT: {
      AsteroidAnchor *ast = diff_getAsteroidsLabel( ssys, hunk->u.name );
      if ( ast != NULL ) {
         asteroid_freeAnchor( ast );
         array_erase( &ssys->asteroids, ast, ast + 1 );
         return 0;
      } else
         WARN( _( "Hunk '%s' can not find an asteroid field with label '%s' in "
                  "system '%s'!" ),
               diff_hunkName( hunk->type ), hunk->u.name, ssys->name );
   }
      return -1;
   /* Removing an asteroid field. */
   case HUNK_TYPE_SSYS_ASTEROIDS_REMOVE: {
      AsteroidAnchor *ast = diff_getAsteroidsLabel( ssys, hunk->u.name );
      if ( ast != NULL ) {
         hunk->o.ptr = ast;
         array_erase( &ssys->asteroids, ast, ast + 1 );
         return 0;
      } else
         WARN( _( "Hunk '%s' can not find an asteroid field with label '%s' in "
                  "system '%s'!" ),
               diff_hunkName( hunk->type ), hunk->u.name, ssys->name );
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_REVERT:
      if ( hunk->o.ptr != NULL ) {
         AsteroidAnchor *ast = hunk->o.ptr;
         array_push_back( &ssys->asteroids, *ast );
         return 0;
      }
      return -1;
   /* Position for asteroid field. */
   case HUNK_TYPE_SSYS_ASTEROIDS_POS_X: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         hunk->o.fdata = ast->pos.x;
         ast->pos.x    = hunk->u.fdata;
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_POS_X_REVERT: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         ast->pos.x = hunk->o.fdata;
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_POS_Y: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         hunk->o.fdata = ast->pos.y;
         ast->pos.y    = hunk->u.fdata;
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_POS_Y_REVERT: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         ast->pos.y = hunk->o.fdata;
         return 0;
      }
   }
      return -1;
   /* Asteroid properties. */
   case HUNK_TYPE_SSYS_ASTEROIDS_DENSITY: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         hunk->o.fdata = ast->density;
         ast->density  = hunk->u.fdata;
         asteroids_computeInternals( ast );
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_DENSITY_REVERT: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         ast->density = hunk->o.fdata;
         asteroids_computeInternals( ast );
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_RADIUS: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         hunk->o.fdata = ast->radius;
         ast->radius   = hunk->u.fdata;
         asteroids_computeInternals( ast );
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_RADIUS_REVERT: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         ast->radius = hunk->o.fdata;
         asteroids_computeInternals( ast );
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_MAXSPEED: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         hunk->o.fdata = ast->maxspeed;
         ast->maxspeed = hunk->u.fdata;
         asteroids_computeInternals( ast );
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_MAXSPEED_REVERT: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         ast->maxspeed = hunk->o.fdata;
         asteroids_computeInternals( ast );
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_ACCEL: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         hunk->o.fdata = ast->accel;
         ast->accel    = hunk->u.fdata;
         asteroids_computeInternals( ast );
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_ACCEL_REVERT: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         ast->accel = hunk->o.fdata;
         asteroids_computeInternals( ast );
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_ADD_TYPE: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         AsteroidTypeGroup *grp = astgroup_getName( hunk->u.name );
         if ( grp == NULL )
            return -1;
         for ( int i = 0; i < array_size( ast->groups ); i++ ) {
            if ( strcmp( grp->name, ast->groups[i]->name ) == 0 ) {
               WARN( _( "Unidiff '%s' trying to add already existing asteroid "
                        "type '%s'." ),
                     diff_hunkName( hunk->type ), grp->name );
               return -1;
            }
         }
         array_push_back( &ast->groups, grp );
         return 0;
      }
   }
      return -1;
   case HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_TYPE: {
      AsteroidAnchor *ast = diff_getAsteroids( ssys, hunk );
      if ( ast != NULL ) {
         AsteroidTypeGroup *grp = astgroup_getName( hunk->u.name );
         if ( grp == NULL )
            return -1;
         for ( int i = 0; i < array_size( ast->groups ); i++ ) {
            if ( strcmp( grp->name, ast->groups[i]->name ) == 0 ) {
               array_erase( &ast->groups, &ast->groups[i],
                            &ast->groups[i + 1] );
               return 0;
            }
         }
         WARN(
            _( "Unidiff '%s' trying to remove inexistent asteroid type '%s'." ),
            diff_hunkName( hunk->type ), grp->name );
      }
   }
      return -1;

   /* Adding a tech. */
   case HUNK_TYPE_TECH_ADD:
      return tech_addItem( hunk->target.u.name, hunk->u.name );
   /* Removing a tech. */
   case HUNK_TYPE_TECH_REMOVE:
      return tech_rmItem( hunk->target.u.name, hunk->u.name );

   /* Position changes. */
   case HUNK_TYPE_SPOB_POS_X:
      hunk->o.fdata = p->pos.x;
      p->pos.x      = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SPOB_POS_X_REVERT:
      p->pos.x = hunk->o.fdata;
      return 0;
   case HUNK_TYPE_SPOB_POS_Y:
      hunk->o.fdata = p->pos.y;
      p->pos.y      = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SPOB_POS_Y_REVERT:
      p->pos.y = hunk->o.fdata;
      return 0;

   /* Changing spob faction. */
   case HUNK_TYPE_SPOB_CLASS:
      hunk->o.name = p->class;
      p->class     = hunk->u.name;
      return 0;
   case HUNK_TYPE_SPOB_CLASS_REVERT:
      p->class = (char *)hunk->o.name;
      return 0;

   /* Changing spob faction. */
   case HUNK_TYPE_SPOB_FACTION:
      if ( p->presence.faction < 0 )
         hunk->o.name = NULL;
      else
         hunk->o.name = faction_name( p->presence.faction );
      diff_universe_changed = 1;
      /* Special case to clear the faction. */
      if ( SDL_strcasecmp( hunk->u.name, "None" ) == 0 )
         return spob_setFaction( p, -1 );
      else
         return spob_setFaction( p, faction_get( hunk->u.name ) );
   case HUNK_TYPE_SPOB_FACTION_REVERT:
      diff_universe_changed = 1;
      if ( hunk->o.name == NULL )
         return spob_setFaction( p, -1 );
      else
         return spob_setFaction( p, faction_get( hunk->o.name ) );

   /* Presence stuff. */
   case HUNK_TYPE_SPOB_PRESENCE_BASE:
      diff_universe_changed = 1;
      hunk->o.fdata         = p->presence.base;
      p->presence.base      = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SPOB_PRESENCE_BASE_REVERT:
      diff_universe_changed = 1;
      p->presence.base      = hunk->o.fdata;
      return 0;
   case HUNK_TYPE_SPOB_PRESENCE_BONUS:
      diff_universe_changed = 1;
      hunk->o.fdata         = p->presence.bonus;
      p->presence.bonus     = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SPOB_PRESENCE_BONUS_REVERT:
      diff_universe_changed = 1;
      p->presence.bonus     = hunk->o.fdata;
      return 0;
   case HUNK_TYPE_SPOB_PRESENCE_RANGE:
      diff_universe_changed = 1;
      hunk->o.data          = p->presence.range;
      p->presence.range     = hunk->u.data;
      return 0;
   case HUNK_TYPE_SPOB_PRESENCE_RANGE_REVERT:
      diff_universe_changed = 1;
      p->presence.range     = hunk->o.data;
      return 0;

   /* Changing spob hide. */
   case HUNK_TYPE_SPOB_HIDE:
      hunk->o.fdata = p->hide;
      p->hide       = hunk->u.fdata;
      return 0;
   case HUNK_TYPE_SPOB_HIDE_REVERT:
      p->hide = hunk->o.fdata;
      return 0;

   /* Changing spob population. */
   case HUNK_TYPE_SPOB_POPULATION:
      hunk->o.fdata = p->population;
      p->population = hunk->u.data;
      return 0;
   case HUNK_TYPE_SPOB_POPULATION_REVERT:
      p->population = hunk->o.fdata;
      return 0;

   /* Changing spob displayname. */
   case HUNK_TYPE_SPOB_DISPLAYNAME:
      hunk->o.name = p->display;
      p->display   = hunk->u.name;
      return 0;
   case HUNK_TYPE_SPOB_DISPLAYNAME_REVERT:
      p->display = (char *)hunk->o.name;
      return 0;

   /* Changing spob description. */
   case HUNK_TYPE_SPOB_DESCRIPTION:
      hunk->o.name   = p->description;
      p->description = hunk->u.name;
      return 0;
   case HUNK_TYPE_SPOB_DESCRIPTION_REVERT:
      p->description = (char *)hunk->o.name;
      return 0;

   /* Changing spob bar description. */
   case HUNK_TYPE_SPOB_BAR:
      hunk->o.name       = p->bar_description;
      p->bar_description = hunk->u.name;
      return 0;
   case HUNK_TYPE_SPOB_BAR_REVERT:
      p->bar_description = (char *)hunk->o.name;
      return 0;

   /* Modifying spob services. */
   case HUNK_TYPE_SPOB_SERVICE_ADD:
      a = spob_getService( hunk->u.name );
      if ( a < 0 )
         return -1;
      if ( spob_hasService( p, a ) )
         return -1;
      spob_addService( p, a );
      diff_universe_changed = 1;
      return 0;
   case HUNK_TYPE_SPOB_SERVICE_REMOVE:
      a = spob_getService( hunk->u.name );
      if ( a < 0 )
         return -1;
      if ( !spob_hasService( p, a ) )
         return -1;
      spob_rmService( p, a );
      diff_universe_changed = 1;
      return 0;

   /* Modifying mission spawn. */
   case HUNK_TYPE_SPOB_NOMISNSPAWN_ADD:
      if ( spob_isFlag( p, SPOB_NOMISNSPAWN ) )
         return -1;
      spob_setFlag( p, SPOB_NOMISNSPAWN );
      return 0;
   case HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE:
      if ( !spob_isFlag( p, SPOB_NOMISNSPAWN ) )
         return -1;
      spob_rmFlag( p, SPOB_NOMISNSPAWN );
      return 0;

   /* Modifying tech stuff. */
   case HUNK_TYPE_SPOB_TECH_ADD:
      if ( p->tech == NULL )
         p->tech = tech_groupCreate();
      tech_addItemTech( p->tech, hunk->u.name );
      return 0;
   case HUNK_TYPE_SPOB_TECH_REMOVE:
      tech_rmItemTech( p->tech, hunk->u.name );
      return 0;

   /* Modifying tag stuff. */
   case HUNK_TYPE_SPOB_TAG_ADD:
      if ( p->tags == NULL )
         p->tags = array_create( char * );
      array_push_back( &p->tags, strdup( hunk->u.name ) );
      return 0;
   case HUNK_TYPE_SPOB_TAG_REMOVE:
      a = -1;
      for ( int i = 0; i < array_size( p->tags ); i++ ) {
         if ( strcmp( p->tags[i], hunk->u.name ) == 0 ) {
            a = i;
            break;
         }
      }
      if ( a < 0 )
         return -1; /* Didn't find tag! */
      free( p->tags[a] );
      array_erase( &p->tags, &p->tags[a], &p->tags[a + 1] );
      return 0;

   /* Changing spob space graphics. */
   case HUNK_TYPE_SPOB_SPACE:
      hunk->o.name          = p->gfx_spaceName;
      p->gfx_spaceName      = hunk->u.name;
      diff_universe_changed = 1;
      return 0;
   case HUNK_TYPE_SPOB_SPACE_REVERT:
      p->gfx_spaceName      = (char *)hunk->o.name;
      diff_universe_changed = 1;
      return 0;

   /* Changing spob exterior graphics. */
   case HUNK_TYPE_SPOB_EXTERIOR:
      hunk->o.name    = p->gfx_exterior;
      p->gfx_exterior = hunk->u.name;
      return 0;
   case HUNK_TYPE_SPOB_EXTERIOR_REVERT:
      p->gfx_exterior = (char *)hunk->o.name;
      return 0;

   /* Change Lua stuff. */
   case HUNK_TYPE_SPOB_LUA:
      hunk->o.name = p->lua_file;
      p->lua_file  = hunk->u.name;
      spob_luaInit( p );
      diff_universe_changed = 1;
      return 0;
   case HUNK_TYPE_SPOB_LUA_REVERT:
      p->lua_file = (char *)hunk->o.name;
      spob_luaInit( p );
      diff_universe_changed = 1;
      return 0;

   /* Making a faction visible. */
   case HUNK_TYPE_FACTION_VISIBLE:
      return faction_setInvisible( f, 0 );
   /* Making a faction invisible. */
   case HUNK_TYPE_FACTION_INVISIBLE:
      return faction_setInvisible( f, 1 );
   /* Making two factions allies. */
   case HUNK_TYPE_FACTION_ALLY:
      a = f;
      b = faction_get( hunk->u.name );
      if ( areAllies( a, b ) )
         hunk->o.data = 'A';
      else if ( areEnemies( a, b ) )
         hunk->o.data = 'E';
      else
         hunk->o.data = 0;
      faction_addAlly( a, b );
      faction_addAlly( b, a );
      return 0;
   /* Making two factions enemies. */
   case HUNK_TYPE_FACTION_ENEMY:
      a = f;
      b = faction_get( hunk->u.name );
      if ( b < 0 )
         return -1;
      if ( areAllies( a, b ) )
         hunk->o.data = 'A';
      else if ( areEnemies( a, b ) )
         hunk->o.data = 'E';
      else if ( areNeutral( a, b ) )
         hunk->o.data = 'N';
      else
         hunk->o.data = 0;
      faction_addEnemy( a, b );
      faction_addEnemy( b, a );
      return 0;
   /* Making two factions neutral (removing enemy/ally statuses). */
   case HUNK_TYPE_FACTION_NEUTRAL:
      a = f;
      b = faction_get( hunk->u.name );
      if ( b < 0 )
         return -1;
      if ( areAllies( a, b ) )
         hunk->o.data = 'A';
      else if ( areEnemies( a, b ) )
         hunk->o.data = 'E';
      else if ( areNeutral( a, b ) )
         hunk->o.data = 'N';
      else
         hunk->o.data = 0;
      faction_rmAlly( a, b );
      faction_rmAlly( b, a );
      faction_rmEnemy( a, b );
      faction_rmEnemy( b, a );
      return 0;
   /* Resetting the alignment state of two factions. */
   case HUNK_TYPE_FACTION_REALIGN:
      a = f;
      b = faction_get( hunk->u.name );
      if ( b < 0 )
         return -1;
      if ( hunk->o.data == 'A' ) {
         faction_rmNeutral( a, b );
         faction_rmNeutral( b, a );
         faction_rmEnemy( a, b );
         faction_rmEnemy( b, a );
         faction_addAlly( a, b );
         faction_addAlly( b, a );
      } else if ( hunk->o.data == 'E' ) {
         faction_rmNeutral( a, b );
         faction_rmNeutral( b, a );
         faction_rmAlly( a, b );
         faction_rmAlly( b, a );
         faction_addEnemy( a, b );
         faction_addEnemy( b, a );
      } else if ( hunk->o.data == 'N' ) {
         faction_rmAlly( a, b );
         faction_rmAlly( b, a );
         faction_rmEnemy( a, b );
         faction_rmEnemy( b, a );
         faction_addNeutral( a, b );
         faction_addNeutral( b, a );
      } else {
         faction_rmNeutral( a, b );
         faction_rmNeutral( b, a );
         faction_rmAlly( a, b );
         faction_rmAlly( b, a );
         faction_rmEnemy( a, b );
         faction_rmEnemy( b, a );
      }
      return 0;

   case HUNK_TYPE_NONE:
   case HUNK_TYPE_SENTINAL:
      break;
   }
   if ( diff_hunkName( hunk->type ) != NULL )
      WARN( _( "Unknown hunk type '%s'." ), diff_hunkName( hunk->type ) );
   else
      WARN( _( "Unknown hunk type '%d'." ), hunk->type );
   return -1;
}

/**
 * @brief Adds a hunk to the failed list.
 *
 *    @param diff Diff to add hunk to.
 *    @param hunk Hunk that failed to apply.
 */
static void diff_hunkFailed( UniDiff_t *diff, const UniHunk_t *hunk )
{
   if ( diff == NULL )
      return;
   if ( diff->failed == NULL )
      diff->failed = array_create( UniHunk_t );
   array_grow( &diff->failed ) = *hunk;
}

/**
 * @brief Adds a hunk to the applied list.
 *
 *    @param diff Diff to add hunk to.
 *    @param hunk Hunk that applied correctly.
 */
static void diff_hunkSuccess( UniDiff_t *diff, const UniHunk_t *hunk )
{
   if ( diff == NULL )
      return;
   if ( diff->applied == NULL )
      diff->applied = array_create( UniHunk_t );
   array_grow( &diff->applied ) = *hunk;
}

/**
 * @brief Removes a diff from the universe.
 *
 *    @param name Diff to remove.
 */
void diff_remove( const char *name )
{
   /* Check if already applied. */
   UniDiff_t *diff = diff_get( name );
   if ( diff == NULL )
      return;

   diff_removeDiff( diff );

   diff_checkUpdateUniverse();
}

/**
 * @brief Removes all active diffs. (Call before economy_destroy().)
 */
void diff_clear( void )
{
   /* Remove in reverse order. */
   while ( array_size( diff_stack ) > 0 )
      diff_removeDiff( &diff_stack[array_size( diff_stack ) - 1] );
   array_free( diff_stack );
   diff_stack = NULL;

   diff_checkUpdateUniverse();
}

/**
 * @brief Creates a new UniDiff_t for usage.
 *
 *    @return A newly created UniDiff_t.
 */
static UniDiff_t *diff_newDiff( void )
{
   /* Check if needs initialization. */
   if ( diff_stack == NULL )
      diff_stack = array_create( UniDiff_t );
   return &array_grow( &diff_stack );
}

/**
 * @brief Removes a diff.
 *
 *    @param diff Diff to remove.
 *    @return 0 on success.
 */
static int diff_removeDiff( UniDiff_t *diff )
{
   /* Remove hunks in reverse order. */
   for ( int i = array_size( diff->applied ) - 1; i >= 0; i-- ) {
      UniHunk_t hunk = diff->applied[i];
      if ( diff_revertHunk( &hunk ) )
         WARN( _( "Failed to remove hunk type '%s'." ),
               hunk_prop[hunk.type].name );
   }

   diff_cleanup( diff );
   array_erase( &diff_stack, diff, &diff[1] );
   return 0;
}

/**
 * @brief Cleans up a diff.
 *
 *    @param diff Diff to clean up.
 */
static void diff_cleanup( UniDiff_t *diff )
{
   for ( int i = 0; i < array_size( diff->applied ); i++ )
      diff_cleanupHunk( &diff->applied[i] );
   array_free( diff->applied );
   for ( int i = 0; i < array_size( diff->failed ); i++ )
      diff_cleanupHunk( &diff->failed[i] );
   array_free( diff->failed );
   memset( diff, 0, sizeof( UniDiff_t ) );
}

/**
 * @brief Gets the human readable name of a hunk.
 */
const char *diff_hunkName( UniHunkType_t t )
{
   return hunk_prop[t].name;
}

/**
 * @brief Gets the XML tag of a hunk.
 */
const char *diff_hunkTag( UniHunkType_t t )
{
   return hunk_prop[t].tag;
}

/**
 * @brief Cleans up a hunk.
 *
 *    @param hunk Hunk to clean up.
 */
void diff_cleanupHunk( UniHunk_t *hunk )
{
   free( hunk->target.u.name );
   if ( hunk->dtype == HUNK_DATA_STRING )
      free( hunk->u.name );
   for ( int i = 0; i < array_size( hunk->attr ); i++ ) {
      UniAttribute_t *attr = &hunk->attr[i];
      free( attr->name );
      free( attr->value );
   }
   array_free( hunk->attr );
   memset( hunk, 0, sizeof( UniHunk_t ) );
}

/**
 * @brief Saves the active diffs.
 *
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int diff_save( xmlTextWriterPtr writer )
{
   xmlw_startElem( writer, "diffs" );
   if ( diff_stack != NULL ) {
      for ( int i = 0; i < array_size( diff_stack ); i++ ) {
         const UniDiff_t *diff = &diff_stack[i];
         xmlw_elem( writer, "diff", "%s", diff->data->name );
      }
   }
   xmlw_endElem( writer ); /* "diffs" */
   return 0;
}

/**
 * @brief Loads the diffs.
 *
 *    @param parent Parent node containing diffs.
 *    @return 0 on success.
 */
int diff_load( xmlNodePtr parent )
{
   xmlNodePtr node;
   int        defer = diff_universe_defer;

   /* Don't update universe here. */
   diff_universe_defer   = 1;
   diff_universe_changed = 0;
   diff_nav_spob         = NULL;
   diff_nav_hyperspace   = NULL;
   diff_clear();
   diff_universe_defer = defer;

   node = parent->xmlChildrenNode;
   do {
      if ( xml_isNode( node, "diffs" ) ) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do {
            if ( xml_isNode( cur, "diff" ) ) {
               const char *diffName = xml_get( cur );
               if ( diffName == NULL ) {
                  WARN( _( "Expected node \"diff\" to contain the name of a "
                           "unidiff. Was empty." ) );
                  continue;
               }
               if ( diff_applyInternal( diffName, 0, 0 ) ) {
                  if ( player_runUpdaterScript( "unidiff", diffName, 0 ) !=
                       0 ) {
                     if ( lua_type( naevL, -1 ) == LUA_TSTRING )
                        diff_applyInternal( lua_tostring( naevL, -1 ), 0, 1 );
                     else if ( lua_type( naevL, -1 ) != LUA_TNIL )
                        WARN( "Invalid value returned from 'unidiff' in "
                              "'save_updater.lua'" );
                     lua_pop( naevL, 1 );
                  }
               }
               continue;
            }
         } while ( xml_nextNode( cur ) );
      }
   } while ( xml_nextNode( node ) );

   /* Update as necessary. */
   diff_checkUpdateUniverse();

   return 0;
}

/**
 * @brief Checks and updates the universe if necessary.
 */
static int diff_checkUpdateUniverse( void )
{
   Pilot *const *pilots;

   if ( !diff_universe_changed || diff_universe_defer )
      return 0;

   /* Reconstruct jumps just in case. */
   systems_reconstructJumps();
   /* Update presences, then safelanes. */
   space_reconstructPresences();
   safelanes_recalculate();

   /* Re-compute the economy. */
   economy_execQueued();
   economy_initialiseCommodityPrices();

   /* Have to update planet graphics if necessary. */
   if ( cur_system != NULL ) {
      space_gfxUnload( cur_system );
      space_gfxLoad( cur_system );
   }

   /* Have to pilot targetting just in case. */
   pilots = pilot_getAll();
   for ( int i = 0; i < array_size( pilots ); i++ ) {
      Pilot *p          = pilots[i];
      p->nav_spob       = -1;
      p->nav_hyperspace = -1;

      /* Hack in case the pilot was actively jumping, this won't run the hook,
       * but I guess it's too much effort to properly fix for a situation that
       * will likely never happen. */
      if ( !pilot_isWithPlayer( p ) && pilot_isFlag( p, PILOT_HYPERSPACE ) )
         pilot_delete( p );
      else
         pilot_rmFlag(
            p, PILOT_HYPERSPACE ); /* Corner case player, just have it not crash
                                      and randomly stop the jump. */

      /* Have to reset in the case of starting. */
      if ( pilot_isFlag( p, PILOT_HYP_BEGIN ) ||
           pilot_isFlag( p, PILOT_HYP_BRAKE ) ||
           pilot_isFlag( p, PILOT_HYP_PREP ) )
         pilot_hyperspaceAbort( p );
   }

   /* Try to restore the targets if possible. */
   if ( diff_nav_spob != NULL ) {
      int found = 0;
      for ( int i = 0; i < array_size( cur_system->spobs ); i++ ) {
         if ( strcmp( cur_system->spobs[i]->name, diff_nav_spob ) == 0 ) {
            found              = 1;
            player.p->nav_spob = i;
            player_targetSpobSet( i );
            break;
         }
      }
      if ( !found )
         player_targetSpobSet( -1 );
   } else
      player_targetSpobSet( -1 );
   if ( diff_nav_hyperspace != NULL ) {
      int found = 0;
      for ( int i = 0; i < array_size( cur_system->jumps ); i++ ) {
         if ( strcmp( cur_system->jumps[i].target->name,
                      diff_nav_hyperspace ) == 0 ) {
            found                    = 1;
            player.p->nav_hyperspace = i;
            player_targetHyperspaceSet( i, 0 );
            break;
         }
      }
      if ( !found )
         player_targetHyperspaceSet( -1, 0 );
   } else
      player_targetHyperspaceSet( -1, 0 );

   diff_universe_changed = 0;
   return 1;
}

/**
 * @brief Sets whether or not to defer universe change stuff.
 *
 *    @param enable Whether or not to enable deferring.
 */
void unidiff_universeDefer( int enable )
{
   int defer           = diff_universe_defer;
   diff_universe_defer = enable;
   if ( defer && !enable )
      diff_checkUpdateUniverse();
}
