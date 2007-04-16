
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
	struct stat file;

	if (!stat( filename, &file ))
		return file.st_size;

	ERR( "Unable to get filesize of %s", filename );
	return 0;
}


/*
 * returns true if filename is a Packfile
 */
int pack_check( char* filename )
{
	int fd = open( filename, O_RDONLY );
	if (fd == -1) {
		ERR("Erroring opening %s: %s", filename, strerror(errno));
		return -1;
	}

	char* buf = malloc(sizeof(magic));
	if (read( fd, buf, sizeof(magic) ) != sizeof(magic)) {
		ERR("Error reading magic number: %s", strerror(errno));
		free(buf);
		return -1;
	}

	int ret = (memcmp(buf,&magic,sizeof(magic))==0) ? 0 : 1 ;
	free(buf);
	return ret;
}


/* 
 * packs nfiles infiles into outfile
 */
#define WRITE(f,b,n)    if (write(f,b,n)==-1) { \
	ERR("Error writing to file: %s", strerror(errno)); \
	free(buf); return -1; }
int pack_files( char* outfile, char** infiles, uint32_t nfiles )
{
	void *buf;
	struct stat file;
	int i, namesize;
	int outfd, infd;
	uint32_t indexsize, pointer;
	int bytes;
	const uint8_t b = '\0';

	for (namesize=0,i=0; i < nfiles; i++) {/* make sure files exist before writing */
		if (stat(infiles[i], &file)) {
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
	outfd = creat( outfile, PERMS );
	if (outfd == -1) {
		ERR("Unable to open %s for writing", outfile);
		return -1;
	}

	/*
	 * INDEX
	 */
	buf = malloc(BLOCKSIZE);
	/* magic number */
	WRITE( outfd, &magic, sizeof(magic));
	DEBUG("Wrote magic number");
	/* number of files */
	WRITE( outfd, &nfiles, sizeof(nfiles));
	DEBUG("Wrote number of files: %d", nfiles);
	/* create file dependent index part */
	pointer = indexsize;
	for (i=0; i<nfiles; i++) {
		WRITE( outfd, infiles[i], strlen(infiles[i]) );
		DEBUG("Fie '%s' at %d", infiles[i], pointer);
		WRITE( outfd, &b, 1 );
		WRITE( outfd, &pointer, 4 );
		pointer += 4 + getfilesize( infiles[i] ) + 16; /* set pointer to be next file pos */
	}
	/*
	 * DATA
	 */
	md5_state_t md5;
	md5_byte_t *md5val = malloc(16);
	for (i=0; i<nfiles; i++) {
		bytes = (uint32_t)getfilesize( infiles[i] );
		WRITE( outfd, &bytes, 4  ); /* filesize */
		DEBUG("About to write file '%s' of %d bytes", infiles[i], bytes);
		infd = open( infiles[i], O_RDONLY );
		md5_init(&md5);
		while ((bytes = read( infd, buf, BLOCKSIZE ))) {
			WRITE( outfd, buf, bytes ); /* data */
			md5_append( &md5, buf, bytes );
		}
		md5_finish(&md5, md5val);
		WRITE( outfd, md5val, 16 );
		close(infd);
		DEBUG("Wrote file '%s'", infiles[i]);
	}
	free(md5val);

	close( outfd );
	free(buf);

	DEBUG("Packfile success\n\t%d files\n\t%d bytes", nfiles, (int)getfilesize(outfile));
	return 0;
}
#undef WRITE


/*
 * opens the filename in packfile for reading
 */
#define READ(f,b,n)  if (read(f,b,n)!=n) { \
	ERR("Fewer bytes read then expected"); \
	free(buf); return -1; }
int pack_open( Packfile* file, char* packfile, char* filename )
{
	int i, j;
	uint32_t nfiles;
	char* buf = (char*)malloc(MAX_FILENAME);

	file->start = file->end = 0;

	file->fd = open( packfile, O_RDONLY );
	if (file->fd == -1) {
		ERR("Erroring opening %s: %s", filename, strerror(errno));
		return -1;
	}

	READ( file->fd, buf, sizeof(magic)); /* make sure it's a packfile */
	if (memcmp(buf, &magic, sizeof(magic))) {
		ERR("File %s is not a valid packfile", filename);
		return -1;
	}

	READ( file->fd, &nfiles, 4 );
	for (i=0; i<nfiles; i++) { /* start to search files */
		j = 0;
		READ( file->fd, &buf[j], 1 ); /* get the name */
		while ( buf[j++] != '\0' )
			READ( file->fd, &buf[j], 1 );

		if (strcmp(filename, buf)==0) { /* found file */
			READ( file->fd, &file->start, 4 );
			DEBUG("'%s' found at %d", filename, file->start);
			break;
		}
		lseek( file->fd, 4, SEEK_CUR ); /* ignore the file location */
	}
	free(buf);
	
	if (file->start) { /* go to the beginning of the file */
		if (lseek( file->fd, file->start, SEEK_SET ) != file->start) {
			ERR("Failure to seek to file start: %s", strerror(errno));
			return -1;
		}
		if (read( file->fd, &file->end, 4 ) != 4) {
			ERR("Fewer bytes read then expected");
			return -1;
		}
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

	if ((bytes = read( file->fd, buf, count )) == -1) {
		ERR("Error while reading file: %s", strerror(errno));
		return -1;
	}
	file->pos += bytes;

	return bytes;
}


/*
 * loads an entire file inte memory and returns a pointer to it
 */
void* pack_readfile( char* packfile, char* filename, uint32_t *filesize )
{
	Packfile* file = (Packfile*)malloc(sizeof(Packfile));
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
	buf = malloc( size );
	if ((bytes = pack_read( file, buf, size)) != size) {
		ERR("Reading '%s' from packfile '%s'.  Expected %d bytes got %d bytes",
				filename, packfile, size, bytes );
		free(buf);
		free(file);
		return NULL;
	}
	DEBUG("Read %d bytes from '%s'", bytes, filename );

	/* check the md5 */
	md5_state_t md5;
	md5_byte_t *md5val = malloc(16);
	md5_byte_t *md5fd = malloc(16);
	md5_init(&md5);
	md5_append( &md5, buf, bytes );
	md5_finish(&md5, md5val);
	if ((bytes = read( file->fd, md5fd, 16 ))== -1)
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
 * closes the packfile
 */
int pack_close( Packfile* file )
{
	int i = close( file->fd );
	return (i) ? -1 : 0 ;
}


