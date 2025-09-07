
/* TODO: re-implement it in rust. */

/**
 * An example:
 *  - This is an example
 *  - of accepted comment
 */

/**
 * This is another one.*/

/* This is *not* another one.*/

/** Neither does this.*/

// Nor this.

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getline.c"

static char *my_strchrnul(char *s, int c)
{
   char *out = strchr(s, c);
   return out ? out : (s + strlen(s));
}

static int do_it(MYFILE *fp, const char *pref)
{
   char *buff = calloc(snprintf(NULL, 0, "%s%u: ", pref, (unsigned) (-1)) + 1,
                       sizeof(char));

   char    *line       = NULL;
   size_t   line_siz   = 0;
   int      in_comment = 0;
   unsigned lineno     = 0;
   int      warn_left  = 5;

   while (my_getline(&line, &line_siz, fp) != -1) {
      lineno++;
      if (in_comment) {
         char *where;
         if ((where = strstr(line, "*/"))) {
            strcpy(where, "\n");
            in_comment = 0;
         }
         if (!strncmp(line, " *", 2)) {
            int n = sprintf(buff, "%s%u: ", pref, lineno);

            char *const str = line + 2 + (line[2] == ' ' ? 1 : 0);
            char *const end = my_strchrnul(str, '\n');
            for (where = end; where[-1] == ' '; where--)
               ;
            if (where == str)
               n--;
            fwrite(buff, sizeof(char), n, stdout);
            strcpy(where, end);
            fwrite(str, sizeof(char), strlen(str), stdout);
         } else if (in_comment && warn_left) {
            *my_strchrnul(line, '\n') = '\0';
            fprintf(stderr, "%s%u: ", pref, lineno);
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

   if (argc > 1 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))
      return usage(argv[0]);

   if (argc == 1) {
      MYFILE mfp = {.fp = stdin};
      res        = do_it(&mfp, "");
   } else
      for (int i = 1; i < argc && !res; i++)
         if ((fp = fopen(argv[i], "rt"))) {
            MYFILE      mfp = {.fp = fp};
            const char *bn  = basename(argv[i]);
            char *pref = calloc(snprintf(NULL, 0, "%s ", bn) + 1, sizeof(char));
            sprintf(pref, "%s ", bn);
            res = do_it(&mfp, pref) || res;
            fclose(fp);
            free(pref);
         } else
            res = 1;
   return res;
}
