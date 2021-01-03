#ifndef DISTANCE_FIELD_H
#  define DISTANCE_FIELD_H

double *
make_distance_mapd( double *data, unsigned int width, unsigned int height, double explicit_max );

float*
make_distance_mapbf( unsigned char *img,
                    unsigned int width, unsigned int height, double explicit_max );

#endif /* DISTANCE_FIELD_H */
