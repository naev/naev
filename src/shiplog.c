/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file shiplog.c
 *
 * @brief Handles a log/journal of the player's playthrough.
 */
#include <inttypes.h>

#include "shiplog.h"
#include <inttypes.h>

/*Hold a single log entry - a double linked list*/
typedef struct {
   int     id;
   ntime_t time;
   char   *msg;
   void   *next;
   void   *prev;
} ShipLogEntry;

/* Holding global information about the log. */
typedef struct {
   int          *idList;
   char        **typeList;
   char        **nameList;
   ntime_t      *removeAfter;
   char        **idstrList;
   int          *maxLen;
   int           nlogs;
   ShipLogEntry *head; /**< The head (newest entry) */
   ShipLogEntry *tail; /**< The tail (oldest entry) */
} ShipLog;

static ShipLog shipLog; /**< The player's ship log. */

/*
 * Prototypes.
 */
static ShipLogEntry *shiplog_removeEntry( ShipLogEntry *e );

/**
 * @brief Creates a new log with given title of given type.
 *
 *    @param idstr ID string for this logset, or NULL if an ID string not
 * required.
 *    @param logname Name of the log (title)
 *    @param type Type of the log, e.g. travel, shipping, etc
 *    @param overwrite Whether to overwrite an existing log of this type and
 * logname (if 1), or all logs of this type (if 2).
 *    @param maxLen Maximum number of entries for this log (longer ones will be
 * purged).
 *    @return log ID.
 */
int shiplog_create( const char *idstr, const char *logname, const char *type,
                    int overwrite, int maxLen )
{
   ShipLogEntry *e;
   int           i, id, indx;
   indx = shipLog.nlogs;

   id = LOG_ID_INVALID;
   if ( overwrite == 1 ) {
      /* check to see whether this idstr or logname and type has been created
       * before, and if so, remove all entries of that logid */
      if ( idstr != NULL ) {
         /* find the matching logid for this idstr */
         for ( i = 0; i < shipLog.nlogs; i++ ) {
            if ( ( shipLog.idstrList[i] != NULL ) &&
                 ( strcmp( shipLog.idstrList[i], idstr ) == 0 ) ) {
               /* matching idstr found. */
               id   = shipLog.idList[i];
               indx = i;
               break;
            }
         }
      } else {
         for ( i = 0; i < shipLog.nlogs; i++ ) {
            if ( ( strcmp( type, shipLog.typeList[i] ) == 0 ) &&
                 ( strcmp( logname, shipLog.nameList[i] ) == 0 ) ) {
               id   = shipLog.idList[i];
               indx = i;
               break;
            }
         }
      }
      if ( i < shipLog.nlogs ) { /* prev id found - so remove all log entries of
                                    this type. */
         e = shipLog.head;
         while ( e != NULL ) {
            if ( e->id == id ) {
               /* remove this entry */
               e = shiplog_removeEntry( e );
            } else {
               e = (ShipLogEntry *)e->next;
            }
         }
         shipLog.maxLen[i] = maxLen;
      }
   } else if ( overwrite == 2 ) {
      /* check to see whether this type has been created before, and if so,
       * remove all entries. */
      int found = 0;
      id        = LOG_ID_INVALID;
      for ( i = 0; i < shipLog.nlogs; i++ ) {
         if ( ( ( idstr != NULL ) && ( shipLog.idstrList[i] != NULL ) &&
                ( strcmp( idstr, shipLog.idstrList[i] ) == 0 ) ) ||
              ( ( idstr == NULL ) &&
                ( strcmp( type, shipLog.typeList[i] ) == 0 ) ) ) {
            e = shipLog.head;
            while ( e != NULL ) {
               if ( e->id == shipLog.idList[i] )
                  e = shiplog_removeEntry( e );
               else
                  e = e->next;
            }
            if ( found == 0 ) { /* This is the first entry of this type */
               found = 1;
               id    = shipLog.idList[i];
               indx  = i;
               free( shipLog.nameList[i] );
               shipLog.nameList[i] = strdup( logname );
               shipLog.maxLen[i]   = maxLen;
            } else { /* a previous entry of this type as been found, so just
                        invalidate this logid. */
               shipLog.idList[i] = LOG_ID_INVALID;
               free( shipLog.idstrList[i] );
               shipLog.idstrList[i] = NULL;
            }
         }
      }
   }

   if ( ( indx == shipLog.nlogs ) && ( idstr != NULL ) ) {
      /* see if existing log with this idstr exists, if so, append to it */
      for ( i = 0; i < shipLog.nlogs; i++ ) {
         if ( ( shipLog.idstrList[i] != NULL ) &&
              ( strcmp( idstr, shipLog.idstrList[i] ) == 0 ) ) {
            id                = shipLog.idList[i];
            indx              = i;
            shipLog.maxLen[i] = maxLen;
            break;
         }
      }
   }
   if ( indx == shipLog.nlogs ) {
      /* create a new id for this log */
      id = -1;
      for ( i = 0; i < shipLog.nlogs; i++ ) { /* get maximum id */
         if ( shipLog.idList[i] > id )
            id = shipLog.idList[i];
      }
      id++;
      shipLog.nlogs++;
      shipLog.idList = realloc( shipLog.idList, sizeof( int ) * shipLog.nlogs );
      shipLog.nameList =
         realloc( shipLog.nameList, sizeof( char * ) * shipLog.nlogs );
      shipLog.typeList =
         realloc( shipLog.typeList, sizeof( char * ) * shipLog.nlogs );
      shipLog.removeAfter =
         realloc( shipLog.removeAfter, sizeof( ntime_t ) * shipLog.nlogs );
      shipLog.idstrList =
         realloc( shipLog.idstrList, sizeof( char * ) * shipLog.nlogs );
      shipLog.maxLen = realloc( shipLog.maxLen, sizeof( int ) * shipLog.nlogs );
      shipLog.removeAfter[indx] = 0;
      shipLog.idList[indx]      = id;
      shipLog.nameList[indx]    = strdup( logname );
      shipLog.typeList[indx]    = strdup( type );
      shipLog.maxLen[indx]      = maxLen;
      shipLog.idstrList[indx]   = ( idstr == NULL ) ? NULL : strdup( idstr );
   }
   return id;
}

/**
 * @brief Appends to the log file
 *
 * @param idstr of the log to add to
 * @param msg Message to be added.
 * @return 0 on success, -1 on failure.
 */
int shiplog_append( const char *idstr, const char *msg )
{
   int i, id;
   for ( i = 0; i < shipLog.nlogs; i++ ) {
      if ( ( ( idstr == NULL ) && ( shipLog.idstrList[i] == NULL ) ) ||
           ( ( idstr != NULL ) && ( shipLog.idstrList[i] != NULL ) &&
             ( strcmp( idstr, shipLog.idstrList[i] ) == 0 ) ) ) {
         break;
      }
   }
   if ( i == shipLog.nlogs ) {
      WARN( _( "Warning - log not found: creating it" ) );
      id = shiplog_create(
         idstr, _( "Please report this log as an error to github.com/naev" ),
         idstr != NULL ? idstr : "", 0, 0 );
   } else {
      id = shipLog.idList[i];
   }
   return shiplog_appendByID( id, msg );
}

/**
 * @brief Adds to the log file
 *
 *    @param logid of the log to add to.
 *    @param msg Message to be added.
 *    @return 0 on success, -1 on failure.
 */
int shiplog_appendByID( int logid, const char *msg )
{
   ShipLogEntry *e;
   ntime_t       now = ntime_get();
   int           i, maxLen = 0;

   if ( logid < 0 )
      return -1;

   /* Check that the log hasn't already been added (e.g. if reloading) */
   e = shipLog.head;
   /* check for identical logs */
   while ( e != NULL ) {
      if ( e->time != now ) { /* logs are created in chronological order */
         break;
      }
      if ( ( logid == e->id ) && ( strcmp( e->msg, msg ) == 0 ) ) {
         /* Identical log already exists */
         return 0;
      }
      e = e->next;
   }
   if ( ( e = calloc( 1, sizeof( ShipLogEntry ) ) ) == NULL ) {
      WARN( _( "Error creating new log entry - crash imminent!\n" ) );
      return -1;
   }
   e->next      = shipLog.head;
   shipLog.head = e;
   if ( shipLog.tail == NULL ) /* first entry - point to both head and tail.*/
      shipLog.tail = e;
   if ( e->next != NULL )
      ( (ShipLogEntry *)e->next )->prev = (void *)e;
   e->id   = logid;
   e->msg  = strdup( msg );
   e->time = now;
   for ( i = 0; i < shipLog.nlogs; i++ ) {
      if ( shipLog.idList[i] == logid ) {
         maxLen = shipLog.maxLen[i];
         break;
      }
   }
   if ( maxLen > 0 ) {
      /* prune log entries if necessary */
      i = 0;
      e = shipLog.head;
      while ( e != NULL ) {
         if ( e->id == logid ) {
            i++;
            if ( i > maxLen )
               e = shiplog_removeEntry( e );
            else
               e = (ShipLogEntry *)e->next;
         } else {
            e = (ShipLogEntry *)e->next;
         }
      }
   }
   return 0;
}

/**
 * @brief Deletes a log (e.g. a cancelled mission may wish to do this, or the
 * user might).
 *
 * @param logid of the log to remove, or LOG_ID_ALL
 */
void shiplog_delete( int logid )
{
   ShipLogEntry *e, *tmp;
   int           i;

   if ( ( logid < 0 ) && ( logid != LOG_ID_ALL ) )
      return;

   e = shipLog.head;
   while ( e != NULL ) {
      if ( logid == LOG_ID_ALL || logid == e->id ) {
         if ( e->prev != NULL )
            ( (ShipLogEntry *)e->prev )->next = e->next;
         if ( e->next != NULL )
            ( (ShipLogEntry *)e->next )->prev = e->prev;
         free( e->msg );
         if ( e == shipLog.head )
            shipLog.head = e->next;
         if ( e == shipLog.tail )
            shipLog.tail = e->prev;
         tmp = e;
         e   = (ShipLogEntry *)e->next;
         free( tmp );
      } else {
         e = (ShipLogEntry *)e->next;
      }
   }

   for ( i = 0; i < shipLog.nlogs; i++ ) {
      if ( logid == LOG_ID_ALL || logid == shipLog.idList[i] ) {
         shipLog.idList[i] = LOG_ID_INVALID;
         free( shipLog.nameList[i] );
         shipLog.nameList[i] = NULL;
         free( shipLog.typeList[i] );
         shipLog.typeList[i] = NULL;
         free( shipLog.idstrList[i] );
         shipLog.idstrList[i]   = NULL;
         shipLog.maxLen[i]      = 0;
         shipLog.removeAfter[i] = 0;
      }
   }
}

/**
 * @brief Sets the remove flag for a log - it will be removed once time
 * increases, eg after a player takes off.
 *
 * @param logid the ID of the log
 * @param when the time at which to remove.  If 0, uses current time, if <0,
 * adds abs to current time, if >0, uses as the time to remove. Rationale:
 * Allows a player to review the log while still landed, and then clears it up
 * once takes off.
 */
void shiplog_setRemove( int logid, ntime_t when )
{
   int i;
   if ( when == 0 )
      when = ntime_get();
   else if ( when < 0 ) /* add this to ntime */
      when = ntime_get() - when;

   for ( i = 0; i < shipLog.nlogs; i++ ) {
      if ( shipLog.idList[i] == logid ) {
         shipLog.removeAfter[i] = when;
         break;
      }
   }
}

/**
 * @brief Deletes all logs of given type.
 *
 * @param type of the log to remove
 */
void shiplog_deleteType( const char *type )
{
   int i;
   if ( type == NULL )
      return;
   for ( i = 0; i < shipLog.nlogs; i++ ) {
      if ( ( shipLog.idList[i] >= 0 ) &&
           ( strcmp( type, shipLog.typeList[i] ) == 0 ) ) {
         shiplog_delete( shipLog.idList[i] );
      }
   }
}

/**
 * @brief Clear the shiplog
 */
void shiplog_clear( void )
{
   shiplog_delete( LOG_ID_ALL );
   free( shipLog.idList );
   free( shipLog.nameList );
   free( shipLog.typeList );
   free( shipLog.idstrList );
   free( shipLog.maxLen );
   free( shipLog.removeAfter );
   memset( &shipLog, 0, sizeof( ShipLog ) );
}

/**
 * @brief Set up the shiplog
 */
void shiplog_new( void )
{
   shiplog_clear();
}

/*
 * @brief Saves the logfile
 */
int shiplog_save( xmlTextWriterPtr writer )
{
   int           i;
   ShipLogEntry *e;
   ntime_t       t = ntime_get();
   xmlw_startElem( writer, "shiplog" );

   for ( i = 0; i < shipLog.nlogs; i++ ) {
      if ( shipLog.removeAfter[i] > 0 && shipLog.removeAfter[i] < t )
         shiplog_delete( shipLog.idList[i] );
      if ( shipLog.idList[i] >= 0 ) {
         xmlw_startElem( writer, "entry" );
         xmlw_attr( writer, "id", "%d", shipLog.idList[i] );
         xmlw_attr( writer, "t", "%s", shipLog.typeList[i] );
         if ( shipLog.removeAfter[i] != 0 )
            xmlw_attr( writer, "r", "%" PRIu64, shipLog.removeAfter[i] );
         if ( shipLog.idstrList[i] != NULL )
            xmlw_attr( writer, "s", "%s", shipLog.idstrList[i] );
         if ( shipLog.maxLen[i] != 0 )
            xmlw_attr( writer, "m", "%d", shipLog.maxLen[i] );
         xmlw_str( writer, "%s", shipLog.nameList[i] );
         xmlw_endElem( writer ); /* entry */
      }
   }
   e = shipLog.head;
   while ( e != NULL ) {
      if ( e->id >= 0 ) {
         xmlw_startElem( writer, "log" );
         xmlw_attr( writer, "id", "%d", e->id );
         xmlw_attr( writer, "t", "%" PRIu64, e->time );
         xmlw_str( writer, "%s", e->msg );
         xmlw_endElem( writer ); /* log */
      }
      e = (ShipLogEntry *)e->next;
   }
   xmlw_endElem( writer ); /* economy */
   return 0;
}

/**
 * @brief Loads the logfile
 * @param parent Parent node for economy.
 * @return 0 on success.
 */
int shiplog_load( xmlNodePtr parent )
{
   xmlNodePtr    node, cur;
   ShipLogEntry *e;
   int           id, i;
   shiplog_clear();

   node = parent->xmlChildrenNode;
   do {
      if ( xml_isNode( node, "shiplog" ) ) {
         cur = node->xmlChildrenNode;
         do {
            if ( xml_isNode( cur, "entry" ) ) {
               xmlr_attr_int( cur, "id", id );
               /* check this ID isn't already present */
               for ( i = 0; i < shipLog.nlogs; i++ ) {
                  if ( shipLog.idList[i] == id )
                     break;
               }
               if ( i == shipLog.nlogs ) { /* a new ID */
                  shipLog.nlogs++;
                  shipLog.idList =
                     realloc( shipLog.idList, sizeof( int ) * shipLog.nlogs );
                  shipLog.nameList = realloc(
                     shipLog.nameList, sizeof( char * ) * shipLog.nlogs );
                  shipLog.typeList = realloc(
                     shipLog.typeList, sizeof( char * ) * shipLog.nlogs );
                  shipLog.removeAfter = realloc(
                     shipLog.removeAfter, sizeof( ntime_t ) * shipLog.nlogs );
                  shipLog.idstrList = realloc(
                     shipLog.idstrList, sizeof( char * ) * shipLog.nlogs );
                  shipLog.maxLen =
                     realloc( shipLog.maxLen, sizeof( int ) * shipLog.nlogs );
                  shipLog.idList[shipLog.nlogs - 1] = id;
                  xmlr_attr_strd( cur, "t",
                                  shipLog.typeList[shipLog.nlogs - 1] );
                  xmlr_attr_long( cur, "r",
                                  shipLog.removeAfter[shipLog.nlogs - 1] );
                  xmlr_attr_strd( cur, "s",
                                  shipLog.idstrList[shipLog.nlogs - 1] );
                  xmlr_attr_int( cur, "m", shipLog.maxLen[shipLog.nlogs - 1] );
                  if ( shipLog.typeList[shipLog.nlogs - 1] == NULL ) {
                     shipLog.typeList[shipLog.nlogs - 1] = strdup( "No type" );
                     WARN( _( "No ID in shipLog entry" ) );
                  }
                  shipLog.nameList[shipLog.nlogs - 1] =
                     strdup( xml_raw( cur ) );
               }
            } else if ( xml_isNode( cur, "log" ) ) {
               e = calloc( 1, sizeof( ShipLogEntry ) );
               /* put this one at the end */
               e->prev = shipLog.tail;
               if ( shipLog.tail == NULL )
                  shipLog.head = e;
               else
                  shipLog.tail->next = e;
               shipLog.tail = e;

               xmlr_attr_int( cur, "id", e->id );
               xmlr_attr_long( cur, "t", e->time );
               e->msg = strdup( xml_raw( cur ) );
            }
         } while ( xml_nextNode( cur ) );
      }
   } while ( xml_nextNode( node ) );
   return 0;
}

void shiplog_listTypes( int *ntypes, char ***logTypes, int includeAll )
{
   int     i, j, n = 0;
   char  **types = NULL;
   ntime_t t     = ntime_get();

   if ( includeAll ) {
      types    = malloc( sizeof( char * ) );
      n        = 1;
      types[0] = strdup( _( "All" ) );
   }
   for ( i = 0; i < shipLog.nlogs; i++ ) {
      if ( shipLog.removeAfter[i] > 0 && shipLog.removeAfter[i] < t ) {
         /* log expired, so remove (which sets id to LOG_ID_INVALID) */
         shiplog_delete( shipLog.idList[i] );
      }
      if ( shipLog.idList[i] >= 0 ) {
         /* log valid */
         for ( j = 0; j < n; j++ ) {
            if ( strcmp( shipLog.typeList[i], types[j] ) == 0 )
               break;
         }
         if ( j == n ) { /*This log type not found, so add.*/
            n++;
            types        = realloc( types, sizeof( char * ) * n );
            types[n - 1] = strdup( shipLog.typeList[i] );
         }
      }
   }
   *ntypes   = n;
   *logTypes = types;
}

/**
 * @brief Lists matching logs (which haven't expired via "removeAfter") into the
 * provided arrays.
 *
 *    @param type The log-type to match (or NULL to match any type).
 *    @param[out] nlogs Number of logs emitted.
 *    @param[out] logsOut Matching log-names. Will be reallocated as needed.
 * Emitted strings must be freed.
 *    @param[out] logIDs Matching log ID lists. Will be reallocated as needed.
 * Emitted lists are owned by the shipLog.
 *    @param includeAll Whether to include the special "All" log.
 */
void shiplog_listLogsOfType( const char *type, int *nlogs, char ***logsOut,
                             int **logIDs, int includeAll )
{
   int     i, n;
   char  **logs;
   int    *logid;
   ntime_t t = ntime_get();

   n     = !!includeAll;
   logs  = realloc( *logsOut, sizeof( char * ) * n );
   logid = realloc( *logIDs, sizeof( int ) * n );
   if ( includeAll ) {
      logs[0]  = strdup( _( "All" ) );
      logid[0] = LOG_ID_ALL;
   }
   if ( shipLog.nlogs > 0 ) {
      for ( i = shipLog.nlogs - 1; i >= 0; i-- ) {
         if ( shipLog.removeAfter[i] > 0 && shipLog.removeAfter[i] < t ) {
            /* log expired, so remove (which sets id to LOG_ID_INVALID) */
            shiplog_delete( shipLog.idList[i] );
         }
         if ( ( shipLog.idList[i] >= 0 ) &&
              ( ( type == NULL ) ||
                ( strcmp( type, shipLog.typeList[i] ) == 0 ) ) ) {
            n++;
            logs         = realloc( logs, sizeof( char * ) * n );
            logs[n - 1]  = strdup( shipLog.nameList[i] );
            logid        = realloc( logid, sizeof( int ) * n );
            logid[n - 1] = shipLog.idList[i];
         }
      }
   }
   *nlogs   = n;
   *logsOut = logs;
   *logIDs  = logid;
}

int shiplog_getIdOfLogOfType( const char *type, int selectedLog )
{
   int     i, n = 0;
   ntime_t t = ntime_get();

   for ( i = shipLog.nlogs - 1; i >= 0; i-- ) {
      if ( ( shipLog.removeAfter[i] > 0 ) && ( shipLog.removeAfter[i] < t ) ) {
         /* log expired, so remove (which sets id to -1) */
         shiplog_delete( shipLog.idList[i] );
      }
      if ( ( shipLog.idList[i] >= 0 ) &&
           ( ( type == NULL ) ||
             ( strcmp( type, shipLog.typeList[i] ) == 0 ) ) ) {
         if ( n == selectedLog )
            break;
         n++;
      }
   }
   if ( i >= 0 )
      i = shipLog.idList[i];
   return i; /* -1 if not found */
}

/**
 * @brief removes an entry from the log
 * @param e the entry to remove
 * @returns the next entry.
 */
static ShipLogEntry *shiplog_removeEntry( ShipLogEntry *e )
{
   ShipLogEntry *tmp;
   /* remove this entry */
   if ( e->prev != NULL )
      ( (ShipLogEntry *)e->prev )->next = e->next;
   if ( e->next != NULL )
      ( (ShipLogEntry *)e->next )->prev = e->prev;
   if ( shipLog.head == e )
      shipLog.head = e->next;
   if ( shipLog.tail == e )
      shipLog.tail = e->prev;
   free( e->msg );
   tmp = e;
   e   = (ShipLogEntry *)e->next;
   free( tmp );
   return e;
}

/**
 * @brief Get all log entries matching logid, or if logid==LOG_ID_ALL, matching
 * type, or if type==NULL, all.
 */
void shiplog_listLog( int logid, const char *type, int *nentries,
                      char ***logentries, int incempty )
{
   int           i, n = 0, all = 0;
   char        **entries = NULL;
   ShipLogEntry *e, *use;
   char          buf[5000];
   int           pos;
   e = shipLog.head;
   if ( logid == LOG_ID_ALL ) {
      all = 1;
   }
   while ( e != NULL ) {
      use = NULL;
      if ( logid == LOG_ID_ALL ) {
         if ( all ) { /* add the log */
            if ( e->id >= 0 )
               use = e;
         } else { /* see if this log is of type */
            for ( i = 0; i < shipLog.nlogs; i++ ) {
               if ( ( shipLog.idList[i] >= 0 ) &&
                    ( e->id == shipLog.idList[i] ) &&
                    ( strcmp( shipLog.typeList[i], type ) == 0 ) ) {
                  /* the type matches current messages */
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
      if ( use != NULL ) {
         n++;
         entries = realloc( entries, sizeof( char * ) * n );
         ntime_prettyBuf( buf, sizeof( buf ), use->time, 2 );
         pos = strlen( buf );
         pos += scnprintf( &buf[pos], sizeof( buf ) - pos, ":  %s", use->msg );
         entries[n - 1] = strdup( buf );
      }

      e = e->next;
   }
   (void)pos;
   if ( ( n == 0 ) && ( incempty != 0 ) ) {
      /*empty list, so add "Empty" */
      n          = 1;
      entries    = realloc( entries, sizeof( char * ) );
      entries[0] = strdup( _( "Empty" ) );
   }
   *logentries = entries;
   *nentries   = n;
}

/**
 * @brief Checks to see if the log family exists
 *
 * @param idstr ID string for the log family
 */
int shiplog_getID( const char *idstr )
{
   int id = -1;
   int i;
   for ( i = 0; i < shipLog.nlogs; i++ ) {
      if ( ( ( shipLog.idstrList[i] == NULL ) && ( idstr == NULL ) ) ||
           ( ( shipLog.idstrList[i] != NULL ) && ( idstr != NULL ) &&
             ( strcmp( idstr, shipLog.idstrList[i] ) == 0 ) ) ) {
         id = shipLog.idList[i];
         break;
      }
   }
   return id;
}
