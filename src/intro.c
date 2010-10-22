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
static int intro_nlines = 0;       /**< Number of introduction text lines. */
static int intro_length = 0;       /**< Length of the text. */
static glFont intro_font;          /**< Introduction font. */


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
   char img_src[128];     /* path to image to be displayed alongside text. */
   int length;
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

      /* Copy the line. */
      if (n+1 > mem) {
         mem += 128;
         intro_lines = realloc( intro_lines, sizeof(char*) * mem );
      }

      if ( sscanf( &intro_buf[p], "[fadein %s", img_src ) == 1 ) {
         /* an image to appear next to text. */
         /* Get the length. */
         /* FIXME: don't do this.  This is silly.  Just find the newline and set
            i accordingly. */
         i = gl_printWidthForText( &intro_font,
                                   &intro_buf[p],
                                   SCREEN_W - 500. );

         length = strlen( img_src );
         intro_lines[n] = malloc( length + 2 );
         intro_lines[n][0] = 'i';
         strncpy( &intro_lines[n][1], img_src, length );
         intro_lines[n][length] = '\0';
      } else {
         /* plain old text. */

         /* Get the length. */
         i = gl_printWidthForText( &intro_font,
                                   &intro_buf[p],
                                   SCREEN_W - 500. );

         intro_lines[n] = malloc( i + 2 );
         intro_lines[n][0] = 't';
         strncpy( &intro_lines[n][1], &intro_buf[p], i );
         intro_lines[n][i+1] = '\0';
      }

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

typedef struct scroll_buf_t scroll_buf_t;

struct scroll_buf_t {
   const char *text;
   scroll_buf_t *next;
};


/**
 * @brief Convert an array of scroll_buf_t into a circularly linked list.
 *
 *    @brief arr Input array.
 *    @brief n Number of elements.
 *    @return A pointer into the circular list.
 */
static scroll_buf_t *arrange_scroll_buf( scroll_buf_t *arr, int n )
{
   scroll_buf_t *sb_list = &arr[n - 1];
   int i;

   for (i = 0; i < n; ++i) {
      arr[i].text = NULL;
      arr[i].next = sb_list;
      sb_list = &arr[i];
   }

   return sb_list;
}


/**
 * @brief Handle user events (mouse clicks, key presses, etc.).
 *
 *    @brief stop Whether to stop the intro.
 *    @brief vel How fast the text should scroll.
 */
static void intro_event_handler(int *stop, double *vel)
{
   SDL_Event event;           /* user key-press, mouse-push, etc. */

   while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN:

         /* Escape skips directly. */
         if (event.key.keysym.sym == SDLK_ESCAPE) {
            *stop = 1;
            break;
         }

         /* User is clearly flailing on keyboard. */
         else {
            *vel = 30.;
         }

      default:
         break;
      }
   } /* while (SDL_PollEvent(&event)) */
}

static void intro_draw_text(scroll_buf_t *sb_list, double offset,
                            double line_height)
{
   double x = 400., y = 0.;   /* render position. */
   scroll_buf_t *list_iter;   /* iterator through sb_list. */

   list_iter = sb_list;
   y = SCREEN_H + offset - line_height;
   do {
      if (NULL != list_iter->text) {
         gl_print( &intro_font, x, y, &cConsole, list_iter->text );
      }

      y -= line_height;
      list_iter = list_iter->next;
   } while (list_iter != sb_list);
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
   double offset = 0.;        /* distance from bottom of the top line. */
   double line_height;        /* # pixels per line. */
   int lines_per_screen;      /* max appearing lines on the screen. */
   scroll_buf_t *sb_arr;      /* array of lines to render. */
   scroll_buf_t *sb_list;     /* list   "   "    "    "    */
   double vel = 15.;          /* velocity: speed of text. */
   int stop = 0;              /* stop the intro. */
   unsigned int tcur, tlast;  /* timers. */
   double delta;              /* time diff from last render to this one. */
   int line_index = 0;        /* index into the big list of intro lines. */
   glTexture *image = NULL;   /* image to go along with the text. */
   double x_img, y_img;       /* offsets for the location of the image. */

   /* Load the introduction. */
   if (intro_load(text) < 0)
      return -1;

   /* Change music to intro music. */
   if (mus != NULL)
      music_choose(mus);

   /* We need to clear key repeat to avoid infinite loops. */
   toolkit_clearKey();

   /* Enable keyrepeat just for the intro. */
   SDL_EnableKeyRepeat( conf.repeat_delay, conf.repeat_freq );

   line_height = (double)intro_font.h * 1.3;
   lines_per_screen = (int)(SCREEN_H / line_height + 1.5); /* round up + 1 */
   sb_arr = (scroll_buf_t*)malloc( sizeof(scroll_buf_t) * lines_per_screen );

   /* create a cycle of lines. */
   sb_list = arrange_scroll_buf( sb_arr, lines_per_screen );

   x_img = 100.;
   y_img = 0.;

   tlast = SDL_GetTicks();
   while (!stop) {
      tcur = SDL_GetTicks();
      delta = (double)(tcur - tlast) / 1000.;
      tlast = tcur;

      /* Handle user events. */
      intro_event_handler(&stop, &vel);

      /* Increment position. */
      offset += vel * delta;
      while (offset > line_height) {
         /* One line has scrolled off, and another one on. */

         if (line_index < intro_nlines) {
            switch (intro_lines[line_index][0]) {
            case 't': /* plain ol' text. */
               sb_list->text = &intro_lines[line_index][1];
               offset -= line_height;
               sb_list = sb_list->next;
               break;
            case 'i': /* fade in image. */
               if (NULL != image) {
                  /* really should have faded out, first. */
                  gl_freeTexture(image);
               }
               image = gl_newImage( &intro_lines[line_index][1], 0 );
               y_img = (double)SCREEN_H / 2.0 - (image->h / 2.0);
               break;
            default:  /* unknown. */
               break;
            }
            ++line_index;
         } else {
            sb_list->text = NULL;
            offset -= line_height;
            sb_list = sb_list->next;
         }
      } /* while (offset > line_height) */

      /* Clear stuff. */
      glClear(GL_COLOR_BUFFER_BIT);

      /* Only thing we actually care about updating is music. */
      music_update( 0. );

      /* Draw text. */
      intro_draw_text(sb_list, offset, line_height);

      if (NULL != image) {
         /* Draw the image next to the text. */
         gl_blitScale( image, x_img, y_img, image->w, image->h, NULL);
      }

      /* Display stuff. */
      SDL_GL_SwapBuffers();

      SDL_Delay(10); /* No need to burn CPU. */

   } /* while (!stop) */

   /* free malloc'd memory. */
   free( sb_arr );

   /* Disable intro's key repeat. */
   SDL_EnableKeyRepeat( 0, 0 );

   /* Stop music, normal music will start shortly after. */
   music_stop();

   /* Clean up after the introduction. */
   intro_cleanup();

   return 0;
}

#if 0
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
   x = 400.;
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

	 gl_print( &intro_font, x, y, &cConsole, &intro_lines[i][1] );

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

#endif /* 0 */

