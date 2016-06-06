/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef DEV_UNIEDIT_H
#  define DEV_UNIEDIT_H

#define HIDE_DEFAULT_JUMP        1.25 /**< Default hide value for new jumps. */
#define RADIUS_DEFAULT           10000 /**< Default radius for new systems. */
#define STARS_DENSITY_DEFAULT    400 /**< Default stars density for new systems. */


void uniedit_open( unsigned int wid_unused, char *unused );
void uniedit_selectText (void);
char *uniedit_nameFilter( char *name );
void uniedit_autosave( unsigned int wid_unused, char *unused );
void uniedit_updateAutosave (void);


#endif /* DEV_UNIEDIT_H */
