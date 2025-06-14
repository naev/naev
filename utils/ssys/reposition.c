
#include <libgen.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define THREADS 1

#include <glib.h>

// defaults to 1.0
const struct {
   char *nam;
   float w;
} weights[] = {
   {"anubis_black_hole", 5.5f}, {NULL} // Sentinel
};

/* pgm output*/
void output_map(FILE *fp, float *map, int w, int h, float fact, float ct)
{
   uint8_t *line;
   char     b[64];
   fwrite(b, sizeof(char), snprintf(b, 63, "P5 %d %d 65535\n", w, h), fp);

   line = (uint8_t *) calloc((size_t) w, 2 * sizeof(uint8_t));
   for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
         const unsigned u = 65535.0f * (fact * map[(h - i - 1) * w + j] + ct);
         line[2 * j]      = u >> 8;
         line[2 * j + 1]  = u & 0xff;
      }
      fwrite(line, sizeof(uint16_t), w, fp);
   }
   fclose(fp);
   free(line);
}

struct s_ssys {
   struct sys {
      float v[2];
      float w;
      float _unused;
   }     *sys;
   char **sys_nam;
   int    nsys;
   int    nosys;

   int *jumps;
   int  njumps;
};

void gen_map_reposition(struct s_ssys *map, bool ignore_alone, bool gen_map,
                        bool only)
{
   char buff[512];
   int  num;

   (void) only;
   (void) ignore_alone;

   for (num = 0; num < THREADS - 1; num++)
      if (fork() == 0)
         break;

   for (int i = num; i < map->nosys; i += THREADS)
      if (map->sys[i].w != 0.0f) {
         const float *v = map->sys[i].v;
         const size_t n =
            snprintf(buff, 511, "%s %f %f\n", map->sys_nam[i], v[0], v[1]);
         fwrite(buff, sizeof(char), n, stdout);
         fflush(stdout);
      }

   if (num < THREADS - 1)
      exit(EXIT_SUCCESS);

   if (gen_map)
      fprintf(stderr, "\e[31m! gen_map not implemented.\e[0m\n");
}

static size_t ssys_num(GHashTable *h, char *s, struct s_ssys *map,
                       const float *src, bool upd)
{
   size_t this = (size_t) g_hash_table_lookup(h, s);

   if (!this) {
      if ((map->nsys & (map->nsys + 1)) == 0) {
         const size_t new_siz = (map->nsys << 1) | 3;
         map->sys             = realloc(map->sys, new_siz * sizeof(struct sys));
         map->sys_nam         = realloc(map->sys_nam, new_siz * sizeof(char *));
      }
      upd                       = true;
      map->sys_nam[map->nsys++] = strdup(s);
      this                      = map->nsys;
      g_hash_table_insert(h, (void *) s, (void *) this);
   }
   this --;
   if (upd) {
      map->sys[this].v[0] = src[0];
      map->sys[this].v[1] = src[1];
      map->sys[this].w    = 0.0;
   }
   return this;
}

/* main */
int do_it(char **onam, int n_onam, const bool ign_alone, const bool gen_map,
          const bool only)
{
   GHashTable   *h       = g_hash_table_new(g_str_hash, g_str_equal);
   struct s_ssys map     = {0};
   const float   nulv[2] = {0.0f, 0.0f};

   for (int i = 0; i < n_onam; i++)
      (void) ssys_num(h, onam[i], &map, nulv, false);
   map.nosys = map.nsys;

   char  *line = NULL;
   size_t n    = 0;
   while (getline(&line, &n, stdin) != -1) {
      char *bufs[3] = {NULL, NULL, NULL};
      float tmp[2]  = {0.0f, 0.0f};

      if (3 == sscanf(line, "%ms %f %f", bufs, tmp, tmp + 1)) {
         const size_t id = ssys_num(h, bufs[0], &map, tmp, true);
         map.sys[id].w   = 1.0;
      } else if (2 == sscanf(line, "%ms %ms", bufs + 1, bufs + 2)) {
         if ((map.njumps & (map.njumps + 1)) == 0) {
            const size_t new_siz = (map.njumps << 1) | 1;
            map.jumps = realloc(map.jumps, new_siz * 2 * sizeof(int));
         }
         map.jumps[map.njumps * 2 + 0] = ssys_num(h, bufs[1], &map, tmp, false);
         map.jumps[map.njumps * 2 + 1] = ssys_num(h, bufs[2], &map, tmp, false);
         map.njumps++;
      } else if (line[0] != '\0' && line[0] != '\n') {
         const int n = strlen(line);
         if (line[n - 1] == '\n')
            line[n - 1] = '\0';
         fprintf(stderr, "Ignored line : \"%s\"\n", line);
      }
      for (int i = 0; i < 3; i++)
         free(bufs[i]);
   }
   free(line);

   if (!map.nosys)
      map.nosys = map.nsys;

   gen_map_reposition(&map, ign_alone, gen_map, only);

   for (int i = 0; i < map.nsys; i++)
      free(map.sys_nam[i]);
   free(map.sys_nam);
   free(map.jumps);
   g_hash_table_destroy(h);
   return EXIT_SUCCESS;
}

int usage(char *nam, int ret)
{
   fprintf(stderr, "Usage:  %s  [-o] [-p] [<names>...]\n", basename(nam));
   fprintf(stderr, "  Reads a set of node pos in input from standard input.\n");
   fprintf(stderr,
           "  Applies repositioning to <names> and outputs the result.\n");
   fprintf(stderr, "  If <names> is not provided, use all input names.\n");
   fprintf(stderr, "  <names> is a set of ssys basenames.\n");
   fprintf(stderr, "  If files are provided instead, their basename without "
                   "\".xml\" is used.");
   fprintf(stderr, "  If -o is set, only output processed systems.\n");
   fprintf(stderr,
           "  If -p is set, outputs a pgm for each processed system.\n");
   return ret;
}

static int cmpstringp(const void *p1, const void *p2)
{
   return strcmp(*(char *const *) p1, *(char *const *) p2);
}

int main(int argc, char **argv)
{
   char *nam            = argv[0];
   bool  processed_only = false;
   bool  gen_map        = false;

   int fst_opt     = 1;
   int fst_non_opt = 1;
   for (int i = 1; i < argc; i++)
      if (argv[i][0] == '-') {
         char *swp           = argv[fst_non_opt];
         argv[fst_non_opt++] = argv[i];
         argv[i]             = swp;
      }

   qsort(argv + fst_opt, fst_non_opt - fst_opt, sizeof(char *), cmpstringp);

   if (fst_non_opt > fst_opt &&
       (!strcmp(argv[fst_opt], "-h") || !strcmp(argv[fst_opt], "--help")))
      return usage(nam,
                   fst_opt == fst_non_opt + 1 ? EXIT_SUCCESS : EXIT_FAILURE);

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-o")) {
      processed_only = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-p")) {
      gen_map = true;
      fst_opt++;
   }

   for (int i = fst_opt; i < fst_non_opt; i++)
      fprintf(stderr, "Ignored: \"%s\"\n", argv[i]);

   for (int i = fst_non_opt; i < argc; i++) {
      argv[i]           = (strrchr(argv[i], '/') ?: argv[i] - 1) + 1;
      char *const where = strrchr(argv[i], '.');
      if (where) {
         if (!strcmp(where, ".xml"))
            *where = '\0';
         else
            return usage(nam, EXIT_FAILURE);
      }
   }
   return do_it(argv + fst_non_opt, argc - fst_non_opt, true, gen_map,
                processed_only);
}
