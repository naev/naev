/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NEBULA_H
#  define NEBULA_H


/*
 * Init/Exit
 */
int nebu_init (void);
void nebu_vbo_init (void);
void nebu_exit (void);

/*
 * Render
 */
void nebu_render( const double dt );
void nebu_renderOverlay( const double dt );

/*
 * Update.
 */
void nebu_update( double dt );

/*
 * Misc
 */
int nebu_isLoaded (void);
void nebu_genOverlay (void);
double nebu_getSightRadius (void);
void nebu_prep( double density, double volatility );
void nebu_forceGenerate (void);
void nebu_movePuffs( double x, double y );


#endif /* NEBULA_H */
