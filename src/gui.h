/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef GUI_H
#  define GUI_H


#include "opengl.h"


/*
 * enums
 */
typedef enum RadarShape_ {
   RADAR_RECT,  /**< Rectangular radar. */
   RADAR_CIRCLE /**< Circular radar. */
} RadarShape; /**< Player's radar shape. */


/*
 * Loading/cleaning up.
 */
int gui_init (void);
void gui_free (void);
int gui_load (const char* name);
void gui_cleanup (void);


/*
 * Triggers.
 */
void gui_setCargo (void);
void gui_setNav (void);
void gui_setTarget (void);
void gui_setShip (void);
void gui_setSystem (void);


/*
 * render
 */
void gui_renderReticles( double dt );
void gui_render( double dt );
void gui_renderTargetReticles( int x, int y, int w, int h, glColour* c );
void gui_forceBlink (void);

/*
 * Messages.
 */
void gui_messageInit( int width, int x, int y );
void gui_clearMessages (void);
void gui_messageScrollUp( int lines );
void gui_messageScrollDown( int lines );
void gui_messageSetVisible( int lines );
int gui_messageGetVisible (void);


/*
 * Radar.
 */
int gui_radarInit( int circle, int w, int h );
void gui_radarRender( double x, double y );


/*
 * Targetting.
 */
void gui_targetPlanetGFX( glTexture *gfx );
void gui_targetPilotGFX( glTexture *gfx );


/*
 * misc
 */
void gui_setViewport( double x, double y, double w, double h );
void gui_clearViewport (void);
void gui_setDefaults (void);
void gui_setRadarRel( int mod );
void gui_getOffset( double *x, double *y );
glTexture* gui_hailIcon (void);
char* gui_pick (void);


#endif /* GUI_H */
