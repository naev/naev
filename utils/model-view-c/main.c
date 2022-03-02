#include <assert.h>
#include <stdio.h>

#include "common.h"

#include "SDL.h"
#include "SDL_image.h"

#include "glad.h"

#include "gltf.h"

#define MAX(a,b)  (((a)>(b))?(a):(b))

typedef struct vec3_ {
   double v[3];
} vec3;

void vec3_add( vec3 *out, const vec3 *a, const vec3 *b )
{
   for (int i=0; i<3; i++)
      out->v[i] = a->v[i] + b->v[i];
}
void vec3_sub( vec3 *out, const vec3 *a, const vec3 *b )
{
   for (int i=0; i<3; i++)
      out->v[i] = a->v[i] - b->v[i];
}
void vec3_wadd( vec3 *out, const vec3 *a, const vec3 *b, double wa, double wb )
{
   for (int i=0; i<3; i++)
      out->v[i] = wa*a->v[i] + wb*b->v[i];
}
double vec3_dot( const vec3 *a, const vec3 *b )
{
   double o = 0.;
   for (int i=0; i<3; i++)
      o += a->v[i] * b->v[i];
   return o;
}
double vec3_dist( const vec3 *a, const vec3 *b )
{
   double o = 0.;
   for (int i=0; i<3; i++)
      o += pow( a->v[i]-b->v[i], 2. );
   return sqrt(o);
}

/**
 * @brief Distance between a point and a triangle.
 *
 * Based on  https://www.geometrictools.com/Documentation/DistancePoint3Triangle3.pdf
 */
double distPointTriangle( const vec3 *point, const vec3 tri[3] )
{
   vec3 diff, edge0, edge1, res;
   double a00, a01, a11, b0, b1, det, s, t;

   vec3_sub( &diff, &tri[0], point );
   vec3_sub( &edge0, &tri[1], &tri[0] );
   vec3_sub( &edge1, &tri[2], &tri[0] );

   a00 = vec3_dot( &edge0, &edge0 );
   a01 = vec3_dot( &edge0, &edge1 );
   a11 = vec3_dot( &edge1, &edge1 );
   b0  = vec3_dot( &diff,  &edge0 );
   b1  = vec3_dot( &diff,  &edge1 );
   det = MAX( a00*a11 - a01*a01, 0. );
   s   = a01 * b1 - a11 * b0;
   t   = a01 * b0 - a00 * b1;

   if (s + t <= det) {
      if (s < 0.) {
         if (t < 0.) { // region 4
            if (b0 < 0.) {
               t = 0.;
               if (-b0 >= a00)
                  s = 1.;
               else
                  s = -b0 / a00;
            }
            else {
               s = 0.;
               if (b1 >= 0.)
                  t = 0.;
               else if (-b1 >= a11)
                  t = 1.;
               else
                  t = -b1 / a11;
            }
         }
         else { // region 3
            s = 0.;
            if (b1 >= 0.)
               t = 0.;
            else if (-b1 >= a11)
               t = 1.;
            else
               t = -b1 / a11;
         }
      }
      else if (t < 0.) { // region 5
         t = 0.;
         if (b0 >= 0.)
            s = 0.;
         else if (-b0 >= a00)
            s = 1.;
         else
            s = -b0 / a00;
      }
      else { // region 0
         // minimum at interior point
         s /= det;
         t /= det;
      }
   }
   else {
      double tmp0, tmp1, numer, denom;

      if (s < 0.) { // region 2
         tmp0 = a01 + b0;
         tmp1 = a11 + b1;
         if (tmp1 > tmp0) {
            numer = tmp1 - tmp0;
            denom = a00 - 2. * a01 + a11;
            if (numer >= denom) {
               s = 1.;
               t = 0.;
            }
            else {
               s = numer / denom;
               t = 1. - s;
            }
         }
         else {
            s = 0.;
            if (tmp1 <= 0.)
               t = 1.;
            else if (b1 >= 0.)
               t = 0.;
            else
               t = -b1 / a11;
         }
      }
      else if (t < 0.) { // region 6
         tmp0 = a01 + b1;
         tmp1 = a00 + b0;
         if (tmp1 > tmp0) {
            numer = tmp1 - tmp0;
            denom = a00 - 2. * a01 + a11;
            if (numer >= denom) {
               t = 1.;
               s = 0.;
            }
            else {
               t = numer / denom;
               s = 1. - t;
            }
         }
         else {
            t = 0.;
            if (tmp1 <= 0.)
               s = 1.;
            else if (b0 >= 0.)
               s = 0.;
            else
               s = -b0 / a00;
         }
      }
      else { // region 1
         numer = a11 + b1 - a01 - b0;
         if (numer <= 0.) {
            s = 0.;
            t = 1.;
         }
         else {
            denom = a00 - 2. * a01 + a11;
            if (numer >= denom) {
               s = 1.;
               t = 0.;
            }
            else {
               s = numer / denom;
               t = 1. - s;
            }
         }
      }
   }

   vec3_wadd( &res, &edge0, &edge1, s, t );
   vec3_add(  &res, &res,   &tri[0] );
   return vec3_dist( point, &res );
   /*
   result.closest[0] = point;
   result.closest[1] = triangle.v[0] + s * edge0 + t * edge1;
   diff = result.closest[0] - result.closest[1];
   result.sqrDistance = Dot(diff, diff);
   result.distance = std::sqrt(result.sqrDistance);
   result.barycentric[0] = one - s - t;
   result.barycentric[1] = s;
   result.barycentric[2] = t;
   return result;
   */
}

static void matmul( GLfloat H[16], const GLfloat R[16] )
{
   for (int i=0; i<4; i++) {
      float l0 = H[i * 4 + 0];
      float l1 = H[i * 4 + 1];
      float l2 = H[i * 4 + 2];

      float r0 = l0 * R[0] + l1 * R[4] + l2 * R[8];
      float r1 = l0 * R[1] + l1 * R[5] + l2 * R[9];
      float r2 = l0 * R[2] + l1 * R[6] + l2 * R[10];

      H[i * 4 + 0] = r0;
      H[i * 4 + 1] = r1;
      H[i * 4 + 2] = r2;
   }
   H[12] += R[12];
   H[13] += R[13];
   H[14] += R[14];
}

int main( int argc, char *argv[] )
{
   (void) argc;
   (void) argv;
   GLuint VaoId;

   if (argc < 2) {
      DEBUG("Usage: %s FILENAME", argv[0]);
      return -1;
   }

   SDL_Init( SDL_INIT_VIDEO );
   SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
   SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
   SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
   SDL_Window *win = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 1280, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
   SDL_SetWindowTitle( win, "Naev Model Viewer" );
   SDL_GL_CreateContext( win );
   gladLoadGLLoader(SDL_GL_GetProcAddress);

   IMG_Init(IMG_INIT_PNG);

   glGenVertexArrays(1, &VaoId);
   glBindVertexArray(VaoId);

   glEnable( GL_FRAMEBUFFER_SRGB );
   glClearColor( 0., 0., 0., 1. );

   if (object_init())
      return -1;

   //Object *obj = object_loadFromFile( "minimal.gltf" );
   //Object *obj = object_loadFromFile( "simple.gltf" );
   //Object *obj = object_loadFromFile( "simple_mat.gltf" );
   //Object *obj = object_loadFromFile( "simple_tex.gltf" );
   //Object *obj = object_loadFromFile( "admonisher.gltf" );
   Object *obj = object_loadFromFile( argv[1] );
   gl_checkErr();

   int quit = 0;
   float rotx = 0.;
   float roty = M_PI_2;
   const double dt = 1.0/60.0;
   while (!quit) {
      SDL_Event event;

      while (SDL_PollEvent( &event )) {
         if (event.type == SDL_QUIT)
            quit = 1;
         else if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;
            switch (key) {
               case SDLK_q:
               case SDLK_ESCAPE:
                  quit = 1;
                  break;

               default:
                  break;
            }
         }
      }
      //glClearColor( 0.2, 0.2, 0.2, 1.0 );
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      const Uint8 *state = SDL_GetKeyboardState(NULL);
      float rotxspeed = 0.0;
      float rotyspeed = 0.0;
      if (state[SDL_SCANCODE_LEFT])
         rotyspeed -= M_PI_2;
      if (state[SDL_SCANCODE_RIGHT])
         rotyspeed += M_PI_2;
      if (state[SDL_SCANCODE_DOWN])
         rotxspeed -= M_PI_2;
      if (state[SDL_SCANCODE_UP])
         rotxspeed += M_PI_2;
      rotx += rotxspeed * dt;
      roty += rotyspeed * dt;
      GLfloat c = cos(rotx);
      GLfloat s = sin(rotx);
      GLfloat Hx[16] = {
         1.0, 0.0, 0.0, 0.0,
         0.0,  c,   s,  0.0,
         0.0, -s,   c,  0.0,
         0.0, 0.0, 0.0, 1.0
      };
      c = cos(roty);
      s = sin(roty);
      GLfloat Hy[16] = {
          c,  0.0, -s,  0.0,
         0.0, 1.0, 0.0, 0.0,
          s,  0.0,  c,  0.0,
         0.0, 0.0, 0.0, 1.0
      };
      matmul( Hy, Hx );
      object_render( obj, Hy );

      SDL_GL_SwapWindow( win );

      gl_checkErr();

      SDL_Delay( 1000 * dt );
   }

   object_free( obj );

   object_exit();

   return 0;
}
