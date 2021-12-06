/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file lvar.c
 *
 * @brief Lua Variables
 */
#include "lvar.h"

#include "array.h"

/*
 * prototypes
 */
static int lvar_cmp( const void *p1, const void *p2 );
static void lvar_free( lvar *var );

static int lvar_cmp( const void *p1, const void *p2 )
{
   const lvar *mv1, *mv2;
   mv1 = (const lvar*) p1;
   mv2 = (const lvar*) p2;
   return strcmp(mv1->name,mv2->name);
}

/**
 * @brief Gets a lua var by name.
 */
lvar *lvar_get( lvar *arr, const char *str )
{
   lvar mv = {.name=(char*)str};
   if (arr == NULL)
      return NULL;
   return bsearch( &mv, arr, array_size(arr), sizeof(lvar), lvar_cmp );
}

/**
 * @brief Frees a mission variable.
 *
 *    @param var Mission variable to free.
 */
static void lvar_free( lvar *var )
{
   switch (var->type) {
      case LVAR_STR:
         free(var->d.str);
         var->d.str = NULL;
         break;
      case LVAR_NIL:
      case LVAR_NUM:
      case LVAR_BOOL:
      case LVAR_TIME:
         break;
   }
   free(var->name);
   var->name = NULL;
}

void lvar_freeArray( lvar *arr )
{
   for (int i=0; i<array_size(arr); i++)
      lvar_free( &arr[i] );
   array_free( arr );
}

int lvar_addArray( lvar **arr, lvar *new_var, int sort )
{
   /* Avoid Duplicates. */
   lvar *mv = lvar_get( *arr, new_var->name );
   if (mv != NULL) {
      lvar_free( mv );
      *mv = *new_var;
      return 0;
   }

   /* need new one. */
   mv = &array_grow( arr );
   *mv = *new_var;

   /* Sort if necessary. */
   if (sort)
      qsort( *arr, array_size(*arr), sizeof(lvar), lvar_cmp );

   return 0;
}

void lvar_rmArray( lvar **arr, lvar *rm_var )
{
   lvar_free( rm_var );
   array_erase( arr, rm_var, rm_var+1 );
}

/**
 * @brief Saves the mission variables.
 *
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int lvar_save( const lvar *arr, xmlTextWriterPtr writer )
{
   xmlw_startElem(writer,"vars");

   for (int i=0; i<array_size(arr); i++) {
      const lvar *v = &arr[i];
      xmlw_startElem(writer,"var");

      xmlw_attr(writer,"name","%s",v->name);

      switch (v->type) {
         case LVAR_NIL:
            xmlw_attr(writer,"type","nil");
            break;
         case LVAR_NUM:
            xmlw_attr(writer,"type","num");
            xmlw_str(writer,"%f",v->d.num);
            break;
         case LVAR_BOOL:
            xmlw_attr(writer,"type","bool");
            xmlw_str(writer,"%d",v->d.b);
            break;
         case LVAR_STR:
            xmlw_attr(writer,"type","str");
            xmlw_str(writer,"%s",v->d.str);
            break;
         case LVAR_TIME:
            xmlw_attr(writer,"type","time");
            xmlw_str(writer,"%"TIME_PRI,v->d.time);
            break;
      }
      xmlw_endElem(writer); /* "var" */
   }
   xmlw_endElem(writer); /* "vars" */

   return 0;
}

/**
 * @brief Loads the vars from XML file.
 *
 *    @param parent Parent node containing the variables.
 *    @return 0 on success.
 */
lvar *lvar_load( xmlNodePtr parent )
{
   lvar *arr = array_create( lvar );
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"vars")) {
         xmlNodePtr cur = node->xmlChildrenNode;

         do {
            xml_onlyNodes(cur);
            if (!xml_isNode(cur,"var")) {
               WARN(_("Lua Var stack has unknown node '%s'!"), xml_get(cur));
               continue;
            }
            lvar var;
            char *str;
            xmlr_attr_strd(cur,"name",var.name);
            xmlr_attr_strd(cur,"type",str);
            if (strcmp(str,"nil")==0)
               var.type = LVAR_NIL;
            else if (strcmp(str,"num")==0) {
               var.type = LVAR_NUM;
               var.d.num = xml_getFloat(cur);
            }
            else if (strcmp(str,"bool")==0) {
               var.type = LVAR_BOOL;
               var.d.b = xml_getInt(cur);
            }
            else if (strcmp(str,"str")==0) {
               var.type = LVAR_STR;
               var.d.str = xml_getStrd(cur);
            }
            else if (strcmp(str,"time")==0) {
               var.type = LVAR_TIME;
               var.d.time = xml_getLong(cur);
            }
            else { /* super error checking */
               WARN(_("Unknown var type '%s'"), str);
               free(var.name);
               free(str);
               continue;
            }
            free(str);
            lvar_addArray( &arr, &var, 0 );
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));
   qsort( arr, array_size(arr), sizeof(lvar), lvar_cmp );

   return arr;
}
