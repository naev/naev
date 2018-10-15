/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef GUI_H
#  define GUI_H


#include "opengl.h"
#include "pilot.h"
#include "space.h"


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
void gui_reload (void);


/*
 * Triggers.
 */
void gui_setCargo (void);
void gui_setNav (void);
void gui_setTarget (void);
void gui_setShip (void);
void gui_setSystem (void);
void gui_updateFaction (void);
void gui_setGeneric ( Pilot* pilot );

/*
 * render
 */
void gui_renderReticles( double dt );
void gui_render( double dt );
void gui_renderTargetReticles( int x, int y, int w, int h, const glColour* c );
void gui_forceBlink (void);

/*
 * Messages.
 */
void gui_messageInit( int width, int x, int y );
void gui_clearMessages (void);
void gui_messageScrollUp( int lines );
void gui_messageScrollDown( int lines );


/*
 * Radar.
 */
int gui_radarInit( int circle, int w, int h );
void gui_radarRender( double x, double y );
void gui_radarGetPos( int *x, int *y );
void gui_radarGetDim( int *w, int *h );
void gui_radarGetRes( int *res );


/*
 * Render radar.
 */
void gui_renderPlanet( int ind, RadarShape shape, double w, double h, double res, int overlay );
void gui_renderJumpPoint( int ind, RadarShape shape, double w, double h, double res, int overlay );
void gui_renderPilot( const Pilot* p, RadarShape shape, double w, double h, double res, int overlay );
void gui_renderAsteroid( const Asteroid* a, double w, double h, double res, int overlay );
void gui_renderPlayer( double res, int overlay );


/*
 * Targeting.
 */
void gui_targetPlanetGFX( glTexture *gfx );
void gui_targetPilotGFX( glTexture *gfx );


/*
 * Mouse.
 */
int gui_handleEvent( SDL_Event *evt );
void gui_mouseClickEnable( int enabel );
void gui_mouseMoveEnable( int enabel );


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
int gui_onScreenPilot( double *rx, double *ry, Pilot *pilot );
int gui_onScreenAsset( double *rx, double *ry, JumpPoint *jp, Planet *pnt );


#endif /* GUI_H */
