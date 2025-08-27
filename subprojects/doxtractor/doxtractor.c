
// gcc -O1 comments.c -Wall -Wextra -lc

/**
 * An example:
 *  - This is an example
 *  - of accepted comment
 */

/**
 * This is another one.*/

/* This is *not* another one.*/

/** Nor this.*/

// Neither does this

#define _GNU_SOURCE
#include <libgen.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

static int do_it(FILE *const fp, const char *nam)
{
   char *buff = calloc(snprintf(NULL, 0, "%s %u: ", nam, (unsigned) (-1)) + 1,
                       sizeof(char));

   char    *line       = NULL;
   size_t   line_siz   = 0;
   int      in_comment = 0;
   unsigned lineno     = 0;
   int      warn_left  = 5;

   while (getline(&line, &line_siz, fp) != -1) {
      lineno++;
      if (in_comment) {
         char *where;
         if ((where = strstr(line, "*/"))) {
            strcpy(where, "\n");
            in_comment = 0;
         }
         if (!strncmp(line, " *", 2)) {
            int n = sprintf(buff, "%s %u: ", nam, lineno);

            char *const str = line + 2 + (line[2] == ' ' ? 1 : 0);
            char *const end = strchrnul(str, '\n');
            for (where = end; where[-1] == ' '; where--)
               ;
            if (where == str)
               n--;
            fwrite(buff, sizeof(char), n, stdout);
            strcpy(where, end);
            fwrite(str, sizeof(char), strlen(str), stdout);
         } else if (in_comment && warn_left) {
            *strchrnul(line, '\n') = '\0';
            fprintf(stderr, "%s %u: ", nam, lineno);
            fprintf(stderr, "Invalid comment line \"%s\"\n", line);
            warn_left--;
         }
      } else if (!strcmp(line, "/**\n")) {
         in_comment = 1;
         fwrite("\n", sizeof(char), 1, stdout);
      }
   }

   free(line);
   free(buff);
   return 0;
}

static int usage(const char *name)
{
   fprintf(stderr,
           "Usage  %s  [<file>...]\n"
           "  Reads stdin if no <file> provided.\n"
           "  Keeps only comments contents. Send the result to stdout.\n"
           "  (and insert a newline at the end of each).\n\n",
           name);
   return 0;
}

int main(int argc, char *const *argv)
{
   FILE *fp;
   int   res = 0;

   if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'h' && argv[1][2] == '\0')
      return usage(argv[0]);

   if (argc == 1)
      res = do_it(stdin, "<stdin>");
   else
      for (int i = 1; i < argc && !res; i++)
         if ((fp = fopen(argv[i], "rt"))) {
            res = do_it(fp, basename(argv[i])) || res;
            fclose(fp);
         } else
            res = 1;
   return res;
}
