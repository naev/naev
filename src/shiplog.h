#ifndef SHIPLOG_H
#  define SHIPLOG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nxml.h"
#include "ntime.h"
#include "nstring.h"
int shiplog_create(const char *logname,const char *type, int overwrite); /* returns a log ID, for log with specified title.  If overwrite set, and title already exists, the log will be cleared and previous log ID returned, otherwise a new log ID will be created. */
int shiplog_append(const int logid,const char *msg);/* Add a message to the log */
void shiplog_delete(const int logid); /* Delete a log.  Use with care (removes all entries with this ID */
void shiplog_setRemove(const int logid, ntime_t when); /* Set a log to be removed once time increases */
void shiplog_deleteType(const char *type);
void shiplog_clear(void);
void shiplog_new(void);
int shiplog_save( xmlTextWriterPtr writer );
int shiplog_load( xmlNodePtr parent );

void shiplog_listTypes(int *ntypes, char ***logTypes, int includeAll);
void shiplog_listLogsOfType(char *type, int *nlogs, char ***logsOut, int **logIDs, int includeAll);
int shiplog_getIdOfLogOfType (const char *type, int selectedLog );
void shiplog_listLog(int logid, char *type,int *nentries, char ***logentries,int incempty);



/*Hold a single log entry - a double linked list*/
typedef struct {
  int id;
  ntime_t time;
  char *msg;
  void *next;
  void *prev;
  
} ShipLogEntry;

/* Holding global information about the log. */
typedef struct {
  int *idList;
  char **typeList;
  char **nameList;
  ntime_t *removeAfter;
  int nlogs;
  ShipLogEntry *head;/*The head (newest entry)*/
  ShipLogEntry *tail;/*The tail (oldest entry)*/
} ShipLog;


#endif
