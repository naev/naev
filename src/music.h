/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MUSIC_H
#  define MUSIC_H


/*
 * thread
 */
int music_thread( void* unused );
void music_kill (void);


/*
 * init/exit
 */
int music_init (void);
int music_makeList (void);
void music_exit (void);


/*
 * music control
 */
void music_volume( const double vol );
void music_load( const char* name );
void music_play (void);
void music_stop (void);


#endif /* MUSIC_H */
