/*
 * See Licensing and Copyright notice in naev.h
 */




#ifndef LOG_H
#  define LOG_H

#include <stdio.h>

/* We'll use asserts in DEBUG is defined */
#ifndef DEBUG
#  define NODEBUG
#endif /* DEBUG */
#include <assert.h>

#define LOG(str, args...)  (fprintf(stdout,str"\n", ## args))
#define WARN(str, args...) (fprintf(stderr,"Warning: "str"\n", ## args), assert(0))
#define ERR(str, args...)  (fprintf(stderr,"ERROR %s:%d [%s]: "str"\n", __FILE__, __LINE__, __func__, ## args), assert(0))
#ifdef DEBUG
#  undef DEBUG
#  define DEBUG(str, args...) LOG(str, ## args)
#  define DEBUGGING
#else /* DEBUG */
#  define DEBUG(str, args...) do {;} while(0)
#endif /* DEBUG */


#endif /* LOG_H */
