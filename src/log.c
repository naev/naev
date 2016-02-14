/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file log.c
 *
 * @brief Home of logprintf.
 */

#include "log.h"

#include "naev.h"
#include "nfile.h"
#include "nstring.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h> /* strftime */
#include <sys/stat.h>
#include <sys/types.h>

#if HAS_POSIX
#include <unistd.h> /* isatty */
#endif

#include "console.h"


/**< Temporary storage buffers. */
static char *outcopy = NULL;
static char *errcopy = NULL;

static size_t moutcopy; /* Allocated size of outcopy. */
static size_t merrcopy; /* Allocated size of errcopy. */

static int noutcopy = 0; /* Number of bytes written to outcopy. */
static int nerrcopy = 0; /* Number of bytes written to errcopy. */

/**< Output filenames for stdout and stderr. */
static char *outfile = NULL;
static char *errfile = NULL;
static char *outfiledouble = NULL;
static char *errfiledouble = NULL;

/* Whether to copy stdout and stderr to temporary buffers. */
int copying = 0;


/*
 * Prototypes
 */
static void log_append( FILE *stream, char *str );

/**
 * @brief Like fprintf but also prints to the naev console.
 */
int logprintf( FILE *stream, const char *fmt, ... )
{
   va_list ap;
   char buf[2048];

   if (fmt == NULL)
      return 0;
   else { /* get the message */
      va_start( ap, fmt );
      vsnprintf( &buf[2], sizeof(buf)-2, fmt, ap );
      va_end( ap );
   }

#ifndef NOLOGPRINTFCONSOLE
   /* Add to console. */
   if (stream == stderr) {
      buf[0] = '\e';
      buf[1] = 'r';
      cli_addMessage( buf );
   }
   else
      cli_addMessage( &buf[2] );
#endif /* NOLOGPRINTFCONSOLE */

   /* Append to buffer. */
   if (copying)
      log_append(stream, &buf[2]);

   /* Also print to the stream. */
   return fprintf( stream, "%s", &buf[2] );
}


/**
 * @brief Redirects stdout and stderr to files.
 *
 * Should only be performed if conf.redirect_file is true and Naev isn't
 * running in a terminal.
 */
void log_redirect (void)
{
   char *buf;
   time_t cur;
   struct tm *ts;
   char timestr[20];

   time(&cur);
   ts = localtime(&cur);
   strftime( timestr, sizeof(timestr), "%Y-%m-%d_%H-%M-%S", ts );

   buf = malloc(PATH_MAX);
   nsnprintf( buf, PATH_MAX, "%slogs/", nfile_dataPath() );
   nfile_dirMakeExist( buf );
   free(buf);

   outfile = malloc(PATH_MAX);
   errfile = malloc(PATH_MAX);
   outfiledouble = malloc(PATH_MAX);
   errfiledouble = malloc(PATH_MAX);

   nsnprintf( outfile, PATH_MAX, "%slogs/stdout.txt", nfile_dataPath() );
   freopen( outfile, "w", stdout );

   nsnprintf( errfile, PATH_MAX, "%slogs/stderr.txt", nfile_dataPath() );
   freopen( errfile, "w", stderr );

   nsnprintf( outfiledouble, PATH_MAX, "%slogs/%s_stdout.txt", nfile_dataPath(), timestr );
   nsnprintf( errfiledouble, PATH_MAX, "%slogs/%s_stderr.txt", nfile_dataPath(), timestr );

   /* stderr should be unbuffered */
   setvbuf( stderr, NULL, _IONBF, 0 );
}


/**
 * @brief Checks whether Naev is connected to a terminal.
 *
 *    @return 1 if Naev is connected to a terminal, 0 otherwise.
 */
int log_isTerminal (void)
{
#if HAS_POSIX
   /* stdin and (stdout or stderr) are connected to a TTY */
   if (isatty(fileno(stdin)) && (isatty(fileno(stdout)) || isatty(fileno(stderr))))
      return 1;

#elif HAS_WIN32
   struct stat buf;

   /* Not interactive if stdin isn't a FIFO or character device. */
   if (fstat(_fileno(stdin), &buf) ||
         !((buf.st_mode & S_IFMT) & (S_IFIFO | S_IFCHR)))
      return 0;

   /* Interactive if stdout is a FIFO or character device. */
   if (!fstat(_fileno(stdout), &buf) &&
         ((buf.st_mode & S_IFMT) & (S_IFIFO | S_IFCHR)))
      return 1;

   /* Interactive if stderr is a FIFO or character device. */
   if (!fstat(_fileno(stderr), &buf) &&
         ((buf.st_mode & S_IFMT) & (S_IFIFO | S_IFCHR)))
      return 1;

#else
#error "Feature needs implementation on this Operating System for Naev to work."
#endif
   return 0;
}


/**
 * @brief Sets up or terminates copying of standard streams into memory.
 *
 * While copying is active, all stdout and stderr-bound messages that pass
 * through logprintf will also be put into a buffer in memory, to be flushed
 * when copying is disabled.
 *
 *    @param Whether to enable or disable copying. Disabling flushes logs.
 */
void log_copy( int enable )
{
   /* Nothing to do. */
   if (copying == enable)
      return;

   if (enable) {
      copying  = 1;

      moutcopy = 1;
      noutcopy = 0;
      outcopy  = calloc(moutcopy, BUFSIZ);

      merrcopy = 1;
      nerrcopy = 0;
      errcopy  = calloc(merrcopy, BUFSIZ);

      return;
   }

   if (noutcopy)
      fprintf( stdout, "%s", outcopy );

   if (nerrcopy)
      fprintf( stderr, "%s", errcopy );

   log_purge();
}


/**
 * @brief Whether log copying is enabled.
 *
 *    @return 1 if copying is enabled, 0 otherwise.
 */
int log_copying (void)
{
   return copying;
}


/**
 * @brief Deletes copied output without printing the contents.
 */
void log_purge (void)
{
   if (!copying)
      return;

   free(outcopy);
   free(errcopy);

   outcopy = NULL;
   errcopy = NULL;

   copying = 0;
}


/**
 * @brief Deletes the current session's log pair if stderr is empty.
 */
void log_clean (void)
{
   struct stat err;

   /* We assume redirection is only done in pairs. */
   if ((outfile == NULL) || (errfile == NULL))
      return;

   fclose(stdout);
   fclose(stderr);

   if (stat(errfile, &err) != 0)
      return;

   if (err.st_size == 0) {
      unlink(outfile);
      unlink(errfile);
   } else {
      nfile_copyIfExists(outfile, outfiledouble);
      nfile_copyIfExists(errfile, errfiledouble);
   }
}


/**
 * @brief Appends a message to a stream's in-memory buffer.
 *
 *    @param stream Destination stream (stdout or stderr)
 *    @param str String to append.
 */
static void log_append( FILE *stream, char *str )
{
   int len;

   len = strlen(str);
   if (stream == stdout) {
      while ((len + noutcopy) >= (int)moutcopy) {
         moutcopy *= 2;
         outcopy = realloc( outcopy, moutcopy );
         if (outcopy == NULL) goto copy_err;
      }

      strncpy( &outcopy[noutcopy], str, len+1 );
      noutcopy += len;
   }
   else if (stream == stderr) {
      while ((len + nerrcopy) >= (int)merrcopy) {
         merrcopy *= 2;
         errcopy = realloc( errcopy, merrcopy );
         if (errcopy == NULL) goto copy_err;
      }

      strncpy( &errcopy[nerrcopy], str, len+1 );
      nerrcopy += len;
   }

   return;

copy_err:
   log_purge();
   WARN("An error occurred while buffering %s!",
      stream == stdout ? "stdout" : "stderr");
}
