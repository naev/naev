
#include <libgen.h>
#include <limits.h>
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

#define THREADS 2
#define SAMP 100

#include <glib.h>

const float ssys_fallscale = 1.5f;

// defaults to 1.0
const struct {
   char *nam;
   float w;
} weights[] = {
   {"anubis_black_hole", 5.5f}, {NULL} // Sentinel
};

#define INIT_RANGE {FLT_MAX, FLT_MIN}
static inline void update_range(float *r, const float v)
{
   r[0] = (v < r[0]) ? v : r[0];
   r[1] = (v > r[1]) ? v : r[1];
}

void output_ppm(const char *basnam, float *map, int w, int h)
{
   size_t len      = strlen(basnam) + 4;
   char  *filename = calloc(len + 1, sizeof(char));
   snprintf(filename, len + 1, "%s.ppm", basnam);
   FILE *fp         = fopen(filename, "wb");
   float mran[3][2] = {INIT_RANGE, INIT_RANGE, INIT_RANGE};

   for (int i = 0; i < w * h; i++)
      for (int k = 0; k < 3; k++)
         if (map[3 * i + k] != 0.0f)
            update_range(mran[k], map[3 * i + k]);

   uint8_t *line;
   char     b[64];

   fwrite(b, sizeof(char), snprintf(b, 63, "P6 %d %d 65535\n", w, h), fp);
   line = (uint8_t *) calloc((size_t) w, 2 * 3 * sizeof(uint8_t));
   for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
         float norm[3] = {0.0f, 0.0f, 0.0f};
         for (int k = 0; k < 3; k++) {
            const float sample = map[3 * ((h - i - 1) * w + j) + k];
            if (sample != 0.0f)
               norm[k] = (sample - mran[k][0]) / (mran[k][1] - mran[k][0]);

            // gamma
            norm[k] *= norm[k];
            if (k == 2 && norm[2] <= 0.5f)
               norm[2] = norm[0];

            const unsigned u          = 65535.0f * norm[k];
            line[2 * (3 * j + k)]     = u >> 8;
            line[2 * (3 * j + k) + 1] = u & 0xff;
         }
      }
      fwrite(line, 2 * 3 * sizeof(uint8_t), w, fp);
   }
   free(line);
   fclose(fp);
   free(filename);
}

/* geometry*/
static inline void vcpy(float *vdst, const float *vsrc)
{
   vdst[0] = vsrc[0];
   vdst[1] = vsrc[1];
}

static inline void vdif(float *vdst, const float *vsrc1, const float *vsrc2)
{
   vdst[0] = vsrc1[0] - vsrc2[0];
   vdst[1] = vsrc1[1] - vsrc2[1];
}

static inline float scal(const float *v1, const float *v2)
{
   return v1[0] * v2[0] + v1[1] * v2[1];
}

static inline float dist_sq(const float *v1, const float *v2)
{
   float tmp[2];
   vdif(tmp, v1, v2);
   return scal(tmp, tmp);
}

struct point_list {
   float (*lst)[2];
   int nb;
};

struct lin_list {
   struct {
      float pt[2];
      float u[2];
   }  *lst;
   int nb;
};

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

static float edge_stretch_score(const float *neigh, int n_neigh,
                                const float v[2], float radius)
{
   // S(x) =
   //   a*x^2         if x <= radius
   //   x+c           if radius <= x
   const float a = 1.0f / (2.0f * radius);
   const float c = -radius / 2.0f;

   float acc = 0.0;
   for (int i = 0; i < n_neigh; i++) {
      const float d = sqrt(dist_sq(neigh + 2 * i, v));

      if (d < radius)
         acc += a * d * d;
      else
         acc += d + c;
   }

   return acc / n_neigh;
}

static float sys_overlap_score(const float *lst, int n, const float v[2],
                               float radius, float falloff)
{
   // S(x) =
   //   S0 - x         if x <= radius
   //   a(x-falloff)^2 if radius <= x <= falloff with
   //   0              if x >= falloff
   // with C1-continuity.
   const float a   = 0.5f / (falloff - radius); // Can be Nan
   const float S0  = (radius + falloff) / 2.0f;
   float       acc = 0.0f;

   for (int i = 0; i < n; i++) {
      const float sq = dist_sq(lst + 2 * i, v);

      if (sq <= falloff * falloff) {
         const float dist = sqrt(sq);
         if (dist <= radius)
            acc += S0 - dist;
         else {
            const float bef_fall = falloff - dist;
            acc += a * bef_fall * bef_fall;
         }
      }
   }
   return acc;
}

static int _compar(const void *a, const void *b)
{
   return *((int *) a) - *((int *) b);
}

static int in_place_rem_duplicates(int *lst, int n)
{
   int w = 1;
   qsort((void *) lst, n, sizeof(int), _compar);
   for (int i = 1; i < n; i++)
      if (lst[i] != lst[w - 1])
         lst[w++] = lst[i];
   // lst = realloc(lst, w, sizeof(int));
   return w;
}

static float *poscpy(int n, const int *lst, const struct s_ssys *map,
                     const char *nam, bool quiet)
{
   float *dst = malloc(n * 2 * sizeof(float));

   for (int i = 0; i < n; i++)
      vcpy(dst + 2 * i, map->sys[lst[i]].v);

   if (!quiet) {
      fprintf(stderr, "\e[033;1m%s:\e[0m", nam);
      for (int i = 0; i < n; i++)
         fprintf(stderr, " %s", map->sys_nam[lst[i]]);
      fprintf(stderr, "\n");
   }
   return dst;
}

static float max_distsq(const float pt[2], const float *pts, int nb)
{
   float dst_sq = 0.0f;

   for (int i = 0; i < nb; i++) {
      const float d = dist_sq(pt, pts + i * 2);
      if (d > dst_sq)
         dst_sq = d;
   }
   return dst_sq;
}

#define SRT3_2 0.8660254037844386
static float bounding_circle(float dst[2], const float *pts, int nb)
{
   const float dirs[6][2] = {{0.0f, +1.0f}, {-0.5f, +SRT3_2}, {-0.5f, -SRT3_2},
                             {0.0f, -1.0f}, {+0.5f, -SRT3_2}, {+0.5f, +SRT3_2}};
   const int   n_dirs     = 6;
   const float eps        = 0.000001f;
   float       delta      = 1.0f;

   float max_d = max_distsq(pts, pts, nb);
   vcpy(dst, pts);
   while (1) {
      int i;
      for (i = 0; i < n_dirs; i++) {
         const float cand[2] = {dst[0] + delta * dirs[i][0],
                                dst[1] + delta * dirs[i][1]};
         const float dcand   = max_distsq(cand, pts, nb);
         if (dcand < max_d) {
            vcpy(dst, cand);
            max_d = dcand;
            break;
         }
      }
      if (i == n_dirs && ((delta /= 2.0f) < eps))
         break;
   }
   return max_d;
}

static float total_score(float score_es, float score_so)
{
   return 0.25 * score_es + score_so;
}

struct s_cost_params {
   const float *neigh;
   int          n_neigh;
   const float *around;
   int          n_around;
   float        radius;
   float        falloff;
};

static float sys_total_score(struct s_cost_params *sys_p, const float v[2])
{
   const float score_es =
      edge_stretch_score(sys_p->neigh, sys_p->n_neigh, v, sys_p->radius);
   const float score_so = sys_overlap_score(sys_p->around, sys_p->n_around, v,
                                            sys_p->radius, sys_p->falloff);
   return total_score(score_es, score_so);
}

int reposition_sys(float *dst, const struct s_ssys *map, int ssys, bool g_opt,
                   bool quiet, bool ppm)
{
   struct s_cost_params ssys_p = {.n_neigh = 0, .n_around = 0};
   int                 *id_neigh;
   float                edge_len = 0.0f;
   int                 *id_around;

   for (int i = 0; i < map->njumps; i++)
      edge_len += sqrt(dist_sq(map->sys[map->jumps[2 * i]].v,
                               map->sys[map->jumps[2 * i + 1]].v));

   edge_len /= map->njumps;
   ssys_p.radius  = 2.0 * edge_len / 3.0;
   ssys_p.falloff = ssys_fallscale * ssys_p.radius;
   // fprintf(stderr, "sys rad %f\n", ssys_p.radius);

   for (int i = 0; i < 2 * map->njumps; i++)
      if (map->jumps[i] == ssys)
         ssys_p.n_neigh++;

   if (!ssys_p.n_neigh) {
      vcpy(dst, map->sys[ssys].v);
      return 1;
   }

   id_neigh       = calloc(ssys_p.n_neigh, sizeof(int));
   ssys_p.n_neigh = 0;
   for (int i = 0; i < map->njumps; i++) {
      int n = 2 * i;

      if (map->jumps[n] == ssys)
         n = n ^ 1;
      if (map->jumps[n ^ 1] == ssys)
         id_neigh[ssys_p.n_neigh++] = map->jumps[n];
   }
   ssys_p.n_neigh = in_place_rem_duplicates(id_neigh, ssys_p.n_neigh);
   if (!quiet)
      fprintf(stderr, "\e[033;1m[%s]\e[0m\n", map->sys_nam[ssys]);
   ssys_p.neigh = poscpy(ssys_p.n_neigh, id_neigh, map, "neigh", quiet);

   float center[2];
   float sqrad = bounding_circle(center, ssys_p.neigh, ssys_p.n_neigh);
   float rad   = sqrt(sqrad);

   if (rad < edge_len) {
      rad   = edge_len;
      sqrad = rad * rad;
   }

   float UL[2] = {center[0] - rad, center[1] - rad};
   float LR[2] = {center[0] + rad, center[1] + rad};

   id_around    = calloc(map->nsys, sizeof(int));
   float max_sq = rad + ssys_p.falloff;
   max_sq *= max_sq;

   for (int i = 0; i < map->nsys; i++)
      if (i != ssys && dist_sq(center, map->sys[i].v) < max_sq)
         id_around[ssys_p.n_around++] = i;
   ssys_p.around = poscpy(ssys_p.n_around, id_around, map, "around", quiet);

   if (g_opt || ppm) {
      float best[2]    = {center[0], center[1]};
      float best_score = FLT_MAX;
      int   best_id    = 0;

      // Iterate over points of the circle
      float  v[2];
      float *samples = calloc(SAMP * SAMP, 3 * sizeof(float)); // IEEE754
      for (int i = 0; i < SAMP; i++) {
         v[0] = (1.0 * i / (SAMP - 1)) * (LR[0] - UL[0]) + UL[0];
         for (int j = 0; j < SAMP; j++) {
            v[1] = (1.0 * j / (SAMP - 1)) * (LR[1] - UL[1]) + UL[1];
            if (dist_sq(center, v) <= sqrad) {
               const float score_es = edge_stretch_score(
                  ssys_p.neigh, ssys_p.n_neigh, v, ssys_p.radius);
               const float score_so =
                  sys_overlap_score(ssys_p.around, ssys_p.n_around, v,
                                    ssys_p.radius, ssys_p.falloff);
               const float score = total_score(score_es, score_so);
               samples[3 * (i * SAMP + j) + 0] = score_es;
               samples[3 * (i * SAMP + j) + 1] = score_so;
               samples[3 * (i * SAMP + j) + 2] = 1.0f;
               if (score < best_score) {
                  vcpy(best, v);
                  best_score = score;
                  best_id    = i * SAMP + j;
               }
            }
         }
      }
      if (ppm) {
         const float blue[] = {0.0f, 0.0f, 2.0f};
         for (int k = 0; k < 3; k++)
            samples[3 * best_id + k] = blue[k];
         output_ppm(map->sys_nam[ssys], samples, SAMP, SAMP);
      }
      free(samples);
      if (g_opt)
         vcpy(dst, best);
   }

   const float dirs[6][2] = {{0.0f, +1.0f}, {-0.5f, +SRT3_2}, {-0.5f, -SRT3_2},
                             {0.0f, -1.0f}, {+0.5f, -SRT3_2}, {+0.5f, +SRT3_2}};
   const int   n_dirs     = 6;
   const float eps        = 0.000001f;
   float       delta      = 1.0f;

   float min_sc = sys_total_score(&ssys_p, dst);
   while (1) {
      int i;
      for (i = 0; i < n_dirs; i++) {
         const float cand[2] = {dst[0] + delta * dirs[i][0],
                                dst[1] + delta * dirs[i][1]};
         const float sc      = sys_total_score(&ssys_p, cand);
         if (sc < min_sc) {
            vcpy(dst, cand);
            min_sc = sc;
            break;
         }
      }
      if (i == n_dirs && ((delta /= 2.0f) < eps))
         break;
   }
   free(id_neigh);
   free((void *) ssys_p.neigh);
   free((void *) ssys_p.around);
   free(id_around);

   return 0;
}

void gen_map_reposition(struct s_ssys *map, bool g_opt, bool quiet,
                        bool gen_map, bool only, float ratio)
{
   char buff[512];
   int  num;

   if (!only)
      for (int i = map->nosys; i < map->nsys; i++) {
         const size_t n = snprintf(buff, 511, "%s %f %f\n", map->sys_nam[i],
                                   map->sys[i].v[0], map->sys[i].v[1]);
         fwrite(buff, sizeof(char), n, stdout);
         fflush(stdout);
      }

   for (num = 0; num < THREADS - 1; num++)
      if (fork() == 0)
         break;

   for (int i = num; i < map->nosys; i += THREADS)
      if (map->sys[i].w != 0.0f) {
         float v[2] = {map->sys[i].v[0], map->sys[i].v[1]};

         if (reposition_sys(v, map, i, g_opt, quiet, gen_map)) {
            if (!quiet)
               fprintf(stderr, "\"%s\" has no neighbor !\n", map->sys_nam[i]);
         } else {
            const size_t n =
               snprintf(buff, 511, "%s %f %f\n", map->sys_nam[i],
                        (v[0] + ratio * map->sys[i].v[0]) / (1.0 + ratio),
                        (v[1] + ratio * map->sys[i].v[1]) / (1.0 + ratio));
            fwrite(buff, sizeof(char), n, stdout);
            fflush(stdout);
         }
      }

   if (num < THREADS - 1)
      exit(EXIT_SUCCESS);
}

static size_t ssys_num(GHashTable *h, const char *s, struct s_ssys *map,
                       const float *src, bool upd)
{
   size_t this = (size_t) g_hash_table_lookup(h, s);

   if (!this) {
      char *str = strdup(s);

      if ((map->nsys & (map->nsys + 1)) == 0) {
         const size_t new_siz = (map->nsys << 1) | 3;
         map->sys             = realloc(map->sys, new_siz * sizeof(struct sys));
         map->sys_nam         = realloc(map->sys_nam, new_siz * sizeof(char *));
      }
      upd                       = true;
      map->sys_nam[map->nsys++] = str;
      this                      = map->nsys;
      g_hash_table_insert(h, (void *) str, (void *) this);
   }
   this --;
   if (upd) {
      vcpy(map->sys[this].v, src);
      map->sys[this].w = 0.0;
   }
   return this;
}

/* main */
int do_it(char **onam, int n_onam, bool g_opt, bool gen_map, bool only,
          float weight)
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
   g_hash_table_destroy(h);

   const bool quiet = !map.nosys;
   if (!map.nosys)
      map.nosys = map.nsys;

   gen_map_reposition(&map, g_opt, quiet, gen_map, only, weight);

   for (int i = 0; i < map.nsys; i++)
      free(map.sys_nam[i]);
   free(map.sys_nam);
   free(map.jumps);
   return EXIT_SUCCESS;
}

static int usage(char *nam, int ret, float w)
{
   fprintf(stderr, "Usage:  %s  [-g] [-o] [-p] [-w<weight>] [<names>...]\n",
           basename(nam));
   fprintf(
      stderr,
      "  Reads a graph from standard input.\n"
      "  Applies repositioning to <names> and outputs the result.\n\n"
      "  If <names> is not provided, use all input names.\n"
      "  <names> is a set of ssys basenames.\n"
      "  If files are provided instead, their basename without "
      "\".xml\" is used.\n\n"
      "  If -g is set, look for global optimum (slower, and\n"
      "    might result in a different planar embedding).\n\n"
      "  If -o is set, only output processed systems.\n"
      "  If -p is set, outputs a ppm for each processed system.\n\n"
      "  If <weight> is set (positive), outputs values in the form:\n"
      "    (<weight>*old_pos + new_pos) / (<weight> + 1.0)\n"
      "   - If not specified, <weight> defaults to %.1f.\n"
      "   - If you repos systems that are neighbors, its is strongly advised\n"
      "       to choose <weight> at the very least greater than 1.0.\n"
      "   - If you repos independent systems, you can (and should) use 0.0.\n",
      w);
   return ret;
}

static int cmpstringp(const void *p1, const void *p2)
{
   return strcmp(*(char *const *) p1, *(char *const *) p2);
}

int main(int argc, char **argv)
{
   char *nam            = argv[0];
   bool  g_opt          = false;
   bool  processed_only = false;
   bool  gen_map        = false;

   int   fst_opt     = 1;
   int   fst_non_opt = 1;
   float weight      = 2.0f;

   for (int i = 1; i < argc; i++)
      if (argv[i][0] == '-') {
         char *swp           = argv[fst_non_opt];
         argv[fst_non_opt++] = argv[i];
         argv[i]             = swp;
      }

   qsort(argv + fst_opt, fst_non_opt - fst_opt, sizeof(char *), cmpstringp);

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-g")) {
      g_opt = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt &&
       (!strcmp(argv[fst_opt], "-h") || !strcmp(argv[fst_opt], "--help")))
      return usage(
         nam, fst_opt == fst_non_opt + 1 ? EXIT_SUCCESS : EXIT_FAILURE, weight);

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-o")) {
      processed_only = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-p")) {
      gen_map = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt && !strncmp(argv[fst_opt], "-w", 2)) {
      if (sscanf(argv[fst_opt], "-w%f", &weight) != 1)
         return usage(nam, EXIT_FAILURE, weight);
      if (weight < 0.0f) {
         fprintf(stderr, "<weight> is supposed to be positive.\n");
         return EXIT_FAILURE;
      }
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
            return usage(nam, EXIT_FAILURE, weight);
      }
   }
   return do_it(argv + fst_non_opt, argc - fst_non_opt, g_opt, gen_map,
                processed_only, weight);
}
