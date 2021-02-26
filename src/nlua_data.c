/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_data.c
 *
 * @brief Handles datas.
 */

/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_data.h"

#include "log.h"
#include "nluadef.h"


/* Helper functions. */
static size_t dataL_checkpos( lua_State *L, LuaData_t *ld, long pos );


/* Data metatable methods. */
static int dataL_gc( lua_State *L );
static int dataL_eq( lua_State *L );
static int dataL_new( lua_State *L );
static int dataL_get( lua_State *L );
static int dataL_set( lua_State *L );
static int dataL_getSize( lua_State *L );
static int dataL_getString( lua_State *L );
static int dataL_paste( lua_State *L );
static int dataL_addWeighted( lua_State *L );
static int dataL_convolve2d( lua_State *L );
static const luaL_Reg dataL_methods[] = {
   { "__gc", dataL_gc },
   { "__eq", dataL_eq },
   { "new", dataL_new },
   { "get", dataL_get },
   { "set", dataL_set },
   { "getSize", dataL_getSize },
   { "getString", dataL_getString },
   { "paste", dataL_paste },
   { "addWeighted", dataL_addWeighted },
   { "convolve2d", dataL_convolve2d },
   {0,0}
}; /**< Data metatable methods. */




/**
 * @brief Loads the data library.
 *
 *    @param env Environment to load data library into.
 *    @return 0 on success.
 */
int nlua_loadData( nlua_env env )
{
   nlua_register(env, DATA_METATABLE, dataL_methods, 1);
   return 0;
}


/**
 * @brief Lua bindings to interact with datas.
 *
 * @luamod data
 */
/**
 * @brief Gets data at index.
 *
 *    @param L Lua state to get data from.
 *    @param ind Index position to find the data.
 *    @return Data found at the index in the state.
 */
LuaData_t* lua_todata( lua_State *L, int ind )
{
   return (LuaData_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets data at index or raises error if there is no data at index.
 *
 *    @param L Lua state to get data from.
 *    @param ind Index position to find data.
 *    @return Data found at the index in the state.
 */
LuaData_t* luaL_checkdata( lua_State *L, int ind )
{
   if (lua_isdata(L,ind))
      return lua_todata(L,ind);
   luaL_typerror(L, ind, DATA_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a data on the stack.
 *
 *    @param L Lua state to push data into.
 *    @param data Data to push.
 *    @return Newly pushed data.
 */
LuaData_t* lua_pushdata( lua_State *L, LuaData_t data )
{
   LuaData_t *c;
   c = (LuaData_t*) lua_newuserdata(L, sizeof(LuaData_t));
   *c = data;
   luaL_getmetatable(L, DATA_METATABLE);
   lua_setmetatable(L, -2);
   return c;
}
/**
 * @brief Checks to see if ind is a data.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a data.
 */
int lua_isdata( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, DATA_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Frees a data.
 *
 *    @luatparam Data data Data to free.
 * @luafunc __gc
 */
static int dataL_gc( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   free(ld->data);
   return 0;
}


/**
 * @brief Compares two datas to see if they are the same.
 *
 *    @luatparam Data d1 Data 1 to compare.
 *    @luatparam Data d2 Data 2 to compare.
 *    @luatreturn boolean true if both datas are the same.
 * @luafunc __eq
 */
static int dataL_eq( lua_State *L )
{
   LuaData_t *d1, *d2;
   d1 = luaL_checkdata(L,1);
   d2 = luaL_checkdata(L,2);
   if (d1->size != d2->size) {
      lua_pushboolean( L, 0 );
      return 1;
   }
   lua_pushboolean( L, (memcmp( d1->data, d2->data, d1->size)==0) );
   return 1;
}


/**
 * @brief Opens a new data.
 *
 *    @luatparam number size Size to allocate for data.
 *    @luatparam string type Type of the data to create ("number")
 *    @luatreturn Data New data object.
 * @luafunc new
 */
static int dataL_new( lua_State *L )
{
   LuaData_t ld;
   size_t size = luaL_checklong(L,1);
   const char *type = luaL_checkstring(L,2);
   NLUA_CHECKRW(L);
   if (strcmp(type,"number")==0) {
      ld.type = LUADATA_NUMBER;
      ld.elem = sizeof(float);
   }
   else
      NLUA_ERROR(L, _("unknown data type '%s'"), type);
   ld.size = size*ld.elem;
   ld.data = calloc( ld.elem, size );
   lua_pushdata( L, ld );
   return 1;
}



static size_t dataL_checkpos( lua_State *L, LuaData_t *ld, long pos )
{
   size_t mpos;
   if (pos < 0)
      NLUA_ERROR(L, _("position argument must be positive!"));
   mpos = pos * ld->elem;
   if (mpos >= ld->size)
      NLUA_ERROR(L, _("position argument out of bounds: %d of %d elements"), pos, ld->size/ld->elem);
   return mpos;
}



/**
 * @brief Gets the value of an element.
 *
 * @luafunc get
 */
static int dataL_get( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   long pos = luaL_checklong(L,2);
   size_t mpos = dataL_checkpos( L, ld, pos );
   switch (ld->type) {
      case LUADATA_NUMBER:
         lua_pushnumber(L, *((float*)(&ld->data[mpos])));
         break;
   }
   return 1;
}


/**
 * @brief Sets the value of an element.
 *
 * @luafunc get
 */
static int dataL_set( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   long pos = luaL_checklong(L,2);
   size_t mpos = dataL_checkpos( L, ld, pos );
   double value;
   switch (ld->type) {
      case LUADATA_NUMBER:
         value = luaL_checknumber(L,3);
         *((float*)(&ld->data[mpos])) = value;
         break;
   }
   return 0;
}


/**
 * @luafunc getSize
 */
static int dataL_getSize( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   lua_pushnumber(L, ld->size);
   return 1;
}


/**
 * @luafunc getString
 */
static int dataL_getString( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   lua_pushlstring(L, ld->data, ld->size);
   return 1;
}


/**
 * @luafunc paste
 */
static int dataL_paste( lua_State *L )
{
   LuaData_t *dest = luaL_checkdata(L,1);
   LuaData_t *source = luaL_checkdata(L,2);
   long dx = luaL_checklong(L,3) * dest->elem;
   long sx = luaL_checklong(L,4) * source->elem;
   long sw = luaL_checklong(L,5) * source->elem;

   /* Check fits. */
   if (dx+sw > (long)dest->size)
      NLUA_ERROR(L, _("size mismatch: out of bound access dest: %d of %d elements"), dx+sw, dest->size);
   else if (sx+sw > (long)source->size)
      NLUA_ERROR(L, _("size mismatch: out of bound access of source: %d of %d elements"), sx+sw, source->size);

   /* Copy memory over. */
   memcpy( &dest->data[dx], &source->data[sx], sw );

   /* Return destination. */
   lua_pushvalue(L,1);
   return 1;
}


/**
 * @luafunc addWeighted
 */
static int dataL_addWeighted( lua_State *L )
{
   LuaData_t *A = luaL_checkdata(L,1);
   LuaData_t *B = luaL_checkdata(L,2);
   LuaData_t out;
   double alpha = luaL_checknumber(L,3);
   double beta = luaL_optnumber(L,4,1.-alpha);
   double bias = luaL_optnumber(L,5,0.);
   int i, n;
   float *o, *a, *b;

   /* Checks. */
   if (A->size != B->size)
      NLUA_ERROR(L, _("size mismatch: A has %d elements but B has %d elements"), A->size, B->size );
   if (A->type != LUADATA_NUMBER || B->type != LUADATA_NUMBER)
      NLUA_ERROR(L, _("%s is only implemented for number types"), __func__);

   /* Create new data. */
   out.size = A->size;
   out.elem = A->elem;
   out.type = A->type;
   out.data = malloc( out.size );

   /* Interpolate. */
   n = out.size / out.elem;
   a = (float*)A->data;
   b = (float*)B->data;
   o = (float*)out.data;
   for (i=0; i<n; i++)
      o[i] = a[i]*alpha + b[i]*beta + bias;

   /* Return new data. */
   lua_pushdata(L,out);
   return 1;
}


/**
 * @luafunc convolve2d
 */
static int dataL_convolve2d( lua_State *L )
{
   LuaData_t *lI = luaL_checkdata(L,1);
   long iw = luaL_checklong(L,2);
   long ih = luaL_checklong(L,3);
   LuaData_t *lK = luaL_checkdata(L,4);
   long kw = luaL_checklong(L,5);
   long kh = luaL_checklong(L,6);
   LuaData_t out;
   int p, u,v, ku,kv, bu,bv;
   int kw2,kh2, bw,bh, ow,oh;
   float *I = (float*)lI->data;
   float *K = (float*)lK->data;
   float *B, *O;

   /* Checks. */
   if (iw*ih*4*lI->elem != lI->size)
      NLUA_ERROR(L,_("size mismatch for data: got %dx%dx4x%d, expected %d"), iw, ih, lI->elem, lI->size);
   if (kw*kh*4*lK->elem != lK->size)
      NLUA_ERROR(L,_("size mismatch for data: got %dx%dx4x%d, expected %d"), kw, kh, lK->elem, lK->size);
   if (lI->type != LUADATA_NUMBER || lK->type != LUADATA_NUMBER)
      NLUA_ERROR(L, _("%s is only implemented for number types"), __func__);

   /* Set up. */
   kw2 = (kw-1)/2;
   kh2 = (kh-1)/2;

   /* Create new data. */
   ow = iw+kw2;
   oh = ih+kw2;
   out.elem = lI->elem;
   out.type = lI->type;
   out.size = ow*oh*4*out.elem;
   out.data = calloc( out.size, 1 );
   O = (float*)out.data;

#define POS(U,V,W)   (4*((V)*(W)+(U)))
   /* Create buffer. */
   bw = ow+2*kw2;
   bh = oh+2*kh2;
   B = calloc( bw*bh*4, sizeof(float) );
   for (v=0; v<ih; v++)
      memcpy( &B[ POS(kw2, v+kh2, bw) ],
              &I[ POS(  0,     v, iw) ],
              4*sizeof(float)*iw );

   /* Convolve. */
   for (v=0; v<oh; v++) {
      for (u=0; u<ow; u++) {
         for (kv=0; kv<kh; kv++) {
            for (ku=0; ku<kw; ku++) {
               bu = u + ku;
               bv = v + kv;
               for (p=0; p<4; p++)
                  O[ POS( u, v, ow )+p ] +=
                        B[ POS( bu, bv, bw )+p ]
                        * K[ POS( ku, kv, kw )+p ];
            }
         }
      }
   }
#undef POS

   /* Cleanup. */
   free(B);

   /* Return new data. */
   lua_pushdata(L,out);
   lua_pushinteger(L,ow);
   lua_pushinteger(L,oh);
   return 3;
}



