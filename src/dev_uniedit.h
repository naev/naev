/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#define HIDE_DEFAULT_JUMP        1.0 /**< Default hide value for new jumps. */
#define RADIUS_DEFAULT           15e3 /**< Default radius for new systems. */
#define DUST_DENSITY_DEFAULT     400 /**< Default stars density for new systems. */

void uniedit_open( unsigned int wid_unused, const char *unused );
void uniedit_selectText (void);
char *uniedit_nameFilter( const char *name );
void uniedit_autosave( unsigned int wid_unused, const char *unused );
void uniedit_updateAutosave (void);

void uniedit_renderMap( double bx, double by, double w, double h, double x, double y, double zoom, double r );
