#include "object.h"

#include <assert.h>
#include <string.h>
#include <libgen.h>
#include "SDL_image.h"

#include "array.h"
#include "gui.h"
#include "log.h"
#include "ndata.h"

#define DELIM " \t\n"


typedef struct {
   GLfloat ver[3];
   GLfloat tex[2];
} Vertex;


static void mesh_create( Mesh **meshes, const char* name,
                         Vertex *corners, int material )
{
   if (array_size(corners) == 0)
      return;
   if (name == NULL)
      ERR("No name for current part");
   if (material == -1)
      ERR("No material for current part");

   Mesh *mesh = &array_grow(meshes);
   mesh->name = strdup(name);
   mesh->vbo = gl_vboCreateStatic(array_size(corners) * sizeof(Vertex), corners);
   mesh->num_corners = array_size(corners);
   mesh->material = material;
   array_clear(corners);
}

static int readGLfloat( GLfloat *dest, int how_many )
{
   char *token;
   int num = 0;

   while ((token = strtok(NULL, DELIM)) != NULL) {
      double d;
      sscanf(token, "%lf", &d);
      dest[num++] = d;
   }

   if (how_many)
      assert(num == how_many);
   return num;
}


static int texture_loadFromFile( const char *filename, GLuint *texture )
{
   SDL_RWops *rw;
   SDL_Surface *brute, *image;
   DEBUG("Loading texture from %s", filename);

   /* Reads image and converts it to RGBA */
   rw = ndata_rwops(filename);
   if (rw != NULL)
      brute = IMG_Load_RW(rw, 1);
   if (brute != NULL)
      image = SDL_DisplayFormatAlpha(brute);
   if (rw == NULL || brute == NULL || image == NULL) {
      WARN("Failed to load texture '%s' from ndata", filename);
      return 0;
   }

   glGenTextures(1, texture);
   glBindTexture(GL_TEXTURE_2D, *texture);

   glTexImage2D(GL_TEXTURE_2D, 0, 4, image->w, image->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, image->pixels);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   SDL_FreeSurface(brute);
   SDL_FreeSurface(image);
   return 1;
}


/*
 * @brief Behaves exactly as standard fgets() but accepts a
 * SDL_RWops context instead of a FILE
 */
static char* SDL_RWgets( char *str, int size, SDL_RWops *ctx )
{
   int i, done = 0;
   int read, to_read = 16;

   while (done + 1 < size) {
      if (to_read + done + 1 > size)
         to_read = size - (done + 1);
      read = SDL_RWread(ctx, str + done, 1, to_read);

      if (read == 0) {
         /* EOF occured */
         if (done == 0)
            /* No bytes read */
            return NULL;
         break;
      }

      for (i = 0; i < read; ++i)
         if (str[done + i] == '\n') {
            str[done + i + 1] = 0;
            SDL_RWseek(ctx, - read + i + 1, SEEK_CUR);
            return str;
         }

      done += read;
      to_read *= 2;
   }

   assert(done + 1 == size);
   str[done] = '\0';
   return str;
}

static void materials_readFromFile( const char *filename, Material **materials )
{
   DEBUG("Loading material from %s", filename);

   SDL_RWops *f = ndata_rwops(filename);
   if (!f)
      ERR("Cannot open object file %s", filename);

   Material *curr = &array_back(*materials);

   char line[256];
   while (SDL_RWgets(line, sizeof(line), f)) {
      const char *token;
      assert("Line too long" && (line[strlen(line) - 1] == '\n'));
      token = strtok(line, DELIM);

      if (token == NULL) {
         /* Missing */
      } else if (strcmp(token, "newmtl") == 0) {
         token = strtok(NULL, DELIM);
         curr = &array_grow(materials);
         curr->name = strdup(token);
         DEBUG("Reading new material %s", curr->name);
      } else if (strcmp(token, "Ns") == 0) {
         readGLfloat(&curr->Ns, 1);
      } else if (strcmp(token, "Ni") == 0) {
         readGLfloat(&curr->Ni, 1);
      } else if (strcmp(token, "d") == 0) {
         readGLfloat(&curr->d, 1);
      } else if (strcmp(token, "Ka") == 0) {
         readGLfloat(curr->Ka, 3);
         curr->Ka[3] = 1.0;
      } else if (strcmp(token, "Kd") == 0) {
         readGLfloat(curr->Kd, 3);
         curr->Kd[3] = 1.0;
      } else if (strcmp(token, "Ks") == 0) {
         readGLfloat(curr->Ks, 3);
         curr->Ks[3] = 1.0;
      } else if (strcmp(token, "map_Kd") == 0) {
         token = strtok(NULL, DELIM);
         if (token[0] == '-')
            ERR("Options not supported for map_Kd");

         /* computes the path to texture */
         char *copy_filename = strdup(filename);
         char *dn = dirname(copy_filename);
         char *texture_filename = malloc(strlen(filename) + 1 + strlen(token) + 1);
         strcpy(texture_filename, dn);
         strcat(texture_filename, "/");
         strcat(texture_filename, token);

         curr->has_texture = texture_loadFromFile(
               texture_filename, &curr->texture) != 0;
         free(copy_filename);
         free(texture_filename);
      } else if (token[0] == '#') {
         /* Comment */
      } else {
         WARN("Can't understand token %s", token);
      }
   }

   SDL_RWclose(f);
}


/**
 * @brief Loads object
 *
 * Object file format is described here
 * http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
 *
 * @param filename base file name
 * @return and Object containing the 3d model
 */
Object *object_loadFromFile( const char *filename )
{
   GLfloat *vertex = array_create(GLfloat, NULL);   /**< vertex coordinates */
   GLfloat *texture = array_create(GLfloat, NULL);  /**< texture coordinates */
   Vertex *corners = array_create(Vertex, NULL);

   SDL_RWops *f = ndata_rwops(filename);
   if (!f)
      ERR("Cannot open object file %s", filename);

   char *name = NULL;
   int material = -1;

   Object *object = calloc(1, sizeof(Object));
   object->meshes = array_create(Mesh, NULL);
   object->materials = array_create(Material, clear_memory);

   char line[256];
   while (SDL_RWgets(line, sizeof(line), f)) {
      const char *token;
      assert("Line too long" && (line[strlen(line) - 1] == '\n'));
      token = strtok(line, DELIM);

      if (token == NULL) {
         /* Missing */
      } else if (strcmp(token, "mtllib") == 0) {
         while ((token = strtok(NULL, DELIM)) != NULL) {
            /* token contains the filename describing materials */

            /* computes the path to materials */
            char *copy_filename = strdup(filename);
            char *dn = dirname(copy_filename);
            char *material_filename = malloc(strlen(filename) + 1 + strlen(token) + 1);
            strcpy(material_filename, dn);
            strcat(material_filename, "/");
            strcat(material_filename, token);

            materials_readFromFile(material_filename, &object->materials);
            free(copy_filename);
            free(material_filename);
         }
      } else if (strcmp(token, "o") == 0) {
         mesh_create(&object->meshes, name, corners, material);
         token = strtok(NULL, DELIM);
         free(name), name = strdup(token);
      } else if (strcmp(token, "v") == 0) {
         (void)array_grow(&vertex);
         (void)array_grow(&vertex);
         (void)array_grow(&vertex);
         readGLfloat(array_end(vertex) - 3, 3);
      } else if (strcmp(token, "vt") == 0) {
         (void)array_grow(&texture);
         (void)array_grow(&texture);
         readGLfloat(array_end(texture) - 2, 2);
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
      } else if (strcmp(token, "usemtl") == 0) {
         mesh_create(&object->meshes, name, corners, material);

         /* a new mesh with the same name */
         token = strtok(NULL, DELIM);
         for (material = 0; material < array_size(object->materials); ++material)
            if (strcmp(token, object->materials[material].name) == 0)
               break;

         if (material == array_size(object->materials))
            ERR("No such material %s", token);
      } else if (token[0] == '#') {
         /* Comment */
      } else {
         WARN("Can't understand token %s", token);
      }
   }

   mesh_create(&object->meshes, name, corners, material);
   free(name);

   /* cleans up */
   array_free(vertex);
   array_free(texture);
   array_free(corners);
   SDL_RWclose(f);

   return object;
}


/**
 * @brief Frees memory reserved for the object
 */
void object_free( Object *object )
{
   int i;

   if (object == NULL)
      return;

   for (i = 0; i < (int)array_size(object->materials); ++i) {
      Material *material = &object->materials[i];
      free(material->name);
      if (material->has_texture)
         glDeleteTextures(1, &material->texture);
   }

   for (i = 0; i < (int)array_size(object->meshes); ++i) {
      Mesh *mesh = &object->meshes[i];
      free(mesh->name);
      gl_vboDestroy(mesh->vbo);
   }

   array_free(object->meshes);
   array_free(object->materials);
}

static void object_fix3d( void )
{
   /* FIXME how much to scale the object? */
   const double scale = 1. / 20.;
   double zoom;

   /* XXX changes the projection */
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();

   /* rotates and scales the object to match projection */
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
}

static void object_restore2d( void )
{
   /* restores all matrices */
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

static void object_renderMesh( Object *object, int part, GLfloat alpha )
{
   Mesh *mesh = &object->meshes[part];

   /* computes relative addresses of the vertice and texture coords */
   const int ver_offset = (int)(&((Vertex *)NULL)->ver);
   const int tex_offset = (int)(&((Vertex *)NULL)->tex);

   /* activates vertices and texture coords */
   gl_vboActivateOffset(mesh->vbo,
         GL_VERTEX_ARRAY, ver_offset, 3, GL_FLOAT, sizeof(Vertex));
   gl_vboActivateOffset(mesh->vbo,
         GL_TEXTURE_COORD_ARRAY, tex_offset, 2, GL_FLOAT, sizeof(Vertex));

   /* Set material */
   /* XXX Ni, d ?? */
   assert("Part has no material" && (mesh->material != -1));
   Material *material = object->materials + mesh->material;
   material->Kd[3] = alpha;

   if (glIsEnabled(GL_LIGHTING)) {
      glMaterialfv(GL_FRONT, GL_AMBIENT,  material->Ka);
      glMaterialfv(GL_FRONT, GL_DIFFUSE,  material->Kd);
      glMaterialfv(GL_FRONT, GL_SPECULAR, material->Ks);
      glMaterialf(GL_FRONT, GL_SHININESS, material->Ns);
   } else {
#if 0
      DEBUG("No lighting kd = %.3lf %.3lf %.3lf %.3lf",
            material->Kd[0], material->Kd[1],
            material->Kd[2], material->Kd[3]);
#endif
      glColor4fv(material->Kd);
   }

   /* binds textures */
   if (material->has_texture) {
      glBindTexture(GL_TEXTURE_2D, material->texture);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glEnable(GL_TEXTURE_2D);
   }

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);  /* XXX this changes the global DepthFunc */

   glDrawArrays(GL_TRIANGLES, 0, mesh->num_corners);

   glDisable(GL_DEPTH_TEST);
   if (material->has_texture)
      glDisable(GL_TEXTURE_2D);
   gl_vboDeactivate();

}


void object_renderSolidPart( Object *object, const Solid *solid, const char *part_name, GLfloat alpha )
{
   double x, y, cx, cy, gx, gy, zoom;
   int i;

   /* get parameters. */
   gl_cameraGet(&cx, &cy);
   gui_getOffset(&gx, &gy);
   gl_cameraZoomGet(&zoom);

   /* calculate position - we'll use relative coords to player */
   x = (solid->pos.x - cx + gx) * zoom / gl_screen.nw * 2;
   y = (solid->pos.y - cy + gy) * zoom / gl_screen.nh * 2;

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glTranslatef(x, y, 0.);
   glRotatef(solid->dir / M_PI * 180. + 90., 0., 0., 1.);
   glRotatef(90., 1., 0., 0.);

   object_fix3d();
   for (i = 0; i < array_size(object->meshes); ++i)
      if (strcmp(part_name, object->meshes[i].name) == 0)
         object_renderMesh(object, i, alpha);
   object_restore2d();

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}
