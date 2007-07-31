

#ifndef MAIN_H
#  define MAIN_H


#define APPNAME         "NAEV"

#define MALLOC_ONE(type)   (malloc(sizeof(type)))
#define CALLOC_ONE(type)	(calloc(1,sizeof(type)))

#define ABS(x)					(((x)<0)?-(x):(x))

#define MAX(x,y)				(((x)>(y))?(x):(y))
#define MIN(x,y)				(((x)>(y))?(y):(x))

extern char* data; /* modifiable datafile */
#define DATA	data /* data file */

/* maximum filename path */
#ifndef PATH_MAX
#  define PATH_MAX				100
#endif /* PATH_MAX */


#endif /* MAIN_H */
