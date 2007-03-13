


#ifndef LOG_H
#  define LOG_H

#include <stdio.h>

#define LOG(str, args...)	(fprintf(stdout,str"\n", ## args))
#define WARN(str,args...)	(fprintf(stderr,"[%d] "str"\n", 0, ## args))
#ifdef DEBUG
#  undef DEBUG
#  define DEBUG(str, args...)	LOG(str, ## args)
#  define DEBUGGING
#else /* DEBUG */
#  define DEBUG(str, args...)	do {;} while(0)
#endif /* DEBUG */


#endif /* LOG_H */
