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

#include "nstring.h"

#include "SDL.h"

#include "log.h"
#include "ndata.h"
#include "font.h"
#include "music.h"
#include "nstd.h"
#include "toolkit.h"
#include "conf.h"


#define INTRO_SPEED        30. /**< Speed of text in characters / second. */


/**
 * @brief Scroll Buffer: For a linked list of render text.
 */
typedef struct scroll_buf_t_ {
   const char *text;            /* Text to render. */
   struct scroll_buf_t_ *next;  /* Next line in the linked list. */
} scroll_buf_t;

/**
 * @brief Intro Image: to be displayed to the side of the scrolling.
 */
typedef struct intro_img_t_ {
   glTexture *tex;     /* Image. */
   double x, y;        /* Position. */
   glColour c;         /* Alpha channel for fading. */
   double fade_rate;   /* Positive for fade-in, negative for fade-out. */
} intro_img_t;

/*
 * Introduction text lines.
 */
static char **intro_lines = NULL;  /**< Introduction text lines. */
static int intro_nlines = 0;       /**< Number of introduction text lines. */
static int intro_length = 0;       /**< Length of the text. */
static glFont intro_font;          /**< Introduction font. */

static int has_side_gfx = 0;       /* Determines how wide to make the text. */

/*
 * Prototypes.
 */
static int intro_load( const char *text );
static void intro_cleanup (void);
static scroll_buf_t *arrange_scroll_buf( scroll_buf_t *arr, int n );
static void intro_event_handler( int *stop, double *offset, double *vel );
static void initialize_image( intro_img_t *img );
static void intro_fade_image_in( intro_img_t *side, intro_img_t *transition,
                                 const char *img_file );
static int intro_draw_text( scroll_buf_t *sb_list, double offset,
                            double line_height );


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

   has_side_gfx = 0;

   /* Load text. */
   intro_buf = ndata_read( text, &intro_size );
   intro_length = intro_size; /* Length approximation. */

   /* Create intro font. */
   gl_fontInit( &intro_font, "dat/mono.ttf", conf.font_size_intro );

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

      if ( intro_buf[p] == '[' /* Don't do sscanf for every line! */
           && sscanf( &intro_buf[p], "[fadein %s", img_src ) == 1 ) {
         /* an image to appear next to text. */
         /* Get the length. */
         for (i = 0; intro_buf[p + i] != '\n' && intro_buf[p + i] != '\0'; ++i);

         length = strlen( img_src );
         intro_lines[n] = malloc( length + 2 );
         intro_lines[n][0] = 'i';
         strncpy( &intro_lines[n][1], img_src, length );
         intro_lines[n][length] = '\0';

         /* Mark that there are graphics. */
         has_side_gfx = 1;

      } else if ( intro_buf[p] == '[' /* Don't do strncmp for every line! */
           && strncmp( &intro_buf[p], "[fadeout]", 9 ) == 0 ) {
         /* fade out the image next to the text. */
         for (i = 0; intro_buf[p + i] != '\n' && intro_buf[p + i] != '\0'; ++i);

         intro_lines[n] = malloc( 2 );
         intro_lines[n][0] = 'o';
         intro_lines[n][1] = '\0';   /* not strictly necessary, but safe. */

      } else { /* plain old text. */
         /* Get the length. */
         i = gl_printWidthForText( &intro_font, &intro_buf[p], SCREEN_W - 500. );

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
 * @brief Initialize an intro_img_t to default values.
 *
 *    @brief img Image to initialize.
 */
static void initialize_image( intro_img_t *img )
{
   img->tex = NULL;
   img->x   = 100.;
   img->c.r = 1.0;
   img->c.g = 1.0;
   img->c.b = 1.0;
   img->c.a = 1.0;
   img->fade_rate = 0.0;
}


/**
 * @brief Fade an image in.
 *
 *    @brief side Present image being displayed.
 *    @brief transition Image in transition or on deck.
 *    @brief img_file Path to the PNG on disk.
 */
static void intro_fade_image_in( intro_img_t *side, intro_img_t *transition,
                                 const char *img_file )
{
   if (NULL == side->tex) {
      /* Simple fade-in. */
      side->tex = gl_newImage( img_file, 0 );
      side->y = (double)SCREEN_H / 2.0 - (side->tex->h / 2.0);
      side->c.a = 0.0;
      side->fade_rate = 0.1;
   }
   else {
      /*
       * Transition or on-deck. The difference is whether one image is
       * replacing the other (transition), or whether the first must fade out
       * completely before the second comes in (on-deck).
       *
       * We can determine which is the case by seeing whether [fadeout] has been
       * called. I.e., is side->fade_rate < 0?
       */
      if (NULL != transition->tex) {
         /* Scrolling is happening faster than fading... */
         WARN( "Intro scrolling too fast!" );
         gl_freeTexture( transition->tex );
      }
      transition->tex = gl_newImage( img_file, 0 );
      transition->y =
         (double)SCREEN_H / 2.0 - (transition->tex->h / 2.0);
      transition->c.a = 0.0;
      if (side->fade_rate < 0.0)
         transition->fade_rate = 0.0; /* put an image on deck. */
      else {
         /* transition. */
         transition->fade_rate = 0.1;
         side->fade_rate = -0.1; /* begin fading out. */
         side->c.a = 0.99;
      }
   }
}


/**
 * @brief Handle user events (mouse clicks, key presses, etc.).
 *
 *    @brief stop Whether to stop the intro.
 *    @brief vel How fast the text should scroll.
 */
static void intro_event_handler( int *stop, double *offset, double *vel )
{
   SDL_Event event;           /* user key-press, mouse-push, etc. */

   while (SDL_PollEvent(&event)) {
      if (event.type != SDL_KEYDOWN)
         continue;

      /* Escape skips directly. */
      if (event.key.keysym.sym == SDLK_ESCAPE) {
         *stop = 1;
         break;
      }

      /* Slow down the text. */
      else if (event.key.keysym.sym == SDLK_UP) {
         *vel -= 8.0;
         if (*vel < 0.0)
            *vel = 0.0;
      }

      /* Speed up the text. */
      else if (event.key.keysym.sym == SDLK_DOWN) {
         *vel += 8.0;
         if (*vel > 100.0)
            *vel = 100.0;
      }

      /* Jump down. */
      else if ((event.key.keysym.sym == SDLK_SPACE) ||
            (event.key.keysym.sym == SDLK_RETURN))
         *offset += 100;

      /* User is clearly flailing on keyboard. */
      else
         *vel = 16.;
   }
}

/**
 * @brief Draw intro text onto the screen.
 *
 *    @brief sb_list List of text lines.
 *    @brief offset For smooth scrolling.
 *    @brief line_height V-space of the font (plus leading).
 *    @return Whether to stop.  1 if no text was rendered, 0 otherwise.
 */
static int intro_draw_text( scroll_buf_t *sb_list, double offset,
                            double line_height)
{
   double x, y;               /* render position. */
   scroll_buf_t *list_iter;   /* iterator through sb_list. */
   register int stop = 1;

   if (has_side_gfx)
      x = 400.0; /* leave some space for graphics if they exist. */
   else
      x = 100.0;

   list_iter = sb_list;
   y = SCREEN_H + offset - line_height;
   do {
      if (NULL != list_iter->text) {
         stop = 0;
         gl_print( &intro_font, x, y, &cConsole, list_iter->text );
      }

      y -= line_height;
      list_iter = list_iter->next;
   } while (list_iter != sb_list);

   return stop;
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
   double offset;             /* distance from bottom of the top line. */
   double line_height;        /* # pixels per line. */
   int lines_per_screen;      /* max appearing lines on the screen. */
   scroll_buf_t *sb_arr;      /* array of lines to render. */
   scroll_buf_t *sb_list;     /* list   "   "    "    "    */
   double vel = 16.;          /* velocity: speed of text. */
   int stop = 0;              /* stop the intro. */
   unsigned int tcur, tlast;  /* timers. */
   double delta;              /* time diff from last render to this one. */
   int line_index = 0;        /* index into the big list of intro lines. */
   intro_img_t side_image;    /* image to go along with the text. */
   intro_img_t transition;    /* image for transitioning. */

   /* Load the introduction. */
   if (intro_load(text) < 0)
      return -1;

   /* Change music to intro music. */
   if (mus != NULL)
      music_choose(mus);

   /* We need to clear key repeat to avoid infinite loops. */
   toolkit_clearKey();

   /* Enable keyrepeat just for the intro. */
#if !SDL_VERSION_ATLEAST(2,0,0)
   SDL_EnableKeyRepeat( conf.repeat_delay, conf.repeat_freq );
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */

   /* Do a few calculations to figure out how many lines can be present on the
      screen at any given time. */
   line_height = (double)intro_font.h * 1.3;
   lines_per_screen = (int)(SCREEN_H / line_height + 1.5); /* round up + 1 */
   sb_arr = (scroll_buf_t*)malloc( sizeof(scroll_buf_t) * lines_per_screen );

   /* Force the first line to be loaded immediately. */
   offset = line_height;

   /* Create a cycle of lines. */
   sb_list = arrange_scroll_buf( sb_arr, lines_per_screen );

   /* Unset the side image. */
   initialize_image( &side_image );
   initialize_image( &transition );

   tlast = SDL_GetTicks();
   while (!stop) {
      tcur = SDL_GetTicks();
      delta = (double)(tcur - tlast) / 1000.;
      tlast = tcur;

      /* Increment position. */
      offset += vel * delta;
      while (! (offset < line_height)) {
         /* One line has scrolled off, and another one on. */

         if (line_index < intro_nlines) {
            switch (intro_lines[line_index][0]) {
            case 't': /* plain ol' text. */
               sb_list->text = &intro_lines[line_index][1];
               offset -= line_height;
               sb_list = sb_list->next;
               break;
            case 'i': /* fade in image. */
               intro_fade_image_in( &side_image, &transition,
                                    &intro_lines[line_index][1] );
               break;
            case 'o': /* fade out image. */
               if (NULL == side_image.tex) {
                  WARN("Tried to fade out without an image." );
                  break;
               }
               side_image.fade_rate = -0.1;
               side_image.c.a = 0.99;
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

      /* Fade the side image. */
      if (side_image.tex != NULL && side_image.c.a < 1.0) {
         side_image.c.a += delta * vel * side_image.fade_rate;

         if (transition.tex != NULL && transition.fade_rate > 0.0)
            transition.c.a += delta * vel * transition.fade_rate;

         if (side_image.c.a > 1.0) {
            /* Faded in... */
            side_image.c.a = 1.0;
            side_image.fade_rate = 0.0;
         }
         else if (side_image.c.a < 0.0) {
            /* Faded out... */
            gl_freeTexture( side_image.tex );
            if (transition.tex != NULL) {
               side_image.tex = transition.tex;
               side_image.c.a = transition.c.a;
               side_image.y   = transition.y;
               side_image.fade_rate = 0.1;
               transition.tex = NULL;
               transition.c.a = 1.0;
            }
            else {
               side_image.c.a = 1.0;
               side_image.tex = NULL;
               side_image.fade_rate = 0.0;
            }
         }
      }

      /* Clear stuff. */
      glClear(GL_COLOR_BUFFER_BIT);

      /* Only thing we actually care about updating is music. */
      music_update( 0. );

      /* Draw text. */
      stop = intro_draw_text( sb_list, offset, line_height );

      if (NULL != side_image.tex)
         /* Draw the image next to the text. */
         gl_blitScale( side_image.tex, side_image.x, side_image.y,
                       side_image.tex->w, side_image.tex->h, &side_image.c );

      if (NULL != transition.tex && transition.c.a > 0.0)
         /* Draw the image in transition. */
         gl_blitScale( transition.tex, transition.x, transition.y,
                       transition.tex->w, transition.tex->h, &transition.c );

      /* Display stuff. */
#if SDL_VERSION_ATLEAST(2,0,0)
      SDL_GL_SwapWindow( gl_screen.window );
#else /* SDL_VERSION_ATLEAST(2,0,0) */
      SDL_GL_SwapBuffers();
#endif /* SDL_VERSION_ATLEAST(2,0,0) */


      SDL_Delay(10); /* No need to burn CPU. */

      /* Handle user events. */
      intro_event_handler( &stop, &offset, &vel );

   } /* while (!stop) */

   /* free malloc'd memory. */
   free( sb_arr );
   if (NULL != side_image.tex)
      gl_freeTexture( side_image.tex );

   if (NULL != transition.tex)
      gl_freeTexture( transition.tex );

   /* Disable intro's key repeat. */
#if !SDL_VERSION_ATLEAST(2,0,0)
   SDL_EnableKeyRepeat( 0, 0 );
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */

   /* Stop music, normal music will start shortly after. */
   music_stop();

   /* Clean up after the introduction. */
   intro_cleanup();

   return 0;
}
