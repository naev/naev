
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

#define THREADS 4
#define GRAV_FACT 0.1f

// defaults to 1.0
const struct {
   char *nam;
   float w;
} weights[] = {
   {"anubis_black_hole", 5.5f}, {NULL} // Sentinel
};

const float fact = 30.0f;
const float _rad = 30.0f;
// See HERE for their computation
float rad, mul_ct, add_ct, inv_fact;

static inline float grav_pot(float xs, float ys, float x, float y)
{
   const float dx = xs - x;
   const float dy = ys - y;
   const float d  = sqrt(dx * dx + dy * dy) * inv_fact;

   if (d < rad)
      return d * d * mul_ct + add_ct;
   else
      return -1.0f / d;
}

// The derivative of the previous ones.
static inline void accum_f(float *vx, float *vy, float xs, float ys, float x,
                           float y, float m, bool alt)
{
   float dx = xs - x;
   float dy = ys - y;
   float d  = sqrt(dx * dx + dy * dy);
   float f;

   dx /= d;
   dy /= d;

   d *= inv_fact;
   if (d < rad)
      f = 2.0f * d * mul_ct;
   else{
      f = 1.0f / d * d;
      if (alt)
         f *= rad / d;
   }
   if (alt)
      f *= 4.0;

   f *= m;
   *vx += f * dx;
   *vy += f * dy;
}

#define F_ATT 0.04f
#define A_ATT 0.00f
static inline float wav_pot(float xs, float ys, float x, float y)
{
   const float dx = xs - x;
   const float dy = ys - y;
   const float d  = sqrt(dx * dx + dy * dy) * inv_fact;

   return cos(d * M_PI / (1.0f + F_ATT * d)) / (1.0f + A_ATT * d) - 1.0f;
}

// The derivative of the previous ones.
static inline void wav_f(float *vx, float *vy, float xs, float ys, float x,
                         float y, float m)
{
   float dx = xs - x;
   float dy = ys - y;
   float d  = sqrt(dx * dx + dy * dy);
   float f;

   if (dx == 0.0f && dy == 0.0f)
      return;

   dx /= d;
   dy /= d;

   d *= inv_fact;
   const float inside = d * M_PI / (1.0f + F_ATT * d);
   const float den    = (1.0 + A_ATT * d) * (1.0 + A_ATT * d);
   f = M_PI / (1.0f + F_ATT * d) / (1.0f + F_ATT * d) * (-sin(inside)) *
          (1.0f + A_ATT) -
       F_ATT * cos(inside);
   f /= den;
   f *= 10.0f;

   f *= m;
   *vx += f * dx;
   *vy += f * dy;
}

void output_map_header(FILE *fp, int w, int h)
{
   char b[64];

   fwrite(b, sizeof(char), snprintf(b, 63, "P5 %d %d 65535\n", w, h), fp);
}

void output_map(FILE *fp, float *map, int w, int h, float fact, float ct)
{
   uint8_t *line;

   output_map_header(fp, w, h);

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

enum e_pot { NONE, GRAVITY, WAVES };

static float potential_at_ij(float *lst, size_t nb, enum e_pot p, int i, int j)
{
   if (p == GRAVITY) {
      float accu = 0.0f;
      for (size_t n = 0; n < nb; n++)
         accu += lst[4 * n + 2] *
                 grav_pot(lst[4 * n], lst[4 * n + 1], (float) j, (float) i);
      return accu;
   } else if (p == WAVES)
      return wav_pot(lst[0], lst[1], (float) j, (float) i);
   else
      return 0.0f;
}

void gen_potential(float *lst, size_t nb, enum e_pot typ, float scale)
{
   static float *maps[THREADS - 1];
   static int   *percent;
   char          buf[32] = {[0] = '\r'};
   float        *map;
   float         minx, maxx, miny, maxy;
   size_t        n;
   float         min_pot = 0.0; // working only with neg potentials

   if (!nb)
      return;

   for (n = 0; n < nb; n++) {
      lst[4 * n + 0] *= scale;
      lst[4 * n + 1] *= scale;
   }

   minx = maxx = lst[0];
   miny = maxy = lst[1];
   for (n = 1; n < nb; n++) {
      minx = lst[4 * n + 0] < minx ? lst[4 * n + 0] : minx;
      maxx = lst[4 * n + 0] > maxx ? lst[4 * n + 0] : maxx;
      miny = lst[4 * n + 1] < miny ? lst[4 * n + 1] : miny;
      maxy = lst[4 * n + 1] > maxy ? lst[4 * n + 1] : maxy;
   }

   if (minx == maxx || miny == maxy)
      return;

   const int minj = (int) floor(minx) - 2;
   const int maxj = (int) ceil(maxx) + 2;
   const int mini = (int) floor(miny) - 2;
   const int maxi = (int) ceil(maxy) + 2;
   const int w    = maxj - minj + 1;
   const int h    = maxi - mini + 1;

   if (isatty(1)) {
      fprintf(stderr, "The (binary) pgm should not be sent to a terminal.\n");
      output_map_header(stdout, w, h);
      fprintf(stderr, "[...] (truncated here)\n");
      return;
   }

#define FROM(num) (mini + (num) * h / THREADS)

   map = malloc(w * h * sizeof(float));

   int num;

   percent = mmap(NULL, THREADS * sizeof(int), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);

   memset(percent, 0, THREADS * sizeof(int));

   for (num = 0; num < THREADS - 1; num++) {
      maps[num] =
         mmap(NULL, (FROM(num + 1) - FROM(num)) * w * sizeof(float),
              PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

      if (fork() == 0) {
         for (int i = FROM(num); i < FROM(num + 1); i++) {
            const int per =
               100 * (i + 1 - FROM(num)) / (FROM(num + 1) - FROM(num));
            if (percent[num] != per) {
               int count    = 1;
               percent[num] = per;
               msync(percent + num, sizeof(int), MS_ASYNC);
               for (int u = 0; u < THREADS; u++)
                  count += sprintf(buf + count, " %2d%%", percent[u]);
               fwrite(buf, sizeof(char), count, stderr);
            }

            for (int j = minj; j <= maxj; j++)
               maps[num][(i - FROM(num)) * w + (j - minj)] =
                  potential_at_ij(lst, nb, typ, i, j);
         }
         msync(maps[num], (FROM(num + 1) - FROM(num)) * w * sizeof(float),
               MS_SYNC);
         exit(EXIT_SUCCESS);
      }
   }

   for (int i = FROM(num); i < FROM(num + 1); i++) {
      const int per = 100 * (i + 1 - FROM(num)) / (FROM(num + 1) - FROM(num));
      if (percent[num] != per) {
         int count    = 1;
         percent[num] = per;
         msync(percent + num, sizeof(int), MS_ASYNC);
         for (int u = 0; u < THREADS; u++)
            count += sprintf(buf + count, " %2d%%", percent[u]);
         fwrite(buf, sizeof(char), count, stderr);
      }

      for (int j = minj; j <= maxj; j++)
         map[(i - mini) * w + (j - minj)] = potential_at_ij(lst, nb, typ, i, j);
   }

   while (wait(NULL) != -1)
      ;
   fprintf(stderr, "\n");
   for (num = 0; num < THREADS - 1; num++) {
      const size_t siz = (FROM(num + 1) - FROM(num)) * w * sizeof(float);
      memcpy(map + (FROM(num) - mini) * w, maps[num], siz);
      munmap(maps[num], siz);
   }

   for (int i = 0; i < w * h; i++)
      if (map[i] <= min_pot)
         min_pot = map[i];

   output_map(stdout, map, w, h, -1.0f / min_pot, 1.0f);
   free(map);
#undef FROM
}

void apply_potential(const float *lst, size_t nb, enum e_pot t,
                     const char **nam, bool alt)
{
   char buff[512];
   int  num;

   for (num = 0; num < THREADS - 1; num++)
      if (fork() == 0)
         break;

#define FROM(num) (((num) * nb) / THREADS)
   for (size_t i = FROM(num); i < FROM(num + 1); i++) {
#undef FROM
      float  x = lst[4 * i + 0], y = lst[4 * i + 1];
      float  dx = 0.0f, dy = 0.0f;
      size_t n;

      if (t == GRAVITY) {
         for (size_t j = 0; j < nb; j++)
            if (j != i)
               accum_f(&dx, &dy, lst[4 * j + 0], lst[4 * j + 1], x, y,
                       lst[4 * j + 2], alt);
      } else if (t == WAVES)
         wav_f(&dx, &dy, lst[0], lst[1], x, y, lst[2]);

      dx *= GRAV_FACT;
      dy *= GRAV_FACT;
      n = snprintf(buff, 511, "%s %f %f\n", nam[i], x + dx, y + dy);
      fwrite(buff, sizeof(char), n, stdout);
      fflush(stdout);
   }

   if (num < THREADS - 1)
      exit(EXIT_SUCCESS);
}

int do_it(const enum e_pot type, const float scale, const bool apply,
          const bool alt)
{
   char   buf[256];
   float *lst = NULL;
   char **nam = NULL;
   size_t nb  = 0;

   char  *line = NULL;
   size_t n    = 0;
   while (getline(&line, &n, stdin) != -1) {
      if ((nb & (nb + 1)) == 0) {
         lst = realloc(lst, ((nb << 1) | 1) * sizeof(float) * 4);
         if (apply)
            nam = realloc(nam, ((nb << 1) | 1) * sizeof(char *));
      }

      float *const dst = lst + 4 * nb;
      if (3 == sscanf(line, "%255s %f %f", buf, dst, dst + 1)) {
         if (apply)
            nam[nb] = strdup(buf);
         dst[2] = 1.0;
         for (int i = 0; weights[i].nam; i++)
            if (!strcmp(weights[i].nam, buf))
               dst[2] = weights[i].w;
         if (type == WAVES && !strcmp(buf, "sol")) { // sol first!
            if (apply) {
               char *const sswp = nam[nb];
               nam[nb]          = nam[0];
               nam[0]           = sswp;
            }
            for (int k = 0; k < 3; k++) {
               const float swp = lst[k];
               lst[k]          = dst[k];
               dst[k]          = swp;
            }
         }
         nb++;
      }else if(apply)
         fputs(line, stdout);
   }
   // fprintf(stderr,"[%zd systems]\n",nb);
   free(line);

   // HERE
   rad      = _rad * scale / fact;
   mul_ct   = 1.0f / (2.0f * rad * rad * rad);
   add_ct   = -3.0f / (2.0f * rad);
   inv_fact = 1.0f / fact;

   if (apply) {
      apply_potential(lst, nb, type, (const char **) nam, alt);
      for (size_t i = 0; i < nb; i++)
         free(nam[i]);
      free(nam);
   } else
      gen_potential(lst, nb, type, scale);
   free(lst);
   return EXIT_SUCCESS;
}

int usage(char *nam, int ret)
{
   fprintf(stderr, "Usage: %s  [ -s<scale> | -a ]  ( -E | -g | -w )\n",
           basename(nam));
   fprintf(
      stderr,
      "  Reads a set of node pos in input from standard input.\n"
      "\n"
      "  If -a is set, applies resulting potential and outputs the result.\n"
      "  If not, outputs a pgm of the potential.\n"
      "  All outputs are sent to standard output.\n"
      "\n"
      "  If -s is set, applies the scaling factor <scale> to "
      "input, e.g. 0.25\n"
      "\n"
      "  If -E is set, uses experimental alternate gravity. DOES ONLY WORK "
      "with -a.\n"
      "  If -g is set, generates the gravity potential.\n"
      "  If -w is set, generates the waves potential.\n"
      "\n");
   return ret;
}

int main(int argc, char **argv)
{
   enum e_pot type  = NONE;
   char      *nam   = argv[0];
   bool       alt   = false;
   bool       apply = false;
   float      scale = 1.0f;

   if (argc > 1 && !strncmp(argv[1], "-s", 2)) {
      if (argc < 3 || sscanf(argv[1], "-s%f", &scale) != 1) {
         fprintf(stderr, "float expected, found \"%s\".\n", argv[1] + 2);
         return usage(nam, EXIT_FAILURE);
      }
      argv++;
      argc--;
   }

   if (argc > 1 && !strcmp(argv[1], "-a")) {
      apply = true;
      argv++;
      argc--;
   }

   if (argc > 1 && !strcmp(argv[1], "-E")) {
      if (!apply)
         return usage(nam, EXIT_FAILURE);
      type = GRAVITY;
      alt  = true;
      argv++;
      argc--;
   }

   if (argc > 1 && !strcmp(argv[1], "-g")) {
      type = GRAVITY;
      argv++;
      argc--;
   }

   if (argc > 1 && !strcmp(argv[1], "-w")) {
      if (type == GRAVITY) {
         fprintf(stderr, "Warning: incompatible -g and -w.\n");
         return usage(nam, EXIT_FAILURE);
      } else
         type = WAVES;
      argv++;
      argc--;
   }

   if (argc > 1)
      if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
         return usage(nam, EXIT_SUCCESS);
      else
         return usage(nam, EXIT_FAILURE);
   else
      return do_it(type, scale, apply, alt);
}
