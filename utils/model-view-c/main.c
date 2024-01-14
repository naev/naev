#include <assert.h>
#include <stdio.h>

#include "common.h"

#include "SDL.h"
#include "SDL_image.h"

#include "glad.h"

#include "gltf.h"
#include "shader_min.h"
#include "mat4.h"

int main( int argc, char *argv[] )
{
   (void) argc;
   (void) argv;
   GLuint VaoId;
   int shadowmap_sel = 0;

   if (argc < 2) {
      DEBUG("Usage: %s FILENAME", argv[0]);
      return -1;
   }

   SDL_Init( SDL_INIT_VIDEO );
   SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
   SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
   SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
   SDL_Window *win = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_W, SCREEN_H, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
   SDL_SetWindowTitle( win, "Naev Model Viewer" );
   SDL_GL_CreateContext( win );
   gladLoadGLLoader(SDL_GL_GetProcAddress);

   IMG_Init(IMG_INIT_PNG);

   glGenVertexArrays(1, &VaoId);
   glBindVertexArray(VaoId);

   glEnable( GL_FRAMEBUFFER_SRGB );
   glClearColor( 0.2, 0.2, 0.2, 1.0 );

   if (object_init())
      return -1;

   /* Load the object. */
   Object *obj = object_loadFromFile( argv[1] );
   gl_checkErr();

   /* Set up some stuff. */
   GLuint shadowvbo;
   const GLfloat shadowvbo_data[8] = {
      0., 0.,
      1., 0.,
      0., 1.,
      1., 1. };
   glGenBuffers( 1, &shadowvbo );
   glBindBuffer( GL_ARRAY_BUFFER, shadowvbo );
   glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, shadowvbo_data, GL_STATIC_DRAW );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   GLuint shadowshader = gl_program_vert_frag( "depth.vert", "depth.frag", "" );
   glUseProgram( shadowshader );
   GLuint shadowvertex = glGetAttribLocation( shadowshader, "vertex" );
   GLuint shadowtex    = glGetUniformLocation( shadowshader, "sampler" );
   glUniform1i( shadowtex, 0 );
   glUseProgram( 0 );

   int rendermode = 1;
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

               case SDLK_m:
                  rendermode = !rendermode;
                  break;

               case SDLK_1:
                  shadowmap_sel = 0;
                  break;

               case SDLK_2:
                  shadowmap_sel = 1;
                  break;

               case SDLK_3:
                  shadowmap_sel = 2;
                  break;

               default:
                  break;
            }
         }
      }
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
      mat4 Hx = { .m = {
         { 1.0, 0.0, 0.0, 0.0 },
         { 0.0,  c,   s,  0.0 },
         { 0.0, -s,   c,  0.0 },
         { 0.0, 0.0, 0.0, 1.0 }
      } };
      c = cos(roty);
      s = sin(roty);
      mat4 Hy = { .m = {
         {  c,  0.0, -s,  0.0 },
         { 0.0, 1.0, 0.0, 0.0 },
         {  s,  0.0,  c,  0.0 },
         { 0.0, 0.0, 0.0, 1.0 }
      } };
      const GLfloat sca = 0.1;
      const mat4 Hscale = { .m = {
         { sca, 0.0, 0.0, 0.0 },
         { 0.0, sca, 0.0, 0.0 },
         { 0.0, 0.0, sca, 0.0 },
         { 0.0, 0.0, 0.0, 1.0 } } };

      mat4 H;
      mat4_mul( &H, &Hy, &Hx );
      mat4_apply( &H, &Hscale );

      /* Draw the object. */
      object_update( obj, (float)SDL_GetTicks64() / 1000. );
      object_render( obj, &H );

      /* Draw the shadowmap to see what's going on (clear the shadowmap). */
      if (rendermode) {
         GLuint shadowmap = object_shadowmap( shadowmap_sel );
         glUseProgram( shadowshader );

         glBindBuffer( GL_ARRAY_BUFFER, shadowvbo );
         glVertexAttribPointer( shadowvertex, 2, GL_FLOAT, GL_FALSE, 0, NULL );
         glEnableVertexAttribArray( shadowvertex );

         glActiveTexture( GL_TEXTURE0 );
         glBindTexture( GL_TEXTURE_2D, shadowmap );

         glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

         glBindBuffer( GL_ARRAY_BUFFER, 0 );
         glDisableVertexAttribArray( shadowvertex );
         glUseProgram( 0 );
      }

      SDL_GL_SwapWindow( win );

      gl_checkErr();

      SDL_Delay( 1000 * dt );
   }

   object_free( obj );

   object_exit();

   return 0;
}
