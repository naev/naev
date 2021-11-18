#include <assert.h>
#include <stdio.h>

#include "common.h"

#include "SDL.h"

#include "glad.h"

#include "gltf.h"

int main( int argc, char *argv[] )
{
   (void) argc;
   (void) argv;
   GLuint VaoId;

   SDL_Init( SDL_INIT_VIDEO );
   SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
   SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
   SDL_Window *win = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 1280, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
   SDL_SetWindowTitle( win, "Naev Model Viewer" );
   SDL_GL_CreateContext( win );
   gladLoadGLLoader(SDL_GL_GetProcAddress);

   glGenVertexArrays(1, &VaoId);
   glBindVertexArray(VaoId);

   object_init();

   Object *obj = object_loadFromFile( "minimal.gltf" );
   //Object *obj = object_loadFromFile( "simple.gltf" );
   //Object *obj = object_loadFromFile( "admonisher.gltf" );
   gl_checkErr();

   int quit = 0;
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

      object_render( obj );

      SDL_GL_SwapWindow( win );

      gl_checkErr();

      SDL_Delay( 100 );
   }

   object_free( obj );

   object_exit();

   return 0;
}

