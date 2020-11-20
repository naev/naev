/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file save.c
 *
 * @brief Handles saving/loading games.
 */

#include "save.h"

#include "naev.h"

#include <errno.h> /* errno */

#include "log.h"
#include "nxml.h"
#include "nstring.h"
#include "player.h"
#include "dialogue.h"
#include "menu.h"
#include "nfile.h"
#include "hook.h"
#include "ndata.h"
#include "unidiff.h"
#include "nlua_var.h"
#include "event.h"
#include "news.h"
#include "conf.h"
#include "land.h"
#include "gui.h"
#include "load.h"
#include "shiplog.h"

int save_loaded   = 0; /**< Just loaded the saved game. */


/*
 * prototypes
 */
/* externs */
/* player.c */
extern int player_save( xmlTextWriterPtr writer ); /**< Saves player related stuff. */
/* mission.c */
extern int missions_saveActive( xmlTextWriterPtr writer ); /**< Saves active missions. */
/* event.c */
extern int events_saveActive( xmlTextWriterPtr writer );
/* news.c */
extern int news_saveArticles( xmlTextWriterPtr writer );
/* nlua_var.c */
extern int var_save( xmlTextWriterPtr writer ); /**< Saves mission variables. */
/* faction.c */
extern int pfaction_save( xmlTextWriterPtr writer ); /**< Saves faction data. */
/* hook.c */
extern int hook_save( xmlTextWriterPtr writer ); /**< Saves hooks. */
/* space.c */
extern int space_sysSave( xmlTextWriterPtr writer ); /**< Saves the space stuff. */
/* economy.c */
extern int economy_sysSave( xmlTextWriterPtr writer ); /**< Saves the economy stuff. */
/* unidiff.c */
extern int diff_save( xmlTextWriterPtr writer ); /**< Saves the universe diffs. */
/* static */
static int save_data( xmlTextWriterPtr writer );


/**
 * @brief Saves all the player's game data.
 *
 *    @param writer XML writer to use.
 *    @return 0 on success.
 */
static int save_data( xmlTextWriterPtr writer )
{
   /* the data itself */
   if (diff_save(writer) < 0) return -1; /* Must save first or can get cleared. */
   if (player_save(writer) < 0) return -1;
   if (missions_saveActive(writer) < 0) return -1;
   if (events_saveActive(writer) < 0) return -1;
   if (news_saveArticles( writer ) < 0) return -1;
   if (var_save(writer) < 0) return -1;
   if (pfaction_save(writer) < 0) return -1;
   if (hook_save(writer) < 0) return -1;
   if (space_sysSave(writer) < 0) return -1;
   if (economy_sysSave(writer) < 0) return -1;
   if (shiplog_save(writer) < 0) return -1;
   return 0;
}


/**
 * @brief Saves the current game.
 *
 *    @return 0 on success.
 */
int save_all (void)
{
   char file[PATH_MAX];
   xmlDocPtr doc;
   xmlTextWriterPtr writer;

   /* Do not save if saving is off. */
   if (player_isFlag(PLAYER_NOSAVE))
      return 0;

   /* Create the writer. */
   writer = xmlNewTextWriterDoc(&doc, conf.save_compress);
   if (writer == NULL) {
      ERR(_("testXmlwriterDoc: Error creating the xml writer"));
      return -1;
   }

   /* Set the writer parameters. */
   xmlw_setParams( writer );

   /* Start element. */
   xmlw_start(writer);
   xmlw_startElem(writer,"naev_save");

   /* Save the version and such. */
   xmlw_startElem(writer,"version");
   xmlw_elem( writer, "naev", "%s", VERSION );
   xmlw_elem( writer, "data", "%s", ndata_name() );
   xmlw_endElem(writer); /* "version" */

   /* Save the data. */
   if (save_data(writer) < 0) {
      ERR(_("Trying to save game data"));
      goto err_writer;
   }

   /* Finish element. */
   xmlw_endElem(writer); /* "naev_save" */
   xmlw_done(writer);

   /* Write to file. */
   if ((nfile_dirMakeExist(nfile_dataPath()) < 0) ||
         (nfile_dirMakeExist(nfile_dataPath(), "saves") < 0)) {
      WARN(_("Failed to create save directory '%ssaves'."), nfile_dataPath());
      goto err_writer;
   }
   nsnprintf(file, PATH_MAX, "%ssaves/%s.ns", nfile_dataPath(), player.name);

   /* Back up old saved game. */
   if (!save_loaded) {
      if (nfile_backupIfExists(file) < 0) {
         WARN(_("Aborting save..."));
         goto err_writer;
      }
   }
   save_loaded = 0;

   /* Critical section, if crashes here player's game gets corrupted.
    * Luckily we have a copy just in case... */
   xmlFreeTextWriter(writer);
   if (xmlSaveFileEnc(file, doc, "UTF-8") < 0) {
      WARN(_("Failed to write saved game!  You'll most likely have to restore it by copying your backup saved game over your current saved game."));
      goto err;
   }
   xmlFreeDoc(doc);

   return 0;

err_writer:
   xmlFreeTextWriter(writer);
err:
   xmlFreeDoc(doc);
   return -1;
}

/**
 * @brief Reload the current saved game.
 */
void save_reload (void)
{
   char path[PATH_MAX];
   nsnprintf(path, PATH_MAX, "%ssaves/%s.ns", nfile_dataPath(), player.name);
   load_gameFile( path );
}


/**
 * @brief Checks to see if there's a saved game available.
 *
 *    @return 1 if a saved game is available, 0 otherwise.
 */
int save_hasSave (void)
{
   char **files;
   size_t nfiles, i, len;
   int has_save;

   /* Look for saved games. */
   files = nfile_readDir( &nfiles, nfile_dataPath(), "saves" );
   has_save = 0;
   for (i=0; i<nfiles; i++) {
      len = strlen(files[i]);

      /* no save extension */
      if ((len >= 5) && (strcmp(&files[i][len-3],".ns")==0)) {
         has_save = 1;
         break;
      }
   }

   /* Clean up. */
   for (i=0; i<nfiles; i++)
      free(files[i]);
   free(files);

   return has_save;
}

