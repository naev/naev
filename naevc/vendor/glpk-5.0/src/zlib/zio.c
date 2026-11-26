/* zio.c (simulation of non-standard low-level i/o functions) */

/* Written by Andrew Makhorin <mao@gnu.org>, April 2011
 * For conditions of distribution and use, see copyright notice in
 * zlib.h */

/* (reserved for copyright notice) */

#include <assert.h>
#include <stdio.h>
#include "zio.h"

static FILE *file[FOPEN_MAX];
static int initialized = 0;

static void initialize(void)
{     int fd;
      assert(!initialized);
      file[0] = stdin;
      file[1] = stdout;
      file[2] = stderr;
      for (fd = 3; fd < FOPEN_MAX; fd++)
         file[fd] = NULL;
      initialized = 1;
      return;
}

int open(const char *path, int oflag, ...)
{     FILE *fp;
      int fd;
      if (!initialized) initialize();
      /* see file gzlib.c, function gz_open */
      if (oflag == O_RDONLY)
         fp = fopen(path, "rb");
      else if (oflag == (O_WRONLY | O_CREAT | O_TRUNC))
         fp = fopen(path, "wb");
      else if (oflag == (O_WRONLY | O_CREAT | O_APPEND))
         fp = fopen(path, "ab");
      else
         assert(oflag != oflag);
      if (fp == NULL)
         return -1;
      for (fd = 0; fd < FOPEN_MAX; fd++)
         if (file[fd] == NULL) break;
      assert(fd < FOPEN_MAX);
      file[fd] = fp;
      return fd;
}

long read(int fd, void *buf, unsigned long nbyte)
{     unsigned long count;
      if (!initialized) initialize();
      assert(0 <= fd && fd < FOPEN_MAX);
      assert(file[fd] != NULL);
      count = fread(buf, 1, nbyte, file[fd]);
      if (ferror(file[fd]))
         return -1;
      return count;
}

long write(int fd, const void *buf, unsigned long nbyte)
{     unsigned long count;
      if (!initialized) initialize();
      assert(0 <= fd && fd < FOPEN_MAX);
      assert(file[fd] != NULL);
      count = fwrite(buf, 1, nbyte, file[fd]);
      if (count != nbyte)
         return -1;
      if (fflush(file[fd]) != 0)
         return -1;
      return count;
}

long lseek(int fd, long offset, int whence)
{     if (!initialized) initialize();
      assert(0 <= fd && fd < FOPEN_MAX);
      assert(file[fd] != NULL);
      if (fseek(file[fd], offset, whence) != 0)
         return -1;
      return ftell(file[fd]);
}

int close(int fd)
{     if (!initialized) initialize();
      assert(0 <= fd && fd < FOPEN_MAX);
      assert(file[fd] != NULL);
      fclose(file[fd]);
      file[fd] = NULL;
      return 0;
}

/* eof */
