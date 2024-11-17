#pragma once

double *make_distance_mapd( double *data, unsigned int width,
                            unsigned int height, double *vmax );

float *make_distance_mapbf( unsigned char *img, unsigned int width,
                            unsigned int height, double *vmax );
