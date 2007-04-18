

#ifndef MAIN_H
#  define MAIN_H


#define MALLOC_ONE(type)   (malloc(sizeof(type)))
#define CALLOC_ONE(type)	(calloc(1,sizeof(type)))

#define ABS(X)					((X<0)?-X:X)

extern char* data; /* modifiable datafile */
#define DATA	data /* data file */


#endif /* MAIN_H */
