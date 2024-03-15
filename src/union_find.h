/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @brief Disjoint set forest on {0, .., n-1}.  */
typedef struct UnionFind_ {
   int *parent; /**< parent[x] is the parent node of x. */
   int *rank;   /**< rank[x] is an upper bound on the height of x. */
} UnionFind;

void unionfind_init( UnionFind *uf, int n );
void unionfind_free( UnionFind *uf );
void unionfind_union( UnionFind *uf, int x, int y );
int  unionfind_find( UnionFind *uf, int x );
int *unionfind_findall( UnionFind *uf );
