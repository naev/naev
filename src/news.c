/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file news.c
 *
 * @brief Handles news generation.
 */


#include "news.h"

#include "naev.h"

#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_misn.h"
#include "nlua_faction.h"
#include "nlua_diff.h"
#include "nlua_var.h"
#include "ndata.h"
#include "toolkit.h"
#include "nstring.h"


#define LUA_NEWS     "dat/news.lua"


/*
 * News state.
 */
static lua_State *news_state  = NULL; /**< Lua news state. */


/*
 * News buffer.
 */
static news_t *news_buf       = NULL; /**< Buffer of news. */
static int news_nbuf          = 0; /**< Size of news buffer. */


/*
 * News line buffer.
 */
static unsigned int news_tick = 0; /**< Last news tick. */
static int news_drag          = 0; /**< Is dragging news? */
static double news_pos        = 0.; /**< Position of the news feed. */
static glFont *news_font      = &gl_defFont; /**< Font to use. */
static char **news_lines      = NULL; /**< Text per line. */
static glFontRestore *news_restores = NULL; /**< Restorations. */
static int news_nlines        = 0; /**< Number of lines used. */
static int news_mlines        = 0; /**< Lines allocated. */


/*
 * Prototypes.
 */
static void news_cleanBuffer (void);
static void news_cleanLines (void);
static void news_render( double bx, double by, double w, double h, void *data );
static void news_mouse( unsigned int wid, SDL_Event *event, double mx, double my,
      double w, double h, void *mouse );


/**
 * @brief Renders a news widget.
 *
 *    @param bx Base X position to render at.
 *    @param by Base Y position to render at.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
static void news_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   int i, s, m, p;
   unsigned int t;
   double y, dt;

   t = SDL_GetTicks();

   /* Calculate offset. */
   if (!news_drag) {
      dt = (double)(t-news_tick)/1000.;
      news_pos += dt * 25.;
   }
   news_tick = t;

   /* Make sure user isn't silly and drags it to negative values. */
   if (news_pos < 0.)
      news_pos = 0.;

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   /* Render the text. */
   p = (int)ceil( news_pos / (news_font->h + 5.));
   m = (int)ceil(        h / (news_font->h + 5.));
   if (p > news_nlines + m + 1) {
      news_pos = 0.;
      return;
   }

   /* Get positions to make sure inbound. */
   s = MAX(0,p-m);
   p = MIN(p+1,news_nlines-1);

   /* Get start position. */
   y = news_pos - s * (news_font->h+5.);

   /* Draw loop. */
   for (i=s; i<p; i++) {

      gl_printRestore( &news_restores[i] );
      gl_printMidRaw( news_font, w-40.,
            bx+10, by+y, &cConsole, news_lines[i] );

      /* Increment line and position. */
      y -= news_font->h + 5.;
   }

}


/**
 * @brief wid Window receiving the mouse events.
 *
 *    @param event Mouse event being received.
 *    @param mx X position of the mouse.
 *    @param my Y position of the mouse.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
static void news_mouse( unsigned int wid, SDL_Event *event, double mx, double my,
      double w, double h, void *data )
{
   (void) wid;
   (void) data;

   switch (event->type) {
      case SDL_MOUSEBUTTONDOWN:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return;

         if (event->button.button == SDL_BUTTON_WHEELUP)
            news_pos -= h/3.;
         else if (event->button.button == SDL_BUTTON_WHEELDOWN)
            news_pos += h/3.;
         else if (!news_drag)
            news_drag = 1;
         break;

      case SDL_MOUSEBUTTONUP:
         if (news_drag)
            news_drag = 0;
         break;

      case SDL_MOUSEMOTION:
         if (news_drag)
            news_pos -= event->motion.yrel;
         break;
   }
}


/**
 * @brief Creates a news widget.
 *
 *    @param wid Window to create news widget on.
 *    @param x X position of the widget to create.
 *    @param y Y position of the widget to create.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
void news_widget( unsigned int wid, int x, int y, int w, int h )
{
   int i, p, len;
   char buf[8192];

   /* Sane defaults. */
   news_pos    = h/3;
   news_tick   = SDL_GetTicks();

   /* Clean news lines. */
   news_cleanLines();

   /* Load up the news in a string. */
   p = 0;
   for (i=0; i<news_nbuf; i++) {
      p += nsnprintf( &buf[p], sizeof(buf)-p,
            "%s\n\n\e0"
            "%s\n\n\n\n\e0"
            , news_buf[i].title, news_buf[i].desc );
   }
   len = p;

   /* Now load up the text. */
   p = 0;
   news_nlines = 0;
   while (p < len) {
      /* Get the length. */
      i = gl_printWidthForText( NULL, &buf[p], w-40 );

      /* Copy the line. */
      if (news_nlines+1 > news_mlines) {
         if (news_mlines == 0)
            news_mlines = 256;
         else
            news_mlines *= 2;
         news_lines    = realloc( news_lines, sizeof(char*) * news_mlines );
         news_restores = realloc( news_restores, sizeof(glFontRestore) * news_mlines );
      }
      news_lines[ news_nlines ]    = malloc( i + 1 );
      strncpy( news_lines[news_nlines], &buf[p], i );
      news_lines[ news_nlines ][i] = '\0';
      if (news_nlines==0)
         gl_printRestoreInit( &news_restores[ news_nlines ] );
      else  {
         memcpy( &news_restores[ news_nlines ], &news_restores[ news_nlines-1 ], sizeof(glFontRestore) );
         gl_printStore( &news_restores[ news_nlines ], news_lines[ news_nlines-1 ] );
      }
 
      p += i + 1; /* Move pointer. */
      news_nlines++; /* New line. */
   }

   /* Create the custom widget. */
   window_addCust( wid, x, y, w, h,
         "cstNews", 1, news_render, news_mouse, NULL );
}


/**
 * @brief Initializes the news.
 *
 *    @return 0 on success.
 */
int news_init (void)
{
   lua_State *L;
   char *buf;
   uint32_t bufsize;

   /* Already initialized. */
   if (news_state != NULL)
      return 0;

   /* Create the state. */
   news_state = nlua_newState();
   L = news_state;

   /* Load the libraries. */
   nlua_loadStandard(L, 1);

   /* Load the news file. */
   buf = ndata_read( LUA_NEWS, &bufsize );
   if (luaL_dobuffer(news_state, buf, bufsize, LUA_NEWS) != 0) {
      WARN("Failed to load news file: %s\n"
           "%s\n"
           "Most likely Lua file has improper syntax, please check",
            LUA_NEWS, lua_tostring(L,-1));
      free(buf);
      return -1;
   }
   free(buf);

   return 0;
}


/**
 * @brief Cleans the news buffer.
 */
static void news_cleanBuffer (void)
{
   int i;

   if (news_buf != NULL) {
      for (i=0; i<news_nbuf; i++) {
         free(news_buf[i].title);
         free(news_buf[i].desc);
      }
      free(news_buf);
      news_buf    = NULL;
      news_nbuf   = 0;
   }
}


/**
 * @brief Cleans the lines.
 */
static void news_cleanLines (void)
{
   int i;

   if (news_nlines != 0) {
      for (i=0; i<news_nlines; i++)
         free(news_lines[i]);
      news_nlines = 0;
   }
}


/**
 * @brief Cleans up the news stuff.
 */
void news_exit (void)
{
   /* Already freed. */
   if (news_state == NULL)
      return;

   /* Clean the buffers. */
   news_cleanBuffer();

   /* Clean the lines. */
   news_cleanLines();
   free(news_lines);
   free(news_restores);
   news_lines  = NULL;
   news_mlines = 0;

   /* Clean up. */
   lua_close(news_state);
   news_state = NULL;
}


/**
 * @brief Gets a news sentence.
 */
const news_t *news_generate( int *ngen, int n )
{
   int i, errf;
   lua_State *L;

   /* Lazy allocation. */
   if (news_state == NULL)
      news_init();
   L = news_state;

   /* Clean up the old buffer. */
   news_cleanBuffer();

   /* Allocate news. */
   news_buf = calloc( sizeof(news_t), n+1 );
   if (news_buf == NULL)
      ERR("Out of Memory.");
   if (ngen != NULL)
      (*ngen)  = 0;

#if DEBUGGING
   lua_pushcfunction(L, nlua_errTrace);
   errf = -3;
#else /* DEBUGGING */
   errf = 0;
#endif /* DEBUGGING */

   /* Run the function. */
   lua_getglobal(L, "news"); /* f */
   lua_pushnumber(L, n); /* f, n */
   if (lua_pcall(L, 1, 2, errf)) { /* error has occurred */
      WARN("News: '%s' : %s", "news", lua_tostring(L,-1));
#if DEBUGGING
      lua_pop(L,2);
#else /* DEBUGGING */
      lua_pop(L,1);
#endif /* DEBUGGING */
      return NULL;
   }
   /* str, t */

   /* Check to see if it's valid. */
   if (!lua_isstring(L, -2) || !lua_istable(L, -1)) {
      WARN("News generated invalid output!");
#if DEBUGGING
      lua_pop(L,3);
#else /* DEBUGGING */
      lua_pop(L,2);
#endif /* DEBUGGING */
      return NULL;
   }

   /* Create the title header. */
   news_buf[0].title = strdup("NEWS HEADLINES");
   news_buf[0].desc  = strdup( lua_tostring(L, -2) );

   /* Pull it out of the table. */
   for (i=0; i<n; i++) {
      lua_pushnumber(L,i+1);
      lua_gettable(L,-2);
      if (!lua_istable(L,-1)) {
         WARN("Failed to generate news, item %d is not a table!",i+1);
#if DEBUGGING
         lua_pop(L,4);
#else /* DEBUGGING */
         lua_pop(L,3);
#endif /* DEBUGGING */
         return NULL;
      }
      /* Pull out of the internal table the data. */
      lua_getfield(L, -1, "title"); /* str, table, val, str */
      news_buf[i+1].title = strdup( luaL_checkstring(L, -1) );
      lua_pop(L,1); /* str, table, val */
      lua_getfield(L, -1, "desc"); /* str, table, val, str */
      news_buf[i+1].desc  = strdup( luaL_checkstring(L, -1) );
      lua_pop(L,1); /* str, table, val */
      /* Go to next element. */
      lua_pop(L,1); /* str, table  */
   }

   /* Clean up results. */
#if DEBUGGING
   lua_pop(L,3);
#else /* DEBUGGING */
   lua_pop(L,2);
#endif /* DEBUGGING */

   /* Save news found. */
   news_nbuf   = n+1;
   if (ngen != NULL)
      (*ngen)  = news_nbuf;

   return news_buf;
}


