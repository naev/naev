/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "attributes.h"

/**
 * @enum UniHunkTargetType_t
 *
 * @brief Represents the possible hunk targets.
 */
typedef enum UniHunkTargetType_ {
   HUNK_TARGET_NONE,
   HUNK_TARGET_SYSTEM,
   HUNK_TARGET_SPOB,
   HUNK_TARGET_TECH,
   HUNK_TARGET_FACTION,
} UniHunkTargetType_t;

/**
 * @struct UniHunkTarget_t
 *
 * @brief Represents the hunk's target.
 */
typedef struct UniHunkTarget_ {
   UniHunkTargetType_t type; /**< Type of hunk target. */
   union {
      char *name; /**< Name of the target. */
   } u;           /**< Union of possible target types. */
} UniHunkTarget_t;

/**
 * @enum UniHunkType_t
 *
 * @brief Represents the different type of hunk actions.
 */
typedef enum UniHunkType_ {
   HUNK_TYPE_NONE,
   /* Target should be HUNK_TARGET_SYSTEM. */
   HUNK_TYPE_SPOB_ADD,
   HUNK_TYPE_SPOB_REMOVE,
   HUNK_TYPE_VSPOB_ADD,
   HUNK_TYPE_VSPOB_REMOVE,
   HUNK_TYPE_JUMP_ADD,
   HUNK_TYPE_JUMP_REMOVE,
   HUNK_TYPE_SSYS_BACKGROUND,
   HUNK_TYPE_SSYS_BACKGROUND_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_FEATURES,
   HUNK_TYPE_SSYS_FEATURES_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_POS_X,
   HUNK_TYPE_SSYS_POS_X_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_POS_Y,
   HUNK_TYPE_SSYS_POS_Y_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_DISPLAYNAME,
   HUNK_TYPE_SSYS_DISPLAYNAME_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_DUST,
   HUNK_TYPE_SSYS_DUST_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_INTERFERENCE,
   HUNK_TYPE_SSYS_INTERFERENCE_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_NEBU_DENSITY,
   HUNK_TYPE_SSYS_NEBU_DENSITY_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_NEBU_VOLATILITY,
   HUNK_TYPE_SSYS_NEBU_VOLATILITY_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_NEBU_HUE,
   HUNK_TYPE_SSYS_NEBU_HUE_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_NOLANES_ADD,
   HUNK_TYPE_SSYS_NOLANES_REMOVE,
   HUNK_TYPE_SSYS_TAG_ADD,
   HUNK_TYPE_SSYS_TAG_REMOVE,
   /* Target should be HUNK_TARGET_SYSTEM, but we use attributes for the
      asteroid label. */
   HUNK_TYPE_SSYS_ASTEROIDS_ADD,
   HUNK_TYPE_SSYS_ASTEROIDS_ADD_REVERT,
   HUNK_TYPE_SSYS_ASTEROIDS_REMOVE,
   HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_REVERT,
   HUNK_TYPE_SSYS_ASTEROIDS_POS_X,
   HUNK_TYPE_SSYS_ASTEROIDS_POS_X_REVERT,
   HUNK_TYPE_SSYS_ASTEROIDS_POS_Y,
   HUNK_TYPE_SSYS_ASTEROIDS_POS_Y_REVERT,
   HUNK_TYPE_SSYS_ASTEROIDS_DENSITY,
   HUNK_TYPE_SSYS_ASTEROIDS_DENSITY_REVERT,
   HUNK_TYPE_SSYS_ASTEROIDS_RADIUS,
   HUNK_TYPE_SSYS_ASTEROIDS_RADIUS_REVERT,
   HUNK_TYPE_SSYS_ASTEROIDS_MAXSPEED,
   HUNK_TYPE_SSYS_ASTEROIDS_MAXSPEED_REVERT,
   HUNK_TYPE_SSYS_ASTEROIDS_ACCEL,
   HUNK_TYPE_SSYS_ASTEROIDS_ACCEL_REVERT,
   HUNK_TYPE_SSYS_ASTEROIDS_ADD_TYPE,
   HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_TYPE,
   /* Target should be HUNK_TARGET_TECH. */
   HUNK_TYPE_TECH_ADD,
   HUNK_TYPE_TECH_REMOVE,
   /* Target should be HUNK_TARGET_SPOB. */
   HUNK_TYPE_SPOB_POS_X,
   HUNK_TYPE_SPOB_POS_X_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_POS_Y,
   HUNK_TYPE_SPOB_POS_Y_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_CLASS,
   HUNK_TYPE_SPOB_CLASS_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_FACTION,
   HUNK_TYPE_SPOB_FACTION_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_PRESENCE_BASE,
   HUNK_TYPE_SPOB_PRESENCE_BASE_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_PRESENCE_BONUS,
   HUNK_TYPE_SPOB_PRESENCE_BONUS_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_PRESENCE_RANGE,
   HUNK_TYPE_SPOB_PRESENCE_RANGE_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_HIDE,
   HUNK_TYPE_SPOB_HIDE_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_POPULATION,
   HUNK_TYPE_SPOB_POPULATION_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_DISPLAYNAME,
   HUNK_TYPE_SPOB_DISPLAYNAME_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_DESCRIPTION,
   HUNK_TYPE_SPOB_DESCRIPTION_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_BAR,
   HUNK_TYPE_SPOB_BAR_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_SERVICE_ADD,
   HUNK_TYPE_SPOB_SERVICE_REMOVE,
   HUNK_TYPE_SPOB_NOMISNSPAWN_ADD,
   HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE,
   HUNK_TYPE_SPOB_TECH_ADD,
   HUNK_TYPE_SPOB_TECH_REMOVE,
   HUNK_TYPE_SPOB_TAG_ADD,
   HUNK_TYPE_SPOB_TAG_REMOVE,
   HUNK_TYPE_SPOB_SPACE,
   HUNK_TYPE_SPOB_SPACE_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_EXTERIOR,
   HUNK_TYPE_SPOB_EXTERIOR_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_LUA,
   HUNK_TYPE_SPOB_LUA_REVERT, /* For internal usage. */
   /* Target should be HUNK_TARGET_FACTION. */
   HUNK_TYPE_FACTION_VISIBLE,
   HUNK_TYPE_FACTION_INVISIBLE,
   HUNK_TYPE_FACTION_ALLY,
   HUNK_TYPE_FACTION_ENEMY,
   HUNK_TYPE_FACTION_NEUTRAL,
   HUNK_TYPE_FACTION_REALIGN, /* For internal usage. */
   /* End marker */
   HUNK_TYPE_SENTINAL
} UniHunkType_t;

typedef enum UniHunkDataType_s {
   HUNK_DATA_NONE,
   HUNK_DATA_STRING,
   HUNK_DATA_INT,
   HUNK_DATA_FLOAT,
   HUNK_DATA_POINTER,
} UniHunkDataType_t;

typedef struct UniAttribute_ {
   char *name;
   char *value;
} UniAttribute_t;

/**
 * @struct UniHunk_t
 *
 * @brief Represents a single hunk in the diff.
 */
typedef struct UniHunk_ {
   UniHunkTarget_t   target; /**< Hunk's target. */
   UniHunkType_t     type;   /**< Type of hunk it is. */
   UniHunkDataType_t dtype;  /**< Type of data to use. */
   union {
      char  *name;
      int    data;
      double fdata;
   } u; /**< Actual data to patch. */
   union {
      void       *ptr;  /* Allow Pointers, only for old attributes. */
      const char *name; /* We just save the pointer, so keep as const. */
      int         data;
      double      fdata;
   } o;                  /** Old data to possibly replace. */
   UniAttribute_t *attr; /**< Attributes. array.h */
} UniHunk_t;

/**
 * @brief Universe diff filepath list.
 */
typedef struct UniDiffData_ {
   char      *name;     /**< Name of the diff (read from XML). */
   char      *filename; /**< Filename of the diff. */
   UniHunk_t *hunks;    /**< Hunks available. */
} UniDiffData_t;

/* Global functions, manily for player stuff. */
int  diff_init( void );
void diff_exit( void );
NONNULL( 1 ) int diff_isApplied( const char *name );
NONNULL( 1 ) int diff_apply( const char *name );
NONNULL( 1 ) void diff_remove( const char *name );
void diff_clear( void );
void unidiff_universeDefer( int enable );
int  diff_parsePhysFS( UniDiffData_t *diff, char *filename );
int  diff_parse( UniDiffData_t *diff, char *filename );
void diff_freeData( UniDiffData_t *diff );

/* Local functions for hunk management. */
void        diff_start( void );
int         diff_patchHunk( UniHunk_t *hunk );
int         diff_revertHunk( const UniHunk_t *hunk );
void        diff_end( void );
const char *diff_hunkName( UniHunkType_t t );
const char *diff_hunkTag( UniHunkType_t t );
void        diff_cleanupHunk( UniHunk_t *hunk );
