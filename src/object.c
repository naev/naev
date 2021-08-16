/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file object.c
 *
 * @brief Object Loader.
 * http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
 */


/** @cond */
#include <assert.h>
#include <libgen.h>
#include <stddef.h>
#include <string.h>
#include "physfsrwops.h"
#include "SDL_image.h"
/** @endcond */

#include "object.h"

#include "array.h"
#include "camera.h"
#include "gui.h"
#include "log.h"

#if defined(_WIN32) || defined(_WIN64)
# define strtok_r strtok_s
#endif

#define DELIM " \t\n"


typedef struct {
   GLfloat ver[3];
   GLfloat tex[2];
} Vertex;


static glTexture *emptyTexture = NULL;
static unsigned int emptyTextureRefs = 0;


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
   array_resize(&corners, 0);
}

static int readGLfloat( GLfloat *dest, int how_many, char **saveptr )
{
   char *token;
   int num = 0;

   while ((token = strtok_r(NULL, DELIM, saveptr)) != NULL) {
      double d;
      sscanf(token, "%lf", &d);
      dest[num++] = d;
   }

   if (how_many)
      assert(num == how_many);
   return num;
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

   SDL_RWops *f = PHYSFSRWOPS_openRead(filename);
   if (!f)
      ERR("Cannot open object file %s", filename);

   Material *curr = &array_back(*materials);

   char line[256];
   while (SDL_RWgets(line, sizeof(line), f)) {
      const char *token;
      char *saveptr, *copy_filename, *texture_filename;
      assert("Line too long" && (line[strlen(line) - 1] == '\n'));
      token = strtok_r(line, DELIM, &saveptr);

      if (token == NULL) {
         /* Missing */
      } else if (strcmp(token, "newmtl") == 0) {
         token = strtok_r(NULL, DELIM, &saveptr);
         curr = &array_grow(materials);
         curr->name = strdup(token);
         curr->texture = NULL;
         DEBUG("Reading new material %s", curr->name);
      } else if (strcmp(token, "Ns") == 0) {
         readGLfloat(&curr->Ns, 1, &saveptr);
      } else if (strcmp(token, "Ni") == 0) {
         readGLfloat(&curr->Ni, 1, &saveptr);
      } else if (strcmp(token, "d") == 0) {
         readGLfloat(&curr->d, 1, &saveptr);
      } else if (strcmp(token, "Ka") == 0) {
         readGLfloat(curr->Ka, 3, &saveptr);
         curr->Ka[3] = 1.0;
      } else if (strcmp(token, "Kd") == 0) {
         readGLfloat(curr->Kd, 3, &saveptr);
         curr->Kd[3] = 1.0;
      } else if (strcmp(token, "Ks") == 0) {
         readGLfloat(curr->Ks, 3, &saveptr);
         curr->Ks[3] = 1.0;
      } else if (strcmp(token, "map_Kd") == 0) {
         token = strtok_r(NULL, DELIM, &saveptr);
         if (token[0] == '-')
            ERR("Options not supported for map_Kd");

         /* computes the path to texture */
         copy_filename = strdup(filename);
	 asprintf(&texture_filename, "%s/%s", dirname(copy_filename), token);
         curr->texture = gl_newImage(texture_filename, 0);
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
   GLfloat *vertex = array_create(GLfloat);   /**< vertex coordinates */
   GLfloat *texture = array_create(GLfloat);  /**< texture coordinates */
   Vertex *corners = array_create(Vertex);    /**< corners of the triangle faces */

   SDL_RWops *f = PHYSFSRWOPS_openRead(filename);
   if (!f)
      ERR("Cannot open object file %s", filename);

   char *name = NULL;
   int material = -1;

   if (emptyTextureRefs++ == 0) {
      float data[] = {1., 1., 1., 1.};
      emptyTexture = gl_loadImageData( data, 1, 1, 1, 1, "solid_white" );
   }

   Object *object = calloc(1, sizeof(Object));
   object->meshes = array_create(Mesh);
   object->materials = array_create(Material);

   char line[256];
   while (SDL_RWgets(line, sizeof(line), f)) {
      const char *token;
      assert("Line too long" && (line[strlen(line) - 1] == '\n'));
      char *saveptr, *copy_filename, *material_filename;
      token = strtok_r(line, DELIM, &saveptr);

      if (token == NULL) {
         /* Missing */
      } else if (strcmp(token, "mtllib") == 0) {
         while ((token = strtok_r(NULL, DELIM, &saveptr)) != NULL) {
            /* computes the path to materials */
            copy_filename = strdup(filename);
	    asprintf(&material_filename, "%s/%s", dirname(copy_filename), token);
            materials_readFromFile(material_filename, &object->materials);
            free(copy_filename);
            free(material_filename);
         }
      } else if (strcmp(token, "o") == 0) {
         mesh_create(&object->meshes, name, corners, material);
         token = strtok_r(NULL, DELIM, &saveptr);
         free(name), name = strdup(token);
      } else if (strcmp(token, "v") == 0) {
         (void)array_grow(&vertex);
         (void)array_grow(&vertex);
         (void)array_grow(&vertex);
         readGLfloat(array_end(vertex) - 3, 3, &saveptr);
      } else if (strcmp(token, "vt") == 0) {
         (void)array_grow(&texture);
         (void)array_grow(&texture);
         readGLfloat(array_end(texture) - 2, 2, &saveptr);
      } else if (strcmp(token, "f") == 0) {
         /* XXX reads only the geometric & texture vertices.
          * The standards says corners can also include normal vertices.
          */
         int num = 0;
         while ((token = strtok_r(NULL, DELIM, &saveptr)) != NULL) {
            int i_v, i_t;
            if (sscanf(token, "%d/%d", &i_v, &i_t) == 1)
               i_t = 0;

            assert("Vertex index out of range." && (0 < i_v && i_v <= array_size(vertex) / 3));
            assert("Texture index out of range." && (0 <= i_t && i_t <= array_size(texture) / 2));

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
         token = strtok_r(NULL, DELIM, &saveptr);
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
      gl_freeTexture(material->texture);
   }

   for (i = 0; i < (int)array_size(object->meshes); ++i) {
      Mesh *mesh = &object->meshes[i];
      free(mesh->name);
      gl_vboDestroy(mesh->vbo);
   }

   array_free(object->meshes);
   array_free(object->materials);

   if (--emptyTextureRefs == 0) {
      gl_freeTexture(emptyTexture);
      emptyTexture = NULL;
   }
}

static void object_renderMesh( Object *object, int part, GLfloat alpha )
{
   Mesh *mesh = &object->meshes[part];

   /* computes relative addresses of the vertice and texture coords */
   const int ver_offset = offsetof(Vertex, ver);
   const int tex_offset = offsetof(Vertex, tex);

   /* activates vertices and texture coords */
   gl_vboActivateAttribOffset(mesh->vbo, shaders.material.vertex, ver_offset, 3, GL_FLOAT, sizeof(Vertex));
   gl_vboActivateAttribOffset(mesh->vbo, shaders.material.tex_coord, tex_offset, 2, GL_FLOAT, sizeof(Vertex));

   /* Set material */
   /* XXX Ni, d ?? */
   assert("Part has no material" && (mesh->material != -1));
   Material *material = object->materials + mesh->material;
   material->Kd[3] = alpha;

   glUniform4fv(shaders.material.Ka, 1, material->Ka);
   glUniform4fv(shaders.material.Kd, 1, material->Kd);
   glUniform4fv(shaders.material.Ks, 1, material->Ks);
   glUniform1f(shaders.material.Ns, material->Ns);

   /* binds textures */
   glBindTexture(GL_TEXTURE_2D, material->texture == NULL ? emptyTexture->texture : material->texture->texture);

   glDrawArrays(GL_TRIANGLES, 0, mesh->num_corners);
}


void object_renderSolidPart( Object *object, const Solid *solid, const char *part_name, GLfloat alpha, double scale )
{
   gl_Matrix4 projection;
   double x, y;
   int i;

   /* get parameters. */
   gl_gameToScreenCoords(&x, &y, solid->pos.x, solid->pos.y);
   scale *= cam_getZoom();

   glUseProgram(shaders.material.program);
   glEnableVertexAttribArray(shaders.material.vertex);
   glEnableVertexAttribArray(shaders.material.tex_coord);

   projection = gl_Matrix4_Translate(gl_view_matrix, x, y, 0. );
   projection = gl_Matrix4_Rotate(projection, M_PI/2, 0., 0., 1.);
   projection = gl_Matrix4_Rotate(projection, -M_PI/4, 0., 1., 0.);
   projection = gl_Matrix4_Rotate(projection, solid->dir, 0., 0., 1.);
   projection = gl_Matrix4_Rotate(projection, M_PI/2, 1., 0., 0.);
   projection = gl_Matrix4_Scale(projection, scale, scale, scale);
   projection = gl_Matrix4_Rotate(projection, M_PI, 0., 1., 0.);
   projection = gl_Matrix4_Rotate(projection, M_PI/2, 1., 0., 0.);
   gl_Matrix4_Uniform(shaders.material.projection, projection);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);  /* XXX this changes the global DepthFunc */

   for (i = 0; i < array_size(object->meshes); ++i)
      if (strcmp(part_name, object->meshes[i].name) == 0)
         object_renderMesh(object, i, alpha);

   glDisable(GL_DEPTH_TEST);
   glUseProgram(0);
   gl_checkErr();
}
