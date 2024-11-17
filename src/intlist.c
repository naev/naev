/*
 * This code was initially authored by the Stackoverflow user dragon-energy and
 * posted under following page:
 * https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
 *
 * As for the license, the author has kindly noted:
 *
 * "Oh and feel free to use this code I post however you want, even for
 * commercial projects. I would really love it if people let me know if they
 * find it useful, but do as you wish."
 *
 * And generally all Stackoverflow-posted code is by default licensed with CC
 * BY-SA 4.0: https://creativecommons.org/licenses/by-sa/4.0/
 */
#include "intlist.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void il_create( IntList *il, int num_fields )
{
   il->data         = il->fixed;
   il->num          = 0;
   il->cap          = il_fixed_cap;
   il->num_fields   = num_fields;
   il->free_element = -1;
}

void il_destroy( IntList *il )
{
   // Free the buffer only if it was heap allocated.
   if ( il->data != il->fixed )
      free( il->data );
}

void il_clear( IntList *il )
{
   il->num          = 0;
   il->free_element = -1;
}

int il_size( const IntList *il )
{
   return il->num;
}

int il_get( const IntList *il, int n, int field )
{
   assert( n >= 0 && n < il->num );
   return il->data[n * il->num_fields + field];
}

void il_set( IntList *il, int n, int field, int val )
{
   assert( n >= 0 && n < il->num );
   il->data[n * il->num_fields + field] = val;
}

int il_push_back( IntList *il )
{
   const int new_pos = ( il->num + 1 ) * il->num_fields;

   // If the list is full, we need to reallocate the buffer to make room
   // for the new element.
   if ( new_pos > il->cap ) {
      // Use double the size for the new capacity.
      const int new_cap = new_pos * 2;

      // If we're pointing to the fixed buffer, allocate a new array on the
      // heap and copy the fixed buffer contents to it.
      if ( il->cap == il_fixed_cap ) {
         il->data = malloc( new_cap * sizeof( *il->data ) );
         memcpy( il->data, il->fixed, sizeof( il->fixed ) );
      } else {
         // Otherwise reallocate the heap buffer to the new size.
         il->data = realloc( il->data, new_cap * sizeof( *il->data ) );
      }
      // Set the old capacity to the new capacity.
      il->cap = new_cap;
   }
   return il->num++;
}

void il_pop_back( IntList *il )
{
   // Just decrement the list size.
   assert( il->num > 0 );
   --il->num;
}

int il_insert( IntList *il )
{
   // If there's a free index in the free list, pop that and use it.
   if ( il->free_element != -1 ) {
      const int index = il->free_element;
      const int pos   = index * il->num_fields;

      // Set the free index to the next free index.
      il->free_element = il->data[pos];

      // Return the free index.
      return index;
   }
   // Otherwise insert to the back of the array.
   return il_push_back( il );
}

void il_erase( IntList *il, int n )
{
   // Push the element to the free list.
   const int pos    = n * il->num_fields;
   il->data[pos]    = il->free_element;
   il->free_element = n;
}
