#include <assert.h>
#include <stdio.h>

#include "common.h"

#include "SDL.h"
#include "SDL_image.h"

#include "glad.h"

#include "gltf.h"

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

   object_init();

   //Object *obj = object_loadFromFile( "minimal.gltf" );
   //Object *obj = object_loadFromFile( "simple.gltf" );
   //Object *obj = object_loadFromFile( "simple_mat.gltf" );
   //Object *obj = object_loadFromFile( "simple_tex.gltf" );
   Object *obj = object_loadFromFile( "admonisher.gltf" );
   gl_checkErr();

   int quit = 0;
   float rotx = 0.;
   float roty = 0.;
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

