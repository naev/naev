
#include <stdio.h>
#include <stdlib.h>
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

#define PROCESSES 4

const float fact = 30.0f;
const float rad = 30.0f / fact;
const float mul_ct = 1.0 / (2.0f*rad*rad*rad);
const float add_ct = (-3.0/2.0) * rad;
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

void output_map(float*map, int w, int h, float fact, float ct){
   char buf[32];
   FILE*fp = fopen("out.pgm", "wb");
   unsigned char*line;

   fwrite(buf, sizeof(char), sprintf(buf, "P5 %d %d 255\n", w, h), fp);

   line = calloc((size_t)w, sizeof(char));
   for(int i=0; i<h; i++){
      for(int j=0; j<w; j++)
         line[j] = (unsigned) (255.0 * (fact * map[(h-i-1)*w + j] + ct));
      fwrite(line, sizeof(char), w, fp);
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

   const int minj = (int) floor(minx);
   const int maxj = (int)  ceil(maxx);
   const int mini = (int) floor(miny);
   const int maxi = (int)  ceil(maxy);

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

   output_map(map, maxj-minj+1, maxi-mini+1, -1.0/min_pot, 1.0);
   free(map);
   #undef FROM
   #undef TO
}

int potential(){
   char buf[256];
   char*line = NULL;
   size_t n = 0;
   float*lst = NULL;
   size_t nb = 0;

   while( getline(&line, &n, stdin) != -1 ){
      if((nb & (nb+1)) == 0)
         lst = realloc(lst, ((nb<<1)|1)*sizeof(float)*4);
      sscanf(line, "%s %f %f", buf, lst+(4*nb), lst+(4*nb+1));
      lst[4*nb+2] = 1.0;
      for(int i=0; weights[i].nam; i++)
         if(!strcmp(weights[i].nam, buf))
            lst[4*nb+2] = weights[i].w;
      nb++;
   }
   free(line);

   gen_potential(lst, nb);
   free(lst);
   return EXIT_SUCCESS;
}


int main(int argc, char**argv){
   if(argc>1){
      fprintf(stderr,"Usage: %s\n", basename(argv[0]));
      return strcmp(argv[1], "-c") ? EXIT_FAILURE : EXIT_SUCCESS;
   }else
      return potential();
}
