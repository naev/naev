/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file safelanes.c
 *
 * @brief Handles factions' safe lanes through systems.
 */

/** @cond */
#include <math.h>

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
static const double CONVERGENCE_THRESHOLD  = 1e-12;     /**< Stop optimizing after a relative change of U~ this small. */
static const double MAX_ITERATIONS         = 20;        /**< Stop optimizing after this many iterations, at most. */
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
   int index;                   /**< Index in the planet stack, or the system's jump stack. */
} Vertex;

/** @brief An edge is a pair of vertex indices. */
typedef int Edge[2];

/** @brief Description of a lane-building faction. */
typedef struct Faction_ {
   int id;                              /**< Faction ID. */
   double lane_length_per_presence;     /**< Weight determining their ability to claim lanes. */
} Faction;

/** @brief A bet that we'll never willingly point this algorithm at 32 factions. */
typedef uint32_t FactionMask;


/*
 * Global state.
 */
static cholmod_common C;        /**< Parameter settings, statistics, and workspace used internally by CHOLMOD. */
static Vertex *vertex_stack;    /**< Array (array.h): Everything eligible to be a lane endpoint. */
static int *sys_to_first_vertex;/**< Array (array.h): For each system index, the id of its first vertex, + sentinel. */
static Edge *edge_stack;        /**< Array (array.h): Everything eligible to be a lane. */
static Faction *faction_stack;  /**< Array (array.h): The faction IDs that can build lanes. */
static int *lane_faction;       /**< Array (array.h): Per edge, ID of faction that built a lane there, if any, else 0. */
static FactionMask *lane_fmask; /**< Array (array.h): Per edge, ID of faction that built a lane there, if any, else 0. */
static int *tmp_planet_indices; /**< Array (array.h): The vertex indices for planets. Used to initialize "ftilde", "PPl". */
static Edge *tmp_jump_edges;    /**< Array (array.h): The vertex ID pairs connected by 2-way jumps. Used to initialize "stiff". */
static double *tmp_edge_conduct;/**< Array (array.h): Conductivity (1/len) of each potential lane. Used to initialize "stiff". */
static int *tmp_anchor_vertices;/**< Array (array.h): One vertex ID per connected component. Used to initialize "stiff". */
static UnionFind tmp_sys_uf;    /**< The partition of {system indices} into connected components (connected by 2-way jumps). */
static cholmod_triplet *stiff;  /**< K matrix, UT triplets: internal edges (E*3), implicit jump connections, anchor conditions. */
static cholmod_sparse *QtQ;     /**< (Q*)Q where Q is the ExV difference matrix. */
static cholmod_dense *ftilde;   /**< Fluxes (bunch of F columns in the KU=F problem). */
static cholmod_dense *utilde;   /**< Potentials (bunch of U columns in the KU=F problem). */
static cholmod_dense **PPl;     /**< Array: (array.h): For each builder faction, The (P*)P in: grad_u(phi)=(Q*)Q U~ (P*)P. */

/*
 * Prototypes.
 */
static double safelanes_changeFromOptimizer (void);
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
static void safelanes_initQtQ (void);
static void safelanes_initFTilde (void);
static void safelanes_initPPl (void);
static int safelanes_triangleTooFlat( const Vector2d* m, const Vector2d* n, const Vector2d* p, double lmn );
static int vertex_faction( int i );
static const Vector2d* vertex_pos( int i );
static inline int FACTION_ID_TO_INDEX( int id );
static inline FactionMask MASK_ANY_FACTION();
static inline FactionMask MASK_ONE_FACTION( int id );
static inline FactionMask MASK_COMPROMISE( int id1, int id2 );

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

   /* FIXME: How slow is this full scan? We could have a sys_to_first_edge stack as well. */
   for (i=0; i<array_size(lane_faction); i++) {
      for (j=0; j<2; j++)
         v[j] = &vertex_stack[edge_stack[i][j]];
      if (v[0]->system == system->id && lane_faction[i] > 0 && (faction < 0 || faction == lane_faction[i])) {
         l = &array_grow( &out );
         l->faction = lane_faction[i];
         for (j=0; j<2; j++) {
            switch (v[j]->type) {
               case VERTEX_PLANET:
                  l->point_type[j]   = SAFELANE_LOC_PLANET;
                  l->point_id[j]     = v[j]->index;
                  break;
               case VERTEX_JUMP:
                  l->point_type[j]   = SAFELANE_LOC_DEST_SYS;
                  l->point_id[j]     = system_getIndex(v[j]->system)->jumps[v[j]->index].targetid;
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
   safelanes_initStacks();
   safelanes_initOptimizer();
   for (int i=0; i<MAX_ITERATIONS; i++)
      if (safelanes_changeFromOptimizer() <= CONVERGENCE_THRESHOLD)
         break;
   safelanes_destroyOptimizer();
   /* Stacks remain available for queries. */
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
 * @brief Run a round of optimization, and return the relative change to U~.
 */
static double safelanes_changeFromOptimizer (void)
{
   return 0; // TODO
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
                     array_push_back_edge( &tmp_jump_edges, j, array_size(vertex_stack)-1 );
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
   }
   array_shrink( &edge_stack );

   lane_faction = array_create_size( int, array_size(edge_stack) );
   memset( lane_faction, 0, array_size(lane_faction)*sizeof(lane_faction[0]) );
}


/**
 * @brief Sets up the local stacks with entry per faction.
 */
static void safelanes_initStacks_faction (void)
{
   int *faction_all;

   faction_stack = array_create( Faction );
   faction_all = faction_getAll();
   for (int i=0; i<array_size(faction_all); i++) {
      Faction f = {.id = i, .lane_length_per_presence = faction_lane_length_per_presence(i)};
      if (f.lane_length_per_presence > 0.)
         array_push_back( &faction_stack, f );
   }
   array_free( faction_all );
   array_shrink( &faction_stack );
}


/**
 * @brief Sets up the local stacks with entry per faction.
 */
static void safelanes_initStacks_anchor (void)
{
   int i, nsys, *systems;

   nsys = array_size(sys_to_first_vertex) - 1;
   unionfind_init( &tmp_sys_uf, nsys );
   for (i=0; i<array_size(tmp_jump_edges); i++)
      unionfind_union( &tmp_sys_uf, vertex_stack[tmp_jump_edges[i][0]].system, vertex_stack[tmp_jump_edges[i][1]].system );
   systems = unionfind_findall( &tmp_sys_uf );
   tmp_anchor_vertices = array_create_size( int, array_size(systems) );

   /* Add an anchor vertex per system, but only if there actually is a vertex in the system. */
   for (i=0; i<array_size(systems); i++)
      if (sys_to_first_vertex[systems[i]] < sys_to_first_vertex[1+systems[i]]) {
         array_push_back( &tmp_anchor_vertices, sys_to_first_vertex[systems[i]] );
         DEBUG( _("Anchoring safelanes graph in system: %s."), system_getIndex(systems[i])->name );
      }
   array_free( systems );
}


/**
 * @brief Tears down the local faction/object stacks.
 */
static void safelanes_destroyStacks (void)
{
   safelanes_destroyTmp();
   array_free( vertex_stack );
   vertex_stack = NULL;
   array_free( edge_stack );
   edge_stack = NULL;
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
   array_free( lane_faction );
   lane_faction = NULL;
}


/**
 * @brief Sets up the stiffness matrix.
 */
static void safelanes_initStiff (void)
{
   int n, v, i;
   int *pi, *pj;
   double *pv;
   double max_conductivity;

   cholmod_free_triplet( &stiff, &C );
   v = array_size(vertex_stack);
   n = 3*(array_size(edge_stack)+array_size(tmp_jump_edges)) + array_size(tmp_anchor_vertices);
   stiff = cholmod_allocate_triplet( v, v, n, STORAGE_MODE_UPPER_TRIANGULAR_PART, CHOLMOD_REAL, &C );
   /* Populate triplets: internal edges (ii ij jj), implicit jump connections (ditto), anchor conditions. */
   pi = stiff->i; pj = stiff->j; pv = stiff->x;
   for (i=0; i<array_size(edge_stack); i++) {
      *pi++ = edge_stack[i][0]; *pj++ = edge_stack[i][0]; *pv++ = +tmp_edge_conduct[i];
      *pi++ = edge_stack[i][0]; *pj++ = edge_stack[i][1]; *pv++ = -tmp_edge_conduct[i];
      *pi++ = edge_stack[i][1]; *pj++ = edge_stack[i][1]; *pv++ = +tmp_edge_conduct[i];
   }
   for (i=0; i<array_size(tmp_jump_edges); i++) {
      *pi++ = tmp_jump_edges[i][0]; *pj++ = tmp_jump_edges[i][0]; *pv++ = +JUMP_CONDUCTIVITY;
      *pi++ = tmp_jump_edges[i][0]; *pj++ = tmp_jump_edges[i][1]; *pv++ = -JUMP_CONDUCTIVITY;
      *pi++ = tmp_jump_edges[i][1]; *pj++ = tmp_jump_edges[i][1]; *pv++ = +JUMP_CONDUCTIVITY;
   }
   /* Add a Robin boundary condition, using the max conductivity (after activation) for spectral reasons. */
   max_conductivity = JUMP_CONDUCTIVITY/(ALPHA+1);
   for (i=0; i<array_size(edge_stack); i++)
      max_conductivity = MAX( max_conductivity, tmp_edge_conduct[i] );
   max_conductivity = MAX( JUMP_CONDUCTIVITY, (1+ALPHA)*max_conductivity ); /* Activation scales entries by 1+ALPHA later. */
   for (i=0; i<array_size(tmp_anchor_vertices); i++) {
      *pi++ = tmp_anchor_vertices[i]; *pj++ = tmp_anchor_vertices[i]; *pv++ = max_conductivity;
   }
#if DEBUGGING
   assert( cholmod_check_triplet( stiff, &C) );
#endif
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
   int np, i, j, k, facti, factj, *pi, *pj, sysi, sysj;
   double *pv;
   Planet *pnti, *pntj;

   np = array_size(tmp_planet_indices);
#define MULTI_INDEX( i, j ) ((i*(i-1))/2 + j)
   P = cholmod_allocate_triplet( np, MULTI_INDEX(np,0), np*(np-1), STORAGE_MODE_UNSYMMETRIC, CHOLMOD_REAL, &C );
   pi = P->i; pj = P->j; pv = P->x;
   for (i=0; i<np; i++)
      for (j=0; j<i; j++) {
         *pi++ = i; *pj++ = MULTI_INDEX(i,j); *pv++ = +1;
         *pi++ = j; *pj++ = MULTI_INDEX(i,j); *pv++ = -1;
      }
#if DEBUGGING
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
      pnti = planet_getIndex( vertex_stack[tmp_planet_indices[i]].index );
      facti = FACTION_ID_TO_INDEX( pnti->faction );
      for (j=0; j<i; j++) {
         sysj = vertex_stack[tmp_planet_indices[j]].system;
         if (unionfind_find( &tmp_sys_uf, sysi ) == unionfind_find( &tmp_sys_uf, sysj )) {
            pntj = planet_getIndex( vertex_stack[tmp_planet_indices[i]].index );
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
static int vertex_faction( int i )
{
   const StarSystem *sys = system_getIndex(vertex_stack[i].system);
   switch (vertex_stack[i].type) {
      case VERTEX_PLANET:
         return sys->planets[vertex_stack[i].index]->faction;
      case VERTEX_JUMP:
         return -1;
      default:
         ERR( _("Invalid vertex type.") );
   }
}


/**
 * @brief Return the vertex's coordinates within its system (by reference since our vec2's are fat).
 */
static const Vector2d* vertex_pos( int i )
{
   const StarSystem *sys = system_getIndex(vertex_stack[i].system);
   switch (vertex_stack[i].type) {
      case VERTEX_PLANET:
         return &sys->planets[vertex_stack[i].index]->pos;
      case VERTEX_JUMP:
         return &sys->jumps[vertex_stack[i].index].pos;
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
