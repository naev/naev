/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NEBULAE_H
#  define NEBULAE_H


/*
 * Init/Exit
 */
void nebu_init (void);
void nebu_exit (void);

/*
 * Render
 */
void nebu_render (void);
void nebu_renderOverlay (void);

/*
 * Misc
 */
void nebu_prep( double density, double volatility );
void nebu_forceGenerate (void);


#endif /* NEBULAE_H */
