/*
 * See Licensing and Copyright notice in naev.h
 */



#include "collision.h"


#include "naev.h"
#include "log.h"


/**
 * @brief Collides sprite at (asx,asy) in at at pos ap with sprite at (bsx,bsy) in
 * bt at bp.
 *
 * @param[in] at texture a.
 * @param[in] asx position of x of sprite a.
 * @param[in] asy position of y of sprita a.
 * @param[in] ap position in space of sprite a.
 * @param[in] bt texture b.
 * @param[in] bsx position of x of sprite b.
 * @param[in] bsy position of y of sprite b.
 * @param[in] bp position in space of sprite b.
 * @return 1 on collision, 0 else.
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


/**
 * @brief Calculates collision path corrections.
 * 
 * @param[in] p Position of object that's trying to collide.
 * @param[in] v Velocity of object that's trying to collide.
 * @param[in] tp Position of the target to collide into.
 * @param[in] tv Velocity of the target to collide into.
 * @param[in] limit Accuracy limit.
 * @return Direction object will need to adjust velocity to for collision.
 */
/* 
 * Doesn't work as expected, needs to get fixed to allow missiles to use
 * the physics engine instead of hacking their velocities
 */
#if 0
double CollidePath( const Vector2d* p, Vector2d* v,
      const Vector2d* tp, Vector2d* tv, double limit )
{
   double mx, my; /* Position modifiers */
   double t, tt; /* Time to target */
   Vector2d test, ttest; /* Test vector and target test vector */
   double offset; /* Direction to face and error it produces */
   double mod, moddir; /* Modifier for iteration */
   int i;

   /* Test vector */
   vect_cset( &test, tp->x - p->x, tp->y - p->y ); /* Start by straight line */
   t = VMOD(test) / VMOD(*v); /* d=v*t ==> t=d/v */

   /* Target test vector */
   vectnull( &ttest ); /* Starting from itself */
   tt = VMOD(ttest) / VMOD(*tv);

   /* Special case object isn't moving */
   if (VMOD(*v) < 1e-6)
      return VANGLE(test);

   /* Target faster then object */
   if (VMOD(*v) < VMOD(*tv)) {
      vect_reflect( &test, v, tv );
      return VANGLE(test);
   }

   /* Loop until error is minimal */
   offset = tt - t;
   moddir = 1.; /* Start off by positive increments */
   mod = 10.; /* Start off by a 10 second jump */
   i = 0;
   while (FABS(offset) > limit) {
      if (i>100) break;

      /* Calculate position modifiers */
      if (offset < -mod/2.) /* tt>t ==> major overshot */
         moddir = -1.;
      else if (offset < 0.) { /* tt>t ==> overshot */
         /* invert direction and half */
         moddir = -1.;
         mod = mod/2.; 
      }
      else if (offset > mod) /* tt<t ==> undershot */
         moddir = 1.; /* make sure it's positive */
      else if (offset > 0.) { /* tt<t ==> minor undershot */
         /* positive and shrink step */
         moddir = 1.;
         mod = FABS(mod)/2.;
      }
      mx = tv->x * mod * moddir;
      my = tv->y * mod * moddir;

      /* Increment test vectors */
      vect_cadd( &test, mx, my );
      vect_cadd( &ttest, mx, my );

      /* Compare results */
      t = VMOD(test) / VMOD(*v);
      tt = VMOD(ttest) / VMOD(*tv);
      offset = tt - t;

      i++;
   }

   return VANGLE(test);
}
#endif

