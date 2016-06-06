/*
 * See Licensing and Copyright notice in naev.h
 */




#ifndef LOG_H
#  define LOG_H


#include <stdio.h>
#include <signal.h>


#define LOG(str, args...)  (logprintf(stdout,str"\n", ## args))
#ifdef DEBUG_PARANOID /* Will cause WARNs to blow up */
#define WARN(str, args...) (logprintf(stderr,"Warning: [%s] "str"\n", __func__, ## args), abort())
#else /* DEBUG_PARANOID */
#define WARN(str, args...) (logprintf(stderr,"Warning: [%s] "str"\n", __func__, ## args))
#endif /* DEBUG_PARANOID */
#define ERR(str, args...)  (logprintf(stderr,"ERROR %s:%d [%s]: "str"\n", __FILE__, __LINE__, __func__, ## args), abort())
#ifdef DEBUG
#  undef DEBUG
#  define DEBUG(str, args...) LOG(str, ## args)
#ifndef DEBUGGING
#  define DEBUGGING
#endif /* DEBUGGING */
#else /* DEBUG */
#  define DEBUG(str, args...) do {;} while(0)
#endif /* DEBUG */


int logprintf( FILE *stream, const char *fmt, ... );
void log_redirect (void);
int log_isTerminal (void);
void log_copy( int enable );
int log_copying (void);
void log_purge (void);
void log_clean (void);


#endif /* LOG_H */
