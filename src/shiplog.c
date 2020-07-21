#include "shiplog.h"
/* Limit the journey log to 6 entries */

static ShipLog *shipLog=NULL;
ShipLogEntry *shiplog_removeEntry(ShipLogEntry *e);

/*
 * @brief Creates a new log with given title of given type.
 *
 *    @param idstr ID string for this logset, or NULL if an ID string not required.
 *    @param logname Name of the log (title)
 *    @param type Type of the log, e.g. travel, shipping, etc
 *    @param overwrite Whether to overwrite an existing log of this type and logname (if 1), or all logs of this type (if 2).
 *    @return log ID.
 */

int shiplog_create(const char *idstr, const char *logname, const char *type, const int overwrite)
{
   ShipLogEntry *e;
   int i, id, indx;
   if ( shipLog == NULL ) {
      shipLog = calloc( sizeof(ShipLog), 1);
   }
   indx = shipLog->nlogs;
   
   if ( overwrite == 1 ) {
      /* check to see whether this idstr or logname and type has been created before, and if so, remove all entries of that logid */
      if ( idstr != NULL ){
         /* find the matching logid for this idstr */
         for ( i=0; i<shipLog->nlogs; i++ ){
            if ( shipLog->idstrList[i]!=NULL && !strcmp( shipLog->idstrList[i], idstr ) ){
               /* matching idstr found. */
               id = shipLog->idList[i];
               indx = i;
               break;
            }
         }
      }else{
         for ( i=0; i<shipLog->nlogs; i++ ) {
            if ( !strcmp ( type, shipLog->typeList[i] ) && !strcmp ( logname, shipLog->nameList[i] ) ) {
               id = shipLog->idList[i];
               indx = i;
               break;
            }
         }
      }
      if ( i < shipLog->nlogs ) { /* prev id found - so remove all log entries of this type. */
         e = shipLog->head;
         while ( e != NULL ) {
            if ( e->id == id ){
               /* remove this entry */
               e = shiplog_removeEntry( e );
            }else{
               e = (ShipLogEntry*)e->next;
            }
         }
      }
   } else if( overwrite == 2 ) {
      /* check to see whether this type has been created before, and if so, remove all entries. */
      int found = 0;
      id = -1;
      for ( i=0; i<shipLog->nlogs; i++ ) {
         if ( (idstr!=NULL && shipLog->idstrList[i]!=NULL && !strcmp( idstr, shipLog->idstrList[i] ) ) || ( idstr==NULL && !strcmp( type, shipLog->typeList[i] )) ) {
            e = shipLog->head;
            while ( e != NULL ){
               if ( e->id == shipLog->idList[i] ) {
                  e = shiplog_removeEntry( e );
               } else {
                  e = e->next;
               }
            }
            if ( found == 0 ) { /* This is the first entry of this type */
               found = 1;
               id = shipLog->idList[i];
               indx = i;
               free(shipLog->nameList[i]);
               shipLog->nameList[i]=strdup(logname);
            } else { /* a previous entry of this type as been found, so just invalidate this logid. */
               shipLog->idList[i] = -1;
               if ( shipLog->idstrList[i]!=NULL){
                  free( shipLog->idstrList[i] );
                  shipLog->idstrList[i]=NULL;
               }
            }
         }
      }

   }
   if ( indx == shipLog->nlogs ){
      /* create a new id for this log */
      id = -1;
      for ( i=0; i<shipLog->nlogs; i++ ){/* get maximum id */
         if ( shipLog->idList[i] > id )
            id = shipLog->idList[i];
      }
      id++;
      shipLog->nlogs++;
      shipLog->idList = realloc(shipLog->idList, sizeof(int) * shipLog->nlogs);
      shipLog->nameList = realloc(shipLog->nameList, sizeof(char*) * shipLog->nlogs);
      shipLog->typeList = realloc(shipLog->typeList, sizeof(char*) * shipLog->nlogs);
      shipLog->removeAfter = realloc(shipLog->removeAfter, sizeof(ntime_t) * shipLog->nlogs);
      shipLog->idstrList = realloc(shipLog->idstrList, sizeof(char*) * shipLog->nlogs);
      
      shipLog->removeAfter[indx] = 0;
      shipLog->idList[indx] = id;
      shipLog->nameList[indx] = strdup(logname);
      shipLog->typeList[indx] = strdup(type);
      if ( idstr == NULL )
         shipLog->idstrList[indx] = NULL;
      else
         shipLog->idstrList[indx] = strdup(idstr);
            
   }
   return id;
}

/*
 * @brief Adds to the log file
 *
 *    @param logid of the log to add to.
 *    @param msg Message to be added.
 *    @return 0 on success, -1 on failure.
 */
int shiplog_append(const int logid,const char *msg)
{
   ShipLogEntry *e;
   ntime_t now = ntime_get();

   if (shipLog == NULL)
      shiplog_new();

   if ( logid < 0 )
      return -1;

   /* Check that the log hasn't already been added (e.g. if reloading) */
   e = shipLog->head;
   /* check for identical logs */
   while ( e != NULL ) {
      if ( e->time != now ){ /* logs are created in chronological order */
         break;
      }
      if ( logid == e->id && !strcmp(e->msg,msg) ) {
         /* Identical log already exists */
         return 0;
      }
      e = e->next;
   }
   if ( (e = calloc(sizeof(ShipLogEntry),1)) == NULL ) {
      WARN(_("Error creating new log entry - crash imminent!\n"));
      return -1;
   }
   e->next = shipLog->head;
   shipLog->head = e;
   if ( shipLog->tail == NULL ) /* first entry - point to both head and tail.*/
      shipLog->tail = e;
   if ( e->next != NULL )
      ((ShipLogEntry*)e->next)->prev = (void*)e;
   e->id = logid;
   e->msg = strdup(msg);
   e->time = now;
   return 0;
}

/*
 * @brief Deletes a log (e.g. a cancelled mission may wish to do this, or the user might).
 *
 * @param logid of the log to remove
 */
void shiplog_delete(const int logid)
{
   ShipLogEntry *e, *tmp;
   int i;

   if(shipLog == NULL)
      return;

   if ( logid < 0 )
      return;

   e = shipLog->head;
   while ( e != NULL ){
      if ( e->id == logid ){
         if ( e->prev != NULL )
            ((ShipLogEntry*)e->prev)->next = e->next;
         if ( e->next != NULL )
            ((ShipLogEntry*)e->next)->prev = e->prev;
         free( e->msg );
         if ( e == shipLog->head )
            shipLog->head = e->next;
         if ( e == shipLog->tail )
            shipLog->tail = e->prev;
         tmp = e;
         e = (ShipLogEntry*) e->next;
         free( tmp );
      }else
         e = (ShipLogEntry*)e->next;
   }

   for ( i=0; i<shipLog->nlogs; i++) {
      if ( shipLog->idList[i] == logid ) {
         shipLog->idList[i] = -1;
         free(shipLog->nameList[i]);
         shipLog->nameList[i] = NULL;
         free(shipLog->typeList[i]);
         shipLog->typeList[i] = NULL;
         if ( shipLog->idstrList[i] != NULL ){
            free ( shipLog->idstrList[i] );
            shipLog->idstrList[i] = NULL;
         }
               
      }
   }
}

/*
 * @brief Sets the remove flag for a log - it will be removed once time increases, eg after a player takes off.
 *
 * @param logid the ID of the log
 * @param when the time at which to remove.  If 0, uses current time, if <0, adds abs to current time, if >0, uses as the time to remove.
 * Rationale: Allows a player to review the log while still landed, and then clears it up once takes off.
 */
void shiplog_setRemove(const int logid, ntime_t when)
{
   int i;
   if ( when == 0 )
      when = ntime_get();
   else if ( when < 0 ) /* add this to ntime */
      when = ntime_get() - when;

   for ( i=0; i<shipLog->nlogs; i++ ) {
      if ( shipLog->idList[i] == logid ) {
         shipLog->removeAfter[i] = when;
         break;
      }
   }
}

/*
 * @brief Deletes all logs of given type.
 *
 * @param type of the log to remove
 */
void shiplog_deleteType(const char *type)
{
   int i;
   if (shipLog == NULL || type == NULL)
      return;
   for ( i=0; i<shipLog->nlogs; i++ ){
      if ( shipLog->idList[i]>=0 && !strcmp( type, shipLog->typeList[i] ) ) {
         shiplog_delete( shipLog->idList[i] );
      }
   }
}


/*
 * @brief Clear the shiplog
 */
void shiplog_clear(void)
{
   ShipLogEntry *e, *tmp;
   if ( shipLog == NULL ) {
      shipLog = calloc( sizeof(ShipLog), 1);
   }
   e=shipLog->head;
   while ( e != NULL ) {
      free( e->msg );
      tmp = e;
      e = e->next;
      free ( tmp );
   }
   if ( shipLog->idList )
      free ( shipLog->idList );
   if ( shipLog->nameList )
      free ( shipLog->nameList );
   if ( shipLog->typeList )
      free ( shipLog->typeList );
   if ( shipLog->idstrList )
      free ( shipLog->idstrList );
   memset(shipLog, 0, sizeof(ShipLog));
}

/*
 * @brief Set up the shiplog
 */
void shiplog_new(void)
{
   shiplog_clear();
}

/*
 * @brief Saves the logfiile
 */
int shiplog_save( xmlTextWriterPtr writer ){
   int i;
   ShipLogEntry *e;
   ntime_t t = ntime_get();
   xmlw_startElem(writer,"shiplog");

   for ( i=0; i<shipLog->nlogs; i++ ){
      if ( shipLog->removeAfter[i]>0 && shipLog->removeAfter[i]<t )
         shiplog_delete( shipLog->idList[i] );
      if ( shipLog->idList[i] >= 0 ){
         xmlw_startElem(writer, "entry");
         xmlw_attr(writer,"id","%d",shipLog->idList[i]);
         xmlw_attr(writer,"t","%s",shipLog->typeList[i]);
         if( shipLog->removeAfter[i]!=0 )
            xmlw_attr(writer,"r","%"PRIu64,shipLog->removeAfter[i]);
         if ( shipLog->idstrList[i] != NULL )
            xmlw_attr(writer,"s","%s",shipLog->idstrList[i]);
         xmlw_str(writer,"%s",shipLog->nameList[i]);
         xmlw_endElem(writer);/* entry */
      }
   }
   e=shipLog->head;
   while ( e != NULL ){
      if ( e->id >= 0 ){
         xmlw_startElem(writer, "log");
         xmlw_attr(writer,"id","%d",e->id);
         xmlw_attr(writer,"t","%"PRIu64,e->time);
         xmlw_str(writer,"%s",e->msg);
         xmlw_endElem(writer);/* log */
      }
      e=(ShipLogEntry*)e->next;
   }
   xmlw_endElem(writer); /* economy */
   return 0;
}

/*
 * @brief Loads the logfiile
 * @param parent Parent node for economy.
 * @return 0 on success.
 */
int shiplog_load( xmlNodePtr parent ){
   xmlNodePtr node, cur;
   ShipLogEntry *e;
   char *str;
   int id,i;
   shiplog_clear();
   
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"shiplog")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur, "entry")) {
               xmlr_attr(cur, "id", str);
               if (str) {
                  id = atoi(str);
                  free(str);
               } else {
                  id = 0;
                  WARN(_("Warning - no ID in shipLog entry\n"));
               }
               /* check this ID isn't already present */
               for ( i=0; i<shipLog->nlogs; i++ ) {
                  if ( shipLog->idList[i] == id )
                     break;
               }
               if ( i==shipLog->nlogs ) { /* a new ID */
                  shipLog->nlogs++;
                  shipLog->idList = realloc( shipLog->idList, sizeof(int) * shipLog->nlogs);
                  shipLog->nameList = realloc( shipLog->nameList, sizeof(char*) * shipLog->nlogs);
                  shipLog->typeList = realloc( shipLog->typeList, sizeof(char*) * shipLog->nlogs);
                  shipLog->removeAfter = realloc( shipLog->removeAfter, sizeof(ntime_t) * shipLog->nlogs);
                  shipLog->idstrList = realloc( shipLog->idstrList, sizeof(char*) * shipLog->nlogs);
                  shipLog->idList[shipLog->nlogs-1] = id;
                  shipLog->removeAfter[shipLog->nlogs-1] = 0;
                  shipLog->idstrList[shipLog->nlogs-1] = NULL;
                  xmlr_attr(cur, "t", str);
                  if (str) {
                     shipLog->typeList[shipLog->nlogs-1] = str;
                  } else {
                     shipLog->typeList[shipLog->nlogs-1] = strdup("No type");
                     WARN(_("No ID in shipLog entry"));
                  }
                  xmlr_attr(cur, "r", str);
                  if (str) {
                     shipLog->removeAfter[shipLog->nlogs-1] = atol(str);
                     free(str);
                  }
                  xmlr_attr(cur, "s", str);
                  if (str) {
                     shipLog->idstrList[shipLog->nlogs-1] = str;
                  }
                  shipLog->nameList[shipLog->nlogs-1] = strdup(xml_raw(cur));
               }
            } else if (xml_isNode(cur, "log")) {
               e = calloc( sizeof(ShipLogEntry), 1);
               /* put this one at the end */
               e->prev = shipLog->tail;
               if ( shipLog->tail == NULL )
                  shipLog->head = e;
               else
                  shipLog->tail->next = e;
               shipLog->tail = e;
               
               xmlr_attr(cur, "id", str);
               if (str) {
                  e->id = atoi(str);
                  free(str);
               } else {
                  WARN(_("Warning - no ID for shipLog entry"));
               }
               xmlr_attr(cur, "t", str);
               if (str) {
                  e->time = atol(str);
                  free(str);
               } else {
                  WARN(_("Warning - no time for shipLog entry"));
               }
               e->msg = strdup(xml_raw(cur));
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));
   return 0;
}

void shiplog_listTypes( int *ntypes, char ***logTypes, int includeAll )
{
   int i, j, n=0;
   char **types = NULL;
   ntime_t t = ntime_get();

   if ( includeAll ) {
      types = malloc(sizeof(char**));
      n = 1;
      types[0] = strdup("All");
   }
   for ( i=0; i<shipLog->nlogs; i++ ) {
      if ( shipLog->removeAfter[i] > 0 && shipLog->removeAfter[i]<t ) {
         /* log expired, so remove (which sets id to -1) */
         shiplog_delete(shipLog->idList[i]);
      }
      if ( shipLog->idList[i] >= 0 ) {
         /* log valid */
         for ( j=0; j<n; j++ ) {
            if ( !strcmp ( shipLog->typeList[i], types[j] ))
               break;
         }
         if ( j==n ) {/*This log type not found, so add.*/
            n++;
            types = realloc(types, sizeof(char**) * n);
            types[n-1] = strdup(shipLog->typeList[i]);
         }
      }
   }
   *ntypes=n;
   *logTypes=types;
}

void shiplog_listLogsOfType( char *type, int *nlogs, char ***logsOut, int **logIDs, int includeAll )
{
   int i,n=0,all=0;
   char **logs=NULL;
   int *logid=NULL;
   ntime_t t = ntime_get();
   if( includeAll ){
      logs = malloc(sizeof(char**));
      n=1;
      logs[0] = strdup( "All" );
      logid = malloc(sizeof(int*));
      logid[0] = -1;
   }
   if ( !strcmp(type,"All") ){
      /*Match all types*/
      all=1;
   }
   if( shipLog->nlogs > 0 ){
      for ( i=shipLog->nlogs-1; i>=0; i-- ){
         if ( shipLog->removeAfter[i] > 0 && shipLog->removeAfter[i]<t ){
            /* log expired, so remove (which sets id to -1) */
            shiplog_delete(shipLog->idList[i]);
         }
         if ( shipLog->idList[i]>=0 && (all==1 || !strcmp(type, shipLog->typeList[i] ))){
            n++;
            logs=realloc(logs,sizeof(char**)*n);
            logs[n-1] = strdup(shipLog->nameList[i]);
            logid=realloc(logid,sizeof(int*)*n);
            logid[n-1] = shipLog->idList[i];
         }
      }
   }
   *nlogs = n;
   *logsOut = logs;
   *logIDs = logid;
}

int shiplog_getIdOfLogOfType( const char *type, int selectedLog )
{
   int i,all=0,n=0;
   ntime_t t = ntime_get();
   if ( !strcmp(type,"All") ){
      /*Match all types*/
      all=1;
   }

   for ( i=shipLog->nlogs-1; i>=0; i-- ){
      if ( shipLog->removeAfter[i] > 0 && shipLog->removeAfter[i]<t ){
         /* log expired, so remove (which sets id to -1) */
         shiplog_delete(shipLog->idList[i]);
      }
      if ( shipLog->idList[i]>=0 && (all==1 || !strcmp(type, shipLog->typeList[i] ))){
         if ( n == selectedLog )
            break;
         n++;
      }
   }
   if ( i>=0 )
      i = shipLog->idList[i];
   return i; /* -1 if not found */
   
}

/*
 * @brief removes an entry from the log
 * @param e the entry to remove
 * @returns the next entry.
 */
ShipLogEntry *shiplog_removeEntry( ShipLogEntry *e )
{
   ShipLogEntry *tmp;
   /* remove this entry */
   if ( e->prev != NULL)
      ((ShipLogEntry*)e->prev)->next = e->next;
   if ( e->next != NULL )
      ((ShipLogEntry*)e->next)->prev = e->prev;
   if ( shipLog->head == e )
      shipLog->head = e->next;
   if ( shipLog->tail == e )
      shipLog->tail = e->prev;
   free(e->msg);
   tmp=e;
   e=(ShipLogEntry*)e->next;
   free(tmp);
   return e;

}

/*
 * @brief Get all log entries matching logid, or if logid==-1, matching type, or if type==NULL, all.
 */
void shiplog_listLog( int logid, char *type,int *nentries, char ***logentries, int incempty )
{
   int i,n = 0,all = 0;
   char **entries = NULL;
   ShipLogEntry *e, *use;
   char buf[256];
   int pos;
   e = shipLog->head;
   if ( ( logid == -1 ) && ( !strcmp(type, "All") ) ) {
      /* Match all types if logid == -1 */
      all = 1;
   }
   while( e != NULL ) {
      use = NULL;
      if ( logid == -1 ){
         if( all ) { /* add the log */
            if ( e->id >= 0)
               use=e;
         } else { /* see if this log is of type */
            for ( i=0; i<shipLog->nlogs; i++ ){
               if ( shipLog->idList[i] >= 0 && e->id == shipLog->idList[i] && !strcmp( shipLog->typeList[i], type ) ){/* the type matches current messages */
                  use = e;
                  break; /* there should only be 1 log of this type and id. */
               }
            }
         }
      } else { /* just this particular log*/
         if ( e->id == logid ) {
            use = e;
         }
      }
      if ( use != NULL ){
         n++;
         entries = realloc(entries, sizeof(char*) * n);
         ntime_prettyBuf(buf, 256, use->time, 2);
         pos = strlen(buf);
         pos += nsnprintf(&buf[pos], 256-pos, ":  %s", use->msg);
         entries[n-1] = strdup(buf);
      }
      
      e = e -> next;
   }
   if ( ( n == 0 ) && ( incempty != 0 ) ) {
      /*empty list, so add "Empty" */
      n = 1;
      entries = realloc(entries,sizeof(char*));
      entries[0] = strdup("Empty");
   }
   *logentries = entries;
   *nentries = n;
   
}
