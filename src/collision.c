/*
 * See Licensing and Copyright notice in naev.h
 */



#include "collision.h"


#include "naev.h"
#include "log.h"


/*
 * Collides sprite at (asx,asy) in at at pos ap with sprite at (bsx,bsy) in
 * bt at bp
 *
 *    @param at texture a
 *    @param asx position of x of sprite a
 *    @param asy position of y of sprita a
 *    @param ap position in space of sprite a
 *    @param bt texture b
 *    @param bsx position of x of sprite b
 *    @param bsy position of y of sprite b
 *    @param bp position in space of sprite b
 */
int CollideSprite( const glTexture* at, const int asx, const int asy, const Vector2d* ap,
      const glTexture* bt, const int bsx, const int bsy, const Vector2d* bp )
{
   int x,y;
   int ax1,ax2, ay1,ay2;
   int bx1,bx2, by1,by2;
   int inter_x0, inter_x1, inter_y0, inter_y1;
   int rasy, rbsy;
   int abx,aby, bbx, bby;

   /* a - cube coordinates */ 
   ax1 = (int)VX(*ap) - (int)(at->sw)/2;
   ay1 = (int)VY(*ap) - (int)(at->sh)/2;
   ax2 = ax1 + (int)(at->sw) - 1;
   ay2 = ay1 + (int)(at->sh) - 1;

   /* b - cube coordinates */ 
   bx1 = (int)VX(*bp) - (int)(bt->sw)/2;
   by1 = (int)VY(*bp) - (int)(bt->sh)/2;
   bx2 = bx1 + bt->sw - 1;
   by2 = by1 + bt->sh - 1;

   /* check if bounding boxes intersect */ 
   if((bx2 < ax1) || (ax2 < bx1)) return 0;
   if((by2 < ay1) || (ay2 < by1)) return 0;


   /* define the remaining binding box */ 
   inter_x0 = MAX( ax1, bx1 );
   inter_x1 = MIN( ax2, bx2 );
   inter_y0 = MAX( ay1, by1 );
   inter_y1 = MIN( ay2, by2 );

   /* real vertical sprite value (flipped) */
   rasy = at->sy - asy - 1;
   rbsy = bt->sy - bsy - 1;

   /* set up the base points */
   abx =  asx*(int)(at->sw) - ax1;
   aby = rasy*(int)(at->sh) - ay1;
   bbx =  bsx*(int)(bt->sw) - bx1;
   bby = rbsy*(int)(bt->sh) - by1;

   for (y=inter_y0; y<=inter_y1; y++)
      for (x=inter_x0; x<=inter_x1; x++)
         /* compute offsets for surface before pass to TransparentPixel test */ 
         if ((!gl_isTrans(at, abx + x, aby + y)) &&
               (!gl_isTrans(bt, bbx + x, bby + y)))
            return 1;

   return 0;
} 
