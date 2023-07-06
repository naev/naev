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
#include "quadtree.h"
#include <stdlib.h>
#include <string.h>

enum
{
   // ----------------------------------------------------------------------------------------
   // Element node fields:
   // ----------------------------------------------------------------------------------------
   enode_num = 2,

   // Points to the next element in the leaf node. A value of -1
   // indicates the end of the list.
   enode_idx_next = 0,

   // Stores the element index.
   enode_idx_elt = 1,

   // ----------------------------------------------------------------------------------------
   // Element fields:
   // ----------------------------------------------------------------------------------------
   elt_num = 5,

   // Stores the rectangle encompassing the element.
   elt_idx_lft = 0, elt_idx_top = 1, elt_idx_rgt = 2, elt_idx_btm = 3,

   // Stores the ID of the element.
   elt_idx_id = 4,

   // ----------------------------------------------------------------------------------------
   // Node fields:
   // ----------------------------------------------------------------------------------------
   node_num = 2,

   // Points to the first child if this node is a branch or the first element
   // if this node is a leaf.
   node_idx_fc = 0,

   // Stores the number of elements in the node or -1 if it is not a leaf.
   node_idx_num = 1,

   // ----------------------------------------------------------------------------------------
   // Node data fields:
   // ----------------------------------------------------------------------------------------
   nd_num = 6,

   // Stores the extents of the node using a centered rectangle and half-size.
   nd_idx_mx = 0, nd_idx_my = 1, nd_idx_sx = 2, nd_idx_sy = 3,

   // Stores the index of the node.
   nd_idx_index = 4,

   // Stores the depth of the node.
   nd_idx_depth = 5,
};

static void node_insert(Quadtree* qt, int index, int depth, int mx, int my, int sx, int sy, int element);

static int intersect( int l1, int t1, int r1, int b1, int l2, int t2, int r2, int b2 )
{
   return l2 <= r1 && r2 >= l1 && t2 <= b1 && b2 >= t1;
}

static void leaf_insert( Quadtree* qt, int node, int depth, int mx, int my, int sx, int sy, int element )
{
   // Insert the element node to the leaf.
   const int nd_fc = il_get(&qt->nodes, node, node_idx_fc);
   il_set(&qt->nodes, node, node_idx_fc, il_insert(&qt->enodes));
   il_set(&qt->enodes, il_get(&qt->nodes, node, node_idx_fc), enode_idx_next, nd_fc);
   il_set(&qt->enodes, il_get(&qt->nodes, node, node_idx_fc), enode_idx_elt, element);

   // If the leaf is full, split it.
   if (il_get(&qt->nodes, node, node_idx_num) == qt->max_elements && depth < qt->max_depth) {
      int fc = 0;
      IntList elts = {0};
      il_create(&elts, 1);

      // Transfer elements from the leaf node to a list of elements.
      while (il_get(&qt->nodes, node, node_idx_fc) != -1) {
         const int index = il_get(&qt->nodes, node, node_idx_fc);
         const int next_index = il_get(&qt->enodes, index, enode_idx_next);
         const int elt = il_get(&qt->enodes, index, enode_idx_elt);

         // Pop off the element node from the leaf and remove it from the qt.
         il_set(&qt->nodes, node, node_idx_fc, next_index);
         il_erase(&qt->enodes, index);

         // Insert element to the list.
         il_set(&elts, il_push_back(&elts), 0, elt);
      }

      // Start by allocating 4 child nodes.
      fc = il_insert(&qt->nodes);
      il_insert(&qt->nodes);
      il_insert(&qt->nodes);
      il_insert(&qt->nodes);
      il_set(&qt->nodes, node, node_idx_fc, fc);

      // Initialize the new child nodes.
      for (int j=0; j < 4; ++j) {
         il_set(&qt->nodes, fc+j, node_idx_fc, -1);
         il_set(&qt->nodes, fc+j, node_idx_num, 0);
      }

      // Transfer the elements in the former leaf node to its new children.
      il_set(&qt->nodes, node, node_idx_num, -1);
      for (int j=0; j < il_size(&elts); ++j)
         node_insert(qt, node, depth, mx, my, sx, sy, il_get(&elts, j, 0));
      il_destroy(&elts);
   }
   else {
      // Increment the leaf element count.
      il_set(&qt->nodes, node, node_idx_num, il_get(&qt->nodes, node, node_idx_num) + 1);
   }
}

static void push_node( IntList* nodes, int nd_index, int nd_depth, int nd_mx, int nd_my, int nd_sx, int nd_sy )
{
   const int back_idx = il_push_back(nodes);
   il_set(nodes, back_idx, nd_idx_mx, nd_mx);
   il_set(nodes, back_idx, nd_idx_my, nd_my);
   il_set(nodes, back_idx, nd_idx_sx, nd_sx);
   il_set(nodes, back_idx, nd_idx_sy, nd_sy);
   il_set(nodes, back_idx, nd_idx_index, nd_index);
   il_set(nodes, back_idx, nd_idx_depth, nd_depth);
}

static void find_leaves( IntList* out, const Quadtree* qt, int node, int depth,
      int mx, int my, int sx, int sy,
      int lft, int top, int rgt, int btm )
{
   IntList to_process = {0};
   il_create( &to_process, nd_num );
   push_node( &to_process, node, depth, mx, my, sx, sy );

   while (il_size(&to_process) > 0) {
      const int back_idx = il_size(&to_process) - 1;
      const int nd_mx = il_get(&to_process, back_idx, nd_idx_mx);
      const int nd_my = il_get(&to_process, back_idx, nd_idx_my);
      const int nd_sx = il_get(&to_process, back_idx, nd_idx_sx);
      const int nd_sy = il_get(&to_process, back_idx, nd_idx_sy);
      const int nd_index = il_get(&to_process, back_idx, nd_idx_index);
      const int nd_depth = il_get(&to_process, back_idx, nd_idx_depth);
      il_pop_back(&to_process);

      // If this node is a leaf, insert it to the list.
      if (il_get(&qt->nodes, nd_index, node_idx_num) != -1)
         push_node(out, nd_index, nd_depth, nd_mx, nd_my, nd_sx, nd_sy);
      else {
         // Otherwise push the children that intersect the rectangle.
         const int fc = il_get(&qt->nodes, nd_index, node_idx_fc);
         const int hx = nd_sx >> 1, hy = nd_sy >> 1;
         const int l = nd_mx-hx, t = nd_my-hy, r = nd_mx+hx, b = nd_my+hy;

         if (top <= nd_my) {
            if (lft <= nd_mx)
               push_node(&to_process, fc+0, nd_depth+1, l,t,hx,hy);
            if (rgt > nd_mx)
               push_node(&to_process, fc+1, nd_depth+1, r,t,hx,hy);
         }
         if (btm > nd_my) {
            if (lft <= nd_mx)
               push_node(&to_process, fc+2, nd_depth+1, l,b,hx,hy);
            if (rgt > nd_mx)
               push_node(&to_process, fc+3, nd_depth+1, r,b,hx,hy);
         }
      }
   }
   il_destroy(&to_process);
}

static void node_insert( Quadtree* qt, int index, int depth, int mx, int my, int sx, int sy, int element )
{
   // Find the leaves and insert the element to all the leaves found.
   IntList leaves = {0};

   const int lft = il_get(&qt->elts, element, elt_idx_lft);
   const int top = il_get(&qt->elts, element, elt_idx_top);
   const int rgt = il_get(&qt->elts, element, elt_idx_rgt);
   const int btm = il_get(&qt->elts, element, elt_idx_btm);

   il_create(&leaves, nd_num);
   find_leaves(&leaves, qt, index, depth, mx, my, sx, sy, lft, top, rgt, btm);
   for (int j=0; j < il_size(&leaves); ++j) {
      const int nd_mx = il_get(&leaves, j, nd_idx_mx);
      const int nd_my = il_get(&leaves, j, nd_idx_my);
      const int nd_sx = il_get(&leaves, j, nd_idx_sx);
      const int nd_sy = il_get(&leaves, j, nd_idx_sy);
      const int nd_index = il_get(&leaves, j, nd_idx_index);
      const int nd_depth = il_get(&leaves, j, nd_idx_depth);
      leaf_insert(qt, nd_index, nd_depth, nd_mx, nd_my, nd_sx, nd_sy, element);
   }
   il_destroy(&leaves);
}

void qt_create( Quadtree* qt, int x1, int y1, int x2, int y2, int max_elements, int max_depth )
{
   qt->max_elements = max_elements;
   qt->max_depth = max_depth;
   qt->temp = NULL;
   qt->temp_size = 0;
   il_create(&qt->nodes, node_num);
   il_create(&qt->elts, elt_num);
   il_create(&qt->enodes, enode_num);

   // Insert the root node to the qt.
   il_insert(&qt->nodes);
   il_set(&qt->nodes, 0, node_idx_fc, -1);
   il_set(&qt->nodes, 0, node_idx_num, 0);

   int half_width = (x2-x1) >> 1;
   int half_height = (y2-y1) >> 1;
   qt->root_sx = half_width;
   qt->root_sy = half_height;

   // Center
   qt->root_mx = x1 + half_width;
   qt->root_my = y1 + half_height;
}

void qt_clear( Quadtree* qt )
{
   free(qt->temp);
   qt->temp = NULL;
   qt->temp_size = 0;

   il_clear( &qt->nodes );
   il_clear( &qt->elts );
   il_clear( &qt->enodes );

   // Insert the root node to the qt.
   il_insert(&qt->nodes);
   il_set(&qt->nodes, 0, node_idx_fc, -1);
   il_set(&qt->nodes, 0, node_idx_num, 0);
}

void qt_destroy( Quadtree* qt )
{
   il_destroy(&qt->nodes);
   il_destroy(&qt->elts);
   il_destroy(&qt->enodes);
   free(qt->temp);
}

int qt_insert (Quadtree* qt, int id, int x1, int y1, int x2, int y2 )
{
   // Insert a new element.
   const int new_element = il_insert(&qt->elts);

   // Set the fields of the new element.
   il_set(&qt->elts, new_element, elt_idx_lft, x1);
   il_set(&qt->elts, new_element, elt_idx_top, y1);
   il_set(&qt->elts, new_element, elt_idx_rgt, x2);
   il_set(&qt->elts, new_element, elt_idx_btm, y2);
   il_set(&qt->elts, new_element, elt_idx_id,  id);

   // Insert the element to the appropriate leaf node(s).
   node_insert(qt, 0, 0, qt->root_mx, qt->root_my, qt->root_sx, qt->root_sy, new_element);
   return new_element;
}

void qt_remove( Quadtree* qt, int element )
{
   // Find the leaves.
   IntList leaves = {0};

   const int lft = il_get(&qt->elts, element, elt_idx_lft);
   const int top = il_get(&qt->elts, element, elt_idx_top);
   const int rgt = il_get(&qt->elts, element, elt_idx_rgt);
   const int btm = il_get(&qt->elts, element, elt_idx_btm);

   il_create(&leaves, nd_num);
   find_leaves(&leaves, qt, 0, 0, qt->root_mx, qt->root_my, qt->root_sx, qt->root_sy, lft, top, rgt, btm);

   // For each leaf node, remove the element node.
   for (int j=0; j < il_size(&leaves); ++j) {
      const int nd_index = il_get(&leaves, j, nd_idx_index);

      // Walk the list until we find the element node.
      int node_index = il_get(&qt->nodes, nd_index, node_idx_fc);
      int prev_index = -1;
      while (node_index != -1 && il_get(&qt->enodes, node_index, enode_idx_elt) != element) {
         prev_index = node_index;
         node_index = il_get(&qt->enodes, node_index, enode_idx_next);
      }

      if (node_index != -1) {
         // Remove the element node.
         const int next_index = il_get(&qt->enodes, node_index, enode_idx_next);
         if (prev_index == -1)
            il_set(&qt->nodes, nd_index, node_idx_fc, next_index);
         else
            il_set(&qt->enodes, prev_index, enode_idx_next, next_index);
         il_erase(&qt->enodes, node_index);

         // Decrement the leaf element count.
         il_set(&qt->nodes, nd_index, node_idx_num, il_get(&qt->nodes, nd_index, node_idx_num)-1);
      }
   }
   il_destroy(&leaves);

   // Remove the element.
   il_erase(&qt->elts, element);
}

void qt_query( Quadtree* qt, IntList* out, int qlft, int qtop, int qrgt, int qbtm )
{
   // Find the leaves that intersect the specified query rectangle.
   IntList leaves = {0};
   const int elt_cap = il_size(&qt->elts);

   if (qt->temp_size < elt_cap) {
      qt->temp_size = elt_cap;
      qt->temp = realloc(qt->temp, qt->temp_size * sizeof(*qt->temp));
      memset(qt->temp, 0, qt->temp_size * sizeof(*qt->temp));
   }

   // For each leaf node, look for elements that intersect.
   il_create(&leaves, nd_num);
   find_leaves(&leaves, qt, 0, 0, qt->root_mx, qt->root_my, qt->root_sx, qt->root_sy, qlft, qtop, qrgt, qbtm);

   il_clear(out);
   for (int j=0; j < il_size(&leaves); ++j) {
      const int nd_index = il_get(&leaves, j, nd_idx_index);

      // Walk the list and add elements that intersect.
      int elt_node_index = il_get(&qt->nodes, nd_index, node_idx_fc);
      while (elt_node_index != -1) {
         const int element = il_get(&qt->enodes, elt_node_index, enode_idx_elt);
         const int lft = il_get(&qt->elts, element, elt_idx_lft);
         const int top = il_get(&qt->elts, element, elt_idx_top);
         const int rgt = il_get(&qt->elts, element, elt_idx_rgt);
         const int btm = il_get(&qt->elts, element, elt_idx_btm);
         if (!qt->temp[element] && intersect(qlft,qtop,qrgt,qbtm, lft,top,rgt,btm)) {
            il_set(out, il_push_back(out), 0, element);
            qt->temp[element] = 1;
         }
         elt_node_index = il_get(&qt->enodes, elt_node_index, enode_idx_next);
      }
   }
   il_destroy(&leaves);

   /* Unmark the elements that were inserted, and convert to IDs. */
   for (int j=0; j < il_size(out); ++j) {
      const int element = il_get(out, j, 0);
      const int id = il_get(&qt->elts, element, elt_idx_id);
      qt->temp[element] = 0;
      il_set(out, j, 0, id);
   }
}

void qt_cleanup( Quadtree* qt )
{
   IntList to_process = {0};
   il_create(&to_process, 1);

   // Only process the root if it's not a leaf.
   if (il_get(&qt->nodes, 0, node_idx_num) == -1) {
      // Push the root index to the stack.
      il_set(&to_process, il_push_back(&to_process), 0, 0);
   }

   while (il_size(&to_process) > 0) {
      // Pop a node from the stack.
      const int node = il_get(&to_process, il_size(&to_process)-1, 0);
      const int fc = il_get(&qt->nodes, node, node_idx_fc);
      int num_empty_leaves = 0;
      il_pop_back(&to_process);

      // Loop through the children.
      for (int j=0; j < 4; ++j) {
         const int child = fc + j;

         // Increment empty leaf count if the child is an empty
         // leaf. Otherwise if the child is a branch, add it to
         // the stack to be processed in the next iteration.
         if (il_get(&qt->nodes, child, node_idx_num) == 0)
            ++num_empty_leaves;
         else if (il_get(&qt->nodes, child, node_idx_num) == -1)
         {
            // Push the child index to the stack.
            il_set(&to_process, il_push_back(&to_process), 0, child);
         }
      }

      // If all the children were empty leaves, remove them and
      // make this node the new empty leaf.
      if (num_empty_leaves == 4) {
         // Remove all 4 children in reverse order so that they
         // can be reclaimed on subsequent insertions in proper
         // order.
         il_erase(&qt->nodes, fc + 3);
         il_erase(&qt->nodes, fc + 2);
         il_erase(&qt->nodes, fc + 1);
         il_erase(&qt->nodes, fc + 0);

         // Make this node the new empty leaf.
         il_set(&qt->nodes, node, node_idx_fc, -1);
         il_set(&qt->nodes, node, node_idx_num, 0);
      }
   }
   il_destroy(&to_process);
}

void qt_traverse( Quadtree* qt, void* user_data, QtNodeFunc* branch, QtNodeFunc* leaf )
{
   IntList to_process = {0};
   il_create(&to_process, nd_num);
   push_node(&to_process, 0, 0, qt->root_mx, qt->root_my, qt->root_sx, qt->root_sy);

   while (il_size(&to_process) > 0) {
      const int back_idx = il_size(&to_process) - 1;
      const int nd_mx = il_get(&to_process, back_idx, nd_idx_mx);
      const int nd_my = il_get(&to_process, back_idx, nd_idx_my);
      const int nd_sx = il_get(&to_process, back_idx, nd_idx_sx);
      const int nd_sy = il_get(&to_process, back_idx, nd_idx_sy);
      const int nd_index = il_get(&to_process, back_idx, nd_idx_index);
      const int nd_depth = il_get(&to_process, back_idx, nd_idx_depth);
      const int fc = il_get(&qt->nodes, nd_index, node_idx_fc);
      il_pop_back(&to_process);

      if (il_get(&qt->nodes, nd_index, node_idx_num) == -1) {
         // Push the children of the branch to the stack.
         const int hx = nd_sx >> 1, hy = nd_sy >> 1;
         const int l = nd_mx-hx, t = nd_my-hy, r = nd_mx+hx, b = nd_my+hy;
         push_node(&to_process, fc+0, nd_depth+1, l,t, hx,hy);
         push_node(&to_process, fc+1, nd_depth+1, r,t, hx,hy);
         push_node(&to_process, fc+2, nd_depth+1, l,b, hx,hy);
         push_node(&to_process, fc+3, nd_depth+1, r,b, hx,hy);
         if (branch)
            branch(qt, user_data, nd_index, nd_depth, nd_mx, nd_my, nd_sx, nd_sy);
      }
      else if (leaf)
         leaf(qt, user_data, nd_index, nd_depth, nd_mx, nd_my, nd_sx, nd_sy);
   }
   il_destroy(&to_process);
}
