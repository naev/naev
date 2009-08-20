#include "object.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

void object_loadFromFile( const char *filename, Object *object)
{
   const char *delim = " \t";
   FILE *f = fopen(filename, "r");
   assert("Cannot open object file" && (f != NULL));

   object->vertices = array_create(Vertex);
   object->faces = array_create(Face);

   char line[256];
   while (fgets(line, sizeof(line), f)) {
      assert("Line too long" && (line[strlen(line) - 1] == '\n'));

      const char *token;
      token = strtok(line, delim);

      if (strcmp(token, "v") == 0) {
         DEBUG("Reading a vertex (%s)", token);

         int num = 0;
         Vertex *vertex = &array_grow(&object->vertices);

         while ((token = strtok(NULL, delim)) != NULL) {
            double coord;
            sscanf(token, "%lf", &coord);
            vertex->coords[num++] = coord;

            DEBUG("Read %.3lf", coord);
         }

         assert("Too few or too many coordinates" && (num == 3));
      } else if (strcmp(token, "f") == 0) {
         DEBUG("Reading a face (%s)", token);

         int num = 0;
         Face *face = &array_grow(&object->faces);

         /* XXX reads only the geometric vertices.
          * The standards says faces can include texture and normal vertices.
          */
         while ((token = strtok(NULL, delim)) != NULL) {
            int index_;
            sscanf(token, "%d", &index_);
            face->indices[num++] = index_ - 1;
         }
      } else if (token[0] == '#') {
         /* Comment */
         DEBUG("%s", line);
      } else {
         WARN("Can't read %s", token);
      }
   }


   fclose(f);
}
