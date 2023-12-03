/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_linopt.c
 *
 * @brief Handles Linear linoptization in Lua.
 */

/** @cond */
#include <glpk.h>
#include <lauxlib.h>
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "nlua_linopt.h"

#include "log.h"
#include "nstring.h"
#include "nluadef.h"

#define LINOPT_MAX_TM   1000  /**< Maximum time to optimize (in ms). Applied to linear relaxation and MIP independently. */

/**
 * @brief Our cute little linear program wrapper.
 */
typedef struct LuaLinOpt_s {
   int ncols;        /**< Number of structural variables. */
   int nrows;        /**< Number of auxiliary variables (constraints). */
   glp_prob *prob;   /**< Problem structure itself. */
} LuaLinOpt_t;

/* Optim metatable methods. */
static int linoptL_gc( lua_State *L );
static int linoptL_eq( lua_State *L );
static int linoptL_new( lua_State *L );
static int linoptL_size( lua_State *L );
static int linoptL_addcols( lua_State *L );
static int linoptL_addrows( lua_State *L );
static int linoptL_setcol( lua_State *L );
static int linoptL_setrow( lua_State *L );
static int linoptL_loadmatrix( lua_State *L );
static int linoptL_solve( lua_State *L );
static int linoptL_readProblem( lua_State *L );
static int linoptL_writeProblem( lua_State *L );
static const luaL_Reg linoptL_methods[] = {
   { "__gc", linoptL_gc },
   { "__eq", linoptL_eq },
   { "new", linoptL_new },
   { "size", linoptL_size },
   { "add_cols", linoptL_addcols },
   { "add_rows", linoptL_addrows },
   { "set_col", linoptL_setcol },
   { "set_row", linoptL_setrow },
   { "load_matrix", linoptL_loadmatrix },
   { "solve", linoptL_solve },
   { "read_problem", linoptL_readProblem },
   { "write_problem", linoptL_writeProblem },
   {0,0}
}; /**< Optim metatable methods. */

/**
 * @brief Loads the linopt library.
 *
 *    @param env Environment to load linopt library into.
 *    @return 0 on success.
 */
int nlua_loadLinOpt( nlua_env env )
{
   nlua_register(env, LINOPT_METATABLE, linoptL_methods, 1);
   return 0;
}

/**
 * @brief Lua bindings to interact with linopts.
 *
 * @luamod linopt
 */
/**
 * @brief Gets linopt at index.
 *
 *    @param L Lua state to get linopt from.
 *    @param ind Index position to find the linopt.
 *    @return Optim found at the index in the state.
 */
LuaLinOpt_t* lua_tolinopt( lua_State *L, int ind )
{
   return (LuaLinOpt_t*) lua_touserdata(L,ind);
}

/**
 * @brief Gets linopt at index or raises error if there is no linopt at index.
 *
 *    @param L Lua state to get linopt from.
 *    @param ind Index position to find linopt.
 *    @return Optim found at the index in the state.
 */
LuaLinOpt_t* luaL_checklinopt( lua_State *L, int ind )
{
   if (lua_islinopt(L,ind))
      return lua_tolinopt(L,ind);
   luaL_typerror(L, ind, LINOPT_METATABLE);
   return NULL;
}

/**
 * @brief Pushes a linopt on the stack.
 *
 *    @param L Lua state to push linopt into.
 *    @param linopt Optim to push.
 *    @return Newly pushed linopt.
 */
LuaLinOpt_t* lua_pushlinopt( lua_State *L, LuaLinOpt_t linopt )
{
   LuaLinOpt_t *c = (LuaLinOpt_t*) lua_newuserdata(L, sizeof(LuaLinOpt_t));
   *c = linopt;
   luaL_getmetatable(L, LINOPT_METATABLE);
   lua_setmetatable(L, -2);
   return c;
}

/**
 * @brief Checks to see if ind is a linopt.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a linopt.
 */
int lua_islinopt( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, LINOPT_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Frees a linopt.
 *
 *    @luatparam Optim linopt Optim to free.
 * @luafunc __gc
 */
static int linoptL_gc( lua_State *L )
{
   LuaLinOpt_t *lp = luaL_checklinopt(L,1);
   glp_delete_prob(lp->prob);
   return 0;
}

/**
 * @brief Compares two linopts to see if they are the same.
 *
 *    @luatparam Optim d1 Optim 1 to compare.
 *    @luatparam Optim d2 Optim 2 to compare.
 *    @luatreturn boolean true if both linopts are the same.
 * @luafunc __eq
 */
static int linoptL_eq( lua_State *L )
{
   LuaLinOpt_t *lp1, *lp2;
   lp1 = luaL_checklinopt(L,1);
   lp2 = luaL_checklinopt(L,2);
   lua_pushboolean( L, (memcmp( lp1, lp2, sizeof(LuaLinOpt_t))==0) );
   return 1;
}

/**
 * @brief Opens a new linopt.
 *
 *    @luatparam[opt=nil] string name Name of the optimization program.
 *    @luatparam number cols Number of columns in the optimization program.
 *    @luatparam number rows Number of rows in the optimization program.
 *    @luatparam[opt=false] boolean maximize Whether to maximize (rather than minimize) the function.
 *    @luatreturn Optim New linopt object.
 * @luafunc new
 */
static int linoptL_new( lua_State *L )
{
   LuaLinOpt_t lp;
   const char *name;
   int max;

   /* Input. */
   name     = luaL_optstring(L,1,NULL);
   lp.ncols = luaL_checkinteger(L,2);
   lp.nrows = luaL_checkinteger(L,3);
   max      = lua_toboolean(L,4);

#ifdef DEBUGGING
   if (lp.ncols <= 0)
      NLUA_ERROR( L, _("Number of columns in a linear optimization problem must be greater than 0!") );
#endif /* DEBUGGING */

   /* Initialize and create. */
   lp.prob = glp_create_prob();
   glp_set_prob_name( lp.prob, name );
   glp_add_cols( lp.prob, lp.ncols );
   glp_add_rows( lp.prob, lp.nrows );
   if (max)
      glp_set_obj_dir( lp.prob, GLP_MAX );

   lua_pushlinopt( L, lp );
   return 1;
}

/**
 * @brief Adds columns to the linear program.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatreturn number Number of columns in the linear program.
 *    @luatreturn number Number of rows in the linear program.
 * @luafunc size
 */
static int linoptL_size( lua_State *L )
{
   LuaLinOpt_t *lp = luaL_checklinopt(L,1);
   lua_pushinteger( L, glp_get_num_cols( lp->prob ) );
   lua_pushinteger( L, glp_get_num_rows( lp->prob ) );
   return 2;
}

/**
 * @brief Adds columns to the linear program.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number cols Number of columns to add.
 * @luafunc add_cols
 */
static int linoptL_addcols( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   int toadd         = luaL_checkinteger(L,2);
   glp_add_cols( lp->prob, toadd );
   lp->ncols += toadd;
   return 0;
}

/**
 * @brief Adds rows to the linear program.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number rows Number of rows to add.
 * @luafunc add_rows
 */
static int linoptL_addrows( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   int toadd         = luaL_checkinteger(L,2);
   glp_add_rows( lp->prob, toadd );
   lp->nrows += toadd;
   return 0;
}

/**
 * @brief Adds an optimization column.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number index Index of the column to set.
 *    @luatparam string name Name of the column being added.
 *    @luatparam number coefficient Coefficient of the objective function being added.
 *    @luatparam[opt="real"] string kind Kind of the column being added. Can be either "real", "integer", or "binary".
 *    @luatparam[opt=nil] number lb Lower bound of the column.
 *    @luatparam[opt=nil] number ub Upper bound of the column.
 * @luafunc set_col
 */
static int linoptL_setcol( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   int idx           = luaL_checkinteger(L,2);
   const char *name  = luaL_checkstring(L,3);
   double coef       = luaL_checknumber(L,4);
   const char *skind = luaL_optstring(L,5,"real");
   int haslb         = !lua_isnoneornil(L,6);
   int hasub         = !lua_isnoneornil(L,7);
   double lb         = luaL_optnumber(L,6,0.0);
   double ub         = luaL_optnumber(L,7,0.0);
   int type = GLP_FR, kind = GLP_CV;

   /* glpk stuff */
   glp_set_col_name( lp->prob, idx, name );
   glp_set_obj_coef( lp->prob, idx, coef );

   /* Determine bounds. */
   if (haslb && hasub) {
      if (fabs(lb-ub) < 1e-5)
         type = GLP_FX;
      else
         type = GLP_DB;
   }
   else if (haslb)
      type = GLP_LO;
   else if (hasub)
      type = GLP_UP;
   else
      type = GLP_FR;
   glp_set_col_bnds( lp->prob, idx, type, lb, ub );

   /* Get kind. */
   if (strcmp(skind,"real")==0)
      kind = GLP_CV;
   else if (strcmp(skind,"integer")==0)
      kind = GLP_IV;
   else if (strcmp(skind,"binary")==0)
      kind = GLP_BV;
   else
      NLUA_ERROR(L,_("Unknown column kind '%s'!"), skind);
   glp_set_col_kind( lp->prob, idx, kind );

   return 0;
}

/**
 * @brief Adds an optimization row.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number index Index of the row to set.
 *    @luatparam string name Name of the row being added.
 *    @luatparam[opt=nil] number lb Lower bound of the row.
 *    @luatparam[opt=nil] number ub Upper bound of the row.
 * @luafunc set_row
 */
static int linoptL_setrow( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   int idx           = luaL_checkinteger(L,2);
   const char *name  = luaL_checkstring(L,3);
   int haslb, hasub, type;
   double lb, ub;

   /* glpk stuff */
   glp_set_row_name( lp->prob, idx, name );

   /* Determine bounds. */
   haslb = !lua_isnoneornil(L,4);
   hasub = !lua_isnoneornil(L,5);
   lb    = luaL_optnumber(L,4,0.0);
   ub    = luaL_optnumber(L,5,0.0);
   if (haslb && hasub) {
      if (fabs(lb-ub) < 1e-5)
         type = GLP_FX;
      else
         type = GLP_DB;
   }
   else if (haslb)
      type = GLP_LO;
   else if (hasub)
      type = GLP_UP;
   else
      type = GLP_FR;
   glp_set_row_bnds( lp->prob, idx, type, lb, ub );

   return 0;
}

/**
 * @brief Loads the entire matrix for the linear program.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatparam number row_indices Indices of the rows.
 *    @luatparam number col_indices Indices of the columns.
 *    @luatparam number coefficients Values of the coefficients.
 * @luafunc load_matrix
 */
static int linoptL_loadmatrix( lua_State *L )
{
   size_t n;
   int *ia, *ja;
   double *ar;
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   luaL_checktype(L, 2, LUA_TTABLE);
   luaL_checktype(L, 3, LUA_TTABLE);
   luaL_checktype(L, 4, LUA_TTABLE);

   /* Make sure size is ok. */
   n = lua_objlen(L,2);
#if DEBUGGING
   if ((n != lua_objlen(L,3)) || (n != lua_objlen(L,4)))
      NLUA_ERROR(L, _("Table lengths don't match!"));
#endif /* DEBUGGING */

   /* Load everything from tables, has to be 1-index based. */
   ia = calloc( n+1, sizeof(int) );
   ja = calloc( n+1, sizeof(int) );
   ar = calloc( n+1, sizeof(double) );
   for (size_t i=1; i<=n; i++) {
      lua_rawgeti(L, 2, i);
      lua_rawgeti(L, 3, i);
      lua_rawgeti(L, 4, i);
#if DEBUGGING
      ia[i] = luaL_checkinteger(L,-3);
      ja[i] = luaL_checkinteger(L,-2);
      ar[i] = luaL_checknumber(L,-1);
#else /* DEBUGGING */
      ia[i] = lua_tointeger(L,-3);
      ja[i] = lua_tointeger(L,-2);
      ar[i] = lua_tonumber(L,-1);
#endif /* DEBUGGING */
      lua_pop(L,3);
   }

   /* Set up the matrix. */
   glp_load_matrix( lp->prob, n, ia, ja, ar );

   /* Clean up. */
   free(ia);
   free(ja);
   free(ar);
   return 0;
}

static const char* linopt_status( int retval )
{
   switch (retval) {
      case GLP_OPT:
         return "solution is optimal";
      case GLP_FEAS:
         return "solution is feasible";
      case GLP_INFEAS:
         return "solution is infeasible";
      case GLP_NOFEAS:
         return "problem has no feasible solution";
      case GLP_UNBND:
         return "problem has unbounded solution";
      case GLP_UNDEF:
         return "solution is undefined";
      default:
         return "unknown GLPK status";
   }
}
static const char* linopt_error( int retval )
{
   switch (retval) {
      case 0:
         return "No error";

      /* COMMON */
      case GLP_EFAIL:
         return "The search was prematurely terminated due to the solver failure.";
      case GLP_ETMLIM:
         return "The search was prematurely terminated, because the time limit has been exceeded.";

      /* SIMPLEX */
      case GLP_EBADB:
         return "Unable to start the search, because the initial basis specified in the problem object is invalid—the number of basic (auxiliary and structural) variables is not the same as the number of rows in the problem object.";
      case GLP_ESING:
         return "Unable to start the search, because the basis matrix corresponding to the initial basis is singular within the working precision.";
      case GLP_ECOND:
         return "Unable to start the search, because the basis matrix corresponding to the initial basis is ill-conditioned, i.e. its condition number is too large.";
      case GLP_EBOUND:
         return "Unable to start the search, because some double-bounded (auxiliary or structural) variables have incorrect bounds.";
      case GLP_EOBJLL:
         return "The search was prematurely terminated, because the objective function being maximized has reached its lower limit and continues decreasing (the dual simplex only).";
      case GLP_EOBJUL:
         return "The search was prematurely terminated, because the objective function being minimized has reached its upper limit and continues increasing (the dual simplex only).";
      case GLP_EITLIM:
         return "The search was prematurely terminated, because the simplex iteration limit has been exceeded.";

      /* INTOPT */
      case GLP_EROOT:
         return "Unable to start the search, because optimal basis for initial LP relaxation is not provided. (This code may appear only if the presolver is disabled.)";
      case GLP_ENOPFS:
         return "Unable to start the search, because LP relaxation of the MIP problem instance has no primal feasible solution. (This code may appear only if the presolver is enabled.)";
      case GLP_ENODFS:
         return "Unable to start the search, because LP relaxation of the MIP problem instance has no dual feasible solution. In other word, this code means that if the LP relaxation has at least one primal feasible solution, its optimal solution is unbounded, so if the MIP problem has at least one integer feasible solution, its (integer) optimal solution is also unbounded. (This code may appear only if the presolver is enabled.)";
      case GLP_EMIPGAP:
         return "The search was prematurely terminated, because the relative mip gap tolerance has been reached.";
      case GLP_ESTOP:
         return "The search was prematurely terminated by application. (This code may appear only if the advanced solver interface is used.)";

      default:
         return "Unknown error.";
   }
}
#if 1 /* GLPK Defaults. */
/* SMCP */
#define METH_DEF     GLP_PRIMAL
#define PRICING_DEF  GLP_PT_PSE
#define R_TEST_DEF   GLP_RT_HAR
#define PRESOLVE_DEF GLP_OFF
/* IOCP */
#define BR_TECH_DEF  GLP_BR_DTH
#define BT_TECH_DEF  GLP_BT_BLB
#define PP_TECH_DEF  GLP_PP_ALL
#define SR_HEUR_DEF  GLP_ON
#define FP_HEUR_DEF  GLP_OFF
#define PS_HEUR_DEF  GLP_OFF
#define GMI_CUTS_DEF GLP_OFF
#define MIR_CUTS_DEF GLP_OFF
#define COV_CUTS_DEF GLP_OFF
#define CLQ_CUTS_DEF GLP_OFF
#else
/* Customized "optimal" defaults. */
#define BR_TECH_DEF  GLP_BR_PCH
#define BT_TECH_DEF  GLP_BT_DFS
#define PP_TECH_DEF  GLP_PP_ALL
#define SR_HEUR_DEF  GLP_ON
#define FP_HEUR_DEF  GLP_OFF
#define PS_HEUR_DEF  GLP_OFF
#define GMI_CUTS_DEF GLP_ON
#define MIR_CUTS_DEF GLP_OFF
#define COV_CUTS_DEF GLP_ON
#define CLQ_CUTS_DEF GLP_ON
#endif
#define STRCHK( val, ret ) if (strcmp(str,(val))==0) return (ret);
static int opt_meth( const char *str, int def )
{
   if (str==NULL) return def;
   STRCHK( "primal", GLP_PRIMAL );
   STRCHK( "dual",   GLP_DUAL );
   STRCHK( "dualp",  GLP_DUALP );
   WARN("Unknown meth value '%s'", str);
   return def;
}
static int opt_pricing( const char *str, int def )
{
   if (str==NULL) return def;
   STRCHK( "std", GLP_PT_STD );
   STRCHK( "pse", GLP_PT_PSE );
   WARN("Unknown pricing value '%s'", str);
   return def;
}
static int opt_r_test( const char *str, int def )
{
   if (str==NULL) return def;
   STRCHK( "std", GLP_RT_STD );
   STRCHK( "har", GLP_RT_HAR );
   WARN("Unknown r_test value '%s'", str);
   return def;
}
static int opt_br_tech( const char *str, int def )
{
   if (str==NULL) return def;
   STRCHK( "ffv", GLP_BR_FFV );
   STRCHK( "lfv", GLP_BR_LFV );
   STRCHK( "mfv", GLP_BR_MFV );
   STRCHK( "dth", GLP_BR_DTH );
   STRCHK( "pch", GLP_BR_PCH );
   WARN("Unknown br_tech value '%s'", str);
   return def;
}
static int opt_bt_tech( const char *str, int def )
{
   if (str==NULL) return def;
   STRCHK( "dfs", GLP_BT_DFS );
   STRCHK( "bfs", GLP_BT_BFS );
   STRCHK( "blb", GLP_BT_BLB );
   STRCHK( "bph", GLP_BT_BPH );
   WARN("Unknown bt_tech value '%s'", str);
   return def;
}
static int opt_pp_tech( const char *str, int def )
{
   if (str==NULL) return def;
   STRCHK( "none", GLP_PP_NONE );
   STRCHK( "root", GLP_PP_ROOT );
   STRCHK( "all", GLP_PP_ALL );
   WARN("Unknown pp_tech value '%s'", str);
   return def;
}
static int opt_onoff( const char *str, int def )
{
   if (str==NULL) return def;
   STRCHK( "on", GLP_ON );
   STRCHK( "off", GLP_OFF );
   WARN("Unknown onoff value '%s'", str);
   return def;
}
#undef STRCHK

#define GETOPT_IOCP( name, func, def ) do {lua_getfield(L,2,#name); parm_iocp.name = func( luaL_optstring(L,-1,NULL), def ); lua_pop(L,1); } while (0)
#define GETOPT_SMCP( name, func, def ) do {lua_getfield(L,2,#name); parm_smcp.name = func( luaL_optstring(L,-1,NULL), def ); lua_pop(L,1); } while (0)
/**
 * @brief Solves the linear optimization problem.
 *
 *    @luatparam LinOpt lp Linear program to modify.
 *    @luatreturn number The value of the primal funcation.
 *    @luatreturn table Table of column values.
 * @luafunc solve
 */
static int linoptL_solve( lua_State *L )
{
   LuaLinOpt_t *lp = luaL_checklinopt(L,1);
   double z;
   int ret, ismip;
   glp_iocp parm_iocp;
   glp_smcp parm_smcp;
#if DEBUGGING
   Uint64 starttime = SDL_GetTicks64();
#endif /* DEBUGGING */

   /* Parameters. */
   ismip = (glp_get_num_int( lp->prob ) > 0);
   glp_init_smcp(&parm_smcp);
   parm_smcp.msg_lev = GLP_MSG_ERR;
   parm_smcp.tm_lim = LINOPT_MAX_TM;
   if (ismip) {
      glp_init_iocp(&parm_iocp);
      parm_iocp.msg_lev  = GLP_MSG_ERR;
      parm_iocp.tm_lim = LINOPT_MAX_TM;
   }

   /* Load parameters. */
   if (!lua_isnoneornil(L,2)) {
      GETOPT_SMCP( meth,    opt_meth,    METH_DEF );
      GETOPT_SMCP( pricing, opt_pricing, PRICING_DEF );
      GETOPT_SMCP( r_test,  opt_r_test,  R_TEST_DEF );
      GETOPT_SMCP( presolve,opt_onoff,   PRESOLVE_DEF );
      if (ismip) {
         GETOPT_IOCP( br_tech,  opt_br_tech, BR_TECH_DEF );
         GETOPT_IOCP( bt_tech,  opt_bt_tech, BT_TECH_DEF );
         GETOPT_IOCP( pp_tech,  opt_pp_tech, PP_TECH_DEF );
         GETOPT_IOCP( sr_heur,  opt_onoff,   SR_HEUR_DEF );
         GETOPT_IOCP( fp_heur,  opt_onoff,   FP_HEUR_DEF );
         GETOPT_IOCP( ps_heur,  opt_onoff,   PS_HEUR_DEF );
         GETOPT_IOCP( gmi_cuts, opt_onoff,   GMI_CUTS_DEF );
         GETOPT_IOCP( mir_cuts, opt_onoff,   MIR_CUTS_DEF );
         GETOPT_IOCP( cov_cuts, opt_onoff,   COV_CUTS_DEF );
         GETOPT_IOCP( clq_cuts, opt_onoff,   CLQ_CUTS_DEF );
      }
   }
#if 0
   else {
      parm_smcp.meth    = METH_DEF;
      parm_smcp.pricing = PRICING_DEF;
      parm_smcp.r_test  = R_TEST_DEF;
      parm_smcp.presolve= PRESOLVE_DEF;
      if (ismip) {
         parm_iocp.br_tech  = BR_TECH_DEF;
         parm_iocp.bt_tech  = BT_TECH_DEF;
         parm_iocp.pp_tech  = PP_TECH_DEF;
         parm_iocp.sr_heur  = SR_HEUR_DEF;
         parm_iocp.fp_heur  = FP_HEUR_DEF;
         parm_iocp.ps_heur  = PS_HEUR_DEF;
         parm_iocp.gmi_cuts = GMI_CUTS_DEF;
         parm_iocp.mir_cuts = MIR_CUTS_DEF;
         parm_iocp.cov_cuts = COV_CUTS_DEF;
         parm_iocp.clq_cuts = CLQ_CUTS_DEF;
      }
   }
#endif

   /* Optimization. */
   if (!ismip || !parm_iocp.presolve) {
      ret = glp_simplex( lp->prob, &parm_smcp );
      if ((ret != 0) && (ret != GLP_ETMLIM)) {
         lua_pushnil(L);
         lua_pushstring(L, linopt_error(ret));
         return 2;
      }
      /* Check for optimality of continuous problem. */
      ret = glp_get_status(lp->prob);
      if ((ret != GLP_OPT) && (ret != GLP_FEAS)) {
         lua_pushnil(L);
         lua_pushstring(L, linopt_status(ret));
         return 2;
      }
   }
   if (ismip) {
      ret = glp_intopt( lp->prob, &parm_iocp );
      if ((ret != 0) && (ret != GLP_ETMLIM)) {
         lua_pushnil(L);
         lua_pushstring(L, linopt_error(ret));
         return 2;
      }
      /* Check for optimality of discrete problem. */
      ret = glp_mip_status(lp->prob);
      if ((ret != GLP_OPT) && (ret != GLP_FEAS)) {
         lua_pushnil(L);
         lua_pushstring(L, linopt_status(ret));
         return 2;
      }
   }
   z = glp_get_obj_val( lp->prob );

   /* Output function value. */
   lua_pushnumber(L,z);

   /* Go over variables and store them. */
   lua_newtable(L); /* t */
   for (int i=1; i<=lp->ncols; i++) {
      if (ismip)
         z = glp_mip_col_val( lp->prob, i );
      else
         z = glp_get_col_prim( lp->prob, i );
      lua_pushnumber( L, z ); /* t, z */
      lua_rawseti( L, -2, i ); /* t */
   }

   /* Go over constraints and store them. */
   lua_newtable(L); /* t */
   for (int i=1; i<=lp->nrows; i++) {
      if (ismip)
         z = glp_mip_row_val( lp->prob, i );
      else
         z = glp_get_row_prim( lp->prob, i );
      lua_pushnumber( L, z ); /* t, z */
      lua_rawseti( L, -2, i ); /* t */
   }

   /* Complain about time. */
#if DEBUGGING
   if (SDL_GetTicks64() - starttime > LINOPT_MAX_TM)
      WARN(_("glpk: too over 1 second to optimize!"));
#endif /* DEBUGGING */

   return 3;
}
#undef GETOPT_IOCP

/**
 * @brief Reads an optimization problem from a file for debugging purposes.
 *
 *    @luatparam string fname Path to the file.
 *    @luatparam[opt=false] boolean glpk_format Whether the program is in GLPK format instead of MPS format.
 *    @luatparam[opt=false] boolean maximize Whether to maximize (rather than minimize) the function.
 *    @luatreturn LinOpt Linear program in the file.
 * @luafunc read_problem
 */
static int linoptL_readProblem( lua_State *L )
{
   const char *fname = luaL_checkstring(L,1);
   int glpk_format   = lua_toboolean(L,2);
   int maximize      = lua_toboolean(L,3);
   const char *dirname = PHYSFS_getRealDir( fname );
   char *fpath;
   int ret;
   LuaLinOpt_t lp;
   if (dirname == NULL)
      NLUA_ERROR( L, _("Failed to read LP problem \"%s\"!"), fname );
   SDL_asprintf( &fpath, "%s/%s", dirname, fname );
   lp.prob = glp_create_prob();
   ret = glpk_format ? glp_read_prob( lp.prob, 0, fpath ) : glp_read_mps(  lp.prob, GLP_MPS_FILE, NULL, fpath );
   free( fpath );
   if (ret != 0) {
      glp_delete_prob( lp.prob );
      NLUA_ERROR( L, _("Failed to read LP problem \"%s\"!"), fname );
   }
   lp.ncols = glp_get_num_cols( lp.prob );
   lp.nrows = glp_get_num_rows( lp.prob );
   if (maximize)
      glp_set_obj_dir( lp.prob, GLP_MAX );
   lua_pushlinopt( L, lp );
   return 1;
}

/**
 * @brief Writes an optimization problem to a file for debugging purposes.
 *
 *    @luatparam LinOpt lp Linear program to write.
 *    @luatparam string fname Path to write the program to.
 *    @luatparam[opt=false] boolean glpk_format Whether to write the program in GLPK format instead of MPS format.
 * @luafunc write_problem
 */
static int linoptL_writeProblem( lua_State *L )
{
   LuaLinOpt_t *lp   = luaL_checklinopt(L,1);
   const char *fname = luaL_checkstring(L,2);
   int glpk_format   = lua_toboolean(L,3);
   const char *dirname = PHYSFS_getWriteDir();
   char *fpath;
   int ret;
   SDL_asprintf( &fpath, "%s/%s", dirname, fname );
   ret = glpk_format ? glp_write_prob( lp->prob, 0, fpath ) : glp_write_mps(  lp->prob, GLP_MPS_FILE, NULL, fpath );
   free( fpath );
   lua_pushboolean( L, ret==0 );
   return 1;
}
