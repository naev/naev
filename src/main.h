

#ifndef MAIN_H
#  define MAIN_H


#define MALLOC_ONE(type)   (malloc(sizeof(type)))
#define CALLOC_ONE(type)	(calloc(1,sizeof(type)))

#define ABS(X)					((X<0)?-X:X)

extern char* data; /* modifiable datafile */
#define DATA	data /* data file */

/* maximum filename path */
#ifndef PATH_MAX
#  define PATH_MAX				100
#endif /* PATH_MAX */

#endif /* MAIN_H */
