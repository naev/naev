/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NEBULA_H
#  define NEBULA_H


/*
 * Init/Exit
 */
int nebu_init (void);
void nebu_exit (void);

/*
 * Render
 */
void nebu_render( const double dt );
void nebu_renderOverlay( const double dt );
void nebu_renderPuffs( const double dt, int below_player );

/*
 * Misc
 */
void nebu_genOverlay (void);
double nebu_getSightRadius (void);
void nebu_prep( double density, double volatility );
void nebu_forceGenerate (void);


#endif /* NEBULA_H */
