/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file save.c
 *
 * @brief Handles saving/loading games.
 */
/** @cond */
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "save.h"

#include "array.h"
#include "conf.h"
#include "load.h"
#include "log.h"
#include "mission.h"
#include "ndata.h"
#include "nxml.h"
#include "player.h"
#include "plugin.h"
#include "shiplog.h"
#include "start.h"

int save_loaded = 0; /**< Just loaded the saved game. */

/*
 * prototypes
 */
/* externs */
/* player.c */
extern int
player_save( xmlTextWriterPtr writer ); /**< Saves player related stuff. */
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
extern int
space_sysSave( xmlTextWriterPtr writer ); /**< Saves the space stuff. */
/* economy.c */
extern int
economy_sysSave( xmlTextWriterPtr writer ); /**< Saves the economy stuff. */
/* unidiff.c */
extern int
diff_save( xmlTextWriterPtr writer ); /**< Saves the universe diffs. */
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
   if ( diff_save( writer ) < 0 )
      return -1; /* Must save first or can get cleared. */
   if ( player_save( writer ) < 0 )
      return -1;
   if ( missions_saveActive( writer ) < 0 )
      return -1;
   if ( events_saveActive( writer ) < 0 )
      return -1;
   if ( news_saveArticles( writer ) < 0 )
      return -1;
   if ( var_save( writer ) < 0 )
      return -1;
   if ( pfaction_save( writer ) < 0 )
      return -1;
   if ( hook_save( writer ) < 0 )
      return -1;
   if ( space_sysSave( writer ) < 0 )
      return -1;
   if ( economy_sysSave( writer ) < 0 )
      return -1;
   if ( shiplog_save( writer ) < 0 )
      return -1;
   return 0;
}

/**
 * @brief Saves the current game.
 *
 *    @return 0 on success.
 */
int save_all( void )
{
   return save_all_with_name( "autosave" );
}

/**
 * @brief Saves the current game.
 *
 *    @param name Name of custom snapshot.
 *    @return 0 on success.
 */
int save_all_with_name( const char *name )
{
   char             file[PATH_MAX];
   const plugin_t  *plugins = plugin_list();
   xmlDocPtr        doc;
   xmlTextWriterPtr writer;

   /* Do not save if saving is off. */
   if ( player_isFlag( PLAYER_NOSAVE ) )
      return 0;

   /* Create the writer. */
   writer = xmlNewTextWriterDoc( &doc, conf.save_compress );
   if ( writer == NULL ) {
      ERR( _( "testXmlwriterDoc: Error creating the xml writer" ) );
      return -1;
   }

   /* Set the writer parameters. */
   xmlw_setParams( writer );

   /* Start element. */
   xmlw_start( writer );
   xmlw_startElem( writer, "naev_save" );

   /* Save the version and such. */
   xmlw_startElem( writer, "version" );
   xmlw_elem( writer, "naev", "%s", naev_version( 0 ) );
   xmlw_elem( writer, "data", "%s", start_name() );
   xmlw_endElem( writer ); /* "version" */

   /* Save last played. */
   xmlw_saveTime( writer, "last_played", time( NULL ) );

   /* Save plugins. */
   xmlw_startElem( writer, "plugins" );
   for ( int i = 0; i < array_size( plugins ); i++ )
      xmlw_elem( writer, "plugin", "%s", plugin_name( &plugins[i] ) );
   xmlw_endElem( writer ); /* "plugins" */

   /* Save the data. */
   if ( save_data( writer ) < 0 ) {
      ERR( _( "Trying to save game data" ) );
      goto err_writer;
   }

   /* Finish element. */
   xmlw_endElem( writer ); /* "naev_save" */
   xmlw_done( writer );

   /* Write to file. */
   if ( PHYSFS_mkdir( "saves" ) == 0 ) {
      snprintf( file, sizeof( file ), "%s/saves", PHYSFS_getWriteDir() );
      WARN( _( "Dir '%s' does not exist and unable to create: %s" ), file,
            _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      goto err_writer;
   }
   snprintf( file, sizeof( file ), "saves/%s", player.name );
   if ( PHYSFS_mkdir( file ) == 0 ) {
      snprintf( file, sizeof( file ), "%s/saves/%s", PHYSFS_getWriteDir(),
                player.name );
      WARN( _( "Dir '%s' does not exist and unable to create: %s" ), file,
            _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      goto err_writer;
   }

   /* Back up old saved game. */
   if ( !strcmp( name, "autosave" ) ) {
      if ( !save_loaded ) {
         char backup[PATH_MAX];
         snprintf( file, sizeof( file ), "saves/%s/autosave.ns", player.name );
         snprintf( backup, sizeof( backup ), "saves/%s/backup.ns",
                   player.name );
         if ( ndata_copyIfExists( file, backup ) < 0 ) {
            WARN( _( "Aborting save…" ) );
            goto err_writer;
         }
      }
      save_loaded = 0;
   }

   /* Critical section, if crashes here player's game gets corrupted.
    * Luckily we have a copy just in case... */
   xmlFreeTextWriter( writer );
   snprintf( file, sizeof( file ), "%s/saves/%s/%s.ns", PHYSFS_getWriteDir(),
             player.name, name ); /* TODO: write via physfs */
   if ( xmlSaveFileEnc( file, doc, "UTF-8" ) < 0 ) {
      WARN( _(
         "Failed to write saved game!  You'll most likely have to restore it "
         "by copying your backup saved game over your current saved game." ) );
      goto err;
   }
   xmlFreeDoc( doc );

   return 0;

err_writer:
   xmlFreeTextWriter( writer );
err:
   xmlFreeDoc( doc );
   return -1;
}

/**
 * @brief Reload the current saved game.
 */
void save_reload( void )
{
   /* Should be refreshed already when menu is opened (load_refresh). */
   const nsave_t *ns = load_getList( player.name );
   if ( array_size( ns ) <= 0 ) {
      WARN( _( "Unable to reload save for '%s'!" ), player.name );
      return;
   }
   load_gameFile( ns[0].path );
}
