#include "shiplog.h"

static ShipLog *shipLog=NULL;

/*
 * @brief Creates a new log with given title of given type.
 *
 *    @param logname Name of the log (title)
 *    @param type Type of the log, e.g. travel, shipping, etc
 *    @param overwrite Whether to overwrite an existing log of this type and logname.
 *    @return log ID.
 */

int shiplog_create(const char *logname,const char *type, const int overwrite){
   ShipLogEntry *e,*tmp;
   int i, id, indx;
   if ( shipLog == NULL ){
      shipLog = calloc( sizeof(ShipLog), 1);
   }
   indx=shipLog->nlogs;
   
   if ( overwrite ){
      /* check to see whether this logname and type has been created before, and if so, remove all entries of that logid */
      for ( i=0; i<shipLog->nlogs; i++ ){
         if ( !strcmp ( type, shipLog->typeList[i] ) && !strcmp ( logname, shipLog->nameList[i] )){
            id = shipLog->idList[i];
            indx=i;
            break;
         }
      }
      if ( i < shipLog->nlogs ){//prev id found - so remove all log entries of this type.
         e=shipLog->head;
         while ( e != NULL ){
            if ( e->id == id ){
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
            }else{
               e=(ShipLogEntry*)e->next;
            }
         }
      }
   }
   if ( indx == shipLog->nlogs ){
      /* create a new id for this log */
      id=-1;
      for ( i=0; i<shipLog->nlogs; i++ ){
         if ( shipLog->idList[i] > id )
            id = shipLog->idList[i];
      }
      id++;
      shipLog->nlogs++;
      shipLog->idList=realloc(shipLog->idList,sizeof(int)*shipLog->nlogs);
      shipLog->nameList=realloc(shipLog->nameList,sizeof(char*)*shipLog->nlogs);
      shipLog->typeList=realloc(shipLog->typeList,sizeof(char*)*shipLog->nlogs);
      shipLog->idList[indx]=id;
      shipLog->nameList[indx]=strdup(logname);
      shipLog->typeList[indx]=strdup(type);
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
int shiplog_append(const int logid,const char *msg){
   ShipLogEntry *e;
   if(shipLog==NULL)
      shiplog_new();
   ntime_t now = ntime_get();
   if ( logid < 0 )
      return -1;
   /*Check that the log hasn't already been added (e.g. if reloading)*/
   e = shipLog->head;
   /* check for identical logs */
   while ( e != NULL ){
      if ( e->time != now ){ /* logs are created in chronological order */
         break;
      }
      if ( logid == e->id && !strcmp(e->msg,msg) ){
         /* Identical log already exists */
         return 0;
      }
      e = e->next;
   }
   if ( (e = calloc(sizeof(ShipLogEntry),1)) == NULL ){
      printf("Error creating new log entry - crash imminent!\n");
      return -1;
   }
   e->next = shipLog->head;
   shipLog->head = e;
   if ( shipLog->tail == NULL )/* first entry - point to both head and tail.*/
      shipLog->tail = e;
   if ( e->next != NULL )
      ((ShipLogEntry*)e->next)->prev = (void*)e;
   e->id = logid;
   e->msg = strdup(msg);
   e->time = now;
   return 0;
}

/*
 * @brief Deletes a log (e.g. a cancelled mission may wish to do this).
 *
 * @param logid of the log to remove
 */
void shiplog_delete(const int logid){
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
   if ( logid == 0 )/* travel - keep the name and type */
      return;
   for ( i=0; i<shipLog->nlogs; i++){
      if ( shipLog->idList[i] == logid ){
         shipLog->idList[i] = -1;
         free(shipLog->nameList[i]);
         shipLog->nameList[i] = NULL;
         free(shipLog->typeList[i]);
         shipLog->typeList[i] = NULL;
      }
   }
}

/*
 * @brief Deletes all logs of given type.
 *
 * @param type of the log to remove
 */
void shiplog_deleteType(const char *type){
   int i;
   if(shipLog == NULL || type == NULL)
      return;
   for ( i=0; i<shipLog->nlogs; i++ ){
      if ( shipLog->idList[i]>=0 && !strcmp( type, shipLog->typeList[i] )){
         shiplog_delete( shipLog->idList[i] );
      }
   }
}


/*
 * @brief Clear the shiplog
 */
void shiplog_clear(void){
   ShipLogEntry *e, *tmp;
   if ( shipLog == NULL ){
      shipLog = calloc( sizeof(ShipLog), 1);
   }
   e=shipLog->head;
   while ( e != NULL ){
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
   memset(shipLog, 0, sizeof(ShipLog));
   if ( shiplog_create("Journey log", "Travel", 0) !=0 ){/* log id 0 is always the travel log */
      printf("Error - travel shiplog id !=0 - unexpected!\n");
   }
}

/*
 * @brief Set up the shiplog
 */
void shiplog_new(void){
   shiplog_clear();
}

/*
 * @brief Saves the logfiile
 */
int shiplog_save( xmlTextWriterPtr writer ){
   int i;
   ShipLogEntry *e;
   xmlw_startElem(writer,"shiplog");

   for ( i=0; i<shipLog->nlogs; i++ ){
      if ( shipLog->idList[i] >= 0 ){
         xmlw_startElem(writer, "entry");
         xmlw_attr(writer,"id","%d",shipLog->idList[i]);
         xmlw_attr(writer,"t","%s",shipLog->typeList[i]);
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
               if(str){
                  id=atoi(str);
                  free(str);
               }else{
                  id = 0;
                  printf("Warning - no ID in shipLog entry\n");
               }
               /* check this ID isn't already present */
               for ( i=0; i<shipLog->nlogs; i++ ){
                  if ( shipLog->idList[i] == id )
                     break;
               }
               if ( i==shipLog->nlogs ){/* a new ID */
                  shipLog->nlogs++;
                  shipLog->idList = realloc( shipLog->idList, sizeof(int)*shipLog->nlogs);
                  shipLog->nameList = realloc( shipLog->nameList, sizeof(char*)*shipLog->nlogs);
                  shipLog->typeList = realloc( shipLog->typeList, sizeof(char*)*shipLog->nlogs);
                  shipLog->idList[shipLog->nlogs-1]=id;
                  xmlr_attr(cur, "t", str);
                  if(str){
                     shipLog->typeList[shipLog->nlogs-1]=str;
                  }else{
                     shipLog->typeList[shipLog->nlogs-1]=strdup("No type");
                     printf("Warning - no ID in shipLog entry\n");
                  }
                  shipLog->nameList[shipLog->nlogs-1]=strdup(xml_raw(cur));
               }
            }else if(xml_isNode(cur, "log")){
               e = calloc( sizeof(ShipLogEntry), 1);
               /* put this one at the end */
               e->prev = shipLog->tail;
               if ( shipLog->tail == NULL )
                  shipLog->head = e;
               else
                  shipLog->tail->next = e;
               shipLog->tail = e;
               
               /*e->next = shipLog->head;
               shipLog->head = e;
               ((ShipLogEntry*)shipLog->head)->prev = e;
               if ( shipLog->tail == NULL )
                  shipLog->tail = e;
               */
               xmlr_attr(cur, "id", str);
               if(str){
                  e->id = atoi(str);
                  free(str);
               }else{
                  printf("Warning - no ID for shipLog entry\n");
               }
               xmlr_attr(cur, "t", str);
               if(str){
                  e->time = atol(str);
                  free(str);
               }else
                  printf("Warning - no time for shipLog entry\n");
               e->msg=strdup(xml_raw(cur));
            }
         } while (xml_nextNode(cur));
      }
   } while(xml_nextNode(node));
   return 0;
}

void shiplog_listTypes(int *ntypes, char ***logTypes, int includeAll){
   int i,j,n=0;
   char **types = NULL;
   if ( includeAll ){
      types = malloc(sizeof(char**));
      n=1;
      types[0] = strdup("All");
   }
   for ( i=0; i<shipLog->nlogs; i++ ){
      if ( shipLog->idList[i]>=0 ){
         for ( j=0; j<n; j++ ){
            if ( !strcmp ( shipLog->typeList[i], types[j] ))
               break;
         }
         if ( j==n ){/*This log type not found, so add.*/
            n++;
            types=realloc(types,sizeof(char**)*n);
            types[n-1] = strdup(shipLog->typeList[i]);
         }
      }
   }
   *ntypes=n;
   *logTypes=types;
}

void shiplog_listLogsOfType(char *type, int *nlogs, char ***logsOut, int **logIDs, int includeAll){
   int i,n=0,all=0;
   char **logs=NULL;
   int *logid=NULL;
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

int shiplog_getIdOfLogOfType (const char *type, int selectedLog ){
   int i,all=0,n=0;
   if ( !strcmp(type,"All") ){
      /*Match all types*/
      all=1;
   }

   for ( i=shipLog->nlogs-1; i>=0; i-- ){
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
 * @brief Get all log entries matching logid, or if logid==-1, matching type, or if type==NULL, all.
 */
void shiplog_listLog(int logid, char *type,int *nentries, char ***logentries, int incempty){
   int i,n=0,all=0;
   char **entries=NULL;
   ShipLogEntry *e, *use;
   char buf[256];
   int pos;
   e=shipLog->head;
   if ( logid==-1 && !strcmp(type,"All") ){
      /*Match all types if logid == -1*/
      all=1;
   }
   
   while( e != NULL ){
      use=NULL;
      if ( logid == -1 ){
         if( all ){/* add the log */
            if ( e->id >= 0)
               use=e;
         }else{/*see if this log is of type */
            for ( i=0; i<shipLog->nlogs; i++ ){
               if ( shipLog->idList[i] >= 0 && e->id == shipLog->idList[i] && !strcmp( shipLog->typeList[i], type ) ){/* the type matches current messages */
                  use = e;
                  break; /*there should only be 1 log of this type and id.*/
               }
            }
         }
      }else{/* just this particular log*/
         if ( e->id == logid ){
            use = e;
         }
      }
      if ( use != NULL ){
         n++;
         entries=realloc(entries,sizeof(char*)*n);
         if ( use->id == 0 ){/*travel log - special case*/

            ntime_prettyBuf(buf, 256, use->time,2);
            pos=strlen(buf);
            if ( ! strncmp("sys: ",use->msg,5) ){
               pos+=nsnprintf(&buf[pos],256-pos,":  Jumped into %s",&use->msg[5]);
            }else{
               pos+=nsnprintf(&buf[pos],256-pos,":  Landed on %s",use->msg);
            }
            entries[n-1] = strdup(buf);
         }else{
            ntime_prettyBuf(buf, 256, use->time,2);
            pos=strlen(buf);
            pos+=nsnprintf(&buf[pos],256-pos,":  %s",use->msg);
            entries[n-1] = strdup(buf);
         }
      }

      e = e -> next;
   }
   if ( n == 0 && incempty!=0 ){
      /*empty list, so add "Empty" */
      n=1;
      entries=realloc(entries,sizeof(char*));
      entries[0]=strdup("Empty");
   }
   *logentries=entries;
   *nentries=n;
   
}
