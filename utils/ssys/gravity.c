
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <math.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


// defaults to 1.0
const struct{char*nam;float w;} weights[]={
   {"anubis_black_hole", 5.5},
   {NULL}   // Sentinel
};

#define PROCESSES 2

const float fact = 30.0f;
const float rad = 30.0f / fact;
const float mul_ct = 1.0 / (2.0f*rad*rad*rad);
const float add_ct = -3.0 / (2.0*rad);
const float inv_fact = 1.0 / fact;

static inline float _pot(float xs, float ys, float x, float y){
   const float dx = xs - x;
   const float dy = ys - y;
   const float d = sqrt(dx*dx + dy*dy) * inv_fact;

   if(d < rad)
      return d*d*mul_ct + add_ct;
   else
      return -1.0f / d;
}

// The derivative of the previous one.
static inline void accum_f(float*vx,float*vy,float xs, float ys, float x, float y, float m){
   float dx = xs - x;
   float dy = ys - y;
   float d = sqrt(dx*dx + dy*dy);
   float f;

   dx /= d;
   dy /= d;

   d *= inv_fact;

   if(d < rad)
      f = 2.0*d * mul_ct;
   else
      f = 1.0f / d*d;

   f *= m;
   *vx += f * dx;
   *vy += f * dy;
}

void output_map(float*map, int w, int h, float fact, float ct, bool head_only){
   char buf[64];
   FILE*fp = stdout;
   uint8_t *line;

   fwrite(buf, sizeof(char), sprintf(buf, "P5 %d %d 65535\n", w, h), fp);
   if(head_only)
      return;

   line = (uint8_t *) calloc((size_t)w, sizeof(uint16_t));
   for(int i=0; i<h; i++){
      for(int j=0; j<w; j++){
         const unsigned u = 65535.0 * (fact * map[(h-i-1)*w + j] + ct);
         line[2*j] = u>>8;
         line[2*j+1] = u&0xff;
      }
      fwrite(line, sizeof(uint16_t), w, fp);
   }
   fclose(fp);
   free(line);
}

void gen_potential(float*lst, size_t nb){
   static float*maps[PROCESSES-1];
   static int*percent;
   char buf[32]={[0]='\r'};
   float*map;
   float minx, maxx, miny, maxy;
   size_t n;
   float min_pot = 0.0; // working only with neg potentials

   if(!nb)
      return;

   minx = maxx = lst[0];
   miny = maxy = lst[1];
   for(n=1; n<nb; n++){
      minx = lst[4*n] < minx ?   lst[4*n]:   minx;
      maxx = lst[4*n] > maxx ?   lst[4*n]:   maxx;
      miny = lst[4*n+1] < miny ? lst[4*n+1]: miny;
      maxy = lst[4*n+1] > maxy ? lst[4*n+1]: maxy;
   }

   if(minx == maxx || miny == maxy)
      return;

   const int minj = (int) floor(minx) - 2;
   const int maxj = (int)  ceil(maxx) + 2;
   const int mini = (int) floor(miny) - 2;
   const int maxi = (int)  ceil(maxy) + 2;

   if(isatty(1)){
      fprintf(stderr, "The (binary) pgm should be sent to a terminal.");
      output_map(NULL, maxj-minj+1, maxi-mini+1, 0.0, 0.0, true);
      fprintf(stderr, "[...] (truncated here)\n");
      return;
   }

   #define FROM(num) (mini + (num) * (maxi-mini+1) / PROCESSES)
   #define TO(num)   (FROM((num)+1) - 1)

   map = malloc((maxi-mini+1)*(maxj-minj+1)*sizeof(float));

   int num;

   percent = mmap(NULL, PROCESSES * sizeof(int),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

   memset(percent, 0, PROCESSES * sizeof(int));

   for(num = 0; num < PROCESSES - 1; num++){
      maps[num] = mmap(NULL, (TO(num)-FROM(num)+1) * (maxj-minj+1) * sizeof(float),
         PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

      if(fork() == 0){
         for(int i=FROM(num); i<=TO(num); i++){
            const float per = 100 * (i-FROM(num)) / (TO(num)-FROM(num));
            if(percent[num] != per ){
               int count=1;
               percent[num] = per;
               msync(percent+num, sizeof(int), MS_ASYNC);
               for(int u=0; u<PROCESSES; u++)
                  count+=sprintf(buf+count," %2d%%", percent[u]);
               fwrite(buf, sizeof(char), count, stderr);
            }

            for(int j=minj; j<=maxj; j++){
               float f = 0.0f;
               for(n=0; n<nb; n++)
                 f += lst[4*n+2] * _pot(lst[4*n], lst[4*n+1], (float)j, (float)i);
               maps[num][(i-FROM(num)) * (maxj-minj+1) + (j-minj)] = f;
            }
         }
         msync(maps[num], (TO(num)-FROM(num)+1) * (maxj-minj+1) * sizeof(float), MS_SYNC);
         exit(EXIT_SUCCESS);
      }
   }

   for(int i=FROM(num); i<=TO(num); i++){
      const float per = 100 * (i-FROM(num)) / (TO(num)-FROM(num));
      if(percent[num] != per ){
         int count=1;
         percent[num] = per;
         msync(percent+num, sizeof(int), MS_ASYNC);
         for(int u=0; u<PROCESSES; u++)
            count+=sprintf(buf+count," %2d%%", percent[u]);
         fwrite(buf, sizeof(char), count, stderr);
      }

      for(int j=minj; j<=maxj; j++){
         float f = 0.0f;
         for(n=0; n<nb; n++)
           f += lst[4*n+2] * _pot(lst[4*n], lst[4*n+1], (float)j, (float)i);
         map[(i-mini) * (maxj-minj+1) + (j-minj)] = f;
      }
   }

   while(wait(NULL) != -1);
   fprintf(stderr, "\n");
   for(num = 0; num < PROCESSES-1; num++){
      memcpy(map+(FROM(num)-mini)*(maxj-minj+1), maps[num],
         (TO(num)-FROM(num)+1)*(maxj-minj+1)*sizeof(float));
      munmap(maps[num], (TO(num)-FROM(num)+1) * (maxj-minj+1) * sizeof(float));
   }

   for(int i=0; i<(maxi-mini+1)*(maxj-minj+1); i++)
      if(map[i] <= min_pot)
         min_pot = map[i];

   output_map(map, maxj-minj+1, maxi-mini+1, -1.0/min_pot, 1.0, false);
   free(map);
   #undef FROM
   #undef TO
}


#define GRAV_FACT 0.1

void apply_gravity(const float*lst, const size_t nb, const char**names){
   char buff[512];
   int num;

   for(num = 0; num < PROCESSES-1; num++)
      if(fork() == 0)
         break;

   #define FROM(num) (((num)*nb) / PROCESSES)
   for(size_t i = FROM(num); i < FROM(num+1); i++){
      float x=lst[4*i+0], y=lst[4*i+1];
      float dx=0.0f, dy=0.0f;
      size_t n;
      for(size_t j=0; j<nb; j++)
         if(j != i)
            accum_f(&dx, &dy, lst[4*j+0], lst[4*j+1], x, y, lst[4*j+2]);
      dx *= GRAV_FACT;
      dy *= GRAV_FACT;
      n = sprintf(buff, "%s %f %f\n", names[i], x+dx, y+dy);
      fwrite(buff, sizeof(char), n, stdout);
      fflush(stdout);
   }
   #undef FROM

   if(num < PROCESSES-1)
      exit(EXIT_SUCCESS);
}

int do_it(bool apply){
   char buf[256];
   char*line = NULL;
   size_t n = 0;
   float*lst = NULL;
   char**names = NULL;
   size_t nb = 0;

   while( getline(&line, &n, stdin) != -1 ){
      if((nb & (nb+1)) == 0){
         lst = realloc(lst, ((nb<<1)|1)*sizeof(float)*4);
         if(apply)
            names = realloc(names, ((nb<<1)|1)*sizeof(char*));
      }
      sscanf(line, "%255s %f %f", buf, lst+(4*nb), lst+(4*nb+1));
      lst[4*nb+2] = 1.0;
      if(apply)
         names[nb] = strdup(buf);
      for(int i=0; weights[i].nam; i++)
         if(!strcmp(weights[i].nam, buf))
            lst[4*nb+2] = weights[i].w;
      nb++;
   }
   free(line);
   if(apply)
      apply_gravity(lst, nb, (const char**)names);
   else
      gen_potential(lst, nb);
   free(lst);
   return EXIT_SUCCESS;
}

int usage(char*nam, int ret){
   fprintf(stderr,"Usage: %s [-a]\n", basename(nam));
   fprintf(stderr,"  Reads a set of node pos in input from standard input.\n");
   fprintf(stderr,"  If -a is set, applies resulting gravity and outputs the result.\n");
   fprintf(stderr,"  If not, outputs a pgm of the gravity potential.\n");
   fprintf(stderr,"  All outputs are sent to standard output.\n");
   return ret;
}

int main(int argc, char**argv){
   char*nam = argv[0];
   bool apply = false;

   if(argc>1 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))
      return usage(nam, EXIT_SUCCESS);

   if(argc>1 && !strcmp(argv[1], "-a")){
      apply = true;
      argv++; argc--;
   }

   if(argc>1)
      return usage(nam, EXIT_FAILURE);
   else
      return do_it(apply);
}
