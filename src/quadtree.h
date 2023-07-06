/*
 * This code was initially authored by the Stackoverflow user dragon-energy and posted under following page:
 * https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
 *
 * As for the license, the author has kindly noted:
 *
 * "Oh and feel free to use this code I post however you want, even for commercial projects.
 *  I would really love it if people let me know if they find it useful, but do as you wish."
 *
 * And generally all Stackoverflow-posted code is by default licensed with CC BY-SA 4.0:
 * https://creativecommons.org/licenses/by-sa/4.0/
 */
#pragma once

#include "intlist.h"

typedef struct Quadtree Quadtree;

struct Quadtree
{
   // Stores all the nodes in the quadtree. The first node in this
   // sequence is always the root.
   IntList nodes;

   // Stores all the elements in the quadtree.
   IntList elts;

   // Stores all the element nodes in the quadtree.
   IntList enodes;

   // Stores the quadtree extents.
   int root_mx, root_my, root_sx, root_sy;

   // Maximum allowed elements in a leaf before the leaf is subdivided/split unless
   // the leaf is at the maximum allowed tree depth.
   int max_elements;

   // Stores the maximum depth allowed for the quadtree.
   int max_depth;

   // Temporary buffer used for queries.

   char* temp;

   // Stores the size of the temporary buffer.
   int temp_size;
};

// Function signature used for traversing a tree node.
typedef void QtNodeFunc( Quadtree* qt, void* user_data, int node, int depth, int mx, int my, int sx, int sy );

// Creates a quadtree with the requested extents, maximum elements per leaf, and maximum tree depth.
void qt_create( Quadtree* qt, int x1, int y1, int x2, int y2, int max_elements, int max_depth );

// Destroys the quadtree.
void qt_destroy( Quadtree* qt );

// Inserts a new element to the tree.
// Returns an index to the new element.
int qt_insert( Quadtree* qt, int id, int x1, int y1, int x2, int y2 );

// Removes the specified element from the tree.
void qt_remove( Quadtree* qt, int element );

// Cleans up the tree, removing empty leaves.
void qt_cleanup( Quadtree* qt );

// Outputs a list of elements found in the specified rectangle.
void qt_query( Quadtree* qt, IntList* out, int x1, int y1, int x2, int y2, int omit_element );

// Traverses all the nodes in the tree, calling 'branch' for branch nodes and 'leaf'
// for leaf nodes.
void qt_traverse( Quadtree* qt, void* user_data, QtNodeFunc* branch, QtNodeFunc* leaf);
