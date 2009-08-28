#include "object.h"

#include <assert.h>
#include <string.h>
#include <libgen.h>
#include "SDL_image.h"

#include "array.h"
#include "gui.h"
#include "log.h"

#define DELIM " \t\n"


typedef struct {
   GLfloat ver[3];
   GLfloat tex[2];
} Vertex;


static void mesh_init( Mesh *mesh )
{
   memset(mesh, 0x00, sizeof(Mesh));
   mesh->material = -1;
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


static GLuint texture_loadFromFile( const char *filename )
{
   /* Reads image and converts it to RGBA */
   SDL_Surface *brute = IMG_Load(filename);
   if (brute == NULL)
      ERR("Cannot load texture from %s", filename);
   SDL_Surface *image = SDL_DisplayFormatAlpha(brute);

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

static void materials_readFromFile( const char *filename, Material **materials )
{
   FILE *f = fopen(filename, "r");
   if (!f)
      ERR("Cannot open material file %s", filename);

   Material *curr = &array_back(*materials);

   char line[256];
   while (fgets(line, sizeof(line), f)) {
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

         DEBUG("texture_filename = %s", texture_filename);
         curr->texture = texture_loadFromFile(texture_filename);
         curr->has_texture = 1;
         free(copy_filename);
         free(texture_filename);
      } else if (token[0] == '#') {
         /* Comment */
      } else {
         WARN("Can't understand token %s", token);
      }
   }

   fclose(f);
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

   FILE *f = fopen(filename, "r");
   if (!f)
      ERR("Cannot open object file %s", filename);

   Mesh *curr = NULL;
   Object *object = calloc(1, sizeof(Object));
   object->meshes = array_create(Mesh, mesh_init);
   object->materials = array_create(Material, clear_memory);

   char line[256];
   while (fgets(line, sizeof(line), f)) {
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

            DEBUG("material_filename = %s", material_filename);
            materials_readFromFile(material_filename, &object->materials);
            free(copy_filename);
            free(material_filename);
         }
      } else if (strcmp(token, "o") == 0) {
         token = strtok(NULL, DELIM);

         if (curr != NULL) {
            curr->num_corners = array_size(corners);
            curr->vbo = gl_vboCreateStatic(
               array_size(corners) * sizeof(Vertex), corners);
            array_clear(corners);
         }

         curr = &array_grow(&object->meshes);
         curr->name = strdup(token);
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
         int i;

         if (curr->material != -1)
            ERR("Current object part already has material");

         token = strtok(NULL, DELIM);
         if (!object->materials)
            ERR("No materials available. Cannot usemtl %s", token);

         for (i = 0; i < array_size(object->materials); ++i)
            if (strcmp(token, object->materials[i].name) == 0)
               curr->material = i;

         if (curr->material == -1)
            ERR("No such material %s", token);
      } else if (token[0] == '#') {
         /* Comment */
      } else {
         WARN("Can't understand token %s", token);
      }
   }

   curr->num_corners = array_size(corners);
   curr->vbo = gl_vboCreateStatic(
      array_size(corners) * sizeof(Vertex), corners);
   array_clear(corners);

   /* cleans up */
   array_free(vertex);
   array_free(texture);
   array_free(corners);
   fclose(f);

   return object;
}


/**
 * @brief Frees memory reserved for the object
 */
void object_free( Object *object )
{
   (void)object;
  /* XXX */
}

static void object_renderMesh( Object *object, int part, GLfloat alpha )
{
   Mesh *mesh = &object->meshes[part];

   /* computes relative addresses of the vertice and texture coords */
   const int ver_offset = (int)(&((Vertex *)NULL)->ver);
   const int tex_offset = (int)(&((Vertex *)NULL)->tex);

   /* FIXME how much to scale the object? */
   const double scale = 1. / 10.;

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

   /* XXX changes the projection */
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();

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

   /* restores all matrices */
   glPopMatrix();
   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}


void object_renderSolidPart( Object *object, const Solid *solid, const char *part_name, GLfloat alpha )
{
   double x, y, cx, cy, gx, gy, zoom;
   int i;

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   /* get parameters. */
   gl_cameraGet(&cx, &cy);
   gui_getOffset(&gx, &gy);
   gl_cameraZoomGet(&zoom);

   /* calculate position - we'll use relative coords to player */
   x = (solid->pos.x - cx + gx) * zoom / gl_screen.nw * 2;
   y = (solid->pos.y - cy + gy) * zoom / gl_screen.nh * 2;

   glTranslatef(x, y, 0.);
   glRotatef(solid->dir / M_PI * 180. + 90., 0., 0., 1.);
   glRotatef(90., 1., 0., 0.);

   for (i = 0; i < array_size(object->meshes); ++i)
      if (strcmp(part_name, object->meshes[i].name) == 0)
         object_renderMesh(object, i, alpha);

   glPopMatrix();
}
