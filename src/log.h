


#ifndef LOG_H
#  define LOG_H

#include <stdio.h>

#define LOG(str, args...)	(fprintf(stdout,str"\n", ## args))
#define WARN(str, args...)	(fprintf(stderr,"Warning: "str"\n", ## args))
#define ERR(str, args...)	(fprintf(stderr,"ERROR %s:%d: "str"\n", __FILE__, __LINE__, ## args))
#ifdef DEBUG
#  undef DEBUG
#  define DEBUG(str, args...)	LOG(str, ## args)
#  define DEBUGGING
#else /* DEBUG */
#  define DEBUG(str, args...)	do {;} while(0)
#endif /* DEBUG */


#endif /* LOG_H */
