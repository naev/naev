/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file image.c
 *
 * @brief Image widget.
 */


#include "tk/toolkit_priv.h"


static void img_render( Widget* img, double bx, double by );
static void img_freeLayers( Widget* img );


/**
 * @brief Adds an image widget to the window.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 *    @param wid ID of the window to add the widget to.
 *    @param x X position within the window to use.
 *    @param y Y position within the window to use.
 *    @param name Name of the widget to use internally.
 *    @param image Image to use.
 *    @param border Whether to use a border.
 */
void window_addImage( const unsigned int wid,
                      const int x, const int y,
                      const int w, const int h,
                      char* name, glTexture* image, int border )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw, name);
   if (wgt == NULL)
      return;

   /* generic */
   wgt->type   = WIDGET_IMAGE;

   /* specific */
   wgt->render          = img_render;
   wgt->cleanup         = img_freeLayers;
   wgt->dat.img.image   = image;
   wgt->dat.img.border  = border;
   wgt->dat.img.colour  = cWhite; /* normal colour */
   wgt->dat.img.layers  = NULL;
   wgt->dat.img.nlayers = 0;

   /* position/size */
   wgt->w = (w > 0) ? w : ((image==NULL) ? 0 : wgt->dat.img.image->sw);
   wgt->h = (h > 0) ? h : ((image==NULL) ? 0 : wgt->dat.img.image->sh);
   toolkit_setPos( wdw, wgt, x, y );
}


/**
 * @brief Renders a image widget.
 *
 *    @param img Image widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void img_render( Widget* img, double bx, double by )
{
   double x,y;
   double w,h;
   int i;

   /* Values. */
   x = bx + img->x;
   y = by + img->y;
   w = img->w;
   h = img->h;

   /*
    * image
    */
   if (img->dat.img.image != NULL) {
      gl_blitScale( img->dat.img.image, x, y,
            w, h, &img->dat.img.colour );
   }
   /* Additional layers. */
   for (i=0; i<img->dat.img.nlayers; i++) {
      gl_blitScale( img->dat.img.layers[i], x, y,
            w, h, &img->dat.img.colour );
   }

   if (img->dat.img.border) {
      /* inner outline (outwards) */
      toolkit_drawOutline( x, y+1, w-1,
         h-1, 1., toolkit_colLight, NULL );
      /* outer outline */
      toolkit_drawOutline( x, y+1, w-1,
            h-1, 2., toolkit_colDark, NULL );
   }
}


/**
 * @brief Gets the image from an image widget
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 */
glTexture* window_getImage( const unsigned int wid, char* name )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return NULL;

   /* Check the type. */
   if (wgt->type != WIDGET_IMAGE) {
      WARN("Trying to get image from non-image widget '%s'.", name);
      return NULL;
   }

   /* Get the value. */
   return (wgt) ? wgt->dat.img.image : NULL;
}


/**
 * Modifies an existing image's image.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to modify image of.
 *    @param w New width to set, 0 uses image, -1 doesn't change and >0 sets directly.
 *    @param w New height to set, 0 uses image, -1 doesn't change and >0 sets directly.
 *    @param image New image to set.
 */
void window_modifyImage( const unsigned int wid,
      char* name, glTexture* image, int w, int h )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_IMAGE) {
      WARN("Not modifying image on non-image widget '%s'.", name);
      return;
   }

   /* Set the image. */
   wgt->dat.img.image = image;

   /* Adjust size. */
   if (w >= 0)
      wgt->w = (w > 0) ? w : ((image==NULL) ? 0 : wgt->dat.img.image->sw);
   if (h >= 0)
      wgt->h = (h > 0) ? h : ((image==NULL) ? 0 : wgt->dat.img.image->sh);
}


/**
 * Modifies an existing image's colour.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to modify image colour of.
 *    @param colour New colour to use.
 */
void window_imgColour( const unsigned int wid,
      char* name, const glColour* colour )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_IMAGE) {
      WARN("Not modifying image on non-image widget '%s'.", name);
      return;
   }

   /* Set the colour. */
   wgt->dat.img.colour = *colour;
}


/**
 * @brief Sets the image widget layers
 */
void window_modifyImageLayers( const unsigned int wid,
      char* name, glTexture** layers, int n )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_IMAGE) {
      WARN("Not modifying image on non-image widget '%s'.", name);
      return;
   }

   /* Set the layers. */
   img_freeLayers( wgt );
   wgt->dat.img.layers  = layers;
   wgt->dat.img.nlayers = n;
}


/**
 * Free layer stuff.
 */
static void img_freeLayers( Widget* img )
{
   free( img->dat.img.layers );
   img->dat.img.layers  = NULL;
   img->dat.img.nlayers = 0;
}

