#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// s/_\([vi][vi]*\)-\([a-z]\)$/_\1\2/;
size_t _parse_it(const char*s){
   size_t out;

   for(out = 0 ; s[out]=='v' || s[out]=='i'; out++);
   if(s[out]=='-' && s[out+1]>='a' && s[out+1]<='z' && 
      (!s[out+2] || s[out+2]=='\n')
   )
      return out;
   else
      return 0;
}

char *uniedit_nameFilter( const char *name ){
   size_t len = strlen( name ) + 1;
   char  *out = malloc( len );
   size_t r, w = 0;
   int res;

   // s/ /_/g;
   for(r = 0; r <= len ; r++)
      if (name[r]==' ')
         out[r] = '_';
      else
         out[r] = name[r];

   // s/["':.()?]//g;
   // s/&amp;/_and_/g;
   // s/-\([0-9]\)/\1/g;
   // s/_\(i*[vi]i*\)-\([a-z]\)$/_\1\2/;
   for(r = 0; r < len ; r++)
      if (strchr("':.()?", out[r]))
         {}
      else if (!strncmp(out+r, "&amp;", 5)){
         memcpy(out+w, "_and_", 5);
         w += 5;
         r += 5-1;
      }else if (out[r]=='-' && out[r+1]>='0' && out[r+1]<='9' )
         {}
      else if (out[r]=='_' && (res = _parse_it(out+r+1))){
         out[w++] = out[r++];
         memmove(out+w, out+r, res);
         w += res;
         r += res;
      }else
         out[w++] = out[r];

   out[w] = '\0';
   len = w;
   w = 0;
   // s/_-/-/g;
   // s/-_/-/g;
   // s/\(._\)_*/\1/g;
   for(r=0; r<len; r++)
      if (out[r]=='_' && out[r+1]=='-')
         {}
      else if (out[r]=='-' && out[r+1]=='_')
         out[w++] = out[r++];
      else if(r && out[r] == '_' && out[r+1] == '_')
         {}
      else
         out[w++] = out[r];
   out[w] = '\0';
   return out;
}

int main(){
   char *line = NULL;
   size_t len = 0;
   ssize_t nread;

   while ((nread = getline(&line, &len, stdin)) != -1) {
      char*tmp = uniedit_nameFilter(line);
      printf("%s", tmp);
      free(tmp);
   }
   free(line);
   return 0;
}
