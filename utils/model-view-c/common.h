#pragma once

#define LOG(str, args...)     (fprintf(stdout, str"\n", ## args))
#define DEBUG(str, args...)   (fprintf(stdout, str"\n", ## args))
#define WARN(str, args...)    (fprintf(stderr, "WARNING %s:%d [%s]: ", __FILE__, __LINE__, __func__), fprintf(stderr, str"\n", ## args))
#define MIN(x,y)              (((x)>(y))?(y):(x)) /**< Returns minimum. */
#define MAX(x,y)              (((x)<(y))?(y):(x)) /**< Returns maximum. */

#define DEBUGGING 1

#ifndef PATH_MAX
#define PATH_MAX  512
#endif /* PATH_MAX */
#define STRMAX    8192

#define SCREEN_W  1024
#define SCREEN_H  1024

#define _(String) String
#define gl_checkErr()   gl_checkHandleError( __func__, __LINE__ )
void gl_checkHandleError( const char *func, int line );
