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
#pragma once

typedef struct IntList IntList;
enum { il_fixed_cap = 128 };

struct IntList {
   // Stores a fixed-size buffer in advance to avoid requiring
   // a heap allocation until we run out of space.
   int fixed[il_fixed_cap];

   // Points to the buffer used by the list. Initially this will
   // point to 'fixed'.
   int *data;

   // Stores how many integer fields each element has.
   int num_fields;

   // Stores the number of elements in the list.
   int num;

   // Stores the capacity of the array.
   int cap;

   // Stores an index to the free element or -1 if the free list
   // is empty.
   int free_element;
};

// ---------------------------------------------------------------------------------
// List Interface
// ---------------------------------------------------------------------------------
// Creates a new list of elements which each consist of integer fields.
// 'num_fields' specifies the number of integer fields each element has.
void il_create( IntList *il, int num_fields );

// Destroys the specified list.
void il_destroy( IntList *il );

// Returns the number of elements in the list.
int il_size( const IntList *il );

// Returns the value of the specified field for the nth element.
int il_get( const IntList *il, int n, int field );

// Sets the value of the specified field for the nth element.
void il_set( IntList *il, int n, int field, int val );

// Clears the specified list, making it empty.
void il_clear( IntList *il );

// ---------------------------------------------------------------------------------
// Stack Interface (do not mix with free list usage; use one or the other)
// ---------------------------------------------------------------------------------
// Inserts an element to the back of the list and returns an index to it.
int il_push_back( IntList *il );

// Removes the element at the back of the list.
void il_pop_back( IntList *il );

// ---------------------------------------------------------------------------------
// Free List Interface (do not mix with stack usage; use one or the other)
// ---------------------------------------------------------------------------------
// Inserts an element to a vacant position in the list and returns an index to
// it.
int il_insert( IntList *il );

// Removes the nth element in the list.
void il_erase( IntList *il, int n );
