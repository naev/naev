
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

#define THREADS 4
#define SAMP 200
#define EDGE_FACT 1.6
#define PEEK_D 4
#define MAX_STRETCH // penalty is max rather than avg of indiv. stretches

#include <glib.h>

const double ssys_fallscale = 1.5;

// defaults to 1.0
const struct {
   char  *nam;
   double w;
} weights[] = {
   {"anubis_black_hole", 5.5}, {NULL} // Sentinel
};

#define INIT_RANGE {DBL_MAX, DBL_MIN}
static inline void update_range(double *r, const double v)
{
   r[0] = (v < r[0]) ? v : r[0];
   r[1] = (v > r[1]) ? v : r[1];
}

void output_ppm(const char *basnam, double *map, int w, int h)
{
   size_t len      = strlen(basnam) + 4;
   char  *filename = calloc(len + 1, sizeof(char));
   snprintf(filename, len + 1, "%s.ppm", basnam);
   FILE  *fp         = fopen(filename, "wb");
   double mran[3][2] = {INIT_RANGE, INIT_RANGE, INIT_RANGE};

   for (int i = 0; i < w * h; i++)
      for (int k = 0; k < 3; k++)
         if (map[3 * i + k] != 0.0)
            update_range(mran[k], map[3 * i + k]);

   uint8_t *line;
   char     b[64];

   fwrite(b, sizeof(char), snprintf(b, 63, "P6 %d %d 65535\n", w, h), fp);
   line = (uint8_t *) calloc((size_t) w, 2 * 3 * sizeof(uint8_t));
   for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
         double norm[3] = {0.0, 0.0, 0.0};
         for (int k = 0; k < 3; k++) {
            const double sample = map[3 * ((h - i - 1) * w + j) + k];
            if (sample != 0.0)
               norm[k] = (sample - mran[k][0]) / (mran[k][1] - mran[k][0]);

            // gamma
            norm[k] *= norm[k];
            const unsigned u          = 65535.0 * norm[k];
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
static inline void vcpy(double *vdst, const double *vsrc)
{
   vdst[0] = vsrc[0];
   vdst[1] = vsrc[1];
}

#define INIT_VDIF(v1, v2) {(v1)[0] - (v2)[0], (v1)[1] - (v2)[1]}
#define INIT_VAVG(v1, v2) {0.5 * ((v1)[0] + (v2)[0]), 0.5 * ((v1)[1] + (v2)[1])}
#define SCAL(v1, v2) ((v1)[0] * (v2)[0] + (v1)[1] * (v2)[1])

static inline void vmul(double *u, double k)
{
   u[0] *= k;
   u[1] *= k;
}

static inline void norm_v(double *u)
{
   vmul(u, 1.0 / sqrt(SCAL(u, u)));
}

static inline double dist_sq(const double *v1, const double *v2)
{
   double tmp[2] = INIT_VDIF(v1, v2);
   return SCAL(tmp, tmp);
}

static inline double seg_dist_sq(const double *A, const double *B,
                                 const double *u, const double *O)
{
   const double OA[2] = INIT_VDIF(A, O);
   const double OB[2] = INIT_VDIF(B, O);
   const double p     = SCAL(OA, u);
   const double q     = SCAL(OB, u);
   const double P[2]  = {A[0] - p * u[0], A[1] - p * u[1]};

   if (p * q < 0.0)
      return dist_sq(O, P);
   else if (p > 0.0)
      return SCAL(OA, OA);
   else
      return SCAL(OB, OB);
}

struct s_edge {
   int    jmp[2];
   double len;
};

struct s_ssys {
   struct sys {
      double v[2];
      double w;
      double _unused;
   }     *sys;
   char **sys_nam;
   char **sys_aux;
   int    nsys;
   int    nosys;

   struct s_edge *jumps;
   int            njumps;
};

#define MAYBE(n) (n)
// #define MAYBE(n)  (1.0)
// #define MAYBE(n)  (n==1.0 ? n : (fprintf(stderr," {%lf}",n)?n: n))

#define OFF_N 4
struct s_cost_params {
   double       *neigh; // N[2] len[1] _[1]
   int           n_neigh;
   const double *around;
   int           n_around;
   double       *edges; // A[2] B[2] u[2] len[1] _[1]
   int           n_edges;
   double        radius;
   double        falloff;
};

static inline double repulsive(double d_sq, double radius, double falloff)
{
   // S(x) =
   //   S0 - x         if x <= radius
   //   a(x-falloff)^2 if radius <= x <= falloff with
   //   0              if x >= falloff
   // with C1-continuity.
   const double a  = 0.5 / (falloff - radius); // Can be Nan
   const double S0 = (radius + falloff) / 2.0;

   if (d_sq < falloff * falloff) {
      const double dist = sqrt(d_sq);
      if (dist <= radius)
         return S0 - dist;
      else {
         const double bef_fall = falloff - dist;
         return a * bef_fall * bef_fall;
      }
   } else
      return 0.0;
}

static double edge_stretch_score(struct s_cost_params *cp, const double v[2])
{
   if (!cp->n_neigh)
      return 0.0;

   // S(x) =
   //   a*x^2         if x <= radius
   //   x+c           if radius <= x
   const double a = 1.0 / (2.0 * cp->radius);
   const double c = -cp->radius / 2.0;

   double acc = 0.0;
#ifndef MAX_STRETCH
   double tot = 0.0;
#endif

   for (int i = 0; i < cp->n_neigh; i++) {
      const double len = MAYBE(cp->neigh[OFF_N * i + 2]);
      const double d   = sqrt(dist_sq(cp->neigh + OFF_N * i, v)) / len;
      double       cost;

      if (d < cp->radius)
         cost = (a * d * d);
      else
         cost = (d + c);

#ifdef MAX_STRETCH
      if (cost > acc)
         acc = cost;
#else
      acc = cost;
      tot += 1.0 / len;
#endif
   }
#ifdef MAX_STRETCH
   return acc;
#else
   return acc / tot;
#endif
}

static double sys_overlap_score(struct s_cost_params *cp, const double v[2])
{
   double mini = DBL_MAX;
   double maxi;

   for (int i = 0; i < cp->n_around; i++) {
      const double sq = dist_sq(cp->around + 2 * i, v);

      if (sq < mini)
         mini = sq;
   }
   maxi = repulsive(mini, cp->radius, cp->falloff);
   for (int i = 0; i < cp->n_neigh; i++) {
      const double len = cp->neigh[i * OFF_N + 2];
      const double sq  = dist_sq(cp->neigh + OFF_N * i, v) / (len * len);
      const double res = len * repulsive(sq, cp->radius, cp->falloff);

      if (res > maxi)
         maxi = res;
   }
   return maxi;
}

static double edge_overlap_score(struct s_cost_params *cp, const double v[2])
{
   const double F   = EDGE_FACT;
   const double Fsq = F * F;
   // S_e(x) = S(F*x)/F

   double maxi = 0.0;

   for (int i = 0; i < cp->n_edges; i++) {
      const double *E   = cp->edges + 8 * i;
      const double  len = MAYBE(E[6]);
      const double  sq  = Fsq / (len * len) * seg_dist_sq(E, E + 2, E + 4, v);
      const double  res = repulsive(sq, cp->radius, cp->falloff) * len;

      if (res > maxi)
         maxi = res;
   }

   for (int i = 0; i < cp->n_neigh; i++) {
      const double *N    = cp->neigh + OFF_N * i;
      const double  len  = MAYBE(cp->neigh[OFF_N * i + 2]);
      double        u[2] = INIT_VDIF(N, v);
      double        mini = DBL_MAX;
      norm_v(u);

      for (int j = 0; j < cp->n_around; j++) {
         const double *P  = cp->around + 2 * j;
         const double  sq = seg_dist_sq(v, N, u, P);

         if (sq < mini)
            mini = sq;
      }
      mini *= Fsq / (len * len);
      const double this_res =
         0.5 * repulsive(mini, cp->radius, cp->falloff) * len;
      if (this_res > maxi)
         maxi = this_res;

      for (int j = 0; j < cp->n_neigh; j++) {
         if (i != j) {
            const double *P    = cp->neigh + OFF_N * j;
            const double  len2 = MAYBE(cp->neigh[OFF_N * j + 2]);
            const double  sq =
               Fsq / (len2 * len2) / (len * len) * seg_dist_sq(v, N, u, P);
            const double res =
               0.5 * repulsive(sq, cp->radius, cp->falloff) * len2 * len;
            if (res > maxi)
               maxi = res;
         }
      }
   }
   return maxi / F;
}

struct s_nlst {
   int   id;
   float len;
};

static int _compar(const void *a, const void *b)
{
   struct s_nlst *na = (struct s_nlst *) a;
   struct s_nlst *nb = (struct s_nlst *) b;
   return na->id - nb->id;
}

static int in_place_sort_u(struct s_nlst *lst, int n)
{
   int w = 1;
   qsort((void *) lst, n, sizeof(struct s_nlst), _compar);
   for (int i = 1; i < n; i++)
      if (lst[i].id != lst[w - 1].id) {
         lst[w].len  = lst[i].len;
         lst[w++].id = lst[i].id;
      }
   return w;
}

static double *poscpy(int n, int off, const void *lst, size_t s,
                      const struct s_ssys *map, const char *nam, bool quiet)
{
   double *dst = malloc(n * off * sizeof(double));

   for (int i = 0; i < n; i++) {
      const int id = *((int *) ((char *) lst + s * i));
      vcpy(dst + off * i, map->sys[id].v);
   }

   if (!quiet) {
      fprintf(stderr, "\e[033;1m%s:\e[0m", nam);
      for (int i = 0; i < n; i++) {
         const int id = *((int *) ((char *) lst + s * i));
         fprintf(stderr, " %s", map->sys_nam[id]);
      }
      fprintf(stderr, "\n");
   }
   return dst;
}

// !! Au & Bu should not be colinear
static inline void inter_lines(double dst[2], const double A[2],
                               const double Au[2], const double B[2],
                               const double Bu[2])
{
   const double *u    = Au;
   const double  v[2] = {-Bu[1], Bu[0]};

   const double O_S[] = INIT_VDIF(B, A);
   const double f     = SCAL(O_S, v) / SCAL(u, v);

   dst[0] = A[0] + f * u[0];
   dst[1] = A[1] + f * u[1];
}

// !! A must not be s.t. CA.AB > AB.BC && > BC.CA
// (we don't want AB and AC colinear)
static void circum(double dst[2], const double *A, const double *B,
                   const double *C)
{
   const double AB[]  = INIT_VDIF(B, A);
   const double AC[]  = INIT_VDIF(C, A);
   const double u[]   = {AB[1], -AB[0]};
   const double v[]   = {AC[1], -AC[0]};
   const double BB[2] = INIT_VAVG(A, B);
   const double CC[2] = INIT_VAVG(A, C);
   inter_lines(dst, BB, u, CC, v);
}

static double bounding_circle(double dst[2], const double *pts, int nb, int off)
{
   double dst_sq = 0.0;
   vcpy(dst, pts);

   for (int i = 0; i < nb; i++)
      for (int j = i + 1; j < nb; j++) {
         const double d = 0.25 * dist_sq(pts + i * off, pts + j * off);
         if (d > dst_sq) {
            const double avg[2] = INIT_VAVG(pts + i * off, pts + j * off);
            dst_sq              = d;
            vcpy(dst, avg);
         }
      }
   for (int i = 0; i < nb; i++)
      for (int j = i + 1; j < nb; j++)
         for (int k = j + 1; k < nb; k++) {
            // Trust your compiler
            const double *A     = pts + i * off;
            const double *B     = pts + j * off;
            const double *C     = pts + k * off;
            const double  AB[2] = INIT_VDIF(B, A);
            const double  BC[2] = INIT_VDIF(C, B);
            const double  CA[2] = INIT_VDIF(A, C);
            const double  sb = SCAL(AB, BC), sc = SCAL(BC, CA),
                         sa = SCAL(CA, AB);

            // if ABC acute
            if (sb < 0.0 && sc < 0.0 && sa < 0.0) {
               double tmpr[2];

               if (sb < sc)
                  circum(tmpr, B, C, A);
               else
                  circum(tmpr, C, A, B);

               const double res = dist_sq(A, tmpr);
               if (res > dst_sq) {
                  dst_sq = res;
                  vcpy(dst, tmpr);
               }
            }
         }
   return dst_sq;
}

static double sys_total_score(double *dst, struct s_cost_params *sys_p,
                              const double v[2])
{
   double res[3];

   if (!dst)
      dst = res;

   dst[0] = edge_stretch_score(sys_p, v);
   dst[1] = sys_overlap_score(sys_p, v);
   dst[2] = edge_overlap_score(sys_p, v);
   return 0.25 * dst[0] + dst[1] + dst[2];
}

static bool can_beat(double alpha, double beta, double total_delta, double goal)
{
   if (alpha > beta) {
      total_delta -= alpha - beta;
      alpha = beta;
   } else
      total_delta -= beta - alpha;

   return alpha - 0.5 * total_delta < goal;
}

static double local_opt_search(double dst[2], struct s_cost_params *ssys_p,
                               const double A[2], double Ca, const double B[2],
                               double Cb, int depth, double beat, double max_d)
{
   if (depth == 0)
      return beat;

   if (!can_beat(Ca, Cb, max_d, beat))
      return beat;

   const double Pmid[2] = INIT_VAVG(A, B);
   const double Cmid    = sys_total_score(NULL, ssys_p, Pmid);
   if (Cmid < beat) {
      vcpy(dst, Pmid);
      return Cmid;
   }
   max_d /= 2.0;
   const double res =
      local_opt_search(dst, ssys_p, A, Ca, Pmid, Cmid, depth - 1, beat, max_d);
   if (res < beat)
      return res;
   else
      return local_opt_search(dst, ssys_p, Pmid, Cmid, A, Cb, depth - 1, beat,
                              max_d);
}

#define SRT3_2 0.8660254037844386
static void local_opt(double dst[2], struct s_cost_params *ssys_p)
{
   const double base[6][2] = {{0.0, +1.0}, {-0.5, +SRT3_2}, {-0.5, -SRT3_2},
                              {0.0, -1.0}, {+0.5, -SRT3_2}, {+0.5, +SRT3_2}};
   const int    n_dirs     = 6;
   double       delta      = 1.0;
   const double eps        = 0.00001;
   // This is the max possible slope.
   const double sl = 2.25;

   double min_sc = sys_total_score(NULL, ssys_p, dst);
   while (1) {
      double beat = min_sc;
      double dirs[6][2], cost[6];

      for (int n = 0; n < n_dirs; n++) {
         dirs[n][0] = dst[0] + delta * base[n][0];
         dirs[n][1] = dst[1] + delta * base[n][1];
         cost[n]    = sys_total_score(NULL, ssys_p, dirs[n]);
         if (cost[n] < beat) {
            beat = cost[n];
            vcpy(dst, dirs[n]);
         }
      }

      for (int n = 0; n < n_dirs; n++) {
         int m = (n + 1) % n_dirs;
         beat  = local_opt_search(dst, ssys_p, dirs[n], cost[n], dirs[m],
                                  cost[m], PEEK_D, beat, sl * delta);
      }
      if (beat < min_sc) {
         min_sc = beat;
         continue;
      } else if ((delta /= 2.0) < eps)
         break;
   }
}

int reposition_sys(double dst[2], const struct s_ssys *map, int ssys,
                   bool g_opt, bool quiet, bool ppm)
{
   struct s_cost_params ssys_p = {.n_neigh = 0, .n_around = 0, .n_edges = 0};
   struct s_nlst       *id_neigh;
   double               edge_len = 0.0;
   int                 *id_around;
   int                  n_around = 0;

   for (int i = 0; i < map->njumps; i++)
      edge_len += sqrt(dist_sq(map->sys[map->jumps[i].jmp[0]].v,
                               map->sys[map->jumps[i].jmp[1]].v));

   edge_len /= map->njumps;
   ssys_p.radius  = 2.0 * edge_len / 3.0;
   ssys_p.falloff = ssys_fallscale * ssys_p.radius;

   for (int i = 0; i < 2 * map->njumps; i++)
      if (map->jumps[i / 2].jmp[i % 2] == ssys)
         ssys_p.n_neigh++;

   if (!ssys_p.n_neigh) {
      vcpy(dst, map->sys[ssys].v);
      return 1;
   }

   id_neigh       = calloc(ssys_p.n_neigh + 1, sizeof(struct s_nlst));
   ssys_p.n_neigh = 0;
   for (int i = 0; i < map->njumps; i++) {
      int n = 0;

      if (map->jumps[i].jmp[n] == ssys)
         n = 1;
      if (map->jumps[i].jmp[n ^ 1] == ssys &&
          map->sys[map->jumps[i].jmp[n]].w != 0.0) {
         id_neigh[ssys_p.n_neigh].len  = map->jumps[i].len;
         id_neigh[ssys_p.n_neigh++].id = map->jumps[i].jmp[n];
      }
   }
   ssys_p.n_neigh = in_place_sort_u(id_neigh, ssys_p.n_neigh);
   if (!quiet)
      fprintf(stderr, "\e[033;1m[%s]\e[0m\n", map->sys_nam[ssys]);
   id_neigh[ssys_p.n_neigh].id  = ssys;
   id_neigh[ssys_p.n_neigh].len = 1.0;
   ssys_p.neigh                 = poscpy(ssys_p.n_neigh + 1, OFF_N, id_neigh,
                                         sizeof(struct s_nlst), map, "neigh", quiet);
   for (int i = 0; i < ssys_p.n_neigh; i++)
      ssys_p.neigh[OFF_N * i + 2] = id_neigh[i].len;

   double center[2];
   // include self in the list -> "+ 1"
   // Allow angles down to 60° -> 3.0 *
   double sqrad =
      3.0 * bounding_circle(center, ssys_p.neigh, ssys_p.n_neigh + 1, OFF_N);
   double rad = sqrt(sqrad);

   double UL[2] = {center[0] - rad, center[1] - rad};
   double LR[2] = {center[0] + rad, center[1] + rad};

   id_around     = calloc(map->nsys, sizeof(int));
   double max_sq = rad + ssys_p.falloff;
   max_sq *= max_sq;

   for (int i = 0; i < map->nsys; i++)
      if (i != ssys && dist_sq(center, map->sys[i].v) < max_sq &&
          map->sys[i].w != 0.0)
         id_around[n_around++] = i;

   ssys_p.n_around = 0;
   for (int i = 0, j = 0; i < n_around;)
      if (j == ssys_p.n_neigh || id_around[i] < id_neigh[j].id)
         id_around[ssys_p.n_around++] = id_around[i++];
      else if (id_around[i] == id_neigh[j++].id)
         i++;

   ssys_p.around =
      poscpy(ssys_p.n_around, 2, id_around, sizeof(int), map, "around", quiet);

   ssys_p.edges = calloc(map->njumps * 8, sizeof(double));
   for (int i = 0; i < map->njumps; i++) {
      if (map->jumps[i].jmp[0] == ssys || map->jumps[i].jmp[1] == ssys)
         continue;

      double *A    = map->sys[map->jumps[i].jmp[0]].v;
      double *B    = map->sys[map->jumps[i].jmp[1]].v;
      double  u[2] = INIT_VDIF(B, A);
      norm_v(u);
      if (seg_dist_sq(A, B, u, center) < max_sq) {
         vcpy(ssys_p.edges + ssys_p.n_edges * 8, A);
         vcpy(ssys_p.edges + ssys_p.n_edges * 8 + 2, B);
         vcpy(ssys_p.edges + ssys_p.n_edges * 8 + 4, u);
         ssys_p.edges[ssys_p.n_edges * 8 + 6] = map->jumps[i].len;
         ssys_p.n_edges++;
      }
   }
   ssys_p.edges =
      realloc((void *) ssys_p.edges, ssys_p.n_edges * 8 * sizeof(double));

   if (g_opt || ppm) {
      double best[2]    = {center[0], center[1]};
      double best_score = DBL_MAX;
      int    best_id    = 0;

      // Iterate over points of the circle
      double  v[2];
      double *samples = calloc(SAMP * SAMP, 3 * sizeof(double)); // IEEE754
      for (int i = 0; i < SAMP; i++)
         for (int j = 0; j < SAMP; j++) {
            v[0] = (1.0 * j / (SAMP - 1)) * (LR[0] - UL[0]) + UL[0];
            v[1] = (1.0 * i / (SAMP - 1)) * (LR[1] - UL[1]) + UL[1];
            if (dist_sq(center, v) <= sqrad) {
               const double score =
                  sys_total_score(samples + 3 * (i * SAMP + j), &ssys_p, v);
               if (score < best_score) {
                  vcpy(best, v);
                  best_score = score;
                  best_id    = i * SAMP + j;
               }
            }
         }
      if (ppm) {
         for (int k = 0; k < 3; k++)
            samples[3 * best_id + k] = 0.0;
         output_ppm(map->sys_nam[ssys], samples, SAMP, SAMP);
      }
      free(samples);
      if (g_opt)
         vcpy(dst, best);
   }

   local_opt(dst, &ssys_p);

   free(id_neigh);
   free(id_around);
   free((void *) ssys_p.neigh);
   free((void *) ssys_p.around);
   free((void *) ssys_p.edges);

   return 0;
}

void gen_map_reposition(struct s_ssys *map, bool g_opt, bool quiet,
                        bool gen_map, bool only, double ratio)
{
   char buff[512];
   int  num;

   if (!only)
      for (int i = map->nosys; i < map->nsys; i++) {
         const size_t n =
            snprintf(buff, 511, "%s %lf %lf%s\n", map->sys_nam[i],
                     map->sys[i].v[0], map->sys[i].v[1], map->sys_aux[i]);
         fwrite(buff, sizeof(char), n, stdout);
         fflush(stdout);
      }

   for (num = 0; num < THREADS - 1; num++)
      if (fork() == 0)
         break;

   for (int i = num; i < map->nosys; i += THREADS) {
      double v[2] = {map->sys[i].v[0], map->sys[i].v[1]};

      if (reposition_sys(v, map, i, g_opt, quiet, gen_map) && (!quiet))
         fprintf(stderr, "\"%s\" has no neighbor !\n", map->sys_nam[i]);

      const size_t n = snprintf(
         buff, 511, "%s %lf %lf%s\n", map->sys_nam[i],
         (v[0] + ratio * map->sys[i].v[0]) / (1.0 + ratio),
         (v[1] + ratio * map->sys[i].v[1]) / (1.0 + ratio), map->sys_aux[i]);
      fwrite(buff, sizeof(char), n, stdout);
      fflush(stdout);
   }

   if (num < THREADS - 1)
      exit(EXIT_SUCCESS);
   while (wait(NULL) != -1)
      ;
}

static size_t ssys_num(GHashTable *h, const char *s, struct s_ssys *map,
                       const double *src, char *aux, bool upd)
{
   size_t this = (size_t) g_hash_table_lookup(h, s);

   if (!this) {
      char *str = strdup(s);

      if ((map->nsys & (map->nsys + 1)) == 0) {
         const size_t new_siz = (map->nsys << 1) | 3;
         map->sys             = realloc(map->sys, new_siz * sizeof(struct sys));
         map->sys_nam         = realloc(map->sys_nam, new_siz * sizeof(char *));
         map->sys_aux         = realloc(map->sys_aux, new_siz * sizeof(char *));
      }
      upd                       = true;
      map->sys_aux[map->nsys]   = strdup("");
      map->sys_nam[map->nsys++] = str;
      this                      = map->nsys;
      g_hash_table_insert(h, (void *) str, (void *) this);
   }
   this --;
   if (upd) {
      vcpy(map->sys[this].v, src);
      map->sys[this].w = 0.0;
      if (aux) {
         aux            = strdup(aux);
         const size_t n = strlen(aux);
         if (n && aux[n - 1] == '\n')
            aux[n] = '\0';
         free(map->sys_aux[this]);
         map->sys_aux[this] = aux;
      }
   }
   return this;
}

int edge_cmp(const void *a, const void *b)
{
   const struct s_edge *sa = (const struct s_edge *) a;
   const struct s_edge *sb = (const struct s_edge *) b;
   return
      (sa->jmp[0] - sb->jmp[0]) ?:
      (sa->jmp[1] - sb->jmp[1]) ?:
      (sa->len < sb->len) ? -1 : (sa->len > sb->len)
   ;
}

/* main */
int do_it(char **onam, int n_onam, bool g_opt, bool gen_map, bool edges,
          bool only, bool ign_alone, bool quiet, double weight)
{
   GHashTable   *h       = g_hash_table_new(g_str_hash, g_str_equal);
   struct s_ssys map     = {0};
   const double  nulv[2] = {0.0, 0.0};

   for (int i = 0; i < n_onam; i++)
      (void) ssys_num(h, onam[i], &map, nulv, NULL, false);
   map.nosys = map.nsys;

   char  *line = NULL;
   size_t _n   = 0;
   int    n;
   while ((n = getline(&line, &_n, stdin)) != -1) {
      int    r1, r2, r3 = 0;
      double tmp[2] = {0.0, 0.0};
      double len    = 1.0;

      if (line[n - 1] == '\n')
         line[--n] = '\0';
      if (2 == sscanf(line, "%*s%n %lf %lf%n", &r1, tmp, tmp + 1, &r2)) {
         line[r1]        = '\0';
         const size_t id = ssys_num(h, line, &map, tmp, line + r2, true);
         map.sys[id].w   = 1.0;
      } else {
         sscanf(line, "%*s%n %n%*s%n%lf", &r1, &r2, &r3, &len);
         if (r3 > 0) {
            if (edges)
               puts(line);
            char *where = line + r3;
            do {
               where = strstr(where, " fake");
            } while (where && !strchr(" \n", *where));
            if (where)
               continue;
            if ((map.njumps & (map.njumps + 1)) == 0) {
               const size_t new_siz = (map.njumps << 1) | 1;
               map.jumps = realloc(map.jumps, new_siz * sizeof(struct s_edge));
            }
            line[r1] = line[r3] = '\0';
            const int a         = ssys_num(h, line, &map, tmp, NULL, false);
            const int b   = ssys_num(h, line + r2, &map, tmp, NULL, false);
            const int swp = (a > b) && 1;
            map.jumps[map.njumps].jmp[0 ^ swp] = a;
            map.jumps[map.njumps].jmp[1 ^ swp] = b;
            map.jumps[map.njumps].len          = len;
            map.njumps++;
         } else if (line[0] != '\0')
            fprintf(stderr, "Ignored line : \"%s\"\n", line);
         else
            break;
      }
   }
   free(line);
   g_hash_table_destroy(h);

   if (ign_alone) {
      // mark alone sys as 0-weighted.
      for (int i = 0; i < 2 * map.njumps; i++) {
         const int n = map.jumps[i / 2].jmp[i % 2];
         if (map.sys[n].w > 0.0 && map.sys[n].w <= 1.0)
            map.sys[n].w += 1.0;
      }
      for (int i = 0; i < map.nsys; i++)
         if (map.sys[i].w > 0.0)
            map.sys[i].w -= 1.0;
   }

   quiet = quiet || !map.nosys;
   if (!map.nosys)
      map.nosys = map.nsys;

   qsort(map.jumps, map.njumps, sizeof(struct s_edge), edge_cmp);
   int w = 1;
   for (int i = 1; i < map.njumps; i++)
      if (edge_cmp((const void *) (map.jumps + i),
                   (const void *) (map.jumps + w - 1)))
         memcpy((void *) (map.jumps + (w++)), (void *) (map.jumps + i),
                sizeof(struct s_edge));
   map.njumps = w;

   fflush(stdout);
   gen_map_reposition(&map, g_opt, quiet, gen_map, only, weight);
   for (int i = 0; i < map.nsys; i++) {
      free(map.sys_aux[i]);
      free(map.sys_nam[i]);
   }
   free(map.sys_nam);
   free(map.sys_aux);
   free(map.jumps);
   return EXIT_SUCCESS;
}

static int usage(char *nam, int ret, double w)
{
   fprintf(
      stderr,
      "Usage:  %s  [-g] [-i] [-o | -e] [-p] [-q] [-w<weight>] [<names>...]\n",
      basename(nam));
   fprintf(stderr,
           "  Reads a graph from standard input.\n"
           "  Applies repositioning to <names> and outputs the result.\n\n"
           "  If <names> is not provided, use all input names.\n"
           "  <names> is a set of ssys basenames.\n"
           "  If files are provided instead, their basename without "
           "\".xml\" is used.\n\n"
           "  If -g is set, look for global optimum (slower, and\n"
           "    might result in a different planar embedding).\n\n"
           "  If -i is set, isolated sys have no effect.\n"
           "  If -o is set, only output processed systems.\n"
           "  If -e is set, also output edges (as is).\n"
           "  If -p is set, outputs a ppm for each processed system.\n"
           "  If -q is set, forces quiet mode.\n\n"
           "  If <weight> is set (positive), outputs values in the form:\n"
           "    (<weight>*old_pos + new_pos) / (<weight> + 1.0)\n"
           "   - If not specified, <weight> defaults to %.1f.\n"
           "   - If you repos systems that are neighbors, its is strongly "
           "advised\n"
           "       to choose <weight> at the very least greater than 1.0.\n"
           "   - If you repos independent systems, you can (and should) use "
           "0.0.\n",
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
   bool  output_edges   = false;
   bool  gen_map        = false;
   bool  ign_alone      = false;
   bool  quiet          = false;

   int    fst_opt     = 1;
   int    fst_non_opt = 1;
   double weight      = 2.0;

   for (int i = 1; i < argc; i++)
      if (argv[i][0] == '-') {
         char *swp           = argv[fst_non_opt];
         argv[fst_non_opt++] = argv[i];
         argv[i]             = swp;
      }

   qsort(argv + fst_opt, fst_non_opt - fst_opt, sizeof(char *), cmpstringp);
   qsort(argv + fst_non_opt, argc - fst_non_opt, sizeof(char *), cmpstringp);
   fst_opt = fst_non_opt;
   for (int i = fst_non_opt - 1; i > 0; i--)
      if (strcmp(argv[i], argv[i - 1]))
         argv[--fst_opt - 1] = argv[i - 1];

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-e")) {
      output_edges = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-g")) {
      g_opt = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt &&
       (!strcmp(argv[fst_opt], "-h") || !strcmp(argv[fst_opt], "--help")))
      return usage(
         nam, fst_opt == fst_non_opt + 1 ? EXIT_SUCCESS : EXIT_FAILURE, weight);

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-i")) {
      ign_alone = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-o")) {
      processed_only = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-p")) {
      gen_map = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt && !strcmp(argv[fst_opt], "-q")) {
      quiet = true;
      fst_opt++;
   }

   if (fst_non_opt > fst_opt && !strncmp(argv[fst_opt], "-w", 2)) {
      if (sscanf(argv[fst_opt], "-w%lf", &weight) != 1)
         return usage(nam, EXIT_FAILURE, weight);
      if (weight < 0.0) {
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
                output_edges, processed_only, ign_alone, quiet, weight);
}
