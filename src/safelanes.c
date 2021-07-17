/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file safelanes.c
 *
 * @brief Handles factions' safe lanes through systems.
 * This implements the algorithm described in utils/lanes-generator (whitepaper and much clearer Python version).
 */

/** @cond */
#include <math.h>

#if HAVE_OPENBLAS_CBLAS_H
#include <openblas/cblas.h>
#elif HAVE_CBLAS_OPENBLAS_H
#include <cblas_openblas.h>
#elif HAVE_CBLAS_HYPHEN_OPENBLAS_H
#include <cblas-openblas.h>
#else
#include <cblas.h>
#endif

#ifdef HAVE_SUITESPARSE_CHOLMOD_H
#include <suitesparse/cholmod.h>
#else
#include <cholmod.h>
#endif

#include "naev.h"
/** @endcond */

#include "safelanes.h"

#include "array.h"
#include "log.h"
#include "union_find.h"


/*
 * Global parameters.
 */
static const double ALPHA                  = 9;         /**< Lane efficiency parameter. */
static const double JUMP_CONDUCTIVITY      = .001;      /**< Conductivity value for inter-system jump-point connections. */
static const double MIN_ANGLE              = M_PI/18;   /**< Path triangles can't be more acute. */
enum {
   STORAGE_MODE_LOWER_TRIANGULAR_PART = -1,             /**< A CHOLMOD "stype" value: matrix is interpreted as symmetric. */
   STORAGE_MODE_UNSYMMETRIC = 0,                        /**< A CHOLMOD "stype" value: matrix holds whatever we put in it. */
   STORAGE_MODE_UPPER_TRIANGULAR_PART = +1,             /**< A CHOLMOD "stype" value: matrix is interpreted as symmetric. */
   UNSORTED                           = 0,              /**< a named bool */
   SORTED                             = 1,              /**< a named bool */
   UNPACKED                           = 0,              /**< a named bool */
   PACKED                             = 1,              /**< a named bool */
   MODE_NUMERICAL                     = 1,              /**< yet another CHOLMOD magic number! */
};

/*
 * Types.
 */
/** @brief Object type: like SafeLaneLocType, but with Naev stack indexing in mind.
 *  @TODO Converge these after beefing up nlua APIs (#1862 or just jump point from internal indices).
 */
typedef enum VertexType_ {VERTEX_PLANET, VERTEX_JUMP} VertexType;

/** @brief Reference to a planet or jump point. */
typedef struct Vertex_ {
   int system;                  /**< ID of the system containing the object. */
   VertexType type;             /**< Which of Naev's list contains it? */
   int index;                   /**< Index in the system's planets or jumps array. */
} Vertex;

/** @brief An edge is a pair of vertex indices. */
typedef int Edge[2];

/** @brief Description of a lane-building faction. */
typedef struct Faction_ {
   int id;                              /**< Faction ID. */
   double lane_length_per_presence;     /**< Weight determining their ability to claim lanes. */
} Faction;

/** @brief A set of lane-building factions, represented as a bitfield. */
typedef uint32_t FactionMask;

/** @brief Some BLAS-compatible matrix data whose size/ordreing isn't pre-determined. */
typedef struct MatWrap_ {
   enum CBLAS_ORDER order; /**< CblasRowMajor or CblasColMajor */
   int nrow;               /**< Rows. */
   int ncol;               /**< Columns. */
   double *x;
} MatWrap;


/*
 * Global state.
 */
static cholmod_common C;        /**< Parameter settings, statistics, and workspace used internally by CHOLMOD. */
static Vertex *vertex_stack;    /**< Array (array.h): Everything eligible to be a lane endpoint. */
static int *sys_to_first_vertex;/**< Array (array.h): For each system index, the id of its first vertex, + sentinel. */
static Edge *edge_stack;        /**< Array (array.h): Everything eligible to be a lane. */
static int *sys_to_first_edge;  /**< Array (array.h): For each system index, the id of its first edge, + sentinel. */
static Faction *faction_stack;  /**< Array (array.h): The faction IDs that can build lanes. */
static int *lane_faction;       /**< Array (array.h): Per edge, ID of faction that built a lane there, if any, else 0. */
static FactionMask *lane_fmask; /**< Array (array.h): Per edge, the set of factions that may build it. */
static double **presence_budget;/**< Array (array.h): Per faction, per system, the amount of presence not yet spent on lanes. */
static int *tmp_planet_indices; /**< Array (array.h): The vertex IDs of planets, to set up ftilde/PPl. Unrelated to planet IDs. */
static Edge *tmp_jump_edges;    /**< Array (array.h): The vertex ID pairs connected by 2-way jumps. Used to set up "stiff". */
static double *tmp_edge_conduct;/**< Array (array.h): Conductivity (1/len) of each potential lane. Used to set up "stiff". */
static int *tmp_anchor_vertices;/**< Array (array.h): One vertex ID per connected component. Used to set up "stiff". */
static UnionFind tmp_sys_uf;    /**< The partition of {system indices} into connected components (connected by 2-way jumps). */
static cholmod_triplet *stiff;  /**< K matrix, UT triplets: internal edges (E*3), implicit jump connections, anchor conditions. */
static cholmod_sparse *QtQ;     /**< (Q*)Q where Q is the ExV difference matrix. */
static cholmod_dense *ftilde;   /**< Fluxes (bunch of F columns in the KU=F problem). */
static cholmod_dense *utilde;   /**< Potentials (bunch of U columns in the KU=F problem). */
static cholmod_dense **PPl;     /**< Array: (array.h): For each builder faction, The (P*)P in: grad_u(phi)=(Q*)Q U~ (P*)P. */
static double* cmp_key_ref;     /**< To qsort() a list of indices by table value, point this at your table and use cmp_key. */

/*
 * Prototypes.
 */
static int safelanes_buildOneTurn (void);
static int safelanes_activateByGradient( MatWrap Lambda_tilde );
static void safelanes_initStacks (void);
static void safelanes_initStacks_edge (void);
static void safelanes_initStacks_faction (void);
static void safelanes_initStacks_vertex (void);
static void safelanes_initStacks_anchor (void);
static void safelanes_initOptimizer (void);
static void safelanes_destroyOptimizer (void);
static void safelanes_destroyStacks (void);
static void safelanes_destroyTmp (void);
static void safelanes_initStiff (void);
static double safelanes_initialConductivity ( int ei );
static void safelanes_updateConductivity ( int ei_activated );
static void safelanes_initQtQ (void);
static void safelanes_initFTilde (void);
static void safelanes_initPPl (void);
static int safelanes_triangleTooFlat( const Vector2d* m, const Vector2d* n, const Vector2d* p, double lmn );
static int vertex_faction( int vi );
static const Vector2d* vertex_pos( int vi );
static inline int FACTION_ID_TO_INDEX( int id );
static inline FactionMask MASK_ANY_FACTION();
static inline FactionMask MASK_ONE_FACTION( int id );
static inline FactionMask MASK_COMPROMISE( int id1, int id2 );
static int cmp_key( const void* p1, const void* p2 );
static inline void triplet_entry( cholmod_triplet* m, int i, int j, double v );
static void matwrap_init( MatWrap* A, enum CBLAS_ORDER order, int nrow, int ncol );
static inline MatWrap matwrap_from_cholmod( cholmod_dense* m );
static inline MatWrap matwrap_transpose( MatWrap A );
static void matwrap_reorder_in_place( MatWrap* A );
static void matwrap_sliceByPresence( MatWrap* A, double* sysPresence, MatWrap* out );
static void matwrap_mul( MatWrap A, MatWrap B, MatWrap* C );
static double matwrap_mul_elem( MatWrap A, MatWrap B, int i, int j );
static void matwrap_free( MatWrap A );

/**
 * @brief Like array_push_back( a, Edge{v0, v1} ), but achievable in C. :-P
 */
static inline void array_push_back_edge( Edge **a, int v0, int v1 )
{
   int *vs = array_grow( a );
   vs[0] = v0;
   vs[1] = v1;
}

/**
 * @brief Initializes the safelanes system.
 */
void safelanes_init (void)
{
   cholmod_start( &C );
   safelanes_recalculate();
}


/**
 * @brief Shuts down the safelanes system.
 */
void safelanes_destroy (void)
{
   safelanes_destroyOptimizer();
   safelanes_destroyStacks();
   cholmod_finish( &C );
}


/**
 * @brief Shuts down the safelanes system.
 *    @param faction ID of the faction whose lanes we want, or a negative value signifying "all of them".
 *    @param system Star system whose lanes we want.
 *    @return Array (array.h) of matching SafeLane structures. Caller frees.
 */
SafeLane* safelanes_get (int faction, const StarSystem* system)
{
   int i, j;
   SafeLane *out, *l;
   const Vertex *v[2];

   out = array_create( SafeLane );

   for (i=sys_to_first_edge[system->id]; i<sys_to_first_edge[1+system->id]; i++) {
      for (j=0; j<2; j++)
         v[j] = &vertex_stack[edge_stack[i][j]];
      if (lane_faction[i] > 0 && (faction < 0 || faction == lane_faction[i])) {
         l = &array_grow( &out );
         l->faction = lane_faction[i];
         for (j=0; j<2; j++) {
            switch (v[j]->type) {
               case VERTEX_PLANET:
                  l->point_type[j]   = SAFELANE_LOC_PLANET;
                  l->point_id[j]     = system->planets[v[j]->index]->id;
                  break;
               case VERTEX_JUMP:
                  l->point_type[j]   = SAFELANE_LOC_DEST_SYS;
                  l->point_id[j]     = system->jumps[v[j]->index].targetid;
                  break;
               default:
                  ERR( _("Invalid vertex type.") );
            }
         }
      }
   }
   return out;
}


/**
 * @brief Update the safe lane locations in response to the universe changing (e.g., diff applied).
 */
void safelanes_recalculate (void)
{
   Uint32 time;

   time = SDL_GetTicks();
   safelanes_initStacks();
   safelanes_initOptimizer();
   while (safelanes_buildOneTurn() > 0)
      ;
   safelanes_destroyOptimizer();
   /* Stacks remain available for queries. */
   time = SDL_GetTicks() - time;
   DEBUG( _("Calculated patrols in %f seconds"), time/1000. );
}


/**
 * @brief Initializes resources used by lane optimization.
 */
static void safelanes_initOptimizer (void)
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
static void safelanes_destroyOptimizer (void)
{
   for (int i=0; i<array_size(PPl); i++)
      cholmod_free_dense( &PPl[i], &C );
   array_free( PPl );
   PPl = NULL;
   cholmod_free_dense( &utilde, &C );
   cholmod_free_dense( &ftilde, &C );
   cholmod_free_sparse( &QtQ, &C );
   cholmod_free_triplet( &stiff, &C );
}


/**
 * @brief Run a round of optimization. Return how many builds (upper bound) have to happen next turn.
 */
static int safelanes_buildOneTurn (void)
{
   cholmod_sparse *stiff_s;
   cholmod_factor *stiff_f;
   cholmod_dense *_QtQutilde, *Lambda_tilde, *Y_workspace, *E_workspace;
   int turns_next_time;
   double zero[] = {0, 0}, neg_1[] = {-1, 0};

   Y_workspace = E_workspace = Lambda_tilde = NULL;
   stiff_s = cholmod_triplet_to_sparse( stiff, 0, &C );
   stiff_f = cholmod_analyze( stiff_s, &C );
   cholmod_factorize( stiff_s, stiff_f, &C );
   cholmod_solve2( CHOLMOD_A, stiff_f, ftilde, NULL, &utilde, NULL, &Y_workspace, &E_workspace, &C );
   _QtQutilde = cholmod_zeros( utilde->nrow, utilde->ncol, CHOLMOD_REAL, &C );
   cholmod_sdmult( QtQ, 0, neg_1, zero, utilde, _QtQutilde, &C );
   cholmod_solve2( CHOLMOD_A, stiff_f, _QtQutilde, NULL, &Lambda_tilde, NULL, &Y_workspace, &E_workspace, &C );
   cholmod_free_dense( &_QtQutilde, &C );
   cholmod_free_dense( &Y_workspace, &C );
   cholmod_free_dense( &E_workspace, &C );
   cholmod_free_factor( &stiff_f, &C );
   cholmod_free_sparse( &stiff_s, &C );
   turns_next_time = safelanes_activateByGradient( matwrap_from_cholmod( Lambda_tilde ) );
   cholmod_free_dense( &Lambda_tilde, &C );

   return turns_next_time;
}


/**
 * @brief Sets up the local faction/object stacks.
 */
static void safelanes_initStacks (void)
{
   safelanes_destroyStacks();
   safelanes_initStacks_vertex();
   safelanes_initStacks_faction();
   safelanes_initStacks_edge();
   safelanes_initStacks_anchor();
   DEBUG( _("Built safelanes graph: V=%d, E=%d"), array_size(vertex_stack), array_size(edge_stack) );
}


/**
 * @brief Sets up the local stacks with entry per vertex (or per jump).
 */
static void safelanes_initStacks_vertex (void)
{
   const StarSystem *systems_stack;
   const Planet *p;
   const JumpPoint *jp;
   int system, i, j;

   systems_stack = system_getAll();

   vertex_stack = array_create( Vertex );
   sys_to_first_vertex = array_create( int );
   array_push_back( &sys_to_first_vertex, 0 );
   tmp_planet_indices = array_create( int );
   tmp_jump_edges = array_create( Edge );
   for (system=0; system<array_size(systems_stack); system++) {
      for (i=0; i<array_size(systems_stack[system].planets); i++) {
         p = systems_stack[system].planets[i];
         if (p->real && p->presenceAmount) {
            Vertex v = {.system = system, .type = VERTEX_PLANET, .index = i};
            array_push_back( &tmp_planet_indices, array_size(vertex_stack) );
            array_push_back( &vertex_stack, v );
         }
      }

      for (i=0; i<array_size(systems_stack[system].jumps); i++) {
         jp = &systems_stack[system].jumps[i];
         if (!jp_isFlag( jp, JP_HIDDEN|JP_EXITONLY )) {
            Vertex v = {.system = system, .type = VERTEX_JUMP, .index = i};
            array_push_back( &vertex_stack, v );
            if (jp->targetid < system && jp->returnJump != NULL)
               for (j = sys_to_first_vertex[jp->targetid]; j < sys_to_first_vertex[1+jp->targetid]; j++)
                  if (vertex_stack[j].type == VERTEX_JUMP && jp->returnJump == &jp->target->jumps[vertex_stack[j].index]) {
                     array_push_back_edge( &tmp_jump_edges, array_size(vertex_stack)-1, j );
                     break;
                  }
         }
      }
      array_push_back( &sys_to_first_vertex, array_size(vertex_stack) );
   }
   array_shrink( &vertex_stack );
   array_shrink( &sys_to_first_vertex );
}


/**
 * @brief Sets up the local stacks with entry per edge. Faction stack must be set up.
 */
static void safelanes_initStacks_edge (void)
{
   const StarSystem *systems_stack;
   int system, i, j, k;

   systems_stack = system_getAll();

   edge_stack = array_create( Edge );
   sys_to_first_edge = array_create( int );
   array_push_back( &sys_to_first_edge, 0 );
   lane_fmask = array_create( FactionMask );
   tmp_edge_conduct = array_create( double );
   for (system=0; system<array_size(systems_stack); system++) {
      for (i = sys_to_first_vertex[system]; i < sys_to_first_vertex[1+system]; i++) {
         const Vector2d *pi = vertex_pos( i );
         for (j = sys_to_first_vertex[system]; j < i; j++) {
            const Vector2d *pj = vertex_pos( j );
            double lij = vect_dist( pi, pj );
            int has_approx_midpoint = 0;
            for (k = sys_to_first_vertex[system]; k < sys_to_first_vertex[1+system]; k++)
               if (k!=i && k!=j && safelanes_triangleTooFlat( pi, pj, vertex_pos( k ), lij )) {
                  has_approx_midpoint = 1;
                  break;
               }
            if (has_approx_midpoint)
               continue;  /* The edge from i to j is disallowed in favor of a path through k. */
            array_push_back_edge( &edge_stack, j, i );
            array_push_back( &lane_fmask, MASK_COMPROMISE( vertex_faction( i ), vertex_faction( j ) ) );
            array_push_back( &tmp_edge_conduct, 1/lij );
         }
      }
      array_push_back( &sys_to_first_edge, array_size(edge_stack) );
   }
   array_shrink( &edge_stack );
   array_shrink( &sys_to_first_edge );

   lane_faction = array_create_size( int, array_size(edge_stack) );
   array_resize( &lane_faction, array_size(edge_stack) );
   memset( lane_faction, 0, array_size(lane_faction)*sizeof(lane_faction[0]) );
}


/**
 * @brief Sets up the local stacks with entry per faction.
 */
static void safelanes_initStacks_faction (void)
{
   int fi, f, s, *faction_all;
   const StarSystem *systems_stack;

   faction_stack = array_create( Faction );
   faction_all = faction_getAll();
   for (fi=0; fi<array_size(faction_all); fi++) {
      f = faction_all[fi];
      Faction rec = {.id = f, .lane_length_per_presence = faction_lane_length_per_presence(f)};
      if (rec.lane_length_per_presence > 0.)
         array_push_back( &faction_stack, rec );
   }
   array_free( faction_all );
   array_shrink( &faction_stack );
   assert( "FactionMask size is sufficient" && (size_t)array_size(faction_stack) <= 8*sizeof(FactionMask) );

   presence_budget = array_create_size( double*, array_size(faction_stack) );
   systems_stack = system_getAll();
   for (fi=0; fi<array_size(faction_stack); fi++) {
      presence_budget[fi] = array_create_size( double, array_size(systems_stack) );
      for (s=0; s<array_size(systems_stack); s++)
         presence_budget[fi][s] = system_getPresence( &systems_stack[s], faction_stack[fi].id );
   }
}


/**
 * @brief Identifies anchor points:
 * The universe graph (with in-system and 2-way-jump edges) could have many connected components.
 * Our impedance problem needs one boundary condition for each, so we choose a representative vertex from each.
 */
static void safelanes_initStacks_anchor (void)
{
   int i, nsys, *anchor_systems;

   nsys = array_size(sys_to_first_vertex) - 1;
   unionfind_init( &tmp_sys_uf, nsys );
   for (i=0; i<array_size(tmp_jump_edges); i++)
      unionfind_union( &tmp_sys_uf, vertex_stack[tmp_jump_edges[i][0]].system, vertex_stack[tmp_jump_edges[i][1]].system );
   anchor_systems = unionfind_findall( &tmp_sys_uf );
   tmp_anchor_vertices = array_create_size( int, array_size(anchor_systems) );

   /* Add an anchor vertex per system, but only if there actually is a vertex in the system. */
   for (i=0; i<array_size(anchor_systems); i++)
      if (sys_to_first_vertex[anchor_systems[i]] < sys_to_first_vertex[1+anchor_systems[i]]) {
         array_push_back( &tmp_anchor_vertices, sys_to_first_vertex[anchor_systems[i]] );
         DEBUG( _("Anchoring safelanes graph in system: %s."), system_getIndex(anchor_systems[i])->name );
      }
   array_free( anchor_systems );
}


/**
 * @brief Tears down the local faction/object stacks.
 */
static void safelanes_destroyStacks (void)
{
   int i;
   safelanes_destroyTmp();
   array_free( vertex_stack );
   vertex_stack = NULL;
   array_free( sys_to_first_vertex );
   sys_to_first_vertex = NULL;
   array_free( edge_stack );
   edge_stack = NULL;
   array_free( sys_to_first_edge );
   sys_to_first_edge = NULL;
   for (i=0; i<array_size(presence_budget); i++)
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
static void safelanes_destroyTmp (void)
{
   unionfind_free( &tmp_sys_uf );
   array_free( tmp_planet_indices );
   tmp_planet_indices = NULL;
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
static void safelanes_initStiff (void)
{
   int nnz, v, i;
   double max_conductivity;

   cholmod_free_triplet( &stiff, &C );
   v = array_size(vertex_stack);
   nnz = 3*(array_size(edge_stack)+array_size(tmp_jump_edges)) + array_size(tmp_anchor_vertices);
   stiff = cholmod_allocate_triplet( v, v, nnz, STORAGE_MODE_UPPER_TRIANGULAR_PART, CHOLMOD_REAL, &C );
   /* Populate triplets: internal edges (ii ij jj), implicit jump connections (ditto), anchor conditions. */
   for (i=0; i<array_size(edge_stack); i++) {
      triplet_entry( stiff, edge_stack[i][0], edge_stack[i][0], +tmp_edge_conduct[i] );
      triplet_entry( stiff, edge_stack[i][0], edge_stack[i][1], -tmp_edge_conduct[i] );
      triplet_entry( stiff, edge_stack[i][1], edge_stack[i][1], +tmp_edge_conduct[i] );
   }
   for (i=0; i<array_size(tmp_jump_edges); i++) {
      triplet_entry( stiff, tmp_jump_edges[i][0], tmp_jump_edges[i][0], +JUMP_CONDUCTIVITY );
      triplet_entry( stiff, tmp_jump_edges[i][0], tmp_jump_edges[i][1], -JUMP_CONDUCTIVITY );
      triplet_entry( stiff, tmp_jump_edges[i][1], tmp_jump_edges[i][1], +JUMP_CONDUCTIVITY );
   }
   /* Add a Robin boundary condition, using the max conductivity (after activation) for spectral reasons. */
   max_conductivity = JUMP_CONDUCTIVITY/(1+ALPHA);
   for (i=0; i<array_size(edge_stack); i++)
      max_conductivity = MAX( max_conductivity, tmp_edge_conduct[i] );
   max_conductivity = MAX( JUMP_CONDUCTIVITY, (1+ALPHA)*max_conductivity ); /* Activation scales entries by 1+ALPHA later. */
   for (i=0; i<array_size(tmp_anchor_vertices); i++)
      triplet_entry( stiff, tmp_anchor_vertices[i], tmp_anchor_vertices[i], max_conductivity );
#if DEBUGGING
   assert( stiff->nnz == stiff->nzmax );
   assert( cholmod_check_triplet( stiff, &C) );
#endif
}


/**
 * @brief Returns the initial conductivity value (1/length) for edge ei.
 * The live value is stored in the stiffness matrix; \see safelanes_initStiff above.
 * When a lane is activated, its conductivity is updated to (1+ALPHA)/length.
 */
static double safelanes_initialConductivity ( int ei )
{
   double *sv = stiff->x;
   return lane_faction[ei] ? sv[3*ei]/(1+ALPHA) : sv[3*ei];
}


/**
 * @brief Updates the stiffness matrix to account for the given edge being activated.
 * \see safelanes_initStiff.
 */
static void safelanes_updateConductivity ( int ei_activated )
{
   double *sv = stiff->x;
   for (int i=3*ei_activated; i<3*(ei_activated+1); i++)
      sv[i] *= 1+ALPHA;
}


/**
 * @brief Sets up the (Q*)Q matrix.
 */
static void safelanes_initQtQ (void)
{
   cholmod_sparse *Q;
   int i, *qp, *qi;
   double *qv;

   cholmod_free_sparse( &QtQ, &C );
   Q = cholmod_allocate_sparse( array_size(vertex_stack), array_size(edge_stack), 2*array_size(edge_stack),
         SORTED, PACKED, STORAGE_MODE_UNSYMMETRIC, CHOLMOD_REAL, &C );
   qp = Q->p;
   qi = Q->i;
   qv = Q->x;
   qp[0] = 0;
   for (i=0; i<array_size(edge_stack); i++) {
      qp[i+1] = 2*(i+1);
      qi[2*i+0] = edge_stack[i][0];
      qv[2*i+0] = +1;
      qi[2*i+1] = edge_stack[i][1];
      qv[2*i+1] = -1;
   }
#if DEBUGGING
   assert( cholmod_check_sparse( Q, &C ) );
#endif
   QtQ = cholmod_aat( Q, NULL, 0, MODE_NUMERICAL, &C );
   cholmod_free_sparse( &Q, &C );
}


/**
 * @brief Sets up the fluxes matrix f~.
 */
static void safelanes_initFTilde (void)
{
   cholmod_sparse *eye = cholmod_speye( array_size(vertex_stack), array_size(vertex_stack), CHOLMOD_REAL, &C );
   cholmod_sparse *sp = cholmod_submatrix( eye, NULL, -1, tmp_planet_indices, array_size(tmp_planet_indices), 1, SORTED, &C );
   cholmod_free_dense( &ftilde, &C );
   ftilde = cholmod_sparse_to_dense( sp, &C );
   cholmod_free_sparse( &sp, &C );
   cholmod_free_sparse( &eye, &C );
}


/**
 * @brief Sets up the PPl matrices appearing in the gradient formula.
 */
static void safelanes_initPPl (void)
{
   cholmod_triplet *P;
   cholmod_dense **D;
   cholmod_sparse *sp, *PPl_sp;
   int np, i, j, k, facti, factj, sysi, sysj;
   Planet *pnti, *pntj;

   np = array_size(tmp_planet_indices);
#define MULTI_INDEX( i, j ) ((i*(i-1))/2 + j)
   P = cholmod_allocate_triplet( np, MULTI_INDEX(np,0), np*(np-1), STORAGE_MODE_UNSYMMETRIC, CHOLMOD_REAL, &C );
   for (i=0; i<np; i++)
      for (j=0; j<i; j++) {
         triplet_entry( P, i, MULTI_INDEX(i,j), +1 );
         triplet_entry( P, j, MULTI_INDEX(i,j), -1 );
      }
#if DEBUGGING
   assert( P->nnz == P->nzmax );
   assert( cholmod_check_triplet( P, &C) );
#endif

   for (k=0; i<array_size(PPl); k++)
      cholmod_free_dense( &PPl[k], &C );
   array_free( PPl );

   D = array_create_size( cholmod_dense*, array_size(faction_stack) );
   for (k=0; k<array_size(faction_stack); k++)
      array_push_back( &D, cholmod_allocate_dense( 1, MULTI_INDEX(np,0), 1, CHOLMOD_REAL, &C ) );

   for (i=0; i<array_size(tmp_planet_indices); i++) {
      sysi = vertex_stack[tmp_planet_indices[i]].system;
      pnti = system_getIndex( sysi )->planets[vertex_stack[tmp_planet_indices[i]].index];
      facti = FACTION_ID_TO_INDEX( pnti->faction );
      for (j=0; j<i; j++) {
         sysj = vertex_stack[tmp_planet_indices[j]].system;
         if (unionfind_find( &tmp_sys_uf, sysi ) == unionfind_find( &tmp_sys_uf, sysj )) {
            pntj = system_getIndex( sysj )->planets[vertex_stack[tmp_planet_indices[j]].index];
            factj = FACTION_ID_TO_INDEX( pntj->faction );
            if (facti >= 0)
               ((double*)D[facti]->x)[MULTI_INDEX(i,j)] += pnti->presenceAmount;
            if (factj >= 0)
               ((double*)D[factj]->x)[MULTI_INDEX(i,j)] += pntj->presenceAmount;
         }
      }
   }

   PPl = array_create_size( cholmod_dense*, array_size(faction_stack) );
   for (k=0; k<array_size(faction_stack); k++) {
      sp = cholmod_triplet_to_sparse( P, 0, &C );
      cholmod_scale( D[k], CHOLMOD_COL, sp, &C );
      PPl_sp = cholmod_aat( sp, NULL, 0, MODE_NUMERICAL, &C );
      array_push_back( &PPl, cholmod_sparse_to_dense( PPl_sp, &C ) );
      cholmod_free_sparse( &PPl_sp, &C );
      cholmod_free_sparse( &sp, &C );
   }
#undef MULTI_INDEX

   for (k=0; i<array_size(D); k++)
      cholmod_free_dense( &D[k], &C );
   array_free( D );
   cholmod_free_triplet( &P, &C );
}


/**
 * @brief Per-system, per-faction, activates the affordable lane with best (grad phi)/L
 * @return How many builds (upper bound) have to happen next turn.
 */
static int safelanes_activateByGradient( MatWrap Lambda_tilde )
{
   int ei, eii, ei_best, fi, fii, *facind_opts, *edgeind_opts, si, sis, sjs, turns_next_time;
   double *facind_vals, score, score_best, Linv;
   StarSystem *sys;
   MatWrap *lal; /**< Per faction index, the Lambda_tilde[myDofs,:] @ PPl[fi] matrices. Calloced and lazily populated. */
   int *lal_bases, lal_base, sys_base; /**< System si's U and Lambda rows start at sys_base; its lal rows start at lal_base. */
   MatWrap UTt = matwrap_transpose( matwrap_from_cholmod( utilde ) );

   lal = calloc( array_size(faction_stack), sizeof(MatWrap) );
   lal_bases = calloc( array_size(faction_stack), sizeof(int) );
   edgeind_opts = array_create( int );
   facind_opts = array_create_size( int, array_size(faction_stack) );
   facind_vals = array_create_size( double, array_size(faction_stack) );
   for (fi=0; fi<array_size(faction_stack); fi++) {
      array_push_back( &facind_opts, fi );
      array_push_back( &facind_vals, 0 );
   }
   turns_next_time = 0;

   for (si=0; si<array_size(sys_to_first_vertex)-1; si++)
   {
      /* Factions with most presence here choose first. */
      sys = system_getIndex( si );
      for (fi=0; fi<array_size(faction_stack); fi++)
         facind_vals[fi] = -system_getPresence( sys, faction_stack[fi].id ); /* FIXME: Is this better, or presence_budget? */
      cmp_key_ref = facind_vals;
      qsort( facind_opts, array_size(faction_stack), sizeof(int), cmp_key );

      for (fii=0; fii<array_size(faction_stack); fii++) {
         fi = facind_opts[fii];
         sys_base = sys_to_first_vertex[si];

         /* Get the base index to use for this system. Save the value we expect to be the next iteration's base index.
          * The current system's rows is in the lal[fi] matrix if there's presence at the time we slice it.
          * What we know is whether there's presence *now*. We can use that as a proxy and fix lal_bases[fi] if we
          * deplete this presence before constructing lal[fi]. This is tricky, so there are assertions below,
          * which can warn us if we fuck this up. */
         lal_base = lal_bases[fi];
         if (presence_budget[fi][si] <= 0)
            continue;
         /* We "should" find these DoF's interesting if/when we slice, and will unless we deplete this presence first. */
         lal_bases[fi] += sys_to_first_vertex[1+si] - sys_to_first_vertex[si];

         array_resize( &edgeind_opts, 0 );
         for (ei=sys_to_first_edge[si]; ei<sys_to_first_edge[1+si]; ei++)
            if (!lane_faction[ei]
                && presence_budget[fi][si] >= 1 / safelanes_initialConductivity(ei) / faction_stack[fi].lane_length_per_presence
                && (lane_fmask[ei] & (1<<fi)))
               array_push_back( &edgeind_opts, ei );

         if (array_size(edgeind_opts) == 0) {
            presence_budget[fi][si] = -1;  /* Nothing to build here! Tell ourselves to stop trying. */
            if (lal[fi].x == NULL)
               lal_bases[fi] -= sys_to_first_vertex[1+si] - sys_to_first_vertex[si];
            continue;
         }

         ei_best = edgeind_opts[0];
         if (array_size(edgeind_opts) > 1) {
            /* There's an actual choice. Search for the best option. Lower is better. */
            score_best = +HUGE_VAL;
            for (eii=0; eii<array_size(edgeind_opts); eii++) {
               ei = edgeind_opts[eii];
               sis = edge_stack[ei][0];
               sjs = edge_stack[ei][1];

               if (lal[fi].x == NULL) { /* Is it time to evaluate the lazily-calculated matrix? */
                  MatWrap lamt;
                  MatWrap PP = matwrap_from_cholmod( PPl[fi] );
                  matwrap_sliceByPresence( &Lambda_tilde, presence_budget[fi], &lamt );
                  matwrap_mul( lamt, PP, &lal[fi] );
                  matwrap_free( lamt );
               }

               score = 0;
               /* Evaluate (LUTll[0,0] + LUTll[1,1] - LUTll[0,1] - LUTll[1,0]), */
               /* where    LUTll = np.dot( lal[[sis,sjs],:] , utilde[[sis,sjs],:].T ) */
               score += matwrap_mul_elem( lal[fi], UTt, sis - sys_base + lal_base, sis );
               score += matwrap_mul_elem( lal[fi], UTt, sjs - sys_base + lal_base, sjs );
               score -= matwrap_mul_elem( lal[fi], UTt, sis - sys_base + lal_base, sjs );
               score -= matwrap_mul_elem( lal[fi], UTt, sjs - sys_base + lal_base, sis );
               Linv = safelanes_initialConductivity(ei);
               score *= ALPHA * Linv * Linv;

               if (score < score_best) {
                  ei_best = ei;
                  score_best = score;
               }
            }
            turns_next_time++; /* We had to forgo a lane-build this time! */
         }

         presence_budget[fi][si] -= 1 / safelanes_initialConductivity(ei_best) / faction_stack[fi].lane_length_per_presence;
         if (presence_budget[fi][si] <= 0 && lal[fi].x == NULL)
            lal_bases[fi] -= sys_to_first_vertex[1+si] - sys_to_first_vertex[si];
         safelanes_updateConductivity( ei_best );
         lane_faction[ ei_best ] = faction_stack[fi].id;
      }
   }

#if DEBUGGING
   for (fi=0; fi<array_size(faction_stack); fi++)
      if ( lal[fi].x != NULL)
         assert( "We correctly tracked the row offsets between the 'lal' and 'utilde' matrices" && lal[fi].nrow == lal_bases[fi] );
#endif

   for (fi=0; fi<array_size(faction_stack); fi++)
      matwrap_free( lal[fi] );
   free( lal );
   free( lal_bases );
   array_free( edgeind_opts );
   array_free( facind_vals );
   array_free( facind_opts );

   return turns_next_time;
}


/** @brief It's a qsort comparator. Set the cmp_key_ref pointer prior to use, or else. */
static int cmp_key( const void* p1, const void* p2 )
{
   double d = cmp_key_ref[*(int*)p1] - cmp_key_ref[*(int*)p2];
   return SIGN( d );
}


/**
 * @brief Return true if this triangle is so flat that lanes from point m to point n aren't allowed.
 */
static int safelanes_triangleTooFlat( const Vector2d* m, const Vector2d* n, const Vector2d* p, double lmn )
{
   const double MAX_COSINE = cos(MIN_ANGLE);
   double lnp = vect_dist( n, p );
   double lmp = vect_dist( m, p );
   double dpn = ((n->x-m->x)*(n->x-p->x) + (n->y-m->y)*(n->y-p->y)) / ( lmn * lnp );
   double dpm = ((m->x-n->x)*(m->x-p->x) + (m->y-n->y)*(m->y-p->y)) / ( lmn * lmp );
   return (dpn > MAX_COSINE && lnp < lmn) || (dpm > MAX_COSINE && lmp < lmn);
}


/**
 * @brief Return the vertex's owning faction (ID, not faction_stack index), or -1 if not applicable.
 */
static int vertex_faction( int vi )
{
   const StarSystem *sys = system_getIndex(vertex_stack[vi].system);
   switch (vertex_stack[vi].type) {
      case VERTEX_PLANET:
         return sys->planets[vertex_stack[vi].index]->faction;
      case VERTEX_JUMP:
         return -1;
      default:
         ERR( _("Invalid vertex type.") );
   }
}


/**
 * @brief Return the vertex's coordinates within its system (by reference since our vec2's are fat).
 */
static const Vector2d* vertex_pos( int vi )
{
   const StarSystem *sys = system_getIndex(vertex_stack[vi].system);
   switch (vertex_stack[vi].type) {
      case VERTEX_PLANET:
         return &sys->planets[vertex_stack[vi].index]->pos;
      case VERTEX_JUMP:
         return &sys->jumps[vertex_stack[vi].index].pos;
      default:
         ERR( _("Invalid vertex type.") );
   }
}


/** @brief Return the faction_stack index corresponding to a faction ID, or -1. */
static inline int FACTION_ID_TO_INDEX( int id )
{
   for (int i=0; i<array_size(faction_stack) && faction_stack[i].id <= id; i++)
      if (faction_stack[i].id == id)
         return i;
   return -1;
}


/** @brief Return a mask matching any faction. */
static inline FactionMask MASK_ANY_FACTION()
{
   return ~(FactionMask)0;
}


/** @brief A mask giving this faction (NOT faction_stack index) exclusive rights to build, if it's a lane-building faction. */
static inline FactionMask MASK_ONE_FACTION( int id )
{
   int i = FACTION_ID_TO_INDEX( id );
   return i>0 ? ((FactionMask)1)<<i : MASK_ANY_FACTION();
}


/** @brief A mask with appropriate lane-building rights given one faction ID owning each endpoint. */
static inline FactionMask MASK_COMPROMISE( int id1, int id2 )
{
   FactionMask m1 = MASK_ONE_FACTION(id1), m2 = MASK_ONE_FACTION(id2);
   return (m1 & m2) ? (m1 & m2) : (m1 | m2);  /* Any/Any -> any, Any/f -> just f, f1/f2 -> either. */
}


static inline void triplet_entry( cholmod_triplet* m, int i, int j, double x )
{
   ((int*)m->i)[m->nnz] = i;
   ((int*)m->j)[m->nnz] = j;
   ((double*)m->x)[m->nnz] = x;
   m->nnz++;
}


/** @brief Allocate a matrix fitting this description. */
static void matwrap_init( MatWrap* A, enum CBLAS_ORDER order, int nrow, int ncol )
{
   A->order = order;
   A->nrow  = nrow;
   A->ncol  = ncol;
   A->x     = malloc( nrow*ncol*sizeof(double) );
}


/** @brief Construct a MatWrap representing the given cholmod_dense. It's just a wrapper, referencing the same memory. */
static inline MatWrap matwrap_from_cholmod( cholmod_dense* m )
{
   MatWrap A = {.order = CblasColMajor, .nrow = m->nrow, .ncol = m->ncol, .x = m->x};
   return A;
}


/** @brief Construct a MatWrap representing the input's transpose. It's just a wrapper, referencing the same memory. */
static inline MatWrap matwrap_transpose( MatWrap A )
{
   MatWrap At = {.order = A.order == CblasColMajor ? CblasRowMajor : CblasColMajor, .nrow = A.ncol, .ncol = A.nrow, .x = A.x};
   return At;
}


/**
 * @brief Construct the matrix-slice of m, selecting those rows where the corresponding presence value is positive.
 */
static void matwrap_sliceByPresence( MatWrap *A, double* sysPresence, MatWrap* out )
{
   int nr, nc, si;
   size_t in_base, out_base, sz;

   nc = A->ncol;
   nr = 0;
   for (si = 0; si < array_size(sys_to_first_vertex) - 1; si++)
      if (sysPresence[si] > 0)
         nr += sys_to_first_vertex[1+si] - sys_to_first_vertex[si];

   if (A->order != CblasRowMajor)
      matwrap_reorder_in_place( A );
   matwrap_init( out, CblasRowMajor, nr, nc );

   in_base = out_base = 0;
   for (si = 0; si < array_size(sys_to_first_vertex) - 1; si++) {
      sz = (sys_to_first_vertex[1+si] - sys_to_first_vertex[si]) * nc;
      if (sysPresence[si] > 0) {
         memcpy( &out->x[out_base], &A->x[in_base], sz * sizeof(double) );
         out_base += sz;
      }
      in_base += sz;
   }
}


/** @brief Convert in-place from column-major to row-major, or vice-versa. */
static void matwrap_reorder_in_place( MatWrap* A )
{
   int ldA, ldAt;
   ldA  = A->order == CblasColMajor ? A->nrow : A->ncol;
   ldAt = A->order == CblasRowMajor ? A->nrow : A->ncol;
   cblas_dimatcopy( A->order, CblasTrans, A->nrow, A->ncol, 1, A->x, ldA, ldAt );
   A->order = A->order == CblasColMajor ? CblasRowMajor : CblasColMajor;
}


/** @brief Initialize C with the product A*B. */
static void matwrap_mul( MatWrap A, MatWrap B, MatWrap* C )
{
   assert( A.ncol == B.nrow );
   matwrap_init( C, CblasColMajor, A.nrow, B.ncol );
   cblas_dgemm(
         C->order,
         A.order == C->order ? CblasNoTrans : CblasTrans,
         B.order == C->order ? CblasNoTrans : CblasTrans,
         A.nrow,
         B.ncol,
         A.ncol,
         1,
         A.x,
         A.order == CblasColMajor ? A.nrow : A.ncol,
         B.x,
         B.order == CblasColMajor ? B.nrow : B.ncol,
         0,
         C->x,
         C->order == CblasColMajor ? C->nrow : C->ncol
   );
}


/** @brief Return the i,j entry of A*B. */
static double matwrap_mul_elem( MatWrap A, MatWrap B, int i, int j )
{
   assert( A.ncol == B.nrow );
   return cblas_ddot(
         A.ncol,
         &A.x[A.order == CblasColMajor ? i : i*A.ncol],
              A.order == CblasRowMajor ? 1 : A.nrow,
         &B.x[B.order == CblasRowMajor ? j : j*B.nrow],
              B.order == CblasColMajor ? 1 : B.ncol
   );
}


/** @brief Cleans up resources allocated by matwrap_init (or indirectly by matwrap_mul, etc.). */
static void matwrap_free( MatWrap A )
{
   free( A.x );
}
