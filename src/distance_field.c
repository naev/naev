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
 * @param[out] vmax The underlying distance value corresponding to +1.0.
 * @return Allocated distance field, values ranging from 0 (farthest inside) to 1 (greatest distance).
 */
   double *
make_distance_mapd( double *data, unsigned int width, unsigned int height, double *vmax )
{
   unsigned int wh = width*height;
   short * xdist = (short *)  malloc( wh * sizeof(short) );
   short * ydist = (short *)  malloc( wh * sizeof(short) );
   double * gx   = (double *) calloc( wh, sizeof(double) );
   double * gy      = (double *) calloc( wh, sizeof(double) );
   double * outside = (double *) calloc( wh, sizeof(double) );
   double * inside  = (double *) calloc( wh, sizeof(double) );

   // Compute outside = edtaa3(bitmap); % Transform background (0's)
   computegradient( data, width, height, gx, gy);
   edtaa3(data, gx, gy, width, height, xdist, ydist, outside);
   for (unsigned int i=0; i<wh; i++)
      if( outside[i] < 0.0 )
         outside[i] = 0.0;

   // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
   memset( gx, 0, sizeof(double)*width*height );
   memset( gy, 0, sizeof(double)*width*height );
   for (unsigned int i=0; i<wh; i++)
      data[i] = 1. - data[i];
   computegradient( data, width, height, gx, gy );
   edtaa3( data, gx, gy, width, height, xdist, ydist, inside );
   for (unsigned int i=0; i<wh; i++)
      if (inside[i] < 0.)
         inside[i] = 0.;

   // distmap = outside - inside; % Bipolar distance field
   *vmax = 0.;
   for (unsigned int i=0; i<wh; i++) {
      outside[i] -= inside[i];
      if( *vmax < fabs( outside[i] ) )
         *vmax = fabs( outside[i] );
   }

   for (unsigned int i=0; i<wh; i++) {
      double v = outside[i];
      if     ( v < -*vmax) outside[i] = -*vmax;
      else if( v > +*vmax) outside[i] = +*vmax;
      data[i] = (outside[i]+*vmax)/(2. * *vmax);
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
 * @param[out] vmax The underlying distance value corresponding to +1.0.
 * @return Allocated distance field, values ranging from 0 (innermost) to 1 (outermost).
 *         The reason for the inversion (relative to make_distance_mapd and the "signed distance"
 *         concept) is so that these can be pasted together in a texture atlas with a buffer of
 *         0.0 values representing "completely outside".
 */
float*
make_distance_mapbf( unsigned char *img,
                    unsigned int width, unsigned int height, double *vmax )
{
   unsigned int wh = width * height;
   double * data  = (double *) calloc( wh, sizeof(double) );
   float *out     = (float *) malloc( wh * sizeof(float) );

   // find minimum and maximum values
   double img_min = DBL_MAX;
   double img_max = DBL_MIN;

   for(unsigned int i=0; i<wh; i++) {
      double v = img[i];
      if (v > img_max)
         img_max = v;
      if (v < img_min)
         img_min = v;
   }

   // Map values from 0 - 255 to 0.0 - 1.0
   for (unsigned int i=0; i<wh; i++)
      data[i] = (img[i]-img_min)/img_max;

   data = make_distance_mapd( data, width, height, vmax );

   // lower to float
   for (unsigned int i=0; i<wh; i++)
      out[i] = (float)(1-data[i]);

   free( data );

   return out;
}
