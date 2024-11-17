/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space.h"
#include "unidiff.h"

#define HIDE_DEFAULT_JUMP 1.0 /**< Default hide value for new jumps. */
#define RADIUS_DEFAULT 15e3   /**< Default radius for new systems. */
#define DUST_DENSITY_DEFAULT                                                   \
   400 /**< Default stars density for new systems.                             \
        */

void  uniedit_open( unsigned int wid_unused, const char *unused );
void  uniedit_selectText( void );
char *uniedit_nameFilter( const char *name );
void  uniedit_options( unsigned int wid_unused, const char *unused );

void uniedit_renderMap( double bx, double by, double w, double h, double x,
                        double y, double zoom, double r );

/* For when working in diff mode. */
extern int uniedit_diffMode;
void       uniedit_diffAdd( UniHunk_t *hunk );
void uniedit_diffCreateSysNone( const StarSystem *sys, UniHunkType_t type );
void uniedit_diffCreateSysStr( const StarSystem *sys, UniHunkType_t type,
                               char *str );
void uniedit_diffCreateSysStrAttr( const StarSystem *sys, UniHunkType_t type,
                                   char *str, UniAttribute_t *attr );
void uniedit_diffCreateSysInt( const StarSystem *sys, UniHunkType_t type,
                               int data );
void uniedit_diffCreateSysIntAttr( const StarSystem *sys, UniHunkType_t type,
                                   int data, UniAttribute_t *attr );
void uniedit_diffCreateSysFloat( const StarSystem *sys, UniHunkType_t type,
                                 double fdata );
void uniedit_diffCreateSysFloatAttr( const StarSystem *sys, UniHunkType_t type,
                                     double fdata, UniAttribute_t *attr );
