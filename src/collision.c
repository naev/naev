

#include "collision.h"


#include "main.h"
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
int CollideSprite( const gl_texture* at, const int asx, const int asy, const Vector2d* ap,
		const gl_texture* bt, const int bsx, const int bsy, const Vector2d* bp )
{
	int x,y;

	/* a - cube coordinates */ 
	int ax1 = (int)VX(*ap) - (int)(at->sw)/2; 
	int ay1 = (int)VY(*ap) - (int)(at->sh)/2; 
	int ax2 = ax1 + (int)(at->sw) - 1; 
	int ay2 = ay1 + (int)(at->sh) - 1; 

	/* b - cube coordinates */ 
	int bx1 = (int)VX(*bp) - (int)(bt->sw)/2; 
	int by1 = (int)VY(*bp) - (int)(bt->sh)/2; 
	int bx2 = bx1 + bt->sw - 1; 
	int by2 = by1 + bt->sh - 1; 

	/* check if bounding boxes intersect */ 
	if((bx2 < ax1) || (ax2 < bx1)) return 0; 
	if((by2 < ay1) || (ay2 < by1)) return 0; 


	/* define the remaining binding box */ 
	int inter_x0 = MAX( ax1, bx1 ); 
	int inter_x1 = MIN( ax2, bx2 ); 
	int inter_y0 = MAX( ay1, by1 ); 
	int inter_y1 = MIN( ay2, by2 ); 

	for (y=inter_y0; y<=inter_y1; y++) 
		for (x=inter_x0; x<=inter_x1; x++)
			/* compute offsets for surface before pass to TransparentPixel test */ 
			if ((!gl_isTrans(at, asx*(int)(at->sw) + x-ax1, asy*(int)(at->sh) + y-ay1)) && 
					(!gl_isTrans(bt, bsx*(int)(bt->sw) + x-bx1, bsy*(int)(bt->sh) + y-by1))) 
				return 1; 

	return 0; 
} 
