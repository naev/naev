/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef ECONOMY_H
#  define ECONOMY_H


typedef struct Commodity_ {
	char* name;
	int low, medium, high; /* prices */
} Commodity;

/* commodity stuff */
Commodity* commodity_get( const char* name );
int commodity_load (void);
void commodity_free (void);

/* misc stuff */
void credits2str( char *str, unsigned int credits, int decimals );


#endif /* ECONOMY_H */
