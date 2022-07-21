/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file physfs_archiver_blacklist.c
 *
 * @brief Archiver that allows us to blacklist certain files by creating empty
 * versions of them.
 */

#include "naev.h"
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include "physfs.h"

#include "array.h"
#include "log.h"

static pcre2_code *blk_re        = NULL;
static pcre2_match_data *blk_match = NULL;
static char **blk_blacklists_re  = NULL;
static char **blk_blacklists     = NULL;

/*
 * Prototypes.
 */
static PHYSFS_Io *blk_unsupportedIO( void *opaque, const char *filename );
static int blk_unsupported( void *opaque, const char *name );
static void *blk_openArchive( PHYSFS_Io *io, const char *name, int forWrite, int *claimed );
static PHYSFS_EnumerateCallbackResult blk_enumerate( void *opaque, const char *dirname, PHYSFS_EnumerateCallback cb, const char *origdir, void *callbackdata );
static PHYSFS_Io *blk_openRead(void *opaque, const char *fnm);
static int blk_stat( void *opaque, const char *fn, PHYSFS_Stat *stat );
static void blk_closeArchive( void *opaque );

static const PHYSFS_Archiver blk_archiver = {
   .version = 0,
   .info = {
      .extension  = "blacklist",
      .description = "Naev blacklist archiver.",
      .author     = "Naev DevTeam",
      .url        = "https://naev.org",
      .supportsSymlinks = 0,
   },
   .openArchive   = blk_openArchive,
   .enumerate     = blk_enumerate,
   .openRead      = blk_openRead,
   .openWrite     = blk_unsupportedIO,
   .openAppend    = blk_unsupportedIO,
   .remove        = blk_unsupported,
   .mkdir         = blk_unsupported,
   .stat          = blk_stat,
   .closeArchive  = blk_closeArchive,
};

static PHYSFS_sint64 blk_read( struct PHYSFS_Io *io, void *buf, PHYSFS_uint64 len );
static PHYSFS_sint64 blk_write( struct PHYSFS_Io *io, const void *buffer, PHYSFS_uint64 len );
static int blk_seek( struct PHYSFS_Io *io, PHYSFS_uint64 offset );
static PHYSFS_sint64 blk_tell( struct PHYSFS_Io *io );
static PHYSFS_sint64 blk_length( struct PHYSFS_Io *io );
static struct PHYSFS_Io *blk_duplicate( struct PHYSFS_Io *io );
static int blk_flush( struct PHYSFS_Io *io );
static void blk_destroy( struct PHYSFS_Io *io );

static const PHYSFS_Io blk_emptyio = {
   .version = 0,
   .opaque  = NULL,
   .read    = blk_read,
   .write   = blk_write,
   .seek    = blk_seek,
   .tell    = blk_tell,
   .length  = blk_length,
   .duplicate = blk_duplicate,
   .flush   = blk_flush,
   .destroy = blk_destroy,
};

static const PHYSFS_Stat blk_emptystat = {
   .filesize      = 0,
   .modtime       = 0,
   .createtime    = 0,
   .accesstime    = 0,
   .filetype      = PHYSFS_FILETYPE_REGULAR,
   .readonly      = 1,
};

static int blk_enumerateCallback( void* data, const char* origdir, const char* fname )
{
   char *path;
   const char *fmt;
   size_t dir_len;
   PHYSFS_Stat stat;

   dir_len = strlen( origdir );
   fmt = dir_len && origdir[dir_len-1]=='/' ? "%s%s" : "%s/%s";
   asprintf( &path, fmt, origdir, fname );
   if (!PHYSFS_stat( path, &stat )) {
      WARN( _("PhysicsFS: Cannot stat %s: %s"), path,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      free( path );
   }
   else if (stat.filetype == PHYSFS_FILETYPE_REGULAR) {
      /* Iterate and build up matches. */
      int rc = pcre2_match( blk_re, (PCRE2_SPTR)path, strlen(path), 0, 0, blk_match, NULL );
      if (rc < 0) {
         switch (rc) {
            case PCRE2_ERROR_NOMATCH:
               free( path );
               break;
            default:
               WARN(_("Matching error %d"), rc );
               free( path );
               break;
         }
      }
      else if (rc == 0)
         free( path );
      else
         array_push_back( &blk_blacklists, path );
   }
   else if (stat.filetype == PHYSFS_FILETYPE_DIRECTORY ) {
      PHYSFS_enumerate( path, blk_enumerateCallback, data );
      free( path );
   }
   else
      free( path );
   return PHYSFS_ENUM_OK;
}

int blacklist_init (void)
{
   char buf[STRMAX];
   int errornumber, l;
   PCRE2_SIZE erroroffset;

   /* No blacklist, ignore. */
   if (array_size(blk_blacklists_re) <= 0)
      return 0;

   /* Set up the string. */
   l = 0;
   for (int i=0; i<array_size(blk_blacklists_re); i++)
      l += snprintf( &buf[l], sizeof(buf)-l-1, "%s%s", (i==0) ? "" : "|", blk_blacklists_re[i] );

   /* Try to compile the regex. */
   blk_re = pcre2_compile( (PCRE2_SPTR)buf, PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL );
   if (blk_re == NULL) {
      PCRE2_UCHAR buffer[256];
      pcre2_get_error_message( errornumber, buffer, sizeof(buffer) );
      WARN(_("Blacklist  PCRE2 compilation failed at offset %d: %s"), (int)erroroffset, buffer );
      return -1;;
   }

   /* Prepare the match data. */
   blk_match = pcre2_match_data_create_from_pattern( blk_re, NULL );

   /* Find the files and match. */
   blk_blacklists = array_create( char * );
   PHYSFS_enumerate( "/", blk_enumerateCallback, NULL );
   qsort( blk_blacklists, array_size(blk_blacklists), sizeof(char*), strsort );

   /* Free stuff up. */
   pcre2_code_free( blk_re );
   pcre2_match_data_free( blk_match );

   return PHYSFS_registerArchiver( &blk_archiver );
}

int blacklist_append( const char *path )
{
   if (blk_blacklists_re == NULL)
      blk_blacklists_re = array_create( char* );

   for (int i=0; i<array_size(blk_blacklists_re); i++)
      if (strcmp( blk_blacklists_re[i], path )==0)
         return 0;
   array_push_back( &blk_blacklists_re, strdup(path) );
   return 0;
}

int blacklist_exit (void)
{
   for (int i=0; i<array_size(blk_blacklists_re); i++)
      free( blk_blacklists_re[i] );
   array_free( blk_blacklists_re );
   blk_blacklists_re = NULL;
   for (int i=0; i<array_size(blk_blacklists); i++)
      free( blk_blacklists[i] );
   array_free( blk_blacklists );
   blk_blacklists = NULL;
   return 0;
}

static int blk_matches( const char *filename )
{
   const char *str = bsearch( filename, blk_blacklists, array_size(blk_blacklists), sizeof(const char*), strsort );
   return (str!=NULL);
}

static PHYSFS_Io *blk_unsupportedIO( void *opaque, const char *filename )
{
   (void) opaque;
   (void) filename;
   return NULL;
}

static int blk_unsupported( void *opaque, const char *filename )
{
   (void) opaque;
   (void) filename;
   return -1;
}

static void *blk_openArchive( PHYSFS_Io *io, const char *name, int forWrite, int *claimed )
{
   (void) io;
   (void) name;
   (void) forWrite;
   (void) claimed;

   return NULL;
}

static PHYSFS_EnumerateCallbackResult blk_enumerate( void *opaque, const char *dirname, PHYSFS_EnumerateCallback cb, const char *origdir, void *callbackdata )
{
   (void) opaque;
   (void) dirname;
   (void) cb;
   (void) origdir;
   (void) callbackdata;
   return 0;
}

static PHYSFS_Io *blk_openRead( void *opaque, const char *fnm )
{
   (void) opaque;
   if (blk_matches( fnm )) {
      PHYSFS_Io *io = malloc( sizeof(PHYSFS_Io) );
      *io = blk_emptyio;
      return io;
   }
   return NULL;
}

static int blk_stat( void *opaque, const char *fn, PHYSFS_Stat *stat )
{
   (void) opaque;
   if (blk_matches( fn )) {
      *stat = blk_emptystat;
      return 0;
   }
   return -1;
}

static void blk_closeArchive( void *opaque )
{
   (void) opaque;
}

static PHYSFS_sint64 blk_read( struct PHYSFS_Io *io, void *buf, PHYSFS_uint64 len )
{
   (void) io;
   (void) buf;
   (void) len;
   return 0;
}

static PHYSFS_sint64 blk_write( struct PHYSFS_Io *io, const void *buffer, PHYSFS_uint64 len )
{
   (void) io;
   (void) buffer;
   (void) len;
   return -1;
}

static int blk_seek( struct PHYSFS_Io *io, PHYSFS_uint64 offset )
{
   (void) io;
   (void) offset;
   return -1;
}

static PHYSFS_sint64 blk_tell( struct PHYSFS_Io *io )
{
   (void) io;
   return 0;
}

static PHYSFS_sint64 blk_length( struct PHYSFS_Io *io )
{
   (void) io;
   return 0;
}

static struct PHYSFS_Io *blk_duplicate( struct PHYSFS_Io *io )
{
   return io;
}

static int blk_flush( struct PHYSFS_Io *io )
{
   (void) io;
   return 0;
}

static void blk_destroy( struct PHYSFS_Io *io )
{
   (void) io;
}
