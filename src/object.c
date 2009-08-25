#include "object.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SDL_image.h"

#include "log.h"
#include "array.h"

#define DELIM " \t"

typedef struct {
   GLfloat ver[3];
   GLfloat tex[2];
} Vertex;

static GLuint texture_loadFromFile( const char *filename)
{
   /* Reads image and converts it to RGBA */
   SDL_Surface *brute = IMG_Load(filename);
   SDL_Surface *image = SDL_DisplayFormatAlpha(brute);

   DEBUG("image = %p bpp %d", image, (int)image->format->BytesPerPixel);

   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);

   glTexImage2D(GL_TEXTURE_2D, 0, 4, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   SDL_FreeSurface(brute);
   SDL_FreeSurface(image);
   return texture;
}

static int readGLfloat(GLfloat *dest)
{
   char *token;
   int num = 0;

   while ((token = strtok(NULL, DELIM)) != NULL) {
      double d;
      sscanf(token, "%lf", &d);
      dest[num++] = d;
   }

   return num;
}

Object *object_loadFromFile( const char *filename )
{
   GLfloat *vertex = array_create(GLfloat);   /**< vertex coordinates */
   GLfloat *texture = array_create(GLfloat);  /**< texture coordinates */
   Vertex *corners = array_create(Vertex);

   FILE *f = fopen(filename, "r");
   if (!f)
      ERR("Cannot open object file %s", filename);

   char line[256];
   while (fgets(line, sizeof(line), f)) {
      assert("Line too long" && (line[strlen(line) - 1] == '\n'));

      const char *token;
      token = strtok(line, DELIM);

      if (strcmp(token, "v") == 0) {
         (void)array_grow(&vertex);
         (void)array_grow(&vertex);
         (void)array_grow(&vertex);

         int num = readGLfloat(array_end(vertex) - 3);
         assert("Too few or too many coordinates in vertex." && (num == 3));
      } else if (strcmp(token, "vt") == 0) {
         (void)array_grow(&texture);
         (void)array_grow(&texture);

         int num = readGLfloat(array_end(texture) - 2);
         assert("Too few or too many coordinates in texture." && (num == 2));
      } else if (strcmp(token, "f") == 0) {
         /* XXX reads only the geometric & texture vertices.
          * The standards says corners can also include normal vertices.
          */
         int num = 0;
         while ((token = strtok(NULL, DELIM)) != NULL) {
            int i_v, i_t;
            sscanf(token, "%d/%d", &i_v, &i_t);

            assert("Vertex index out of range." && (0 < i_v && i_v <= array_size(vertex) / 3));
            assert("Texture index out of range." && (0 < i_t && i_t <= array_size(texture) / 2));

            Vertex *face = &array_grow(&corners);
            --i_v, --i_t;
            memcpy(face->ver, vertex  + i_v * 3, sizeof(GLfloat) * 3);
            memcpy(face->tex, texture + i_t * 2, sizeof(GLfloat) * 2);
            ++num;
         }

         assert("Too few or too many vertices for a face." && (num == 3));
      } else if (token[0] == '#') {
         /* Comment */
      } else {
         WARN("Can't understand token %s", token);
      }
   }

   DEBUG("Read vertex %d texture %d face %d %d",
         array_size(vertex), array_size(texture), array_size(corners), array_size(corners) / 3);

   /* creates the object */
   Object *object = (Object *)malloc(sizeof(Object));
   object->num_corners = array_size(corners);

   /* stores the mesh in a vbo */
   object->object = gl_vboCreateStatic(
      array_size(corners) * sizeof(Vertex), corners);
   assert(!glGetError());

   /* texture */
   object->texture = texture_loadFromFile("/home/alexandru/src/admonisher.png");

   /* cleans up */
   fclose(f);
   /*
   array_free(vertex);
   array_free(texture);
   array_free(corners);
   */

   return object;
}

void object_render( Object *object )
{
   /* computes relative addresses of the vertice and texture coords */
   int ver_offset = (int)(&((Vertex *)NULL)->ver);
   int tex_offset = (int)(&((Vertex *)NULL)->tex);

   /* FIXME how much to scale the object */
   const double scale = 1. / 40.;

   /* rotates the object to match projection */
   double zoom;
   gl_cameraZoomGet(&zoom);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glScalef(scale * zoom, scale * zoom, scale * zoom);
   glRotatef(180., 0., 1., 0.);
   glRotatef(90., 1., 0., 0.);

   /* texture is initially flipped vertically */
   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glScalef(+1., -1., +1.);

   /* XXX wrong projection */
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();

   /* activates vertices and texture coords */
   gl_vboActivateOffset(object->object,
         GL_VERTEX_ARRAY, ver_offset, 3, GL_FLOAT, sizeof(Vertex));
   gl_vboActivateOffset(object->object,
         GL_TEXTURE_COORD_ARRAY, tex_offset, 2, GL_FLOAT, sizeof(Vertex));

   /* binds textures */
   glBindTexture(GL_TEXTURE_2D, object->texture);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   glEnable(GL_TEXTURE_2D);
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);  /* this changes the global DepthFunc */

   glColor4f(1.0, 1.0, 1.0, 1.0);
   glDrawArrays(GL_TRIANGLES, 0, object->num_corners);

   gl_vboDeactivate();
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_DEPTH_TEST);

   /* restores all matrices */
   glPopMatrix();
   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}
