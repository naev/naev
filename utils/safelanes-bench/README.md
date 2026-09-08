# safelanes-bench

Compares sparse solvers that could replace CHOLMOD in `safelanes.c`.

    cargo run --release

It uses `problems/universe.txt` unless you pass it a file.

Each solver does what the game does in one turn: factor the matrix, solve it,
multiply the result through `QtQ`, then solve again for the gradient that picks
which lanes get built. It reports how long that took and how far its answer is
from CHOLMOD's, which is what the game uses now. Check the answers agree before
paying attention to the times.

Agreement is measured as a relative L2 difference over the whole solution, with
the largest single difference alongside it. A per-element relative error is not
useful here, because the values run over many orders of magnitude and the ones
near zero make it look terrible no matter how good the solve was.

Be careful what you conclude from it. Matching to a dozen digits says the
solver is sound, not that the game builds the same lanes. The gradient is used
to rank candidate lanes against each other, and ranks can swap on differences
far smaller than these. The only way to settle that is to swap the solver in
and compare the lanes the game actually produces.

## What is being solved

Safe lanes are worked out for the whole universe at once, not one star system at
a time. There is one unknown for every place a lane can end: each planet or
station that can anchor a lane, and each jump point. That single matrix is then
solved against one load per anchoring planet or station.

## Making your own

Add `#include "physfs.h"` to `safelanes.c`, then add this and call it at the
top of `safelanes_buildOneTurn`:

```c
static void dump_write( PHYSFS_File *f, const char *fmt, ... )
{
   char    buf[256];
   va_list ap;
   int     len;
   va_start( ap, fmt );
   len = vsnprintf( buf, sizeof( buf ), fmt, ap );
   va_end( ap );
   PHYSFS_writeBytes( f, buf, len );
}

static void dump_system( void )
{
   static int   done = 0;
   PHYSFS_File *f;
   double      *fx;
   size_t       nz = 0;

   if ( done )
      return;
   done = 1;
   f = PHYSFS_openWrite( "logs/safelanes-system.txt" );
   if ( f == NULL ) {
      WARN( _( "Unable to write safelanes system: %s" ),
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      return;
   }

   dump_write( f, "stiff %zu %zu %zu\n", stiff->nrow, stiff->ncol, stiff->nnz );
   for ( size_t k = 0; k < stiff->nnz; k++ )
      dump_write( f, "%d %d %.17g\n", ( (int *)stiff->i )[k],
                  ( (int *)stiff->j )[k], ( (double *)stiff->x )[k] );

   /* Dense in cholmod, but built from a sparse matrix and almost all zero. */
   fx = (double *)ftilde->x;
   for ( size_t k = 0; k < ftilde->nrow * ftilde->ncol; k++ )
      if ( fx[k] != 0. )
         nz++;
   dump_write( f, "ftilde %zu %zu %zu\n", ftilde->nrow, ftilde->ncol, nz );
   for ( size_t c = 0; c < ftilde->ncol; c++ )
      for ( size_t r = 0; r < ftilde->nrow; r++ )
         if ( fx[c * ftilde->nrow + r] != 0. )
            dump_write( f, "%zu %zu %.17g\n", r, c, fx[c * ftilde->nrow + r] );

   dump_write( f, "qtq %zu %zu %zu\n", QtQ->nrow, QtQ->ncol,
               cholmod_nnz( QtQ, &C ) );
   for ( size_t c = 0; c < QtQ->ncol; c++ )
      for ( int k = ( (int *)QtQ->p )[c]; k < ( (int *)QtQ->p )[c + 1]; k++ )
         dump_write( f, "%d %zu %.17g\n", ( (int *)QtQ->i )[k], c,
                     ( (double *)QtQ->x )[k] );

   PHYSFS_close( f );
   DEBUG( "wrote logs/safelanes-system.txt" );
}
```

Safe lanes are charted when the main menu loads, so running the game once is
enough. Turn plugins off if you want the stock universe. The file lands under
`logs/` in naev's write directory.

## Reading the file

Two things will catch you out:

- The matrix is marked upper triangular, but it holds entries below the
  diagonal as well, and the same entry can appear several times. CHOLMOD moves
  the low ones up and adds the repeats together. Anything reading the file has
  to do the same, or it will solve a different matrix and look fast doing it.
- `QtQ` is not triangular. Both halves are stored.
