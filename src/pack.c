/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file pack.c
 *
 * @brief Stores data in funky format.
 *
 * Format Overview:
 *
 *   1.1) Index
 *     1.1.1) Magic Number (16 bytes)
 *     1.1.2) Number of Files (uint32_t)
 *     1.1.3) Files in format Name/Location
 *       1.1.3.1) File Name (128 bytes max, ended in NUL)
 *       1.1.3.2) File Location (uint32_t)
 *   1.2) File data in format Size/Data
 *     1.2.1) File Size (uint32_t)
 *     1.2,2) File Data (char*)
 *     1.2.3) File MD5 (16 byte char*)
 *   1.3) EOF
 *
 * 
 * Program Overview:
 *
 *   2.1) Write Magic Number and Number of Files (1.1 and 1.2 above)
 *   2,2) Write the Index
 *   3,3) Pack the files
 */


#include "pack.h"

#include "naev.h"

#include <stdio.h> /* printf() */
#include <fcntl.h> /* creat() and friends */
#include <stdint.h> /* uint32_t */
#if HAS_FD
#include <sys/types.h> /* ssize_t */
#include <sys/stat.h> /* S_IRUSR */
#endif /* HAS_FD */
#include <unistd.h> /* WRITE() */
#include <errno.h> /* error numbers */
#include <string.h> /* strlen() and friends */
#include <stdlib.h> /* malloc */
#if HAS_POSIX
#include <arpa/inet.h> /* ntohl */
#endif /* HAS_POSIX */
#if HAS_WIN32
#include <winsock2.h> /* ntohl */
#endif /* HAS_WIN32 */

#include "log.h"
#include "md5.h"


#if HAS_BIGENDIAN
#define htonll(x)   (x)
#define ntohll(x)   (x)
#else /* HAS_BIGENDIAN */
#define htonll(x)   ((((uint64_t)htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x)   ((((uint64_t)ntohl(x)) << 32) + ntohl(x >> 32))
#endif /* HAS_BIGENDIAN */


/**
 * @brief Abstracts around packfiles.
 */
struct Packfile_s {
#if HAS_FD
   int fd; /**< file descriptor */
#else /* not HAS_FD */
   FILE* fp; /**< For non-posix. */
#endif /* HAS_FD */
   uint32_t pos; /**< cursor position */
   uint32_t start; /**< File start. */
   uint32_t end; /**< File end. */

   uint32_t flags; /**< Special control flags. */
};


/**
 * @brief Allows much faster creation of packfiles.
 */
struct Packcache_s {
#if HAS_FD
   int fd; /**< file descriptor */
#else /* not HAS_FD */
   FILE *fp; /**< For non-posix. */
#endif /* HAS_FD */

   char *name; /**< To open the file again. */
   char **index; /**< Cached index for faster lookups. */
   uint32_t *start; /**< Cached index starts. */
   uint32_t nindex; /**< Number of index entries. */
};

/*
 * Helper defines.
 */
#if HAS_FD
#define READ(f,b,n)  if (read((f)->fd,(b),(n))!=(n)) { \
   WARN("Fewer bytes read then expected"); \
   return NULL; } /**< Helper define to check for errors. */
#else /* not HAS_FD */
#define READ(f,b,n)  if (fread((b),1,(n),(f)->fp)!=(n)) { \
   WARN("Fewer bytes read then expected"); \
   return NULL; } /**< Helper define to check for errors. */
#endif /* HAS_FD */

#undef DEBUG /* mucho spamo */
#define DEBUG(str, args...)      do {;} while(0) /**< Hack to disable debugging. */


#define BLOCKSIZE    128*1024 /**< The read/write block size. */

#ifndef PATH_MAX
#define PATH_MAX     256   /**< maximum file name length. */
#endif


#if HAS_FD
#define PERMS   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH /**< default permissions. */
#endif /* HAS_FD */


const uint64_t magic =  0x25524573; /**< File magic number: sER% */


/*
 * Flags.
 */
#define PACKFILE_FROMCACHE    (1<<0) /**< Packfile comes from a packcache. */


/**
 * Prototypes.
 */
static off_t getfilesize( const char* filename );
/* RWops stuff. */
#if SDL_VERSION_ATLEAST(1,3,0)
static long packrw_seek( SDL_RWops *rw, long offset, int whence );
static size_t packrw_read( SDL_RWops *rw, void *ptr, size_t size, size_t maxnum );
static size_t packrw_write( SDL_RWops *rw, const void *ptr, size_t size, size_t num );
#else /* SDL_VERSION_ATLEAST(1,3,0) */
static int packrw_seek( SDL_RWops *rw, int offset, int whence );
static int packrw_read( SDL_RWops *rw, void *ptr, int size, int maxnum );
static int packrw_write( SDL_RWops *rw, const void *ptr, int size, int num );
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
static int packrw_close( SDL_RWops *rw );


/**
 * @brief Opens a Packfile as a cache.
 *
 *    @param packfile Name of the packfile to cache.
 *    @return NULL if an error occured or the Packcache.
 */
Packcache_t* pack_openCache( const char* packfile )
{
   int j;
   uint32_t i;
   char buf[PATH_MAX];
   Packcache_t *cache;
   uint64_t end64;

   /*
    * Allocate memory.
    */
   cache = calloc(1, sizeof(Packcache_t));
   if (cache == NULL) {
      WARN("Out of Memory.");
      return NULL;
   }

   /*
    * Open file.
    */
   cache->name = strdup(packfile);
#if HAS_FD
   cache->fd = open( packfile, O_RDONLY );
   if (cache->fd == -1) {
#else /* not HAS_FD */
   cache->fp = fopen( packfile, "rb" );
   if (cache->fp == NULL) {
#endif /* HAS_FD */
      WARN("Erroring opening %s: %s", packfile, strerror(errno));
      return NULL;
   }

   /*
    * Check for validity.
    */
   READ( cache, buf, sizeof(magic));
   end64 = ntohll(magic);
   if (memcmp(buf, &end64, sizeof(magic))) {
      WARN("File %s is not a valid packfile", packfile);
      return NULL;
   }

   /*
    * Get number of files and allocate memory.
    */
   READ( cache, &cache->nindex, 4 );
   cache->nindex = htonl( cache->nindex );
   cache->index = calloc( cache->nindex, sizeof(char*) );
   cache->start = calloc( cache->nindex, sizeof(uint32_t) );

   /*
    * Read index.
    */
   for (i=0; i<cache->nindex; i++) { /* start to search files */
      j = 0;
      READ( cache, &buf[j], 1 ); /* get the name */
      while ( buf[j++] != '\0' )
         READ( cache, &buf[j], 1 );

      cache->index[i] = strdup(buf);
      READ( cache, &cache->start[i], 4 );
      cache->start[i] = htonl( cache->start[i] );
      DEBUG("'%s' found at %d", cache->index[i], cache->start[i]);
   }

   /*
    * Return the built cache.
    */
   return cache;
}


/**
 * @brief Closes a Packcache.
 *
 *    @param cache Packcache to close.
 */
void pack_closeCache( Packcache_t* cache )
{
   uint32_t i;

   /*
    * Close file.
    */
#if HAS_FD
   close( cache->fd );
#else /* not HAS_FD */
   fclose( cache->fp );
#endif /* HAS_FD */
   free( cache->name );

   /*
    * Free memory.
    */
   if (cache->nindex > 0) {
      for (i=0; i<cache->nindex; i++)
         free(cache->index[i]);
      free(cache->index);
      free(cache->start);
   }
   free(cache);
}


/**
 * @brief Opens a Packfile from a Packcache.
 *
 *    @param cache Packcache to create Packfile from.
 *    @param filename Name of the file to open in the Cache.
 *    @return A packfile for filename from cache.
 */
Packfile_t* pack_openFromCache( Packcache_t* cache, const char* filename )
{
   uint32_t i;
   Packfile_t *file;

   file = calloc( 1, sizeof(Packfile_t) );

   for (i=0; i<cache->nindex; i++) {
      if (strcmp(cache->index[i], filename)==0) {
         /* Copy file. */
#if HAS_FD
         file->fd = open( cache->name, O_RDONLY );
#else /* not HAS_FD */
         file->fp = fopen( cache->name, "rb" );
#endif /* HAS_FD */

         /* Copy information. */
         file->flags |= PACKFILE_FROMCACHE;
         file->start  = cache->start[i];

         /* Seek. */
         if (file->start) { /* go to the beginning of the file */
#if HAS_FD
            if ((uint32_t)lseek( file->fd, file->start, SEEK_SET ) != file->start) {
#else /* not HAS_FD */
            if (fseek( file->fp, file->start, SEEK_SET )) {
#endif /* HAS_FD */
               WARN("Failure to seek to file start: %s", strerror(errno));
               return NULL;
            }
            READ( file, &file->end, 4 );
            file->end = htonl( file->end );
            file->start += 4;
            file->pos    = file->start;
            file->end   += file->start;
            DEBUG("Opened '%s' from cache from %u to %u (%u long)", filename,
                  file->start, file->end, file->end - file->start);
         }

         return file;
      }
   }

   free(file);
   WARN("File '%s' not found in packfile.", filename);
   return NULL;
}


/**
 * @brief Gets the file's size.
 *
 *    @param filename File to get the size of.
 *    @return The size of the file.
 */
static off_t getfilesize( const char* filename )
{
#if HAS_FD
   struct stat file;

   if (!stat( filename, &file ))
      return file.st_size;

   WARN( "Unable to get filesize of %s", filename );
   return 0;
#else  /* not HAS_FD */
   long size;
   FILE* fp = fopen( filename, "rb" );
   if (fp == NULL) return 0;

   fseek( fp, 0, SEEK_END );
   size = ftell(fp);

   fclose(fp);

   return size;
#endif /* HAS_FD */
}


/**
 * @brief Checks to see if a file is a packfile.
 *
 *    @param filename Name of the file to check.
 *    @return 0 if it is a packfile, 1 if it isn't and -1 on error.
 */
int pack_check( const char* filename )
{
   int ret;
   char *buf;
   
   buf = malloc(sizeof(magic));

#if HAS_FD
   int fd = open( filename, O_RDONLY );
   if (fd == -1) {
      WARN("Erroring opening %s: %s", filename, strerror(errno));
      return -1;
   }

   if (read( fd, buf, sizeof(magic) ) != sizeof(magic)) {
      WARN("Error reading magic number: %s", strerror(errno));
      free(buf);
      return -1;
   }

   ret = (memcmp(buf,&magic,sizeof(magic))==0) ? 0 : 1 ;
   close(fd);
#else /* not HAS_FD */
   FILE* file = fopen( filename, "rb" );
   if (file == NULL) {
      WARN("Erroring opening '%s': %s", filename, strerror(errno));
      return -1;
   }

   buf = malloc(sizeof(magic));
   if (fread( buf, 1, sizeof(magic), file ) != sizeof(magic)) {
      WARN("Error reading magic number: %s", strerror(errno));
      free(buf);
      return -1;
   }

   ret = (memcmp(buf,&magic,sizeof(magic))==0) ? 0 : 1 ;
   fclose( file );
#endif /* HAS_FD */

   free(buf);

   return ret;
}


#if HAS_FD
#define WRITE(b,n)    if (write(outfd,b,n)==-1) { \
   WARN("Error writing to file: %s", strerror(errno)); \
   free(buf); return -1; } /**< Macro to help check for errors. */
#else /* not HAS_FD */
#define WRITE(b,n)    if (fwrite(b,1,n,outf)==0) { \
   WARN("Error writing to file: %s", strerror(errno)); \
   free(buf); return -1; } /**< Macro to help check for errors. */
#endif /* HAS_FD */
/**
 * @brief Packages files into a packfile.
 *
 *    @param outfile Name of the file to output to.
 *    @param infiles Array of filenames to package.
 *    @param nfiles Number of filenames in infiles.
 *    @return 0 on success.
 */
int pack_files( const char* outfile, const char** infiles, const uint32_t nfiles )
{
   void *buf;
#if HAS_FD
   struct stat file;
   int outfd, infd;
#else /* HAS_FD */
   FILE *outf, *inf;
#endif /* HAS_FD */
   uint32_t i;
   int namesize;
   uint32_t indexsize, pointer;
   int bytes;
   const uint8_t b = '\0';
   uint32_t end32;
   uint64_t end64;

   
   for (namesize=0,i=0; i < nfiles; i++) { /* make sure files exist before writing */
#if HAS_FD
      if (stat(infiles[i], &file)) {
#else /* not HAS_FD */
      if (getfilesize(infiles[i]) == 0) {
#endif /* HAS_FD */
         WARN("File %s does not exist", infiles[i]);
         return -1;
      }
      if (strlen(infiles[i]) > PATH_MAX) {
         WARN("Filename '%s' is too long, should be only %d characters",
               infiles[i], PATH_MAX );
         return -1;
      }
      namesize += strlen(infiles[i]);
   }
   indexsize = (sizeof(magic) + 4 + /* magic number and number of files */
         namesize + /* total length of file names */
         (1+4)*nfiles); /* file size and extra end of string char '\0' */
   DEBUG("Index size is %d", indexsize );

   /* creates the output file */
#if HAS_FD
   outfd = creat( outfile, PERMS );
   if (outfd == -1) {
#else /* not HAS_FD */
   outf = fopen( outfile, "wb" );
   if (outf == NULL) {
#endif /* HAS_FD */
      WARN("Unable to open %s for writing", outfile);
      return -1;
   }

   /*
    * INDEX
    */
   buf = malloc(BLOCKSIZE);
   /* magic number */
   end64 = htonll(magic);
   WRITE( &end64, sizeof(magic));
   DEBUG("Wrote magic number");
   /* number of files */
   end32 = htonl(nfiles);
   WRITE( &end32, sizeof(nfiles));
   DEBUG("Wrote number of files: %d", nfiles);
   /* create file dependent index part */
   pointer = indexsize;
   for (i=0; i<nfiles; i++) {
      WRITE( infiles[i], strlen(infiles[i]) );
      DEBUG("File '%s' at %d", infiles[i], pointer);
      WRITE( &b, 1 );
      end32 = htonl(pointer);
      WRITE( &end32, 4 );
      pointer += 4 + getfilesize( infiles[i] ) + 16; /* set pointer to be next file pos */
   }
   /*
    * DATA
    */
   md5_state_t md5;
   md5_byte_t *md5val = malloc(16);
   for (i=0; i<nfiles; i++) {
      bytes = (uint32_t)getfilesize( infiles[i] );
      end32 = htonl(bytes);
      WRITE( &end32, 4 ); /* filesize */
      DEBUG("About to write file '%s' of %d bytes", infiles[i], bytes);
      md5_init(&md5);
#if HAS_FD
      infd = open( infiles[i], O_RDONLY );
      while ((bytes = read( infd, buf, BLOCKSIZE ))) {
#else /* not HAS_FD */
      inf = fopen( infiles[i], "rb" );
      while ((bytes = fread( buf, 1, BLOCKSIZE, inf ))) {
#endif /* HAS_FD */
         WRITE( buf, bytes ); /* data */
         md5_append( &md5, buf, bytes );
      }
      md5_finish(&md5, md5val);
      WRITE( md5val, 16 );
#if HAS_FD
      close(infd);
#else /* not HAS_FD */
      fclose(inf);
#endif /* HAS_FD */
      DEBUG("Wrote file '%s'", infiles[i]);
   }
   free(md5val);

#if HAS_FD
   close( outfd );
#else /* not HAS_FD */
   fclose( outf );
#endif /* HAS_FD */
   free(buf);

   DEBUG("Packfile success\n\t%d files\n\t%d bytes", nfiles, (int)getfilesize(outfile));
   return 0;
}
#undef WRITE


/**
 * @brief Opens a file in the packfile for reading.
 *
 *    @param packfile Path to the real packfile.
 *    @param filename Name of the file within th. packfile.
 *    @return The newly created packfile or NULL on error.
 */
Packfile_t* pack_open( const char* packfile, const char* filename )
{
   int j;
   uint32_t nfiles, i;
   char buf[PATH_MAX];
   Packfile_t *file;
   uint64_t end64;

   /* Allocate memory. */
   file = malloc(sizeof(Packfile_t));
   memset( file, 0, sizeof(Packfile_t) );

#if HAS_FD
   file->fd = open( packfile, O_RDONLY );
   if (file->fd == -1) {
#else /* not HAS_FD */
   file->fp = fopen( packfile, "rb" );
   if (file->fp == NULL) {
#endif /* HAS_FD */
      WARN("Erroring opening %s: %s", filename, strerror(errno));
      return NULL;
   }

   READ( file, buf, sizeof(magic)); /* make sure it's a packfile */
   end64 = ntohll(magic);
   if (memcmp(buf, &end64, sizeof(magic))) {
      WARN("File %s is not a valid packfile", filename);
      return NULL;
   }

   READ( file, &nfiles, 4 );
   nfiles = htonl(nfiles);
   for (i=0; i<nfiles; i++) { /* start to search files */
      j = 0;
      READ( file, &buf[j], 1 ); /* get the name */
      while ( buf[j++] != '\0' )
         READ( file, &buf[j], 1 );

      if (strcmp(filename, buf)==0) { /* found file */
         READ( file, &file->start, 4 );
         file->start = htonl( file->start );
         DEBUG("'%s' found at %d", filename, file->start);
         break;
      }
#if HAS_FD
      lseek( file->fd, 4, SEEK_CUR ); /* ignore the file location */
#else /* not HAS_FD */
      fseek( file->fp, 4, SEEK_CUR );
#endif /* HAS_FD */
   }
   
   if (file->start) { /* go to the beginning of the file */
#if HAS_FD
      if ((uint32_t)lseek( file->fd, file->start, SEEK_SET ) != file->start) {
#else /* not HAS_FD */
      if (fseek( file->fp, file->start, SEEK_SET )) {
#endif /* HAS_FD */
         WARN("Failure to seek to file start: %s", strerror(errno));
         return NULL;
      }
      READ( file, &file->end, 4 );
      file->end = htonl( file->end );
      DEBUG("\t%d bytes", file->end);
      file->start += 4;
      file->pos    = file->start;
      file->end   += file->start;
   }
   else {
      WARN("File '%s' not found in packfile '%s'", filename, packfile);
      return NULL;
   }

   return file;
}


/**
 * @brief Reads data from a packfile.
 *
 * Behaves like FD read.
 *
 *    @param file Opened packfile to read data from.
 *    @param buf Allocated buffer to read into.
 *    @param count Bytes to read.
 *    @return Bytes read or -1 on error.
 */
ssize_t pack_read( Packfile_t* file, void* buf, size_t count )
{
   int bytes;

   if ((file->pos + count) > file->end)
      count = file->end - file->pos; /* can't go past end */
   if (count == 0)
      return 0;

#if HAS_FD
   if ((bytes = read( file->fd, buf, count )) == -1) {
#else /* not HAS_FD */
   if ((bytes = fread( buf, 1, count, file->fp)) == -1) {
#endif /* HAS_FD */
      WARN("Error while reading file: %s", strerror(errno));
      return -1;
   }
   file->pos += bytes;

   DEBUG("Read %d bytes from packfile: offset = %u", bytes, file->pos);
   DEBUG("start: %u, pos: %u, end: %u -> %d", file->start, file->pos, file->end,
         bytes );

   return bytes;
}


/**
 * @brief Seeks within a file inside a packfile.
 *
 * Behaves like lseek/fseek.
 *
 *    @param file File to seek.
 *    @param offset Position to seek to.
 *    @param whence Either SEEK_SET, SEEK_CUR or SEEK_END.
 *    @return The position moved to.
 */
off_t pack_seek( Packfile_t* file, off_t offset, int whence)
{
   uint32_t base, target, ret;

   DEBUG("attempting to seek offset: %ld, whence: %d", offset, whence);

   /* Find where offset is relative to. */
   switch (whence) {
      case SEEK_SET:
         base = file->start;
         break;

      case SEEK_CUR:
         base = file->pos;
         break;

      case SEEK_END:
         base = file->end;
         break;

      default:
         WARN("Whence is not one of SEEK_SET, SEEK_CUR or SEEK_END");
         return -1;
   }

   /* Get the target. */
   target = base + offset;

   /* Limit checks. */
   if ((target < file->start) || (target >= file->end))
      return -1;

#if HAS_FD
   ret = lseek( file->fd, target, SEEK_SET );
   if (ret != target)
      return -1;
#else /* not HAS_FD */
   ret = fseek( file->fp, target, SEEK_SET );
   if (ret != 0)
      return -1;
   ret = target; /* fseek returns 0. */
#endif /* HAS_FD */

   /* Set the position in the file. */
   file->pos = ret;
   DEBUG("start: %u, pos: %u, end: %u -> %u", file->start, file->pos, file->end,
         file->pos - file->start );

   return file->pos - file->start;
}


/**
 * @brief Gets the current position in the file.
 *
 *    @param file Packfile to get the position from.
 *    @return The current position in the file.
 */
long pack_tell( Packfile_t* file )
{
   return file->pos - file->start;
}


/**
 * @brief Reads a file from a Packfile.
 */
static void* pack_readfilePack( Packfile_t *file,
      const char* filename, uint32_t *filesize )
{
   void* buf;
   char* str;
   int size, bytes;

   /* read the entire file */
   size = file->end - file->start;
   buf = malloc( size + 1 );
   if (buf == NULL) {
      WARN("Unable to allocate %d bytes of memory!", size+1);
      free(file);
      return NULL;
   }
   if ((bytes = pack_read( file, buf, size)) != size) {
      WARN("Reading '%s' from packfile.  Expected %d bytes got %d bytes",
            filename, size, bytes );
      free(buf);
      free(file);
      return NULL;
   }
   DEBUG("Read %d bytes from '%s'", bytes, filename );
   str = buf;
   str[size] = '\0'; /* append size '\0' for it to validate as a string */

   /* check the md5 */
   md5_state_t md5;
   md5_byte_t *md5val = malloc(16);
   md5_byte_t *md5fd = malloc(16);
   md5_init(&md5);
   md5_append( &md5, buf, bytes );
   md5_finish(&md5, md5val);
#if HAS_FD
   if ((bytes = read( file->fd, md5fd, 16 )) == -1)
#else /* not HAS_FD */
   if ((bytes = fread( md5fd, 1, 16, file->fp )) == -1)
#endif /* HAS_FD */
      WARN("Failure to read MD5, continuing anyways...");
   else if (memcmp( md5val, md5fd, 16 ))
      WARN("MD5 gives different value, possible memory corruption, continuing...");
   free(md5val);
   free(md5fd);


   /* cleanup */
   if (pack_close( file ) == -1) {
      WARN("Closing packfile");
      free(file);
      return NULL;
   }
   DEBUG("Closed '%s' in packfile", filename );

   if (filesize)
      *filesize = size;
   return buf;
}


/**
 * @brief Reads an entire file into memory.
 *
 *    @param packfile Name of the packfile to read frome.
 *    @param filename Name of the packed file to read.
 *    @param filesize Is set to the size of the file.
 *    @return A pointer to the data in the file or NULL if an error occurred.
 */
void* pack_readfile( const char* packfile, const char* filename, uint32_t *filesize )
{
   Packfile_t* file;

   /* Initialize size to 0. */
   if (filesize)
      *filesize = 0;

   /* Open the packfile. */
   file = pack_open( packfile, filename );
   if (file == NULL) {
      WARN("Opening packfile '%s'.", packfile);
      return NULL;
   }
   DEBUG("Opened file '%s' from '%s'", filename, packfile );

   return pack_readfilePack( file, filename, filesize );
}


/**
 * @brief char** pack_listfiles( const char* packfile, uint32_t* nfiles )
 *
 * @brief Gets what files are in the packfile.
 *
 * Each name must be freed individually afterwarsd and the array of names too.
 *
 *    @param packfile Packfile to query it's internal files.
 *    @param nfiles Stores the amount of files in packfile.
 *    @return An array of filenames in packfile.
 */
char** pack_listfiles( const char* packfile, uint32_t* nfiles )
{
   int j;
   uint32_t i;
   Packfile_t file;
   char** filenames;
   char* buf = malloc(sizeof(magic));
   uint64_t end64;

   *nfiles = 0;

#if HAS_FD
   file.fd = open( packfile, O_RDONLY );
   if (file.fd == -1) {
#else /* not HAS_FD */
   file.fp = fopen( packfile, "rb" );
   if (file.fp == NULL) {
#endif /* HAS_FD */
      WARN("Erroring opening %s: %s", packfile, strerror(errno));
      return NULL;
   }

   READ( &file, buf, sizeof(magic)); /* make sure it's a packfile */
   end64 = ntohll( magic );
   if (memcmp(buf, &end64, sizeof(magic))) {
      WARN("File %s is not a valid packfile", packfile);
      return NULL;
   }

   READ( &file, nfiles, 4 );
   *nfiles = htonl( *nfiles );
   filenames = malloc(((*nfiles)+1)*sizeof(char*));
   for (i=0; i<*nfiles; i++) { /* start to search files */
      j = 0;
      filenames[i] = malloc(PATH_MAX*sizeof(char));
      READ( &file, &filenames[i][j], 1 ); /* get the name */
      while ( filenames[i][j++] != '\0' )
         READ( &file, &filenames[i][j], 1 );
      READ( &file, buf, 4 ); /* skip the location */
   }
   free(buf);
#if HAS_FD
   close(file.fd);
#else /* not HAS_FD */
   fclose(file.fp);
#endif /* HAS_FD */

   return filenames;
}


/**
 * @brief Reads an entire file from the cache.
 */
void* pack_readfileCached( Packcache_t* cache, const char* filename, uint32_t *filesize )
{
   Packfile_t *file;

   file = pack_openFromCache( cache, filename );
   if (file == NULL) {
      WARN("Unable to create packfile from packcache.");
      return NULL;
   }
   return pack_readfilePack( file, filename, filesize );
}


/**
 * @brief Gets the list of files en a Packcache.
 *
 *    @param cache Cache to get list of files from.
 *    @param nfiles Number of files in the list.
 *    @return A read only list of files from the pack cache.
 */
const char** pack_listfilesCached( Packcache_t* cache, uint32_t* nfiles )
{
   *nfiles = cache->nindex;
   return (const char**) cache->index;
}


/**
 * @brief Closes a packfile.
 *
 *    @param file Packfile to close.
 *    @return 0 on success.
 */
int pack_close( Packfile_t* file )
{
   int i;

   /* Close files. */
#if HAS_FD
   i = close( file->fd );
#else /* not HAS_FD */
   i = fclose( file->fp );
#endif /* HAS_FD */

   /* Free memory. */
   free(file);

   DEBUG("Closing packfile.");

   return (i) ? -1 : 0 ;
}


#if SDL_VERSION_ATLEAST(1,3,0)
static long packrw_seek( SDL_RWops *rw, long offset, int whence )
#else /* SDL_VERSION_ATLEAST(1,3,0) */
static int packrw_seek( SDL_RWops *rw, int offset, int whence )
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
{
   int wh;
   Packfile_t *packfile;
   packfile = rw->hidden.unknown.data1;

   if (whence == RW_SEEK_SET)
      wh = SEEK_SET;
   else if (whence == RW_SEEK_CUR)
      wh = SEEK_CUR;
   else if (whence == RW_SEEK_END)
      wh = SEEK_END;
   else
      return -1;

   return pack_seek( packfile, offset, whence );
}
#if SDL_VERSION_ATLEAST(1,3,0)
static size_t packrw_read( SDL_RWops *rw, void *ptr, size_t size, size_t maxnum )
#else /* SDL_VERSION_ATLEAST(1,3,0) */
static int packrw_read( SDL_RWops *rw, void *ptr, int size, int maxnum )
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
{
   ssize_t ret;
   Packfile_t *packfile;
   packfile = rw->hidden.unknown.data1;

   /* Read the data. */
   ret = pack_read( packfile, ptr, size*maxnum );

   return ret / size;
}
#if SDL_VERSION_ATLEAST(1,3,0)
static size_t packrw_write( SDL_RWops *rw, const void *ptr, size_t size, size_t num )
#else /* SDL_VERSION_ATLEAST(1,3,0) */
static int packrw_write( SDL_RWops *rw, const void *ptr, int size, int num )
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
{
   (void) rw;
   (void) ptr;
   (void) size;
   (void) num;
   return -1;
}
static int packrw_close( SDL_RWops *rw )
{
   Packfile_t *packfile;
   packfile = rw->hidden.unknown.data1;

   /* Free the data. */
   SDL_FreeRW(rw);

   return pack_close( packfile );
}


/**
 * @brief Creates a rwops from a packfile.
 *
 *    @param packfile Packfile to create rwops from.
 *    @return rwops created from packfile.
 */
static SDL_RWops *pack_rwopsRaw( Packfile_t *packfile )
{
   SDL_RWops *rw;

   /* Create the rwops. */
   rw = SDL_AllocRW();
   if (rw == NULL) {
      WARN("Unable to allocate SDL_RWops.");
      return NULL;
   }

   /* Set the functions. */
   rw->seek  = packrw_seek;
   rw->read  = packrw_read;
   rw->write = packrw_write;
   rw->close = packrw_close;

   /* Set the packfile as the hidden data. */
   rw->hidden.unknown.data1 = packfile;

   return rw;
}


/**
 * @brief Creates an rwops for a file in a packfile.
 *
 *    @param packfile Packfile to get file from.
 *    @param filename File within packfile to create rwops from.
 *    @return SDL_RWops interacting with the file in the packfile.
 */
SDL_RWops *pack_rwops( const char* packfile, const char* filename )
{
   Packfile_t *pack;

   /* Open the packfile. */
   pack = pack_open( packfile, filename );
   if (pack == NULL)
      return NULL;

   /* Return the rwops. */
   return pack_rwopsRaw( pack );
}


/**
 * @brief Creates an rwops for a file in a packcache.
 *
 *    @param cache Packcache to get file from.
 *    @param filename File within the cache to create rwops from.
 *    @return SDL_RWops interacting with the file in the packcache.
 */
SDL_RWops *pack_rwopsCached( Packcache_t* cache, const char* filename )
{
   Packfile_t *packfile;

   /* Open the packfile. */
   packfile = pack_openFromCache( cache, filename );
   if (packfile == NULL)
      return NULL;

   /* Return the rwops. */
   return pack_rwopsRaw( packfile );
}

