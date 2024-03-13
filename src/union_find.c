/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file union_find.c
 *
 * @brief Implements ye olde disjoint-set-forest data structure.
 */
#include "union_find.h"

#include "array.h"

/** @brief Creates a UnionFind structure on {0, ..., n}. */
void unionfind_init( UnionFind *uf, int n )
{
   uf->parent = array_create_size( int, n );
   uf->rank   = array_create_size( int, n );
   for ( int i = 0; i < n; i++ ) {
      array_push_back( &uf->parent, i );
      array_push_back( &uf->rank, 0 );
   }
}

/** @brief Frees resources associated with uf. */
void unionfind_free( UnionFind *uf )
{
   array_free( uf->parent );
   uf->parent = NULL;
   array_free( uf->rank );
   uf->rank = NULL;
}

/** @brief Declares x and y to be in the same subset. */
void unionfind_union( UnionFind *uf, int x, int y )
{
   x = unionfind_find( uf, x );
   y = unionfind_find( uf, y );
   if ( uf->rank[x] > uf->rank[y] )
      uf->parent[y] = x;
   else {
      uf->parent[x] = y;
      if ( uf->rank[x] == uf->rank[y] )
         uf->rank[y]++;
   }
}

/** @brief Finds the designated representative of the subset containing x. */
int unionfind_find( UnionFind *uf, int x )
{
   if ( uf->parent[x] != x )
      uf->parent[x] = unionfind_find( uf, uf->parent[x] );
   return uf->parent[x];
}

/** @brief Returns a designated representative of each subset in an array
 * (array.h). */
int *unionfind_findall( UnionFind *uf )
{
   int *a = array_create( int );
   for ( int i = 0; i < array_size( uf->parent ); i++ )
      if ( uf->parent[i] == i )
         array_push_back( &a, i );
   return a;
}
