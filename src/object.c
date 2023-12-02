/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file object.c
 *
 * @brief Object Loader.
 * http://www.paulbourke.net/dataformats/obj/
 */
/** @cond */
#include <assert.h>
#include <libgen.h>
#include <stddef.h>
#include <string.h>
#include "SDL_image.h"

#include "naev.h"
/** @endcond */

#include "object.h"

#include "array.h"
#include "camera.h"
#include "gui.h"
#include "log.h"
#include "ndata.h"

#define DELIM " \t\n"
#define NAEV_ORTHO_SCALE 10.       /**< The cam.ortho_scale defined in the Blender script */
#define NAEV_ORTHO_DIST 9.*M_SQRT2/**< Distance from camera to origin in the Blender script */

typedef struct Material_ {
   char *name;
   GLfloat Ka[3], Kd[3], Ks[3], Ke[3];
   GLfloat Ns, Ni, d, bm;
   glTexture *map_Kd, *map_Ks, *map_Ke, *map_Bump;
} Material;

typedef struct Mesh_ {
   char *name;
   gl_vbo *vbo;
   int num_corners;
   int material;
} Mesh;

typedef struct Object_ {
   Mesh *meshes;
   Material *materials;
   GLfloat radius;
} Object;

typedef struct {
   GLfloat ver[3];
   GLfloat tex[2];
   GLfloat nor[3];
} Vertex;

static glTexture *zeroTexture       = NULL;
static glTexture *oneTexture        = NULL;
static unsigned int emptyTextureRefs= 0;

static void mesh_create( Mesh **meshes, const char* name,
                         Vertex *corners, int material )
{
   if (array_size(corners) == 0)
      return;
   if (name == NULL)
      ERR(_("No name for current part"));
   if (material == -1)
      ERR(_("No material for current part"));

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

   while ((token = SDL_strtokr(NULL, DELIM, saveptr)) != NULL) {
      double d;
      sscanf(token, "%lf", &d);
      assert(num <= how_many);
      dest[num++] = d;
   }

   assert(num == how_many);
   return num;
}

static int readGLmaterial( GLfloat col[3], char **saveptr )
{
   int ret = readGLfloat( col, 3, saveptr );
   /*
    * Not strictly colours, so we ignore gamma, although this might not be the best idea.
    * TODO Probably should look at what blender expects us to do.
   for (int i=0; i<3; i++)
      col[i] = gammaToLinear( col[i] );
   */
   return ret;
}

static void materials_readFromFile( const char *filename, Material **materials )
{
   char *filebuf, *filesaveptr, *line;
   size_t filesize;
   DEBUG(_("Loading material from %s"), filename);

   filebuf = ndata_read( filename, &filesize );
   if (filebuf == NULL)
      ERR(_("Cannot open object file %s"), filename);

   Material *curr = &array_back(*materials);

   line = SDL_strtokr(filebuf, "\n", &filesaveptr);
   while (line != NULL) {
      const char *token;
      char *saveptr, *copy_filename, *texture_filename;
      token = SDL_strtokr(line, DELIM, &saveptr);

      if (token == NULL) {
         /* Missing */
      } else if (strcmp(token, "newmtl") == 0) {
         token = SDL_strtokr(NULL, DELIM, &saveptr);
         curr = &array_grow(materials);
         curr->name = strdup(token);
         curr->Ni = 0.;
         curr->Ns = 0.;
         curr->d = 1.;
         curr->bm = 0.;
         curr->map_Kd = curr->map_Ks = curr->map_Ke = curr->map_Bump = NULL;
         DEBUG(_("Reading new material %s"), curr->name);
      } else if (strcmp(token, "Ns") == 0) {
         readGLfloat(&curr->Ns, 1, &saveptr);
      } else if (strcmp(token, "Ni") == 0) {
         readGLfloat(&curr->Ni, 1, &saveptr);
      } else if (strcmp(token, "d") == 0) {
         readGLfloat(&curr->d, 1, &saveptr);
      } else if (strcmp(token, "Ka") == 0) {
         readGLmaterial( curr->Ka, &saveptr );
      } else if (strcmp(token, "Kd") == 0) {
         readGLmaterial( curr->Kd, &saveptr );
      } else if (strcmp(token, "Ks") == 0) {
         readGLmaterial( curr->Ks, &saveptr );
      } else if (strcmp(token, "Ke") == 0) {
         readGLmaterial( curr->Ke, &saveptr );
      } else if (strncmp(token, "map_", 4) == 0) {
         glTexture **map = NULL;
         if (strcmp(token, "map_Kd") == 0)
            map = &curr->map_Kd;
         else if (strcmp(token, "map_Ks") == 0)
            map = &curr->map_Ks;
         else if (strcmp(token, "map_Ke") == 0)
            map = &curr->map_Ke;
         else if (strcmp(token, "map_Bump") == 0)
            map = &curr->map_Bump;
         else
            LOG(_("Can't understand token %s"), token);
         /* Note: we can't tokenize the command line here; options may be follwed by a filename containing whitespace chars. */
         if (map != NULL) {
            char *args = SDL_strtokr(NULL, "\n", &saveptr), *endp;
            while (1) {
               while (isspace(*args))
                  args++;
               if (strncmp(args, "-bm", 3) == 0 && isspace(args[3])) {
                  args += 3;
                  curr->bm = strtof(args, &endp);
                  assert("Bad -bm argument" && endp != args);
                  args = endp;
               }
               else if (strncmp(args, "-s", 2) == 0 && isspace(args[2])) {
                  endp = args + 2;
                  LOG(_("-s (texture scaling) option ignored for %s"), token);
                  do {
                     args = endp;
                     (void) strtof(args, &endp);
                  }
                  while (endp != args);
               }
               else if (args[0] == '-')
                  ERR(_("Options not supported for %s"), token);
               else
                  break;
            }

            /* computes the path to texture */
            copy_filename = strdup(filename);
            SDL_asprintf(&texture_filename, "%s/%s", dirname(copy_filename), args);
            *map = gl_newImage(texture_filename, 0);
            free(copy_filename);
            free(texture_filename);
         }
      } else if (strcmp(token, "Ke") == 0 || strcmp(token, "illum") == 0) {
         /* Ignore commands: [e]missive coefficient, illumination mode */
      } else if (token[0] == '#') {
         /* Comment */
      } else {
         LOG(_("Can't understand token %s"), token);
      }

      line = SDL_strtokr(NULL, "\n", &filesaveptr);
   }

   free(filebuf);
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
   GLfloat *v;
   GLfloat *vertex = array_create(GLfloat);   /**< vertex coordinates */
   GLfloat *texture = array_create(GLfloat);  /**< texture coordinates */
   GLfloat *normal = array_create(GLfloat);  /**< normal coordinates */
   Vertex *corners = array_create(Vertex);    /**< corners of the triangle faces */
   char *filebuf, *filesaveptr, *line;
   size_t filesize;

   filebuf = ndata_read( filename, &filesize );
   if (filebuf == NULL)
      ERR(_("Cannot open object file %s"), filename);
   DEBUG(_("Loading object file %s"), filename);

   char *name = NULL;
   int material = -1;

   if (emptyTextureRefs++ == 0) {
      float zero[]= {0., 0., 0., 0.};
      float one[] = {1., 1., 1., 1.};
      zeroTexture = gl_loadImageData( zero, 1, 1, 1, 1, "solid_zero" );
      oneTexture  = gl_loadImageData( one, 1, 1, 1, 1, "solid_white" );
   }

   Object *object = calloc(1, sizeof(Object));
   object->meshes = array_create(Mesh);
   object->materials = array_create(Material);

   line = SDL_strtokr(filebuf, "\n", &filesaveptr);
   while (line != NULL) {
      const char *token;
      char *saveptr, *copy_filename, *material_filename;
      token = SDL_strtokr(line, DELIM, &saveptr);

      if (token == NULL) {
         /* Missing */
      } else if (strcmp(token, "mtllib") == 0) {
         while ((token = SDL_strtokr(NULL, DELIM, &saveptr)) != NULL) {
            /* computes the path to materials */
            copy_filename = strdup(filename);
            SDL_asprintf(&material_filename, "%s/%s", dirname(copy_filename), token);
            materials_readFromFile(material_filename, &object->materials);
            free(copy_filename);
            free(material_filename);
         }
      } else if (strcmp(token, "o") == 0) {
         mesh_create(&object->meshes, name, corners, material);
         token = SDL_strtokr(NULL, DELIM, &saveptr);
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
      } else if (strcmp(token, "vn") == 0) {
         (void)array_grow(&normal);
         (void)array_grow(&normal);
         (void)array_grow(&normal);
         readGLfloat(array_end(normal) - 3, 3, &saveptr);
      } else if (strcmp(token, "f") == 0) {
         int num = 0;
         while ((token = SDL_strtokr(NULL, DELIM, &saveptr)) != NULL) {
            int i_v = 0, i_t = 0, i_n = 0;
            if (sscanf(token, "%d//%d", &i_v, &i_n) < 2)
               sscanf(token, "%d/%d/%d", &i_v, &i_t, &i_n);

            assert("Vertex index out of range." && (0 < i_v && i_v <= array_size(vertex) / 3));
            assert("Texture index out of range." && (0 <= i_t && i_t <= array_size(texture) / 2));
            assert("Normal index out of range." && (0 < i_n && i_n <= array_size(normal) / 3));

            Vertex *face = &array_grow(&corners);
            --i_v, --i_t, --i_n;
            memcpy(face->ver, vertex  + i_v * 3, sizeof(GLfloat) * 3);
            if (i_t >= 0)
               memcpy(face->tex, texture + i_t * 2, sizeof(GLfloat) * 2);
            else
               memset(face->tex, 0, sizeof(GLfloat) * 2);
            memcpy(face->nor, normal  + i_n * 3, sizeof(GLfloat) * 3);
            ++num;
         }

         assert("Too few or too many vertices for a face." && (num == 3));
      } else if (strcmp(token, "usemtl") == 0) {
         mesh_create(&object->meshes, name, corners, material);

         /* a new mesh with the same name */
         token = SDL_strtokr(NULL, DELIM, &saveptr);
         for (material = 0; material < array_size(object->materials); ++material)
            if (strcmp(token, object->materials[material].name) == 0)
               break;

         if (material == array_size(object->materials))
            ERR(_("No such material %s"), token);
      } else if (token[0] == '#') {
         /* Comment */
      } else if (strcmp(token, "l") == 0 || strcmp(token, "s") == 0) {
         /* Ignore commands: line, smoothing */
      } else {
         /* TODO Ignore s (smoothing), l (line) with no regrets? */
         LOG(_("Can't understand token %s"), token);
      }

      line = SDL_strtokr(NULL, "\n", &filesaveptr);
   }

   mesh_create(&object->meshes, name, corners, material);
   free(name);

   /* Calculate maximum mesh size (from center). */
   for (int i=0; i<array_size(corners); i++) {
      v = corners[i].ver;
      object->radius = MAX( object->radius, v[0]*v[0]+v[1]*v[1]+v[2]*v[2] );
   }
   object->radius = sqrt( object->radius );

   /* cleans up */
   free(filebuf);
   array_free(vertex);
   array_free(texture);
   array_free(normal);
   array_free(corners);

   return object;
}

/**
 * @brief Frees memory reserved for the object
 */
void object_free( Object *object )
{
   if (object == NULL)
      return;

   for (int i=0; i < array_size(object->materials); ++i) {
      Material *material = &object->materials[i];
      free(material->name);
      gl_freeTexture(material->map_Kd);
      gl_freeTexture(material->map_Ke);
      gl_freeTexture(material->map_Ks);
      gl_freeTexture(material->map_Bump);
   }

   for (int i=0; i < array_size(object->meshes); ++i) {
      Mesh *mesh = &object->meshes[i];
      free(mesh->name);
      gl_vboDestroy(mesh->vbo);
   }

   array_free(object->meshes);
   array_free(object->materials);

   if (--emptyTextureRefs == 0) {
      gl_freeTexture(zeroTexture);
      gl_freeTexture(oneTexture);
      zeroTexture = oneTexture = NULL;
   }

   free(object);
}

static void object_renderMesh( const Object *object, int part, GLfloat alpha )
{
   const Mesh *mesh = &object->meshes[part];

   /* computes relative addresses of the vertice and texture coords */
   const int ver_offset = offsetof(Vertex, ver);
   const int tex_offset = offsetof(Vertex, tex);
   const int nor_offset = offsetof(Vertex, nor);

   /* activates vertices and texture coords */
   glEnableVertexAttribArray(shaders.material.vertex);
   gl_vboActivateAttribOffset(mesh->vbo, shaders.material.vertex, ver_offset, 3, GL_FLOAT, sizeof(Vertex));
   glEnableVertexAttribArray(shaders.material.vertex_tex);
   gl_vboActivateAttribOffset(mesh->vbo, shaders.material.vertex_tex, tex_offset, 2, GL_FLOAT, sizeof(Vertex));
   glEnableVertexAttribArray(shaders.material.vertex_normal);
   gl_vboActivateAttribOffset(mesh->vbo, shaders.material.vertex_normal, nor_offset, 3, GL_FLOAT, sizeof(Vertex));

   /* Set material */
   assert("Part has no material" && (mesh->material != -1));
   Material *material = object->materials + mesh->material;
   //material->Kd[3] = alpha;

   glUniform1f(shaders.material.Ns, material->Ns);
   glUniform3f(shaders.material.Ka, material->Ka[0], material->Ka[1], material->Ka[2] );
   glUniform3f(shaders.material.Kd, material->Kd[0], material->Kd[1], material->Kd[2] );
   glUniform3f(shaders.material.Ks, material->Ks[0], material->Ks[1], material->Ks[2] );
   glUniform3f(shaders.material.Ke, material->Ke[0], material->Ke[1], material->Ke[2] );
   glUniform1f(shaders.material.Ni, material->Ni);
   glUniform1f(shaders.material.d,  material->d * alpha);
   glUniform1f(shaders.material.bm, material->bm);

   /* binds textures */
   glUniform1i(shaders.material.map_Kd, 0);
   glUniform1i(shaders.material.map_Ks, 1);
   glUniform1i(shaders.material.map_Ke, 2);
   glUniform1i(shaders.material.map_Bump, 3);
   glActiveTexture(GL_TEXTURE3);
   glBindTexture(GL_TEXTURE_2D, material->map_Bump == NULL ? zeroTexture->texture : material->map_Bump->texture);
   glActiveTexture(GL_TEXTURE2);
   glBindTexture(GL_TEXTURE_2D, material->map_Ke == NULL ? oneTexture->texture : material->map_Ke->texture);
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, material->map_Ks == NULL ? oneTexture->texture : material->map_Ks->texture);
   /* Need TEXTURE0 to be last. */
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, material->map_Kd == NULL ? oneTexture->texture : material->map_Kd->texture);

   glDrawArrays(GL_TRIANGLES, 0, mesh->num_corners);
}

void object_renderSolidPart( const Object *object, const Solid *solid, const char *part_name, GLfloat alpha, double scale )
{
   mat4 view, projection, model, ortho;
   const GLfloat od = NAEV_ORTHO_DIST;
   const GLfloat os = NAEV_ORTHO_SCALE / scale;
   double x, y; //, r;

   x = solid->pos.x;
   y = solid->pos.y;
   //r = object->radius * scale;

   /*
   // TODO fix this check to avoid rendering when not necessary
   if ((x+r < 0.) || (x-r > SCREEN_W) ||
         (y+r < 0.) || (y-r > SCREEN_H))
      return;
   */

   glUseProgram(shaders.material.program);

   projection = gl_gameToScreenMatrix(gl_view_matrix);
   mat4_translate( &projection, x, y, 0. );
   ortho = mat4_ortho(-os, os, -os, os, od, -od);
   mat4_mul( &view, &projection, &ortho );
   //projection = mat4_rotate(projection, M_PI/4., 1., 0., 0.);

   model = mat4_identity();
   mat4_rotate( &model, M_PI/2. + solid->dir, 0., 1., 0.);

   gl_uniformMat4(shaders.material.projection, &view);
   gl_uniformMat4(shaders.material.model, &model);

   /* Actually need depth testing now. */
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glClear( GL_DEPTH_BUFFER_BIT );

   for (int i=0; i < array_size(object->meshes); ++i)
      if (strcmp(part_name, object->meshes[i].name) == 0)
         object_renderMesh(object, i, alpha);

   /* Restore defaults. */
   glDisable(GL_DEPTH_TEST);
   glUseProgram(0);
   gl_checkErr();
}
