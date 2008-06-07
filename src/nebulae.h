/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NEBULAE_H
#  define NEBULAE_H


/*
 * Tries to find nebulae to load up, if not will generate them.
 */
void nebu_init (void);
void nebu_exit (void);

void nebu_render (void);
void nebu_forceGenerate (void);


#endif /* NEBULAE_H */
