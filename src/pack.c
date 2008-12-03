/*
 * See Licensing and Copyright notice in naev.h
 */


#include "pack.h"

#include <stdio.h> /* printf() */
#include <sys/stat.h> /* S_IRUSR */
#include <unistd.h> /* WRITE() */
#include <errno.h> /* error numbers */
#include <string.h> /* strlen() and friends */
#include <stdlib.h> /* malloc */

#include "log.h"
#include "md5.h"


/**
 * @file pack.c
 *
 * @brief Stores data in funky format.
 *
 *
 * Format Overview:
 *
 *   1.1) Index
 *     1.1.1) Magic Number (16 bytes)
 *     1.1.2) Number of Files (uint32_t)
 *     1.1.3) Files in format Name/Location
 *       1.1.3.1) File Name (128 bytes max, ended in \0)
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



/**
 * @brief Abstracts around packfiles.
 */
struct Packfile_s {
#ifdef _POSIX_SOURCE
   int fd; /**< file descriptor */
#else /* not _POSIX_SOURCE */
   FILE* fp; /**< For non-posix. */
#endif /* _POSIX_SOURCE */
   uint32_t pos; /**< cursor position */
   uint32_t start; /**< File start. */
   uint32_t end; /**< File end. */

   uint32_t flags; /**< Special control flags. */
};


/**
 * @brief Allows much faster creation of packfiles.
 */
struct Packcache_s {
#ifdef _POSIX_SOURCE
   int fd; /**< file descriptor */
#else /* not _POSIX_SOURCE */
   FILE* fp; /**< For non-posix. */
#endif /* _POSIX_SOURCE */

   char **index; /**< Cached index for faster lookups. */
   uint32_t *start; /**< Cached index starts. */
   uint32_t nindex; /**< Number of index entries. */
};

/*
 * Helper defines.
 */
#ifdef _POSIX_SOURCE
#define READ(f,b,n)  if (read((f)->fd,(b),(n))!=(n)) { \
   ERR("Fewer bytes read then expected"); \
   return NULL; }
#else /* not _POSIX_SOURCE */
#define READ(f,b,n)  if (fread((b),1,(n),(f)->fp)!=(n)) { \
   ERR("Fewer bytes read then expected"); \
   return NULL; }
#endif /* _POSIX_SOURCE */

#undef DEBUG /* mucho spamo */
#define DEBUG(str, args...)      do {;} while(0)


#define BLOCKSIZE    128*1024 /**< The read/write block size. */

#ifndef PATH_MAX
#define PATH_MAX     256   /**< maximum file name length. */
#endif


#define PERMS   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH /**< default permissions. */


const uint64_t magic =  0x25524573; /**< File magic number: sER% */


/*
 * Flags.
 */
#define PACKFILE_FROMCACHE    (1<<0) /**< Packfile comes from a packcache. */


/**
 * Prototypes.
 */
static off_t getfilesize( const char* filename );


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

   /*
    * Allocate memory.
    */
   cache = calloc(1, sizeof(Packcache_t));
   if (cache == NULL) {
      ERR("Out of Memory.");
      return NULL;
   }

   /*
    * Open file.
    */
#ifdef _POSIX_SOURCE
   cache->fd = open( packfile, O_RDONLY );
   if (cache->fd == -1) {
#else /* not _POSIX_SOURCE */
   cache->fp = fopen( packfile, "rb" );
   if (cache>fp == NULL) {
#endif /* _POSIX_SOURCE */
      ERR("Erroring opening %s: %s", packfile, strerror(errno));
      return NULL;
   }

   /*
    * Check for validity.
    */
   READ( cache, buf, sizeof(magic));
   if (memcmp(buf, &magic, sizeof(magic))) {
      ERR("File %s is not a valid packfile", packfile);
      return NULL;
   }

   /*
    * Get number of files and allocate memory.
    */
   READ( cache, &cache->nindex, 4 );
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
      DEBUG("'%s' found at %d", filename, cache->start[i]);
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
#ifdef _POSIX_SOURCE
   close( cache->fd );
#else /* not _POSIX_SOURCE */
   fclose( cache->fp );
#endif /* _POSIX_SOURCE */

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
#ifdef _POSIX_SOURCE
         file->fd = dup(cache->fd);
#else
         file->fp = cache->fp;
#endif

         /* Copy information. */
         file->flags |= PACKFILE_FROMCACHE;
         file->start  = cache->start[i];

         /* Seek. */
         if (file->start) { /* go to the beginning of the file */
#ifdef _POSIX_SOURCE
            if ((uint32_t)lseek( file->fd, file->start, SEEK_SET ) != file->start) {
#else /* not _POSIX_SOURCE */
            fseek( file->fp, file->start, SEEK_SET );
            if (errno) {
#endif /* _POSIX_SOURCE */
               ERR("Failure to seek to file start: %s", strerror(errno));
               return NULL;
            }
            READ( file, &file->end, 4 );
            DEBUG("\t%d bytes", file->end);
            file->pos = file->start;
            file->end += file->start;
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
#ifdef _POSIX_SOURCE
   struct stat file;

   if (!stat( filename, &file ))
      return file.st_size;

   ERR( "Unable to get filesize of %s", filename );
   return 0;
#else  /* not _POSIX_SOURCE */
   long size;
   FILE* fp = fopen( filename, "rb" );
   if (fp == NULL) return 0;

   fseek( fp, 0, SEEK_END );
   size = ftell(fp);

   fclose(fp);

   return size;
#endif /* _POSIX_SOURCE */
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

#ifdef _POSIX_SOURCE
   int fd = open( filename, O_RDONLY );
   if (fd == -1) {
      ERR("Erroring opening %s: %s", filename, strerror(errno));
      return -1;
   }

   if (read( fd, buf, sizeof(magic) ) != sizeof(magic)) {
      ERR("Error reading magic number: %s", strerror(errno));
      free(buf);
      return -1;
   }

   ret = (memcmp(buf,&magic,sizeof(magic))==0) ? 0 : 1 ;
   close(fd);
#else /* not _POSIX_SOURCE */
   FILE* file = fopen( filename, "rb" );
   if (file == NULL) {
      ERR("Erroring opening '%s': %s", filename, strerror(errno));
      return -1;
   }

   buf = malloc(sizeof(magic));
   if (fread( buf, 1, sizeof(magic), file ) != sizeof(magic)) {
      ERR("Error reading magic number: %s", strerror(errno));
      free(buf);
      return -1;
   }

   ret = (memcmp(buf,&magic,sizeof(magic))==0) ? 0 : 1 ;
   fclose( file );
#endif /* _POSIX_SOURCE */

   free(buf);

   return ret;
}


/**
 * @brief Packages files into a packfile.
 *
 *    @param outfile Name of the file to output to.
 *    @param infiles Array of filenames to package.
 *    @param nfiles Number of filenames in infiles.
 *    @return 0 on success.
 */
#ifdef _POSIX_SOURCE
#define WRITE(b,n)    if (write(outfd,b,n)==-1) { \
   ERR("Error writing to file: %s", strerror(errno)); \
   free(buf); return -1; }
#else /* not _POSIX_SOURCE */
#define WRITE(b,n)    if (fwrite(b,1,n,outf)==0) { \
   ERR("Error writing to file: %s", strerror(errno)); \
   free(buf); return -1; }
#endif /* _POSIX_SOURCE */
int pack_files( const char* outfile, const char** infiles, const uint32_t nfiles )
{
   void *buf;
#ifdef _POSIX_SOURCE
   struct stat file;
   int outfd, infd;
#else /* _POSIX_SOURCE */
   FILE *outf, *inf;
#endif /* _POSIX_SOURCE */
   uint32_t i;
   int namesize;
   uint32_t indexsize, pointer;
   int bytes;
   const uint8_t b = '\0';

   
   for (namesize=0,i=0; i < nfiles; i++) { /* make sure files exist before writing */
#ifdef _POSIX_SOURCE
      if (stat(infiles[i], &file)) {
#else /* not _POSIX_SOURCE */
      if (getfilesize(infiles[i]) == 0) {
#endif /* _POSIX_SOURCE */
         ERR("File %s does not exist", infiles[i]);
         return -1;
      }
      if (strlen(infiles[i]) > PATH_MAX) {
         ERR("Filename '%s' is too long, should be only %d characters",
               infiles[i], PATH_MAX );
         return -1;
      }
      namesize += strlen(infiles[i]);
   }
   indexsize = (sizeof(magic)+ 4 + /* magic number and number of files */
         namesize + /* total length of file names */
         (1+4)*nfiles); /* file size and extra end of string char '\0' */
   DEBUG("Index size is %d", indexsize );

   /* creates the output file */
#ifdef _POSIX_SOURCE
   outfd = creat( outfile, PERMS );
   if (outfd == -1) {
#else /* not _POSIX_SOURCE */
   outf = fopen( outfile, "wb" );
   if (outf == NULL) {
#endif /* _POSIX_SOURCE */
      ERR("Unable to open %s for writing", outfile);
      return -1;
   }

   /*
    * INDEX
    */
   buf = malloc(BLOCKSIZE);
   /* magic number */
   WRITE( &magic, sizeof(magic));
   DEBUG("Wrote magic number");
   /* number of files */
   WRITE( &nfiles, sizeof(nfiles));
   DEBUG("Wrote number of files: %d", nfiles);
   /* create file dependent index part */
   pointer = indexsize;
   for (i=0; i<nfiles; i++) {
      WRITE( infiles[i], strlen(infiles[i]) );
      DEBUG("Fie '%s' at %d", infiles[i], pointer);
      WRITE( &b, 1 );
      WRITE( &pointer, 4 );
      pointer += 4 + getfilesize( infiles[i] ) + 16; /* set pointer to be next file pos */
   }
   /*
    * DATA
    */
   md5_state_t md5;
   md5_byte_t *md5val = malloc(16);
   for (i=0; i<nfiles; i++) {
      bytes = (uint32_t)getfilesize( infiles[i] );
      WRITE( &bytes, 4 ); /* filesize */
      DEBUG("About to write file '%s' of %d bytes", infiles[i], bytes);
      md5_init(&md5);
#ifdef _POSIX_SOURCE
      infd = open( infiles[i], O_RDONLY );
      while ((bytes = read( infd, buf, BLOCKSIZE ))) {
#else /* not _POSIX_SOURCE */
      inf = fopen( infiles[i], "rb" );
      while ((bytes = fread( buf, 1, BLOCKSIZE, inf ))) {
#endif /* _POSIX_SOURCE */
         WRITE( buf, bytes ); /* data */
         md5_append( &md5, buf, bytes );
      }
      md5_finish(&md5, md5val);
      WRITE( md5val, 16 );
#ifdef _POSIX_SOURCE
      close(infd);
#else /* not _POSIX_SOURCE */
      fclose(inf);
#endif /* _POSIX_SOURCE */
      DEBUG("Wrote file '%s'", infiles[i]);
   }
   free(md5val);

#ifdef _POSIX_SOURCE
   close( outfd );
#else /* not _POSIX_SOURCE */
   fclose( outf );
#endif /* _POSIX_SOURCE */
   free(buf);

   DEBUG("Packfile success\n\t%d files\n\t%d bytes", nfiles, (int)getfilesize(outfile));
   return 0;
}
#undef WRITE


/**
 * @brief Opens a file in the packfile for reading.
 *
 *    @param file Packfile to store data into.
 *    @param packfile Path to the real packfile.
 *    @param filename Name of the file within th. packfile.
 *    @return 0 on success.
 */
Packfile_t* pack_open( const char* packfile, const char* filename )
{
   int j;
   uint32_t nfiles, i;
   char buf[PATH_MAX];
   Packfile_t *file;

   /* Allocate memory. */
   file = malloc(sizeof(Packfile_t));
   memset( file, 0, sizeof(Packfile_t) );

#ifdef _POSIX_SOURCE
   file->fd = open( packfile, O_RDONLY );
   if (file->fd == -1) {
#else /* not _POSIX_SOURCE */
   file->fp = fopen( packfile, "rb" );
   if (file->fp == NULL) {
#endif /* _POSIX_SOURCE */
      ERR("Erroring opening %s: %s", filename, strerror(errno));
      return NULL;
   }

   READ( file, buf, sizeof(magic)); /* make sure it's a packfile */
   if (memcmp(buf, &magic, sizeof(magic))) {
      ERR("File %s is not a valid packfile", filename);
      return NULL;
   }

   READ( file, &nfiles, 4 );
   for (i=0; i<nfiles; i++) { /* start to search files */
      j = 0;
      READ( file, &buf[j], 1 ); /* get the name */
      while ( buf[j++] != '\0' )
         READ( file, &buf[j], 1 );

      if (strcmp(filename, buf)==0) { /* found file */
         READ( file, &file->start, 4 );
         DEBUG("'%s' found at %d", filename, file->start);
         break;
      }
#ifdef _POSIX_SOURCE
      lseek( file->fd, 4, SEEK_CUR ); /* ignore the file location */
#else /* not _POSIX_SOURCE */
      fseek( file->fp, 4, SEEK_CUR );
#endif /* _POSIX_SOURCE */
   }
   
   if (file->start) { /* go to the beginning of the file */
#ifdef _POSIX_SOURCE
      if ((uint32_t)lseek( file->fd, file->start, SEEK_SET ) != file->start) {
#else /* not _POSIX_SOURCE */
      fseek( file->fp, file->start, SEEK_SET );
      if (errno) {
#endif /* _POSIX_SOURCE */
         ERR("Failure to seek to file start: %s", strerror(errno));
         return NULL;
      }
      READ( file, &file->end, 4 );
      DEBUG("\t%d bytes", file->end);
      file->pos = file->start;
      file->end += file->start;
   }
   else {
      ERR("File '%s' not found in packfile '%s'", filename, packfile);
      return NULL;
   }

   return file;
}


/**
 * @brief Reads data from a packfile.
 *
 * Behaves like POSIX read.
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
   if (count == 0) return 0;

#ifdef _POSIX_SOURCE
   if ((bytes = read( file->fd, buf, count )) == -1) {
#else /* not _POSIX_SOURCE */
   if ((bytes = fread( buf, 1, count, file->fp)) == -1) {
#endif /* _POSIX_SOURCE */
      ERR("Error while reading file: %s", strerror(errno));
      return -1;
   }
   file->pos += bytes;

   return bytes;
}


/**
 * @brief Seeks within a file inside a packfile.
 *
 * Behaves like lseek/fseek.
 *
 * @todo It's broken, needs fixing.
 *
 *    @param file File to seek.
 *    @param offset Position to seek to.
 *    @param whence Either SEEK_SET, SEEK_CUR or SEEK_END.
 *    @return The position moved to.
 */
off_t pack_seek( Packfile_t* file, off_t offset, int whence)
{
   DEBUG("attempting to seek offset: %d, whence: %d", offset, whence);
   off_t ret;
   switch (whence) {
#ifdef _POSIX_SOURCE
      case SEEK_SET:
         if ((file->start + offset) > file->end) return -1;
         ret = lseek( file->fd, file->start + offset, SEEK_SET );
         if (ret != ((off_t)file->start + offset)) return -1;
         break;
      case SEEK_CUR:
         if ((file->pos + offset) > file->end) return -1;
         ret = lseek( file->fd, file->pos + offset, SEEK_SET );
         if (ret != ((off_t)file->pos + offset)) return -1;
         break;
      case SEEK_END:
         if ((file->end - offset) < file->start) return -1;
         ret = lseek( file->fd, file->end - offset - 1, SEEK_SET );
         if (ret != ((off_t)file->end - offset)) return -1;
         break;
#else /* not _POSIX_SOURCE */
      case SEEK_SET:
         if ((file->start + offset) > file->end) return -1;
         ret = fseek( file->fp, file->start + offset, SEEK_SET );
         if (ret == -1) return -1;
         break;
      case SEEK_CUR:
         if ((file->pos + offset) > file->end) return -1;
         ret = fseek( file->fp, file->pos + offset, SEEK_SET );
         if (ret == -1) return -1;
         break;
      case SEEK_END:
         if ((file->end - offset) < file->start) return -1;
         ret = fseek( file->fp, file->end - offset, SEEK_SET );
         if (ret == -1) return -1;
         break;
#endif /* _POSIX_SOURCE */

      default:
         ERR("Whence is not one of SEEK_SET, SEEK_CUR or SEEK_END");
         return -1;
   }
   return ret - file->start;
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
      ERR("Unable to allocate %d bytes of memory!", size+1);
      free(file);
      return NULL;
   }
   if ((bytes = pack_read( file, buf, size)) != size) {
      ERR("Reading '%s' from packfile.  Expected %d bytes got %d bytes",
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
#ifdef _POSIX_SOURCE
   if ((bytes = read( file->fd, md5fd, 16 )) == -1)
#else /* not _POSIX_SOURCE */
   if ((bytes = fread( md5fd, 1, 16, file->fp )) == -1)
#endif /* _POSIX_SOURCE */
      WARN("Failure to read MD5, continuing anyways...");
   else if (memcmp( md5val, md5fd, 16 ))
      WARN("MD5 gives different value, possible memory corruption, continuing...");
   free(md5val);
   free(md5fd);


   /* cleanup */
   if (pack_close( file ) == -1) {
      ERR("Closing packfile");
      free(file);
      return NULL;
   }
   DEBUG("Closed '%s' in '%s'", filename, packfile );

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
      ERR("Opening packfile '%s'.", packfile);
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

   *nfiles = 0;

#ifdef _POSIX_SOURCE
   file.fd = open( packfile, O_RDONLY );
   if (file.fd == -1) {
#else /* not _POSIX_SOURCE */
   file.fp = fopen( packfile, "rb" );
   if (file.fp == NULL) {
#endif /* _POSIX_SOURCE */
      ERR("Erroring opening %s: %s", packfile, strerror(errno));
      return NULL;
   }

   READ( &file, buf, sizeof(magic)); /* make sure it's a packfile */
   if (memcmp(buf, &magic, sizeof(magic))) {
      ERR("File %s is not a valid packfile", packfile);
      return NULL;
   }

   READ( &file, nfiles, 4 );
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
#ifdef _POSIX_SOURCE
   close(file.fd);
#else /* not _POSIX_SOURCE */
   fclose(file.fp);
#endif /* _POSIX_SOURCE */

   return filenames;
}


/**
 * @brief Reads an entire file from the cache.
 */
void* pack_readfileCached( Packcache_t* cache, const char* filename, uint32_t *filesize )
{
   Packfile_t *file;

   file = pack_openFromCache( cache, filename );
   if (file == NULL)
      ERR("Unable to create packfile from packcache.");
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
#ifdef _POSIX_SOURCE
   i = close( file->fd );
#else /* not _POSIX_SOURCE */
   if (file->flags & PACKFILE_FROMCACHE)
      i = 0;
   else
      i = fclose( file->fp );
#endif /* _POSIX_SOURCE */

   /* Free memory. */
   free(file);

   return (i) ? -1 : 0 ;
}


