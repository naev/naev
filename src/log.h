/*
 * See Licensing and Copyright notice in naev.h
 */




#ifndef LOG_H
#  define LOG_H


#include <stdio.h>
#include <signal.h>

/* Get text stuff. */
#include <libintl.h>
#define _(String) gettext(String)
#define gettext_noop(String) String

#define LOG(str, args...)  (logprintf(stdout, 1, str, ## args))
#ifdef DEBUG_PARANOID /* Will cause WARNs to blow up */
#define WARN(str, args...) (logprintf(stderr, 0, _("Warning: [%s] "), __func__), logprintf( stderr, 1, str, ## args), abort())
#else /* DEBUG_PARANOID */
#define WARN(str, args...) (logprintf(stderr, 0, _("Warning: [%s] "), __func__), logprintf( stderr, 1, str, ## args))
#endif /* DEBUG_PARANOID */
#define ERR(str, args...)  (logprintf(stderr, 0, _("ERROR %s:%d [%s]: "), __FILE__, __LINE__, __func__), logprintf( stderr, 1, str, ## args), abort())
#ifdef DEBUG
#  undef DEBUG
#  define DEBUG(str, args...) LOG(str, ## args)
#ifndef DEBUGGING
#  define DEBUGGING
#endif /* DEBUGGING */
#else /* DEBUG */
#  define DEBUG(str, args...) do {;} while(0)
#endif /* DEBUG */


int logprintf( FILE *stream, int newline, const char *fmt, ... );
void log_redirect (void);
int log_isTerminal (void);
void log_copy( int enable );
int log_copying (void);
void log_purge (void);
void log_clean (void);


#endif /* LOG_H */
