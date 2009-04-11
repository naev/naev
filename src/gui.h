/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef GUI_H
#  define GUI_H


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
 * render
 */
void gui_renderBG( double dt );
void gui_renderTarget( double dt );
void gui_render( double dt );


/*
 * misc
 */
void gui_setDefaults (void);
void player_message( const char *fmt, ... );
void gui_setRadarRel( int mod );
void gui_getOffset( double *x, double *y );


#endif /* GUI_H */
