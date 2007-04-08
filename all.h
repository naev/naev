

#ifndef ALL_H
#  define ALL_H

#define MALLOC_ONE(type)   (malloc(sizeof(type)))
#define CALLOC_ONE(type)	(calloc(1,sizeof(type)))

#define ABS(X)					((X<0)?-X:X)
#define FABS(X)				((X<0.)?-X:X)

#define DATA	"data"

#endif /* ALL_H */
