/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file intro.c
 *
 * @brief Handles the introduction sequence.
 *
 * @todo Allow handling of images and other fancy things once we get them.
 */


#include "intro.h"

#include "naev.h"

#include <string.h>

#include "SDL.h"

#include "log.h"
#include "ndata.h"
#include "font.h"
#include "music.h"
#include "nstd.h"
#include "toolkit.h"
#include "conf.h"


#define INTRO_FONT_SIZE    18. /**< Intro text font size. */
#define INTRO_SPEED        30. /**< Speed of text in characters / second. */


/*
 * Introduction text lines.
 */
static char **intro_lines = NULL;  /**< Introduction text lines. */
static int intro_nlines = 0; /**< Number of introduction text lines. */
static int intro_length = 0; /**< Length of the text. */
static glFont intro_font; /**< Introduction font. */


/*
 * Prototypes.
 */
static int intro_load( const char *text );
static void intro_cleanup (void);


/**
 * @brief Loads the intro stuff.
 */
static int intro_load( const char *text )
{
   uint32_t intro_size;
   char *intro_buf;
   int i, p, n;
   int mem;

   /* Load text. */
   intro_buf = ndata_read( text, &intro_size );
   intro_length = intro_size; /* Length aproximation. */

   /* Create intro font. */
   gl_fontInit( &intro_font, NULL, INTRO_FONT_SIZE );

   /* Load lines. */
   p = 0;
   n = 0;
   mem = 0;
   while ((uint32_t)p < intro_size) {
      /* Get the length. */
      i = gl_printWidthForText( &intro_font, &intro_buf[p], SCREEN_W - 200. );

      /* Copy the line. */
      if (n+1 > mem) {
         mem += 128;
         intro_lines = realloc( intro_lines, sizeof(char*) * mem );
      }
      intro_lines[n] = malloc( i + 1 );
      strncpy( intro_lines[n], &intro_buf[p], i );
      intro_lines[n][i] = '\0';

      p += i + 1; /* Move pointer. */
      n++; /* New line. */
   }

   /* Clean up. */
   free(intro_buf);

   intro_nlines = n;
   return 0;
}


/**
 * @brief Cleans up the intro stuff.
 */
static void intro_cleanup (void)
{
   int i;

   /* Free memory. */
   for (i=0; i<intro_nlines; i++)
      free(intro_lines[i]);
   free(intro_lines);
   gl_freeFont(&intro_font);

   /* Set defaults. */
   intro_lines = NULL;
   intro_nlines = 0;
}


/**
 * @brief Displays the introduction sequence.
 *
 *    @brief text Path of text file to use.
 *    @brief mus Type of music to use (run through music.lua).
 *    @return 0 on success.
 */
int intro_display( const char *text, const char *mus )
{
   int i, max, stop;
   unsigned int tcur, tlast;
   double dt;
   double x, y, vel;
   double offset;
   double density;
   SDL_Event event;

   /* Load the introduction. */
   if (intro_load(text) < 0)
      return -1;

   /* Calculate velocity. */
   density  = ((double)intro_length / (double)intro_nlines); /* char / line */
   density /= (double)intro_font.h; /* char / pixel */
   vel = INTRO_SPEED / density;  /* (char / s) * (pixel / char) = pixel / s */

   /* Change music to intro music. */
   if (mus != NULL)
      music_choose(mus);

   /* We need to clear key repeat to avoid infinite loops. */
   toolkit_clearKey();

   /* Enable keyrepeat just for the intro. */
   SDL_EnableKeyRepeat( conf.repeat_delay, conf.repeat_freq );

   /* Prepare for intro loop. */
   x = 100.;
   y = 0.;
   tlast = SDL_GetTicks();
   offset = 0.;
   max = intro_nlines + SCREEN_H / ((intro_font.h + 5.));
   stop = 0;
   while (!stop) {

      /* Get delta tick in seconds. */
      tcur = SDL_GetTicks();
      dt = (double)(tcur - tlast) / 1000.;
      tlast = tcur;

      /* Handle events. */
      while (SDL_PollEvent(&event)) {
         switch (event.type) {
            case SDL_KEYDOWN:

               /* Escape skips directly. */
               if (event.key.keysym.sym == SDLK_ESCAPE) {
                  stop = 1;
                  break;
               }

               /* Up arrow decreases speed. */
               else if (event.key.keysym.sym == SDLK_UP) {
                  vel -= 12.;
                  break;
               }

               /* Down arrow increases speed. */
               else if (event.key.keysym.sym == SDLK_DOWN) {
                  vel += 12.;
               }

               /* Pageup and backspace jump up. */
               else if (event.key.keysym.sym == SDLK_PAGEUP || event.key.keysym.sym == SDLK_BACKSPACE ) {
                  offset -= 250.;
                  break;
               }

               /* Pagedown and space jump down. */
               else if (event.key.keysym.sym == SDLK_PAGEDOWN || event.key.keysym.sym == SDLK_SPACE) {
                  offset += 250.;
                  break;
               }

               /* User is clearly flailing on keyboard. */
               else {
                  vel = 30.;
               }

               /* Purpose fallthrough. */
            case SDL_JOYBUTTONDOWN:
               vel += 12.;
            default:
               break;
         }
      }
   
      /* Increment position. */
      offset += vel * dt;

      /* Clear stuff. */
      glClear(GL_COLOR_BUFFER_BIT);

      /* Draw the text. */
      i = (int)(offset / (intro_font.h + 5.));
      if (i > max) /* Out of lines. */
         break;
      y = offset - (i+1) * (intro_font.h + 5.);

      while (i >= 0) {

         /* Skip in line isn't valid. */
         if (i >= intro_nlines) {
            i--;
            y += intro_font.h + 5.;
            continue;
         }

         gl_print( &intro_font, x, y, &cConsole, intro_lines[i] );

         /* Increment line and position. */
         i--;
         y += intro_font.h + 5.;
      }

      /* Only thing we actually care about updating is music. */
      music_update( 0. );

      /* Display stuff. */
      SDL_GL_SwapBuffers();

      SDL_Delay(10); /* No need to burn CPU. */
   }

   /* Disable intro's key repeat. */
   SDL_EnableKeyRepeat( 0, 0 );

   /* Stop music, normal music will start shortly after. */
   music_stop();

   /* Clean up after the introduction. */
   intro_cleanup();

   return 0;
}

