/* Freetype GL - A C OpenGL Freetype engine
 *
Copyright 2011-2016 Nicolas P. Rougier
Copyright 2013-2016 Marcel Metz
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the freetype-gl project.
 */


/**
 * @file distance_field.c
 *
 * @brief Code for generating our distance fields (\see font.c).
 * Based on the corresponding file in https://github.com/rougier/freetype-gl
 */

/** @cond */
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
/** @endcond */

#include "edtaa3func.h"

/**
 * @brief Like the original: perform a Euclidean Distance Transform on the input and
 *        normalize to [0,1], with a value of 0.5 on the boundary.
 * @param data width*height values, row-major order. Positive values are object pixels.
 *             Negative/zero values are background pixels.
 * @param width Number of columns.
 * @param height Number of rows.
 * @param explicit_max If a positive number is passed in, saturate at that outer distance.
 * @return Allocated distance field, values ranging from 0 (outermost) to 1 (innermost).
 */
double *
make_distance_mapd( double *data, unsigned int width, unsigned int height, double explicit_max )
{
    short * xdist = (short *)  malloc( width * height * sizeof(short) );
    short * ydist = (short *)  malloc( width * height * sizeof(short) );
    double * gx   = (double *) calloc( width * height, sizeof(double) );
    double * gy      = (double *) calloc( width * height, sizeof(double) );
    double * outside = (double *) calloc( width * height, sizeof(double) );
    double * inside  = (double *) calloc( width * height, sizeof(double) );
    double vmin = DBL_MAX, vmax = -DBL_MAX;
    unsigned int i;

    // Compute outside = edtaa3(bitmap); % Transform background (0's)
    computegradient( data, width, height, gx, gy);
    edtaa3(data, gx, gy, width, height, xdist, ydist, outside);
    for( i=0; i<width*height; ++i)
        if( outside[i] < 0.0 )
            outside[i] = 0.0;

    // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
    memset( gx, 0, sizeof(double)*width*height );
    memset( gy, 0, sizeof(double)*width*height );
    for( i=0; i<width*height; ++i)
        data[i] = 1 - data[i];
    computegradient( data, width, height, gx, gy );
    edtaa3( data, gx, gy, width, height, xdist, ydist, inside );
    for( i=0; i<width*height; ++i )
        if( inside[i] < 0 )
            inside[i] = 0.0;

    // distmap = outside - inside; % Bipolar distance field
    for( i=0; i<width*height; ++i)
    {
        outside[i] -= inside[i];
        if( outside[i] < vmin )
            vmin = outside[i];
    }

    vmax = explicit_max > 0 ? explicit_max : fabs(vmin);

    for( i=0; i<width*height; ++i)
    {
        double v = outside[i];
        if ( v < vmin)
            data[i] = 0;
        else if( v < 0)
            data[i] = .5-.5*outside[i]/vmin;
        else if( v <= vmax)
            data[i] = .5+.5*outside[i]/vmax;
        else
            data[i] = +1;
    }

    free( xdist );
    free( ydist );
    free( gx );
    free( gy );
    free( outside );
    free( inside );
    return data;
}

/**
 * @brief Perform a Euclidean Distance Transform on the input and normalize to [0,1],
 *        with a value of 0.5 on the boundary.
 * @param img Pixel values, row-major order.
 * @param width Number of columns.
 * @param height Number of rows.
 * @param explicit_max If a positive number is passed in, saturate at that outer distance.
 * @return Allocated distance field, values ranging from 0 (innermost) to 1 (outermost).
 */
float*
make_distance_mapbf( unsigned char *img,
                    unsigned int width, unsigned int height, double explicit_max )
{
    double * data    = (double *) calloc( width * height, sizeof(double) );
    float *out       = (float *) malloc( width * height * sizeof(float) );
    unsigned int i;

    // find minimum and maximum values
    double img_min = DBL_MAX;
    double img_max = DBL_MIN;

    for( i=0; i<width*height; ++i)
    {
        double v = img[i];
        data[i] = v;
        if (v > img_max)
            img_max = v;
        if (v < img_min)
            img_min = v;
    }

    // Map values from 0 - 255 to 0.0 - 1.0
    for( i=0; i<width*height; ++i)
        data[i] = (img[i]-img_min)/img_max;

    data = make_distance_mapd(data, width, height, explicit_max);

    // lower to float
    for( i=0; i<width*height; ++i)
        out[i] = (float)(1-data[i]);

    free( data );

    return out;
}
