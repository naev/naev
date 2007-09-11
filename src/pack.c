/*
 * See Licensing and Copyright notice in naev.h
 */


#include "pack.h"

#include <stdio.h> /* printf() */
#include <sys/stat.h> /* S_IRUSR */
#include <unistd.h> /* WRITE() */
#include <errno.h> /* error numbers */
#include <string.h> /* strlen() and friends */
#include <malloc.h> /* malloc */

#include "log.h"
#include "md5.h"


/*
 * STORES DATA IN FUNKY FORMAT
 *
 * Format Overview:
 *
 *   1.1) Index (in 512 byte chunks)
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


#undef DEBUG /* mucho spamo */
#define DEBUG(str, args...)		do {;} while(0)


/* the read/WRITE block size */
#define BLOCKSIZE 	128*1024

/* maximum filename length */
#define MAX_FILENAME	100


#define PERMS	 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH


const uint64_t magic =  0x25524573; /* sER% */


/*
 * gets the file's size
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


/*
 * returns true if filename is a Packfile
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


/* 
 * packs nfiles infiles into outfile
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

	for (namesize=0,i=0; i < nfiles; i++) {/* make sure files exist before writing */
#ifdef _POSIX_SOURCE
		if (stat(infiles[i], &file)) {
#else /* not _POSIX_SOURCE */
		if (getfilesize(infiles[i]) == 0) {
#endif /* _POSIX_SOURCE */
			ERR("File %s does not exist", infiles[i]);
			return -1;
		}
		if (strlen(infiles[i]) > MAX_FILENAME) {
			ERR("Filename '%s' is too long, should be only %d characters",
					infiles[i], MAX_FILENAME );
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


/*
 * opens the filename in packfile for reading
 */
#ifdef _POSIX_SOURCE
#define READ(b,n)  if (read(file->fd,(b),(n))!=(n)) { \
	ERR("Fewer bytes read then expected"); \
	free(buf); return -1; }
#else /* not _POSIX_SOURCE */
#define READ(b,n)  if (fread((b),1,(n),file->fp)!=(n)) { \
	ERR("Fewer bytes read then expected"); \
	free(buf); return -1; }
#endif /* _POSIX_SOURCE */
int pack_open( Packfile* file, const char* packfile, const char* filename )
{
	int j;
	uint32_t nfiles, i;
	char* buf = malloc(MAX_FILENAME);

	file->start = file->end = 0;

#ifdef _POSIX_SOURCE
	file->fd = open( packfile, O_RDONLY );
	if (file->fd == -1) {
#else /* not _POSIX_SOURCE */
	file->fp = fopen( packfile, "rb" );
	if (file->fp == NULL) {
#endif /* _POSIX_SOURCE */
		ERR("Erroring opening %s: %s", filename, strerror(errno));
		return -1;
	}

	READ( buf, sizeof(magic)); /* make sure it's a packfile */
	if (memcmp(buf, &magic, sizeof(magic))) {
		ERR("File %s is not a valid packfile", filename);
		return -1;
	}

	READ( &nfiles, 4 );
	for (i=0; i<nfiles; i++) { /* start to search files */
		j = 0;
		READ( &buf[j], 1 ); /* get the name */
		while ( buf[j++] != '\0' )
			READ( &buf[j], 1 );

		if (strcmp(filename, buf)==0) { /* found file */
			READ( &file->start, 4 );
			DEBUG("'%s' found at %d", filename, file->start);
			break;
		}
#ifdef _POSIX_SOURCE
		lseek( file->fd, 4, SEEK_CUR ); /* ignore the file location */
#else /* not _POSIX_SOURCE */
		fseek( file->fp, 4, SEEK_CUR );
#endif /* _POSIX_SOURCE */
	}
	free(buf);
	
	if (file->start) { /* go to the beginning of the file */
#ifdef _POSIX_SOURCE
		if ((uint32_t)lseek( file->fd, file->start, SEEK_SET ) != file->start) {
#else /* not _POSIX_SOURCE */
		fseek( file->fp, file->start, SEEK_SET );
		if (errno) {
#endif /* _POSIX_SOURCE */
			ERR("Failure to seek to file start: %s", strerror(errno));
			return -1;
		}
		READ( &file->end, 4 );
		DEBUG("\t%d bytes", file->end);
		file->pos = file->start;
		file->end += file->start;
	}
	else {
		ERR("File '%s' not found in packfile '%s'", filename, packfile);
		return -1;
	}

	return 0;
}
#undef READ


/*
 * reads count bytes from file and puts them into buf
 */
ssize_t pack_read( Packfile* file, void* buf, size_t count )
{
	if (file->pos + count > file->end) count = file->end - file->pos; /* can't go past end */
	if (count == 0) return 0;

	int bytes;

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


/*
 * loads an entire file inte memory and returns a pointer to it
 */
void* pack_readfile( const char* packfile, const char* filename, uint32_t *filesize )
{
	Packfile* file = malloc(sizeof(Packfile));
	void* buf;
	int size, bytes;

	if (filesize)
		*filesize = 0;

	if (pack_open( file, packfile, filename )) {
		ERR("Opening packfile");
		return NULL;
	}
	DEBUG("Opened file '%s' from '%s'", filename, packfile );

	/* read the entire file */
	size = file->end - file->start;
	buf = malloc( size + 1 );
	if ((bytes = pack_read( file, buf, size)) != size) {
		ERR("Reading '%s' from packfile '%s'.  Expected %d bytes got %d bytes",
				filename, packfile, size, bytes );
		free(buf);
		free(file);
		return NULL;
	}
	DEBUG("Read %d bytes from '%s'", bytes, filename );
	memset(buf+size,'\0',1); /* append size '\0' for it to validate as a string */

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
	free(file);

	if (filesize)
		*filesize = size;
	return buf;
}


/*
 * loads the filenames in the packfile to filenames
 * filenames should be freed after use
 * on error it filenames is (char**)-1
 */
#ifdef _POSIX_SOURCE
#define READ(b,n)		if (read(fd,(b),(n))!=(n)) { \
	ERR("Fewer bytes read then expected"); \
	return NULL; }
#else /* not _POSIX_SOURCE */
#define READ(b,n)		if (fread((b),1,(n),fp)!=(n)) { \
	ERR("Fewer bytes read then expected"); \
	return NULL; }
#endif /* _POSIX_SOURCE */
char** pack_listfiles( const char* packfile, uint32_t* nfiles )
{
#ifdef _POSIX_SOURCE
	int fd;
#else /* not _POSIX_SOURCE */
	FILE *fp;
#endif /* _POSIX_SOURCE */
	int j;
	uint32_t i;
	char** filenames;
	char* buf = malloc(sizeof(magic));

	*nfiles = 0;

#ifdef _POSIX_SOURCE
	fd = open( packfile, O_RDONLY );
	if (fd == -1) {
#else /* not _POSIX_SOURCE */
	fp = fopen( packfile, "rb" );
	if (fp == NULL) {
#endif /* _POSIX_SOURCE */
		ERR("Erroring opening %s: %s", packfile, strerror(errno));
		return NULL;
	}

	READ( buf, sizeof(magic)); /* make sure it's a packfile */
	if (memcmp(buf, &magic, sizeof(magic))) {
		ERR("File %s is not a valid packfile", packfile);
		return NULL;
	}

	READ( nfiles, 4 );
	filenames = malloc(((*nfiles)+1)*sizeof(char*));
	for (i=0; i<*nfiles; i++) { /* start to search files */
		j = 0;
		filenames[i] = malloc(MAX_FILENAME*sizeof(char));
		READ( &filenames[i][j], 1 ); /* get the name */
		while ( filenames[i][j++] != '\0' )
			READ( &filenames[i][j], 1 );
		READ( buf, 4 ); /* skip the location */
	}
	free(buf);
#ifdef _POSIX_SOURCE
	close(fd);
#else /* not _POSIX_SOURCE */
	fclose(fp);
#endif /* _POSIX_SOURCE */

	return filenames;
}
#undef READ



/*
 * closes the packfile
 */
int pack_close( Packfile* file )
{
	int i;
#ifdef _POSIX_SOURCE
	i = close( file->fd );
#else /* not _POSIX_SOURCE */
	i = fclose( file->fp );
#endif /* _POSIX_SOURCE */
	return (i) ? -1 : 0 ;
}


