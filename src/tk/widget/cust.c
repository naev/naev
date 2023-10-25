/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file cust.c
 *
 * @brief Custom widget.
 */


#include "opengl.h"
#include "tk/toolkit_priv.h"


static void cst_render( Widget* cst, double bx, double by );
static void cst_renderOverlay( Widget* cst, double bx, double by );
static void cst_focusGain( Widget* cst );
static void cst_focusLose( Widget* cst );
static void cst_cleanup( Widget* cst );
static Widget *cst_getWidget( unsigned int wid, const char *name );


/**
 * @brief Adds a custom widget to a window.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 * You are in charge of the rendering and handling mouse input for this widget.
 *  Mouse events outside the widget position won't be passed on.
 *
 *    @param wid ID of the window to add the widget to.
 *    @param x X position within the window to use.
 *    @param y Y position within the window to use.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 *    @param name Name of the widget to use internally.
 *    @param border Whether or not it should have a border.
 *    @param render Render function, passes the position and dimensions of the
 *                  widget as parameters.
 *    @param mouse Mouse function, passes the window id, event and position as
 *                 parameters.
 */
void window_addCust( unsigned int wid,
                     const int x, const int y, /* position */
                     const int w, const int h, /* size */
                     char* name, const int border,
                     void (*render) (double x, double y, double w, double h, void *data),
                     int (*mouse) (unsigned int wid, SDL_Event* event,
                                    double x, double y, double w, double h,
                                    double rx, double ry, void *data),
                     void (*focusGain) (unsigned int wid, const char* wgtname),
                     void (*focusLose) (unsigned int wid, const char* wgtname),
                     void *data )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw, name);
   if (wgt == NULL)
      return;

   /* generic */
   wgt->type   = WIDGET_CUST;

   /* specific */
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->cleanup         = cst_cleanup;
   wgt->focusGain       = cst_focusGain;
   wgt->focusLose       = cst_focusLose;
   wgt->render          = cst_render;
   wgt->renderOverlay   = cst_renderOverlay;
   wgt->dat.cst.border  = border;
   wgt->dat.cst.render  = render;
   wgt->dat.cst.mouse   = mouse;
   wgt->dat.cst.clip    = 1;
   wgt->dat.cst.focusGain  = focusGain;
   wgt->dat.cst.focusLose  = focusLose;
   wgt->dat.cst.userdata   = data;
   wgt->dat.cst.autofree   = 0;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/**
 * @brief Custom widget gains focus.
 */
static void cst_focusGain( Widget* cst )
{
   if (cst->dat.cst.focusGain)
      cst->dat.cst.focusGain( cst->wdw, cst->name );
}


/**
 * @brief Custom widget loses focus.
 */
static void cst_focusLose( Widget* cst )
{
   if (cst->dat.cst.focusLose)
      cst->dat.cst.focusLose( cst->wdw, cst->name );
}


/**
 * @brief Renders a custom widget.
 *
 *    @param cst Custom widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void cst_render( Widget* cst, double bx, double by )
{
   double x,y;

   x = bx + cst->x;
   y = by + cst->y;

   glClear( GL_DEPTH_BUFFER_BIT );

   if (cst->dat.cst.border) {
      /* inner outline */
      toolkit_drawOutline( x, y+1, cst->w+1, cst->h+1, 0.,
            toolkit_colLight, NULL );
      /* outer outline */
      toolkit_drawOutline( x, y, cst->w+1, cst->h+1, 1.,
            toolkit_colDark, NULL );
   }

   if (cst->dat.cst.clip != 0)
      gl_clipRect( x, y, cst->w, cst->h );
   cst->dat.cst.render ( x, y, cst->w, cst->h, cst->dat.cst.userdata );
   if (cst->dat.cst.clip != 0)
      gl_unclipRect();
}


/**
 * @brief Renders the widget overlay.
 */
static void cst_renderOverlay( Widget* cst, double bx, double by )
{
   double x,y;

   x = bx + cst->x;
   y = by + cst->y;

   if (cst->dat.cst.clip != 0)
      gl_clipRect( x, y, cst->w, cst->h );
   if (cst->dat.cst.renderOverlay != NULL)
      cst->dat.cst.renderOverlay ( x, y, cst->w, cst->h, cst->dat.cst.userdata );
   if (cst->dat.cst.clip != 0)
      gl_unclipRect();
}


/**
 * @brief Clean up function for custom widgets.
 *
 *    @param cst Custom widget to clean up.
 */
static void cst_cleanup( Widget *cst )
{
   if (cst->dat.cst.autofree)
      free( cst->dat.cst.userdata );
}


/**
 * @brief Gets a custom widget.
 */
static Widget *cst_getWidget( unsigned int wid, const char *name )
{
   Widget *wgt;

   /* Get widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return NULL;

   /* Make sure it is a custom widget. */
   if (wgt->type != WIDGET_CUST) {
      DEBUG("Widget is not a custom widget: '%s'", name);
      return NULL;
   }

   return wgt;
}


/**
 * @brief Changes clipping settings on a custom widget.
 *
 *    @param wid Window to which widget belongs.
 *    @param name Name of the widget.
 *    @param clip If 0 disables clipping, otherwise enables clipping.
 */
void window_custSetClipping( unsigned int wid, const char *name, int clip )
{
   Widget *wgt = cst_getWidget( wid, name );
   if (wgt == NULL)
      return;

   /* Set the clipping. */
   wgt->dat.cst.clip = clip;
}


/**
 * @brief Sets the widget overlay.
 *
 *    @param wid Window to which widget belongs.
 *    @param name Name of the widget.
 *    @param renderOverlay Function to render widget overlay, NULL disables.
 */
void window_custSetOverlay( unsigned int wid, const char *name,
      void (*renderOverlay) (double bx, double by, double bw, double bh, void *data) )
{
   Widget *wgt = cst_getWidget( wid, name );
   if (wgt == NULL)
      return;

   wgt->dat.cst.renderOverlay = renderOverlay;
}


/**
 * @brief Gets the widget user data.
 *
 *    @param wid Window to which widget belongs.
 *    @param name Name of the widget.
 *    @return A pointer to the widget user data.
 */
void* window_custGetData( unsigned int wid, const char *name )
{
   Widget *wgt = cst_getWidget( wid, name );
   if (wgt == NULL)
      return NULL;

   return wgt->dat.cst.userdata;
}


/**
 * @brief Marks the widget's data as owned, so that it will be freed upon cleanup.
 *
 *    @param wid Window to which widget belongs.
 *    @param name Name of the widget.
 */
void window_custAutoFreeData( unsigned int wid, const char *name )
{
   Widget *wgt = cst_getWidget( wid, name );
   if (wgt != NULL)
      wgt->dat.cst.autofree = 1;
}

void window_custSetDynamic( unsigned int wid, const char *name, int dynamic )
{
   Widget *wgt = cst_getWidget( wid, name );
   if (wgt != NULL) {
      if (dynamic)
         wgt->flags |= dynamic;
      else
         wgt->flags &= ~dynamic;
   }
}
