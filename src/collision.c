/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file collision.c
 *
 * @brief Deals with 2d collisions.
 */


#include "collision.h"

#include "naev.h"

#include "log.h"


/**
 * @brief Checks whether or not two sprites collide.
 *
 * This function does pixel perfect checks.  If the collision actually occurs,
 *  crash is set to store the real position of the collision.
 *
 *    @param[in] at Texture a.
 *    @param[in] asx Position of x of sprite a.
 *    @param[in] asy Position of y of sprite a.
 *    @param[in] ap Position in space of sprite a.
 *    @param[in] bt Texture b.
 *    @param[in] bsx Position of x of sprite b.
 *    @param[in] bsy Position of y of sprite b.
 *    @param[in] bp Position in space of sprite b.
 *    @param[out] crash Actual position of the collision (only set on collision).
 *    @return 1 on collision, 0 else.
 */
int CollideSprite( const glTexture* at, const int asx, const int asy, const Vector2d* ap,
      const glTexture* bt, const int bsx, const int bsy, const Vector2d* bp,
      Vector2d* crash )
{
   int x,y;
   int ax1,ax2, ay1,ay2;
   int bx1,bx2, by1,by2;
   int inter_x0, inter_x1, inter_y0, inter_y1;
   int rasy, rbsy;
   int abx,aby, bbx, bby;

#if DEBUGGING
   /* Make sure the surfaces have transparency maps. */
   if (at->trans == NULL) {
      WARN("Texture '%s' has no transparency map.", at->name);
      return 0;
   }
   if (bt->trans == NULL) {
      WARN("Texture '%s' has no transparency map.", bt->name);
      return 0;
   }
#endif /* DEBUGGING */

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
               (!gl_isTrans(bt, bbx + x, bby + y))) {

            /* Set the crash position. */
            crash->x = x;
            crash->y = y;
            return 1;
         }

   return 0;
}


/**
 * @brief Checks to see if two lines collide.
 *
 *    @param[in] s1x X start point of line 1.
 *    @param[in] s1y Y start point of line 1.
 *    @param[in] e1x X end point of line 1.
 *    @param[in] e1y Y end point of line 1.
 *    @param[in] s2x X start point of line 2.
 *    @param[in] s2y Y start point of line 2.
 *    @param[in] e2x X end point of line 2.
 *    @param[in] e2y Y end point of line 2.
 *    @param[out] crash Position of the collision.
 *    @return 3 if lines are coincident, 2 if lines are parallel,
 *              1 if lines just collide on a point, or 0 if they don't.
 */
int CollideLineLine( double s1x, double s1y, double e1x, double e1y,
      double s2x, double s2y, double e2x, double e2y, Vector2d* crash )
{
   double ua_t, ub_t, u_b;
   double ua, ub;

   ua_t = (e2x - s2x) * (s1y - s2y) - (e2y - s2y) * (s1x - s2x);
   ub_t = (e1x - s1x) * (s1y - s2y) - (e1y - s1y) * (s1x - s2x);
   u_b  = (e2y - s2y) * (e1x - s1x) - (e2x - s2x) * (e1y - s1y);

   if (u_b != 0.) {
      ua = ua_t / u_b;
      ub = ub_t / u_b;

      /* Intersection at a point. */
      if ((0. <= ua) && (ua <= 1.) && (0. <= ub) && (ub <= 1.)) {
         crash->x = s1x + ua * (e1x - s1x);
         crash->y = s1y + ua * (e1y - s1y);
         return 1;
      }
      /* No intersection. */
      else
         return 0;
   }
   else {
      /* Coincident. */
      if ((ua_t == 0.) || (ub_t == 0.))
         return 3;
      /* Parallel. */
      else
         return 2;
   }
}


/**
 * @brief Checks to see if a line collides with a sprite.
 *
 * First collisions are detected on all the walls of the sprite's rectangle.
 *  Then the collisions are tested by pixel perfectness until collisions are
 *  actually found with the ship itself.
 *
 *    @param[in] ap Origin of the line.
 *    @param[in] ad Direction of the line.
 *    @param[in] al Length of the line.
 *    @param[in] bt Texture b.
 *    @param[in] bsx Position of x of sprite b.
 *    @param[in] bsy Position of y of sprite b.
 *    @param[in] bp Position in space of sprite b.
 *    @param[out] crash Position of the collision.
 *    @return 1 on collision, 0 else.
 *
 * @sa CollideSprite
 */
int CollideLineSprite( const Vector2d* ap, double ad, double al,
      const glTexture* bt, const int bsx, const int bsy, const Vector2d* bp,
      Vector2d crash[2] )
{
   int x,y, rbsy, bbx,bby;
   double ep[2], bl[2], tr[2], v[2], mod;
   int hits, real_hits;
   Vector2d tmp_crash, border[2];

   /* Make sure texture has transparency map. */
   if (bt->trans == NULL) {
      WARN("Texture '%s' has no transparency map.", bt->name);
      return 0;
   }

   /* Set up end point of line. */
   ep[0] = ap->x + al*cos(ad);
   ep[1] = ap->y + al*sin(ad);

   /* Set up top right corner of the rectangle. */
   tr[0] = bp->x + bt->sw/2.;
   tr[1] = bp->y + bt->sh/2.;
   /* Set up bottom left corner of the rectangle. */
   bl[0] = bp->x - bt->sw/2.;
   bl[1] = bp->y - bt->sh/2.;

   /*
    * Start check for rectangular collisions.
    */
   hits = 0;
   /* Left border. */
   if (CollideLineLine(ap->x, ap->y, ep[0], ep[1],
         bl[0], bl[1], bl[0], tr[1], &tmp_crash) == 1) {
      border[hits].x = tmp_crash.x;
      border[hits].y = tmp_crash.y;
      hits++;
   }
   /* Top border. */
   if (CollideLineLine(ap->x, ap->y, ep[0], ep[1],
         bl[0], tr[1], tr[0], tr[1], &tmp_crash) == 1) {
      border[hits].x = tmp_crash.x;
      border[hits].y = tmp_crash.y;
      hits++;
   }
   /* Now we have to make sure hits isn't 2. */
   /* Right border. */
   if ((hits < 2) && CollideLineLine(ap->x, ap->y, ep[0], ep[1],
         tr[0], tr[1], tr[0], bl[1], &tmp_crash) == 1) {
      border[hits].x = tmp_crash.x;
      border[hits].y = tmp_crash.y;
      hits++;
   }
   /* Bottom border. */
   if ((hits < 2) && CollideLineLine(ap->x, ap->y, ep[0], ep[1],
         tr[0], bl[1], bl[0], bl[1], &tmp_crash) == 1) {
      border[hits].x = tmp_crash.x;
      border[hits].y = tmp_crash.y;
      hits++;
   }

   /* No hits - missed. */
   if (hits == 0)
      return 0;

   /* Beam must die in the rectangle. */
   if (hits == 1) {
      border[1].x = ep[0];
      border[1].y = ep[1];
   }

   /*
    * Now we do a pixel perfect approach.
    */
   real_hits = 0;
   /* Directionality vector (normalized). */
   v[0] = border[1].x - border[0].x;
   v[1] = border[1].y - border[0].y;
   /* Normalize. */
   mod = MOD(v[0],v[1])/2.; /* Multiply by two to reduce check amount. */
   v[0] /= mod;
   v[1] /= mod;

   /* real vertical sprite value (flipped) */
   rbsy = bt->sy - bsy - 1;
   /* set up the base points */
   bbx =  bsx*(int)(bt->sw);
   bby = rbsy*(int)(bt->sh);

   /* We start checking first border until we find collision. */
   x = border[0].x - bl[0] + v[0];
   y = border[0].y - bl[1] + v[1];
   while ((x > 0.) && (x < bt->sw) && (y > 0.) && (y < bt->sh)) {
      /* Is non-transparent. */
      if (!gl_isTrans(bt, bbx+(int)x, bby+(int)y)) {
         crash[real_hits].x = x + bl[0];
         crash[real_hits].y = y + bl[1];
         real_hits++;
         break;
      }
      x += v[0];
      y += v[1];
   }

   /* Now we check the second border. */
   x = border[1].x - bl[0] - v[0];
   y = border[1].y - bl[1] - v[1];
   while ((x > 0.) && (x < bt->sw) && (y > 0.) && (y < bt->sh)) {
      /* Is non-transparent. */
      if (!gl_isTrans(bt, bbx+(int)x, bby+(int)y)) {
         crash[real_hits].x = x + bl[0];
         crash[real_hits].y = y + bl[1];
         real_hits++;
         break;
      }
      x -= v[0];
      y -= v[1];
   }

   /* Actually missed. */
   if (real_hits == 0)
      return 0;

   /* Strange situation, should never happen but just in case we duplicate
    *  the hit. */
   if (real_hits == 1) {
      crash[1].x = crash[0].x;
      crash[1].y = crash[0].y;
   }

   /* We hit. */
   return 1;
}


