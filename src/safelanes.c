/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file safelanes.c
 *
 * @brief Handles factions' safe lanes through systems.
 * This implements the algorithm described in utils/lanes-generator (whitepaper
 * and much clearer Python version).
 */
/** @cond */
#include <math.h>

#if HAVE_SUITESPARSE_CHOLMOD_H
#include <suitesparse/cholmod.h>
#else /* HAVE_SUITESPARSE_CHOLMOD_H */
#include <cholmod.h>
#endif /* HAVE_SUITESPARSE_CHOLMOD_H */

#if HAVE_OPENBLAS_CBLAS_H
#include <openblas/cblas.h>
#elif HAVE_CBLAS_OPENBLAS_H
#include <cblas_openblas.h>
#elif HAVE_CBLAS_HYPHEN_OPENBLAS_H
#include <cblas-openblas.h>
#elif HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#elif HAVE_CBLAS_H
#include <cblas.h>
#elif HAVE_F77BLAS_H
#include <f77blas.h>
#define I_LOVE_FORTRAN 1
#elif HAVE_OPENBLAS_F77BLAS_H
#include <openblas/f77blas.h>
#define I_LOVE_FORTRAN 1
#endif

#include "naev.h"
/** @endcond */

#include "safelanes.h"

#include "array.h"
#include "conf.h"
#include "log.h"
#include "union_find.h"

/*
 * Global parameters.
 */
static const double ALPHA  = 9.;   /**< Lane efficiency parameter. */
static const double LAMBDA = 2e10; /**< Regularization term for score. */
static const double JUMP_CONDUCTIVITY =
   0.001; /**< Conductivity value for inter-system jump-point connections. */
static const double MIN_ANGLE =
   M_PI / 18.; /**< Path triangles can't be more acute. */
enum {
   STORAGE_MODE_LOWER_TRIANGULAR_PART =
      -1, /**< A CHOLMOD "stype" value: matrix is interpreted as symmetric. */
   STORAGE_MODE_UNSYMMETRIC =
      0, /**< A CHOLMOD "stype" value: matrix holds whatever we put in it. */
   STORAGE_MODE_UPPER_TRIANGULAR_PART =
      +1, /**< A CHOLMOD "stype" value: matrix is interpreted as symmetric. */
   SORTED         = 1, /**< a named bool */
   PACKED         = 1, /**< a named bool */
   MODE_NUMERICAL = 1, /**< yet another CHOLMOD magic number! */
};

/*
 * Types.
 */
/** @brief Object type: like SafeLaneLocType, but with Naev stack indexing in
 * mind.
 *  @TODO Converge these after beefing up nlua APIs (#1862 or just jump point
 * from internal indices).
 */
typedef enum VertexType_ { VERTEX_SPOB, VERTEX_JUMP } VertexType;

/** @brief Reference to a spob or jump point. */
typedef struct Vertex_ {
   int        system; /**< ID of the system containing the object. */
   VertexType type;   /**< Which of Naev's list contains it? */
   int        index;  /**< Index in the system's spobs or jumps array. */
} Vertex;

/** @brief An edge is a pair of vertex indices. */
typedef int Edge[2];

/** @brief Description of a lane-building faction. */
typedef struct Faction_ {
   int    id;                       /**< Faction ID. */
   double lane_length_per_presence; /**< Weight determining their ability to
                                       claim lanes. */
   double lane_base_cost;           /**< Base cost of a lane. */
} Faction;

/** @brief A set of lane-building factions, represented as a bitfield. */
typedef uint32_t         FactionMask;
static const FactionMask MASK_0 = 0, MASK_1 = 1;

/*
 * Global state.
 */
static cholmod_common C; /**< Parameter settings, statistics, and workspace used
                            internally by CHOLMOD. */
static Vertex *vertex_stack; /**< Array (array.h): Everything eligible to be a
                                lane endpoint. */
static FactionMask *vertex_fmask; /**< Malloced: Per vertex, the set of factions
                                     that have built on it. */
static int *sys_to_first_vertex;  /**< Array (array.h): For each system index,
                                     the id of its first vertex, + sentinel. */
static Edge
   *edge_stack; /**< Array (array.h): Everything eligible to be a lane. */
static int *sys_to_first_edge; /**< Array (array.h): For each system index, the
                                  id of its first edge, + sentinel. */
static Faction *
   faction_stack; /**< Array (array.h): The faction IDs that can build lanes. */
static int *lane_faction; /**< Array (array.h): Per edge, ID of faction that
                             built a lane there, if any, else 0. */
static FactionMask *lane_fmask; /**< Array (array.h): Per edge, the set of
                                   factions that may build it. */
static double *
   *presence_budget; /**< Array (array.h): Per faction, per system, the amount
                        of presence not yet spent on lanes. */
static int *tmp_spob_indices; /**< Array (array.h): The vertex IDs of spobs, to
                                 set up ftilde/PPl. Unrelated to spob IDs. */
static Edge *tmp_jump_edges; /**< Array (array.h): The vertex ID pairs connected
                                by 2-way jumps. Used to set up "stiff". */
static double
   *tmp_edge_conduct; /**< Array (array.h): Conductivity (1/len) of each
                         potential lane. Used to set up "stiff". */
static int
   *tmp_anchor_vertices; /**< Array (array.h): One vertex ID per connected
                            component. Used to set up "stiff". */
static UnionFind
   tmp_sys_uf; /**< The partition of {system indices} into connected components
                  (connected by 2-way jumps). */
static cholmod_triplet
   *stiff; /**< K matrix, UT triplets: internal edges (E*3), implicit jump
              connections, anchor conditions. */
static cholmod_sparse *QtQ; /**< (Q*)Q where Q is the ExV difference matrix. */
static cholmod_dense
   *ftilde; /**< Fluxes (bunch of F columns in the KU=F problem). */
static cholmod_dense
   *utilde; /**< Potentials (bunch of U columns in the KU=F problem). */
static cholmod_dense **PPl; /**< Array: (array.h): For each builder faction, The
                               (P*)P in: grad_u(phi)=(Q*)Q U~ (P*)P. */
static double *cmp_key_ref; /**< To qsort() a list of indices by table value,
                               point this at your table and use cmp_key. */
static int safelanes_calculated_once =
   0; /**< Whether or not the safe lanes have been computed once. */

/*
 * Prototypes.
 */
static int    safelanes_buildOneTurn( int iters_done );
static int    safelanes_activateByGradient( const cholmod_dense *Lambda_tilde,
                                            int                  iters_done );
static void   safelanes_initStacks( void );
static void   safelanes_initStacks_edge( void );
static void   safelanes_initStacks_faction( void );
static void   safelanes_initStacks_vertex( void );
static void   safelanes_initStacks_anchor( void );
static void   safelanes_initOptimizer( void );
static void   safelanes_destroyOptimizer( void );
static void   safelanes_destroyStacks( void );
static void   safelanes_destroyTmp( void );
static void   safelanes_initStiff( void );
static double safelanes_initialConductivity( int ei );
static void   safelanes_updateConductivity( int ei_activated );
static void   safelanes_initQtQ( void );
static void   safelanes_initFTilde( void );
static void   safelanes_initPPl( void );
static int    safelanes_triangleTooFlat( const vec2 *m, const vec2 *n,
                                         const vec2 *p, double lmn );
static int    vertex_faction( int vi );
static const vec2        *vertex_pos( int vi );
static inline int         FACTION_ID_TO_INDEX( int id );
static inline FactionMask MASK_ANY_FACTION();
static inline FactionMask MASK_ONE_FACTION( int id );
static inline FactionMask MASK_COMPROMISE( int id1, int id2 );
static int                cmp_key( const void *p1, const void *p2 );
static inline void triplet_entry( cholmod_triplet *m, int i, int j, double v );
static cholmod_dense *safelanes_sliceByPresence( const cholmod_dense *m,
                                                 const double *sysPresence );
static cholmod_dense *ncholmod_ddmult( cholmod_dense *A, int transA,
                                       cholmod_dense *B );
static double safelanes_row_dot_row( cholmod_dense *A, cholmod_dense *B, int i,
                                     int j );

/**
 * @brief Like array_push_back( a, Edge{v0, v1} ), but achievable in C. :-P
 */
static inline void array_push_back_edge( Edge **a, int v0, int v1 )
{
   int *vs = array_grow( a );
   vs[0]   = v0;
   vs[1]   = v1;
}

/**
 * @brief Initializes the safelanes system.
 */
void safelanes_init( void )
{
   cholmod_start( &C );
   /* Ideally we would want to recalculate here, but since we load the first
    * save and try to use unidiffs there, we instead defer the safe lane
    * computation to only if necessary after loading save unidiffs. */
   /* safelanes_recalculate(); */
}

/**
 * @brief Shuts down the safelanes system.
 */
void safelanes_destroy( void )
{
   safelanes_destroyOptimizer();
   safelanes_destroyStacks();
   cholmod_finish( &C );
}

/**
 * @brief Gets a set of safelanes for a faction and system.
 *    @param faction ID of the faction whose lanes we want, or a negative value
 * signifying "all of them".
 *    @param standing Bit-mask indicating what standing to get.
 *    @param system Star system whose lanes we want.
 *    @return Array (array.h) of matching SafeLane structures. Caller frees.
 */
SafeLane *safelanes_get( int faction, int standing, const StarSystem *system )
{
   SafeLane *out = array_create( SafeLane );

   for ( int i = sys_to_first_edge[system->id];
         i < sys_to_first_edge[1 + system->id]; i++ ) {
      SafeLane     *l;
      const Vertex *v[2];
      int           lf = lane_faction[i];

      /* No lane on edge. */
      if ( lf <= 0 )
         continue;

      /* Filter by standing. */
      if ( faction >= 0 ) {
         if ( standing == 0 ) {
            /* Only match exact faction. */
            if ( lf != faction )
               continue;
         } else {
            /* Try to do more advanced matching. */
            int fe   = areEnemies( faction, lf );
            int fa   = areAllies( faction, lf );
            int skip = 1;
            if ( ( standing & SAFELANES_FRIENDLY ) && fa )
               skip = 0;
            if ( ( standing & SAFELANES_NEUTRAL ) && !fe )
               skip = 0;
            if ( ( standing & SAFELANES_HOSTILE ) && fe )
               skip = 0;
            if ( skip )
               continue;
         }
      }

      for ( int j = 0; j < 2; j++ )
         v[j] = &vertex_stack[edge_stack[i][j]];

      l          = &array_grow( &out );
      l->faction = lane_faction[i];
      for ( int j = 0; j < 2; j++ ) {
         switch ( v[j]->type ) {
         case VERTEX_SPOB:
            l->point_type[j] = SAFELANE_LOC_SPOB;
            l->point_id[j]   = system->spobs[v[j]->index]->id;
            break;
         case VERTEX_JUMP:
            l->point_type[j] = SAFELANE_LOC_DEST_SYS;
            l->point_id[j]   = system->jumps[v[j]->index].targetid;
            break;
         default:
            ERR( _( "Safe-lane vertex type is invalid." ) );
         }
      }
   }
   return out;
}

/**
 * @brief Update the safe lane locations in response to the universe changing
 * (e.g., diff applied).
 */
void safelanes_recalculate( void )
{
#if DEBUGGING
   Uint32 time = SDL_GetTicks();
#endif /* DEBUGGING */

   /* Don't recompute on exit. */
   if ( naev_isQuit() )
      return;

   safelanes_initStacks();
   safelanes_initOptimizer();
   for ( int iters_done = 0; safelanes_buildOneTurn( iters_done ) > 0;
         iters_done++ )
      ;
   safelanes_destroyOptimizer();
   /* Stacks remain available for queries. */
#if DEBUGGING
   if ( conf.devmode )
      DEBUG( n_( "Charted safe lanes for %d object in %.3f s",
                 "Charted safe lanes for %d objects in %.3f s",
                 array_size( vertex_stack ) ),
             array_size( vertex_stack ), ( SDL_GetTicks() - time ) / 1000. );
#endif /* DEBUGGING */

   safelanes_calculated_once = 1;
}

/**
 * @brief Whether or not the safe lanes have been calculated at least once.
 */
int safelanes_calculated( void )
{
   return safelanes_calculated_once;
}

/**
 * @brief Initializes resources used by lane optimization.
 */
static void safelanes_initOptimizer( void )
{
   safelanes_initStiff();
   safelanes_initQtQ();
   safelanes_initFTilde();
   safelanes_initPPl();
   safelanes_destroyTmp();
}

/**
 * @brief Frees resources used by lane optimization.
 */
static void safelanes_destroyOptimizer( void )
{
   for ( int i = 0; i < array_size( PPl ); i++ )
      cholmod_free_dense( &PPl[i], &C );
   array_free( PPl );
   PPl = NULL;
   cholmod_free_dense( &utilde,
                       &C ); /* CAUTION: if we instead save it, ensure it's
                                updated after the final activateByGradient. */
   cholmod_free_dense( &ftilde, &C );
   cholmod_free_sparse( &QtQ, &C );
   cholmod_free_triplet( &stiff, &C );
}

/**
 * @brief Run a round of optimization. Return how many builds (upper bound) have
 * to happen next turn.
 */
static int safelanes_buildOneTurn( int iters_done )
{
   cholmod_sparse *stiff_s;
   cholmod_factor *stiff_f;
   cholmod_dense  *_QtQutilde, *Lambda_tilde, *Y_workspace, *E_workspace;
   int             turns_next_time;
   double          zero[] = { 0, 0 }, neg_1[] = { -1, 0 };

   Y_workspace = E_workspace = Lambda_tilde = NULL;
   stiff_s = cholmod_triplet_to_sparse( stiff, 0, &C );
   stiff_f = cholmod_analyze( stiff_s, &C );
   cholmod_factorize( stiff_s, stiff_f, &C );
   cholmod_solve2( CHOLMOD_A, stiff_f, ftilde, NULL, &utilde, NULL,
                   &Y_workspace, &E_workspace, &C );
   _QtQutilde = cholmod_zeros( utilde->nrow, utilde->ncol, CHOLMOD_REAL, &C );
   cholmod_sdmult( QtQ, 0, neg_1, zero, utilde, _QtQutilde, &C );
   cholmod_solve2( CHOLMOD_A, stiff_f, _QtQutilde, NULL, &Lambda_tilde, NULL,
                   &Y_workspace, &E_workspace, &C );
   cholmod_free_dense( &_QtQutilde, &C );
   cholmod_free_dense( &Y_workspace, &C );
   cholmod_free_dense( &E_workspace, &C );
   cholmod_free_factor( &stiff_f, &C );
   cholmod_free_sparse( &stiff_s, &C );
   turns_next_time = safelanes_activateByGradient( Lambda_tilde, iters_done );
   cholmod_free_dense( &Lambda_tilde, &C );

   return turns_next_time;
}

/**
 * @brief Sets up the local faction/object stacks.
 */
static void safelanes_initStacks( void )
{
   safelanes_destroyStacks();
   safelanes_initStacks_faction(); /* Dependency for vertex. */
   safelanes_initStacks_vertex();  /* Dependency for edge. */
   safelanes_initStacks_edge();
   safelanes_initStacks_anchor();
}

/**
 * @brief Sets up the local stacks with entry per vertex (or per jump).
 */
static void safelanes_initStacks_vertex( void )
{
   const StarSystem *systems_stack = system_getAll();

   vertex_stack        = array_create( Vertex );
   sys_to_first_vertex = array_create( int );
   array_push_back( &sys_to_first_vertex, 0 );
   tmp_spob_indices = array_create( int );
   tmp_jump_edges   = array_create( Edge );
   for ( int system = 0; system < array_size( systems_stack ); system++ ) {
      const StarSystem *sys = &systems_stack[system];
      if ( sys_isFlag( sys, SYSTEM_NOLANES ) ) {
         array_push_back( &sys_to_first_vertex, array_size( vertex_stack ) );
         continue;
      }

      for ( int i = 0; i < array_size( sys->spobs ); i++ ) {
         const Spob *p = sys->spobs[i];
         if ( spob_isFlag( p, SPOB_NOLANES ) )
            continue;
         if ( p->presence.base != 0. || p->presence.bonus != 0. ) {
            Vertex v = { .system = system, .type = VERTEX_SPOB, .index = i };
            array_push_back( &tmp_spob_indices, array_size( vertex_stack ) );
            array_push_back( &vertex_stack, v );
         }
      }

      for ( int i = 0; i < array_size( sys->jumps ); i++ ) {
         const JumpPoint *jp = &sys->jumps[i];
         if ( jp_isFlag( jp, JP_HIDDEN | JP_EXITONLY | JP_NOLANES ) )
            continue;
         Vertex v = { .system = system, .type = VERTEX_JUMP, .index = i };
         array_push_back( &vertex_stack, v );
         if ( jp->targetid < system && jp->returnJump != NULL )
            for ( int j = sys_to_first_vertex[jp->targetid];
                  j < sys_to_first_vertex[1 + jp->targetid]; j++ )
               if ( vertex_stack[j].type == VERTEX_JUMP &&
                    jp->returnJump ==
                       &jp->target->jumps[vertex_stack[j].index] ) {
                  array_push_back_edge( &tmp_jump_edges,
                                        array_size( vertex_stack ) - 1, j );
                  break;
               }
      }

      array_push_back( &sys_to_first_vertex, array_size( vertex_stack ) );
   }
   // array_shrink( &vertex_stack );
   // array_shrink( &sys_to_first_vertex );

   vertex_fmask = calloc( array_size( vertex_stack ), sizeof( FactionMask ) );
}

/**
 * @brief Sets up the local stacks with entry per edge. Faction stack must be
 * set up.
 */
static void safelanes_initStacks_edge( void )
{
   const StarSystem *systems_stack = system_getAll();

   edge_stack        = array_create( Edge );
   sys_to_first_edge = array_create( int );
   array_push_back( &sys_to_first_edge, 0 );
   lane_fmask       = array_create( FactionMask );
   tmp_edge_conduct = array_create( double );
   for ( int system = 0; system < array_size( systems_stack ); system++ ) {
      for ( int i = sys_to_first_vertex[system];
            i < sys_to_first_vertex[1 + system]; i++ ) {
         const vec2 *pi = vertex_pos( i );
         for ( int j = sys_to_first_vertex[system]; j < i; j++ ) {
            const vec2 *pj                  = vertex_pos( j );
            double      lij                 = vec2_dist( pi, pj );
            int         has_approx_midpoint = 0;
            for ( int k = sys_to_first_vertex[system];
                  k < sys_to_first_vertex[1 + system]; k++ )
               if ( k != i && k != j &&
                    safelanes_triangleTooFlat( pi, pj, vertex_pos( k ),
                                               lij ) ) {
                  has_approx_midpoint = 1;
                  break;
               }
            if ( has_approx_midpoint )
               continue; /* The edge from i to j is disallowed in favor of a
                            path through k. */
            array_push_back_edge( &edge_stack, j, i );
            array_push_back(
               &lane_fmask,
               MASK_COMPROMISE( vertex_faction( i ), vertex_faction( j ) ) );
            array_push_back( &tmp_edge_conduct, 1 / lij );
         }
      }
      array_push_back( &sys_to_first_edge, array_size( edge_stack ) );
   }
   array_shrink( &edge_stack );
   array_shrink( &sys_to_first_edge );

   lane_faction = array_create_size( int, array_size( edge_stack ) );
   array_resize( &lane_faction, array_size( edge_stack ) );
   memset( lane_faction, 0,
           array_size( lane_faction ) * sizeof( lane_faction[0] ) );
}

/**
 * @brief Sets up the local stacks with entry per faction.
 */
static void safelanes_initStacks_faction( void )
{
   int              *faction_all;
   const StarSystem *systems_stack;

   faction_stack = array_create( Faction );
   faction_all   = faction_getAllVisible();
   for ( int fi = 0; fi < array_size( faction_all ); fi++ ) {
      int     f   = faction_all[fi];
      Faction rec = { .id = f,
                      .lane_length_per_presence =
                         faction_lane_length_per_presence( f ),
                      .lane_base_cost = faction_lane_base_cost( f ) };
      if ( rec.lane_length_per_presence > 0. )
         array_push_back( &faction_stack, rec );
   }
   array_free( faction_all );
   array_shrink( &faction_stack );
   assert( "FactionMask size is sufficient" &&
           (size_t)array_size( faction_stack ) <= 8 * sizeof( FactionMask ) );

   presence_budget = array_create_size( double *, array_size( faction_stack ) );
   systems_stack   = system_getAll();
   for ( int fi = 0; fi < array_size( faction_stack ); fi++ ) {
      array_push_back(
         &presence_budget,
         array_create_size( double, array_size( systems_stack ) ) );
      for ( int s = 0; s < array_size( systems_stack ); s++ ) {
         const StarSystem *sys = &systems_stack[s];
         double budget = system_getPresence( sys, faction_stack[fi].id );
         array_push_back( &presence_budget[fi], budget );
      }
   }
}

/**
 * @brief Identifies anchor points:
 * The universe graph (with in-system and 2-way-jump edges) could have many
 * connected components. Our impedance problem needs one boundary condition for
 * each, so we choose a representative vertex from each.
 */
static void safelanes_initStacks_anchor( void )
{
   int *anchor_systems;
   int  nsys = array_size( sys_to_first_vertex ) - 1;
   unionfind_init( &tmp_sys_uf, nsys );
   for ( int i = 0; i < array_size( tmp_jump_edges ); i++ )
      unionfind_union( &tmp_sys_uf, vertex_stack[tmp_jump_edges[i][0]].system,
                       vertex_stack[tmp_jump_edges[i][1]].system );
   anchor_systems      = unionfind_findall( &tmp_sys_uf );
   tmp_anchor_vertices = array_create_size( int, array_size( anchor_systems ) );

   /* Add an anchor vertex per system, but only if there actually is a vertex in
    * the system. */
   for ( int i = 0; i < array_size( anchor_systems ); i++ )
      if ( sys_to_first_vertex[anchor_systems[i]] <
           sys_to_first_vertex[1 + anchor_systems[i]] )
         array_push_back( &tmp_anchor_vertices,
                          sys_to_first_vertex[anchor_systems[i]] );
   array_free( anchor_systems );
}

/**
 * @brief Tears down the local faction/object stacks.
 */
static void safelanes_destroyStacks( void )
{
   safelanes_destroyTmp();
   array_free( vertex_stack );
   vertex_stack = NULL;
   free( vertex_fmask );
   vertex_fmask = NULL;
   array_free( sys_to_first_vertex );
   sys_to_first_vertex = NULL;
   array_free( edge_stack );
   edge_stack = NULL;
   array_free( sys_to_first_edge );
   sys_to_first_edge = NULL;
   for ( int i = 0; i < array_size( presence_budget ); i++ )
      array_free( presence_budget[i] );
   array_free( presence_budget );
   presence_budget = NULL;
   array_free( faction_stack );
   faction_stack = NULL;
   array_free( lane_faction );
   lane_faction = NULL;
   array_free( lane_fmask );
   lane_fmask = NULL;
}

/**
 * @brief Tears down the local faction/object stacks.
 */
static void safelanes_destroyTmp( void )
{
   unionfind_free( &tmp_sys_uf );
   array_free( tmp_spob_indices );
   tmp_spob_indices = NULL;
   array_free( tmp_jump_edges );
   tmp_jump_edges = NULL;
   array_free( tmp_edge_conduct );
   tmp_edge_conduct = NULL;
   array_free( tmp_anchor_vertices );
   tmp_anchor_vertices = NULL;
}

/**
 * @brief Sets up the stiffness matrix.
 */
static void safelanes_initStiff( void )
{
   int    nnz, v;
   double max_conductivity;

   cholmod_free_triplet( &stiff, &C );
   v   = array_size( vertex_stack );
   nnz = 3 * ( array_size( edge_stack ) + array_size( tmp_jump_edges ) ) +
         array_size( tmp_anchor_vertices );
   stiff = cholmod_allocate_triplet(
      v, v, nnz, STORAGE_MODE_UPPER_TRIANGULAR_PART, CHOLMOD_REAL, &C );
   /* Populate triplets: internal edges (ii ij jj), implicit jump connections
    * (ditto), anchor conditions. */
   for ( int i = 0; i < array_size( edge_stack ); i++ ) {
      triplet_entry( stiff, edge_stack[i][0], edge_stack[i][0],
                     +tmp_edge_conduct[i] );
      triplet_entry( stiff, edge_stack[i][0], edge_stack[i][1],
                     -tmp_edge_conduct[i] );
      triplet_entry( stiff, edge_stack[i][1], edge_stack[i][1],
                     +tmp_edge_conduct[i] );
   }
   for ( int i = 0; i < array_size( tmp_jump_edges ); i++ ) {
      triplet_entry( stiff, tmp_jump_edges[i][0], tmp_jump_edges[i][0],
                     +JUMP_CONDUCTIVITY );
      triplet_entry( stiff, tmp_jump_edges[i][0], tmp_jump_edges[i][1],
                     -JUMP_CONDUCTIVITY );
      triplet_entry( stiff, tmp_jump_edges[i][1], tmp_jump_edges[i][1],
                     +JUMP_CONDUCTIVITY );
   }
   /* Add a Robin boundary condition, using the max conductivity (after
    * activation) for spectral reasons. */
   max_conductivity = JUMP_CONDUCTIVITY / ( 1 + ALPHA );
   for ( int i = 0; i < array_size( edge_stack ); i++ )
      max_conductivity = MAX( max_conductivity, tmp_edge_conduct[i] );
   max_conductivity = MAX(
      JUMP_CONDUCTIVITY,
      ( 1 + ALPHA ) *
         max_conductivity ); /* Activation scales entries by 1+ALPHA later. */
   for ( int i = 0; i < array_size( tmp_anchor_vertices ); i++ )
      triplet_entry( stiff, tmp_anchor_vertices[i], tmp_anchor_vertices[i],
                     max_conductivity );
#if DEBUGGING
   assert( stiff->nnz == stiff->nzmax );
   assert( cholmod_check_triplet( stiff, &C ) );
#endif /* DEBUGGING */
}

/**
 * @brief Returns the initial conductivity value (1/length) for edge ei.
 * The live value is stored in the stiffness matrix; \see safelanes_initStiff
 * above. When a lane is activated, its conductivity is updated to
 * (1+ALPHA)/length.
 */
static double safelanes_initialConductivity( int ei )
{
   double *sv = stiff->x;
   return lane_faction[ei] ? sv[3 * ei] / ( 1 + ALPHA ) : sv[3 * ei];
}

/**
 * @brief Updates the stiffness matrix to account for the given edge being
 * activated. \see safelanes_initStiff.
 */
static void safelanes_updateConductivity( int ei_activated )
{
   double *sv = stiff->x;
   for ( int i = 3 * ei_activated; i < 3 * ( ei_activated + 1 ); i++ )
      sv[i] *= 1 + ALPHA;
}

/**
 * @brief Sets up the (Q*)Q matrix.
 */
static void safelanes_initQtQ( void )
{
   cholmod_sparse *Q;

   cholmod_free_sparse( &QtQ, &C );
   /* Form Q, the edge-vertex projection where (Dirac notation) Q |edge> =
    * |edge[0]> - |edge[1]>. It has a +1 and -1 per column. */
   Q                  = cholmod_allocate_sparse( array_size( vertex_stack ),
                                                 array_size( edge_stack ),
                                                 2 * array_size( edge_stack ), SORTED, PACKED,
                                                 STORAGE_MODE_UNSYMMETRIC, CHOLMOD_REAL, &C );
   ( (int *)Q->p )[0] = 0;
   for ( int i = 0; i < array_size( edge_stack ); i++ ) {
      ( (int *)Q->p )[i + 1]        = 2 * ( i + 1 );
      ( (int *)Q->i )[2 * i + 0]    = edge_stack[i][0];
      ( (int *)Q->i )[2 * i + 1]    = edge_stack[i][1];
      ( (double *)Q->x )[2 * i + 0] = +1;
      ( (double *)Q->x )[2 * i + 1] = -1;
   }
#if DEBUGGING
   assert( cholmod_check_sparse( Q, &C ) );
#endif /* DEBUGGING */
   QtQ = cholmod_aat( Q, NULL, 0, MODE_NUMERICAL, &C );
   cholmod_free_sparse( &Q, &C );
}

/**
 * @brief Sets up the fluxes matrix f~.
 */
static void safelanes_initFTilde( void )
{
   cholmod_sparse *eye =
      cholmod_speye( array_size( vertex_stack ), array_size( vertex_stack ),
                     CHOLMOD_REAL, &C );
   cholmod_sparse *sp =
      cholmod_submatrix( eye, NULL, -1, tmp_spob_indices,
                         array_size( tmp_spob_indices ), 1, SORTED, &C );
   cholmod_free_dense( &ftilde, &C );
   ftilde = cholmod_sparse_to_dense( sp, &C );
   cholmod_free_sparse( &sp, &C );
   cholmod_free_sparse( &eye, &C );
}

/**
 * @brief Sets up the PPl matrices appearing in the gradient formula.
 */
static void safelanes_initPPl( void )
{
   int *component;
   int  np = array_size( tmp_spob_indices );

   for ( int fi = 0; fi < array_size( PPl ); fi++ )
      cholmod_free_dense( &PPl[fi], &C );
   array_free( PPl );
   PPl = array_create_size( cholmod_dense *, array_size( faction_stack ) );
   for ( int fi = 0; fi < array_size( faction_stack ); fi++ )
      array_push_back( &PPl, cholmod_zeros( np, np, CHOLMOD_REAL, &C ) );

   /* Form P, the pair-vertex projection where (Dirac notation) P |pair(i,j)> =
    * |i> - |j>. It has a +1 and -1 per column. */
   /* At least, pretend we did. We want (PD)(PD)*, where D is a diagonal matrix
    * whose pair(i,j) are these presence sums: */

   component = calloc( np, sizeof( int ) );
   for ( int i = 0; i < np; i++ )
      component[i] = unionfind_find( &tmp_sys_uf,
                                     vertex_stack[tmp_spob_indices[i]].system );

   for ( int i = 0; i < np; i++ ) {
      double *Di;
      int     sys = vertex_stack[tmp_spob_indices[i]].system;
      Spob   *pnt =
         system_getIndex( sys )->spobs[vertex_stack[tmp_spob_indices[i]].index];
      double pres =
         pnt->presence.base +
         pnt->presence.bonus; /* TODO distinguish between base and bonus? */
      int fi = FACTION_ID_TO_INDEX( pnt->presence.faction );
      if ( fi < 0 )
         continue;
      Di = PPl[fi]->x;
      for ( int j = 0; j < i; j++ )
         if ( component[i] == component[j] )
            Di[np * i + j] += pres;
      for ( int j = i + 1; j < np; j++ )
         if ( component[i] == component[j] )
            Di[np * j + i] += pres;
   }

   /* At this point, PPl[fi]->x[np*i+j] holds the pair(i,j) entry of D. */
   for ( int fi = 0; fi < array_size( faction_stack ); fi++ )
      for ( int i = 0; i < np; i++ )
         for ( int j = 0; j < i; j++ ) {
            double d = ( (double *)PPl[fi]->x )[np * i + j];
            d *= d;
            ( (double *)PPl[fi]->x )[np * i + j] = -d;
            ( (double *)PPl[fi]->x )[np * j + i] = -d;
            ( (double *)PPl[fi]->x )[np * i + i] += d;
            ( (double *)PPl[fi]->x )[np * j + j] += d;
         }

   free( component );
}

/**
 * @brief Per-system, per-faction, activates the affordable lane with best (grad
 * phi)/L
 * @return How many builds (upper bound) have to happen next turn.
 */
static int safelanes_activateByGradient( const cholmod_dense *Lambda_tilde,
                                         int                  iters_done )
{
   int            *facind_opts, *edgeind_opts, turns_next_time;
   double         *facind_vals, Linv;
   cholmod_dense **lal; /**< Per faction index, the Lambda_tilde[myDofs,:] @
                           PPl[fi] matrices. Calloced and lazily populated. */
   size_t *lal_bases, lal_base; /**< System si's U and Lambda rows start at
                                   sys_base; its lal rows start at lal_base. */

   lal       = calloc( array_size( faction_stack ), sizeof( cholmod_dense       *) );
   lal_bases = calloc( array_size( faction_stack ), sizeof( size_t ) );
   edgeind_opts = array_create( int );
   facind_opts  = array_create_size( int, array_size( faction_stack ) );
   facind_vals  = array_create_size( double, array_size( faction_stack ) );
   for ( int fi = 0; fi < array_size( faction_stack ); fi++ ) {
      array_push_back( &facind_opts, fi );
      array_push_back( &facind_vals, 0 );
   }
   turns_next_time = 0;

   for ( int si = 0; si < array_size( sys_to_first_vertex ) - 1; si++ ) {
      /* Factions with most presence here choose first. */
      const StarSystem *sys = system_getIndex( si );
      for ( int fi = 0; fi < array_size( faction_stack ); fi++ )
         facind_vals[fi] = -system_getPresence(
            sys, faction_stack[fi]
                    .id ); /* FIXME: Is this better, or presence_budget? */
      cmp_key_ref = facind_vals;
      qsort( facind_opts, array_size( faction_stack ), sizeof( int ), cmp_key );

      for ( int fii = 0; fii < array_size( faction_stack ); fii++ ) {
         int    ei_best;
         double cost_best, cost_cheapest_other;
         int    fi         = facind_opts[fii];
         size_t sys_base   = sys_to_first_vertex[si];
         double score_best = 0.; /* Negative scores get ignored. */

         /* Get the base index to use for this system. Save the value we expect
          * to be the next iteration's base index. The current system's rows are
          * in the lal[fi] matrix if there's presence at the time we slice it.
          * What we know is whether there's presence *now*. We can use that as a
          * proxy and fix lal_bases[fi] if we deplete this presence before
          * constructing lal[fi]. This is tricky, so there are assertions below,
          * which can warn us if we fuck this up. */
         lal_base = lal_bases[fi];
         if ( presence_budget[fi][si] <= 0. )
            continue;
         /* We "should" find these DoF's interesting if/when we slice, and will
          * unless we deplete this presence first. */
         lal_bases[fi] += sys_to_first_vertex[1 + si] - sys_to_first_vertex[si];

         array_resize( &edgeind_opts, 0 );
         for ( int ei = sys_to_first_edge[si]; ei < sys_to_first_edge[1 + si];
               ei++ ) {
            int sis           = edge_stack[ei][0];
            int sjs           = edge_stack[ei][1];
            int disconnecting = iters_done &&
                                !( vertex_fmask[sis] & ( MASK_1 << fi ) ) &&
                                !( vertex_fmask[sjs] & ( MASK_1 << fi ) );
            double cost = 1. / safelanes_initialConductivity( ei ) /
                             faction_stack[fi].lane_length_per_presence +
                          faction_stack[fi].lane_base_cost;
            if ( !lane_faction[ei] && !disconnecting &&
                 presence_budget[fi][si] >= cost &&
                 ( lane_fmask[ei] & ( MASK_1 << fi ) ) )
               array_push_back( &edgeind_opts, ei );
         }

         if ( array_size( edgeind_opts ) == 0 ) {
            presence_budget[fi][si] =
               0.; /* Nothing to build here! Tell ourselves to stop trying. */
            if ( lal[fi] == NULL )
               lal_bases[fi] -=
                  sys_to_first_vertex[1 + si] - sys_to_first_vertex[si];
            continue;
         }

         ei_best   = edgeind_opts[0];
         cost_best = 1. / safelanes_initialConductivity( ei_best ) /
                        faction_stack[fi].lane_length_per_presence +
                     faction_stack[fi].lane_base_cost;
         cost_cheapest_other = +HUGE_VAL;
         if ( array_size( edgeind_opts ) > 0 ) {
            /* There's an actual choice. Search for the best option. Lower is
             * better. */
            for ( int eii = 0; eii < array_size( edgeind_opts ); eii++ ) {
               int    ei    = edgeind_opts[eii];
               int    sis   = edge_stack[ei][0];
               int    sjs   = edge_stack[ei][1];
               double score = 0.;
               double cost;

               if ( lal[fi] == NULL ) { /* Is it time to evaluate the
                                           lazily-calculated matrix? */
                  cholmod_dense *lamt = safelanes_sliceByPresence(
                     Lambda_tilde, presence_budget[fi] );
                  lal[fi] = ncholmod_ddmult( lamt, 0, PPl[fi] );
                  cholmod_free_dense( &lamt, &C );
               }

               /* Evaluate (LUTll[0,0] + LUTll[1,1] - LUTll[0,1] - LUTll[1,0]),
                */
               /* where    LUTll = np.dot( lal[[sis,sjs],:] ,
                * utilde[[sis,sjs],:].T ) */
               score += safelanes_row_dot_row( utilde, lal[fi], sis,
                                               sis - sys_base + lal_base );
               score += safelanes_row_dot_row( utilde, lal[fi], sjs,
                                               sjs - sys_base + lal_base );
               score -= safelanes_row_dot_row( utilde, lal[fi], sjs,
                                               sis - sys_base + lal_base );
               score -= safelanes_row_dot_row( utilde, lal[fi], sis,
                                               sjs - sys_base + lal_base );
               Linv = safelanes_initialConductivity( ei );
               score *= ALPHA * Linv * Linv;
               score += LAMBDA;

               cost = 1. / safelanes_initialConductivity( ei ) /
                         faction_stack[fi].lane_length_per_presence +
                      faction_stack[fi].lane_base_cost;
               if ( score < score_best ) {
                  ei_best             = ei;
                  score_best          = score;
                  cost_cheapest_other = MIN( cost_cheapest_other, cost_best );
                  cost_best           = cost;
               } else
                  cost_cheapest_other = MIN( cost_cheapest_other, cost );
            }
         }

         /* Ignore positive scores. */
         if ( score_best >= 0. )
            continue;

         /* Add the lane. */
         presence_budget[fi][si] -= cost_best;
         if ( presence_budget[fi][si] >= cost_cheapest_other )
            turns_next_time++;
         else {
            presence_budget[fi][si] =
               0.; /* Nothing more to do here; tell ourselves. */
            if ( lal[fi] == NULL )
               lal_bases[fi] -=
                  sys_to_first_vertex[1 + si] - sys_to_first_vertex[si];
         }
         safelanes_updateConductivity( ei_best );
         vertex_fmask[edge_stack[ei_best][0]] |= ( MASK_1 << fi );
         vertex_fmask[edge_stack[ei_best][1]] |= ( MASK_1 << fi );
         lane_faction[ei_best] = faction_stack[fi].id;
      }
   }

#if DEBUGGING
   for ( int fi = 0; fi < array_size( faction_stack ); fi++ )
      if ( lal[fi] != NULL )
         assert( "Correctly tracked row offsets between the 'lal' and 'utilde' "
                 "matrices" &&
                 lal[fi]->nrow == lal_bases[fi] );
#endif /* DEBUGGING */

   for ( int fi = 0; fi < array_size( faction_stack ); fi++ )
      cholmod_free_dense( &lal[fi], &C );
   free( lal );
   free( lal_bases );
   array_free( edgeind_opts );
   array_free( facind_vals );
   array_free( facind_opts );

   return turns_next_time;
}

/** @brief It's a qsort comparator. Set the cmp_key_ref pointer prior to use, or
 * else. */
static int cmp_key( const void *p1, const void *p2 )
{
   double d = cmp_key_ref[*(int *)p1] - cmp_key_ref[*(int *)p2];
   return SIGN( d );
}

/**
 * @brief Return true if this triangle is so flat that lanes from point m to
 * point n aren't allowed.
 */
static int safelanes_triangleTooFlat( const vec2 *m, const vec2 *n,
                                      const vec2 *p, double lmn )
{
   const double MAX_COSINE = cos( MIN_ANGLE );
   double       lnp        = vec2_dist( n, p );
   double       lmp        = vec2_dist( m, p );
   double       dpn        = ( ( n->x - m->x ) * ( n->x - p->x ) +
                  ( n->y - m->y ) * ( n->y - p->y ) ) /
                ( lmn * lnp );
   double dpm = ( ( m->x - n->x ) * ( m->x - p->x ) +
                  ( m->y - n->y ) * ( m->y - p->y ) ) /
                ( lmn * lmp );
   return ( dpn > MAX_COSINE && lnp < lmn ) ||
          ( dpm > MAX_COSINE && lmp < lmn );
}

/**
 * @brief Return the vertex's owning faction (ID, not faction_stack index), or
 * -1 if not applicable.
 */
static int vertex_faction( int vi )
{
   const StarSystem *sys = system_getIndex( vertex_stack[vi].system );
   switch ( vertex_stack[vi].type ) {
   case VERTEX_SPOB:
      return sys->spobs[vertex_stack[vi].index]->presence.faction;
   case VERTEX_JUMP:
      return -1;
   default:
      ERR( _( "Safe-lane vertex type is invalid." ) );
   }
}

/**
 * @brief Return the vertex's coordinates within its system (by reference since
 * our vec2's are fat).
 */
static const vec2 *vertex_pos( int vi )
{
   const StarSystem *sys = system_getIndex( vertex_stack[vi].system );
   switch ( vertex_stack[vi].type ) {
   case VERTEX_SPOB:
      return &sys->spobs[vertex_stack[vi].index]->pos;
   case VERTEX_JUMP:
      return &sys->jumps[vertex_stack[vi].index].pos;
   default:
      ERR( _( "Safe-lane vertex type is invalid." ) );
   }
}

/** @brief Return the faction_stack index corresponding to a faction ID, or -1.
 */
static inline int FACTION_ID_TO_INDEX( int id )
{
   for ( int i = 0;
         i < array_size( faction_stack ) && faction_stack[i].id <= id; i++ )
      if ( faction_stack[i].id == id )
         return i;
   return -1;
}

/** @brief Return a mask matching any faction. */
static inline FactionMask MASK_ANY_FACTION()
{
   return ~MASK_0;
}

/** @brief A mask giving this faction (NOT faction_stack index) exclusive rights
 * to build, if it's a lane-building faction. */
static inline FactionMask MASK_ONE_FACTION( int id )
{
   int ind = FACTION_ID_TO_INDEX( id );
   return ind > 0 ? ( MASK_1 ) << ind : MASK_ANY_FACTION();
}

/** @brief A mask with appropriate lane-building rights given one faction ID
 * owning each endpoint. */
static inline FactionMask MASK_COMPROMISE( int id1, int id2 )
{
   FactionMask m1 = MASK_ONE_FACTION( id1 ), m2 = MASK_ONE_FACTION( id2 );
   return ( m1 & m2 )
             ? ( m1 & m2 )
             : ( m1 |
                 m2 ); /* Any/Any -> any, Any/f -> just f, f1/f2 -> either. */
}

static inline void triplet_entry( cholmod_triplet *m, int i, int j, double x )
{
   ( (int *)m->i )[m->nnz]    = i;
   ( (int *)m->j )[m->nnz]    = j;
   ( (double *)m->x )[m->nnz] = x;
   m->nnz++;
}

/**
 * @brief Construct the matrix-slice of m, selecting those rows where the
 * corresponding presence value is positive.
 */
static cholmod_dense *safelanes_sliceByPresence( const cholmod_dense *m,
                                                 const double *sysPresence )
{
   size_t         nr, nc, in_r, out_r;
   cholmod_dense *out;

   nr = 0;
   nc = m->ncol;
   for ( int si = 0; si < array_size( sys_to_first_vertex ) - 1; si++ )
      if ( sysPresence[si] > 0 )
         nr += sys_to_first_vertex[1 + si] - sys_to_first_vertex[si];

   out = cholmod_allocate_dense( nr, nc, nr, CHOLMOD_REAL, &C );

   in_r = out_r = 0;
   for ( int si = 0; si < array_size( sys_to_first_vertex ) - 1; si++ ) {
      int sz = sys_to_first_vertex[1 + si] - sys_to_first_vertex[si];
      if ( sysPresence[si] > 0 ) {
         for ( size_t c = 0; c < nc; c++ )
            memcpy( &( (double *)out->x )[c * out->d + out_r],
                    &( (double *)m->x )[c * m->d + in_r],
                    sz * sizeof( double ) );
         out_r += sz;
      }
      in_r += sz;
   }
   return out;
}

/** @brief Dense times dense matrix. Return A*B, or A'*B if transA is true. */
static cholmod_dense *ncholmod_ddmult( cholmod_dense *A, int transA,
                                       cholmod_dense *B )
{
#if I_LOVE_FORTRAN
   blasint M = transA ? A->ncol : A->nrow, K = transA ? A->nrow : A->ncol,
           N = B->ncol, lda = A->d, ldb = B->d, ldc = M;
   assert( K == (blasint)B->nrow );
   cholmod_dense *out   = cholmod_allocate_dense( M, N, M, CHOLMOD_REAL, &C );
   double         alpha = 1., beta = 0.;
   BLASFUNC( dgemm )
   ( transA ? "T" : "N", "N", &M, &N, &K, &alpha, A->x, &lda, B->x, &ldb, &beta,
     out->x, &ldc );
#else  /* I_LOVE_FORTRAN */
   size_t M = transA ? A->ncol : A->nrow, K = transA ? A->nrow : A->ncol,
          N = B->ncol;
   assert( K == B->nrow );
   cholmod_dense *out = cholmod_allocate_dense( M, N, M, CHOLMOD_REAL, &C );
   cblas_dgemm( CblasColMajor, transA ? CblasTrans : CblasNoTrans, CblasNoTrans,
                M, N, K, 1, A->x, A->d, B->x, B->d, 0, out->x, out->d );
#endif /* I_LOVE_FORTRAN */
   return out;
}

/** @brief Return the i,j entry of A*B', or equivalently the dot product of row
 * i of A with row j of B. */
static double safelanes_row_dot_row( cholmod_dense *A, cholmod_dense *B, int i,
                                     int j )
{
   assert( A->ncol == B->ncol );
#if I_LOVE_FORTRAN
   blasint N = A->ncol, incA = A->d, incB = B->d;
   return BLASFUNC( ddot )( &N, (double *)A->x + i, &incA, (double *)B->x + j,
                            &incB );
#else  /* I_LOVE_FORTRAN */
   return cblas_ddot( A->ncol, (double *)A->x + i, A->d, (double *)B->x + j,
                      B->d );
#endif /* I_LOVE_FORTRAN */
}
