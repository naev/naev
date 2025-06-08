
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <math.h>


const float fact = 25.0f;
const float rad = 25.0f / fact;

static float _pot(float xs, float ys, float x, float y){
   const float dx = x - xs;
   const float dy = y - ys;
   const float d = sqrt(dx*dx + dy*dy) / fact;

   if(d < rad)
      return d*d*(1.0/(2.0f*rad*rad*rad)) + (-3.0/2.0*rad);
   else
      return -1.0f/d;
}

void output_map(float*map, int w, int h){
   char buf[1024];
   FILE*fp = fopen("out.pgm", "wb");
   unsigned char*line;

   fwrite(buf, sizeof(char), sprintf(buf, "P5 %d %d 255\n", w, h), fp);

   line = calloc((size_t)w, sizeof(char));
   for(int i=0; i<h; i++){
      for(int j=0; j<w; j++)
         line[j] = (unsigned) (255.0 * map[(h - i -1) * w + j]);
      fwrite(line, sizeof(char), w, fp);
   }
   fclose(fp);
   free(line);
}

void gen_potential(float*lst, size_t nb){
   float minx, maxx, miny, maxy;
   int minj, maxj, mini, maxi;
   size_t n;
   float*map;
   float min_pot = 0.0; // working only with neg potentials

   if(!nb)
      return;

   minx = maxx = lst[0];
   miny = maxy = lst[1];
   for(n=1; n<nb; n++){
      minx = lst[2*n] < minx ?   lst[2*n]:   minx;
      maxx = lst[2*n] > maxx ?   lst[2*n]:   maxx;
      miny = lst[2*n+1] < miny ? lst[2*n+1]: miny;
      maxy = lst[2*n+1] > maxy ? lst[2*n+1]: maxy;
   }

   if(minx == maxx || miny == maxy)
      return;

   minj = (int) floor(minx);
   maxj = (int)  ceil(maxx);
   mini = (int) floor(miny);
   maxi = (int)  ceil(maxy);

   map = malloc((maxi-mini+1)*(maxj-minj+1)*sizeof(float));

   for(int i=mini; i<=maxi; i++){
      if(!(i&0x1f))
         fprintf(stderr,"\r%2d%%",100*(i-mini)/(maxi-mini));
      for(int j=minj; j<=maxj; j++){
         float f = 0.0f;
         for(n=0; n<nb; n++)
           f += _pot(lst[2*n], lst[2*n+1], (float)j, (float)i);

         if(f <= min_pot)
            min_pot = f;
         map[(i-mini) * (maxj-minj+1) + (j-minj)] = f;
      }
   }
   fprintf(stderr,"\r%10s\r", "");
   for(int i=0; i<(maxi-mini+1)*(maxj-minj+1) ; i++)
      map[i] = 1.0 + map[i]/(-min_pot);

   output_map(map, maxj-minj+1, maxi-mini+1);
   free(map);
}

int potential(){
   char*line = NULL;
   size_t n = 0;
   float*lst = NULL;
   size_t nb = 0;

   while( getline(&line, &n, stdin) != -1 ){
      if((nb & (nb+1)) == 0)
         lst = realloc(lst, ((nb<<1)|1)*sizeof(float)*2);
      sscanf(line, "%*s %f %f", lst+(2*nb), lst+(2*nb+1));
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
