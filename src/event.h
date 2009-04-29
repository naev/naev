/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef EVENT_H
#  define EVENT_H


/*
 * Loading/exitting.
 */
int events_load (void);
void events_exit (void);


/*
 * Triggering.
 */
void events_trigger( int where );


#endif /* EVENT_H */


