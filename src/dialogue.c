/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dialogue.c
 *
 * @brief Is a high level api around toolkit.c for easy window creation.
 *
 * Only one dialogue may be open at once or behaviour is unspecified.
 *
 * All these dialogues use what I call the secondary main loop hack.
 *  Basically they spawn another main loop identical to the primary whose only
 *  difference is that it breaks on loop_done.  Therefore this loop hijacks
 *  the main loop until it's over, making these functions seem to be blocking
 *  without really being blocking.
 *
 * @todo Make dialogue system more flexible.
 *
 * @sa toolkit.c
 */


#include "dialogue.h"

#include "naev.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "toolkit.h"
#include "pause.h"
#include "opengl.h"
#include "input.h"
#include "menu.h"


int dialogue_open; /**< Number of dialogues open. */


/*
 * Prototypes.
 */
/* extern */
extern void main_loop (void); /* from naev.c */
/* generic */
static void dialogue_close( unsigned int wid, char* str );
static void dialogue_cancel( unsigned int wid, char* str );
/* dialogues */
static glFont* dialogue_getSize( const char* title,
      const char* msg, int* width, int* height );
static void dialogue_YesNoClose( unsigned int wid, char* str );
static void dialogue_inputClose( unsigned int wid, char* str );
static void dialogue_choiceClose( unsigned int wid, char* str );
static void dialogue_listClose( unsigned int wid, char* str );
static void dialogue_listCancel( unsigned int wid, char* str );
/* secondary loop hack */
static int toolkit_loop( int *loop_done );


/**
 * @brief Checks to see if a dialogue is open.
 */
int dialogue_isOpen (void)
{
   return !!dialogue_open;
}


/**
 * @brief Generic window close.
 */
static void dialogue_close( unsigned int wid, char* str )
{
   (void) str;
   int *loop_done;
   loop_done = window_getData( wid );
   window_destroy( wid );
   *loop_done = 1;
   dialogue_open--;
}


/**
 * @brief Generic window cancel.
 */
static void dialogue_cancel( unsigned int wid, char* str )
{
   (void) str;
   int *loop_done;
   loop_done = window_getData( wid );
   window_destroy( wid );
   *loop_done = -1;
   dialogue_open--;
}


/**
 * @brief Displays an alert popup with only an ok button and a message.
 *
 *    @param fmt Printf style message to display.
 */
void dialogue_alert( const char *fmt, ... )
{
   char msg[512];
   va_list ap;
   unsigned int wdw;
   int h, done;

   if (fmt == NULL) return;
   else { /* get the message */
      va_start(ap, fmt);
      vsnprintf(msg, 512, fmt, ap);
      va_end(ap);
   }

   h = gl_printHeightRaw( &gl_smallFont, 260, msg );

   /* create the window */
   wdw = window_create( "Warning", -1, -1, 300, 90 + h );
   window_setData( wdw, &done );
   window_addText( wdw, 20, -30, 260, h,  0, "txtAlert",
         &gl_smallFont, &cBlack, msg );
   window_addButton( wdw, 135, 20, 50, 30, "btnOK", "OK",
         dialogue_close );

   dialogue_open++;
   toolkit_loop( &done );
}


/**
 * @brief Gets the size needed for the dialogue.
 *
 *    @param title Title of the dialogue.
 *    @param msg Message of the dialogue.
 *    @param[out] width Gets the width needed.
 *    @param[out] height Gets the height needed.
 *    @return The font that matches the size.
 */
static glFont* dialogue_getSize( const char* title,
      const char* msg, int* width, int* height )
{
   glFont* font;
   double w, h, d;
   int len, titlelen;

   /* Get title length. */
   titlelen = gl_printWidthRaw( &gl_defFont, title );
   w = MAX(300, titlelen+40); /* Default width to try. */
   len = strlen(msg);

   /* First we split by text length. */
   if (len < 50) {
      font = &gl_defFont;
      h = gl_printHeightRaw( font, w-40, msg );
   }
   else {
      /* Now we look at proportion. */
      font = &gl_smallFont;
      /* font = &gl_defFont; */
      h = gl_printHeightRaw( font, w-40, msg );

      d = ((double)w/(double)h)*(3./4.); /* deformation factor. */
      if (fabs(d) > 0.3) {
         if (h > w)
            w = h;
         h = gl_printHeightRaw( font, w-40, msg );
      }
   }

   /* Set values. */
   (*width) = w;
   (*height) = h;

   return font;
}


/**
 * @brief Opens a dialogue window with an ok button and a message.
 *
 *    @param caption Window title.
 *    @param fmt Printf style message to display.
 */
void dialogue_msg( const char* caption, const char *fmt, ... )
{
   char msg[4096];
   va_list ap;

   if (fmt == NULL) return;
   else { /* get the message */
      va_start(ap, fmt);
      vsnprintf(msg, 4096, fmt, ap);
      va_end(ap);
   }

   dialogue_msgRaw( caption, msg );
}


/**
 * @brief Opens a dialogue window with an ok button and a fixed message.
 *
 *    @param caption Window title.
 *    @param msg Message to display.
 */
void dialogue_msgRaw( const char* caption, const char *msg )
{
   int w,h;
   glFont* font;
   unsigned int msg_wid;
   int done;

   font = dialogue_getSize( caption, msg, &w, &h );

   /* create the window */
   msg_wid = window_create( caption, -1, -1, w, 110 + h );
   window_setData( msg_wid, &done );
   window_addText( msg_wid, 20, -40, w-40, h,  0, "txtMsg",
         font, &cBlack, msg );
   window_addButton( msg_wid, (w-50)/2, 20, 50, 30, "btnOK", "OK",
         dialogue_close );

   dialogue_open++;
   toolkit_loop( &done );
}


/**
 * @brief Runs a dialogue with both yes and no options.
 *
 *    @param caption Caption to use for the dialogue.
 *    @param fmt Printf style message.
 *    @return 1 if yes is clicked or 0 if no is clicked.
 */
int dialogue_YesNo( const char* caption, const char *fmt, ... )
{
   char msg[4096];
   va_list ap;

   if (fmt == NULL) return -1;
   else { /* get the message */
      va_start(ap, fmt);
      vsnprintf(msg, 4096, fmt, ap);
      va_end(ap);
   }

   return dialogue_YesNoRaw( caption, msg );
}


/**
 * @brief Runs a dialogue with both yes and no options.
 *
 *    @param caption Caption to use for the dialogue.
 *    @param msg Message to display.
 *    @return 1 if yes is clicked or 0 if no is clicked.
 */
int dialogue_YesNoRaw( const char* caption, const char *msg )
{
   unsigned int wid;
   int w,h;
   glFont* font;
   int done[2];

   font = dialogue_getSize( caption, msg, &w, &h );

   /* create window */
   wid = window_create( caption, -1, -1, w, h+110 );
   window_setData( wid, &done );
   /* text */
   window_addText( wid, 20, -40, w-40, h,  0, "txtYesNo",
         font, &cBlack, msg );
   /* buttons */
   window_addButton( wid, w/2-50-10, 20, 50, 30, "btnYes", "Yes",
         dialogue_YesNoClose );
   window_addButton( wid, w/2+10, 20, 50, 30, "btnNo", "No",
         dialogue_YesNoClose );

   /* tricky secondary loop */
   dialogue_open++;
   done[1] = -1; /* Default to negative. */
   toolkit_loop( done );

   /* Close the dialogue. */
   dialogue_close( wid, NULL );

   /* return the result */
   return done[1];
}
/**
 * @brief Closes a yesno dialogue.
 *    @param wid Window being closed.
 *    @param str Unused.
 */
static void dialogue_YesNoClose( unsigned int wid, char* str )
{
   int *loop_done, result;
   
   /* store the result */
   if (strcmp(str,"btnYes")==0)
      result = 1;
   else if (strcmp(str,"btnNo")==0)
      result = 0;

   /* set data. */
   loop_done = window_getData( wid );
   loop_done[0] = 1;
   loop_done[1] = result;
}


static unsigned int input_wid = 0; /**< Stores the input window id. */
/**
 * @brief Creates a dialogue that allows the player to write a message.
 *
 * You must free the result if it's not null.
 *
 *    @param title Title of the dialogue window.
 *    @param min Minimum length of the message (must be non-zero).
 *    @param max Maximum length of the message (must be non-zero).
 *    @param fmt Printf style message to display on the dialogue.
 *    @return The message the player typed or NULL if it was cancelled.
 */
char* dialogue_input( const char* title, int min, int max, const char *fmt, ... )
{
   char msg[512];
   va_list ap;

   if (input_wid) return NULL;

   if (fmt == NULL) return NULL;
   else { /* get the message */
      va_start(ap, fmt);
      vsnprintf(msg, 512, fmt, ap);
      va_end(ap);
   }

   return dialogue_inputRaw( title, min, max, msg );
}

/**
 * @brief Creates a dialogue that allows the player to write a message.
 *
 * You must free the result if it's not null.
 *
 *    @param title Title of the dialogue window.
 *    @param min Minimum length of the message (must be non-zero).
 *    @param max Maximum length of the message (must be non-zero).
 *    @param msg Message to be displayed.
 *    @return The message the player typed or NULL if it was cancelled.
 */
char* dialogue_inputRaw( const char* title, int min, int max, const char *msg )
{
   char *input;
   int h, done;

   /* get text height */
   h = gl_printHeightRaw( &gl_smallFont, 200, msg );

   /* create window */
   input_wid = window_create( title, -1, -1, 240, h+140 );
   window_setData( input_wid, &done );
   window_setAccept( input_wid, dialogue_inputClose );
   window_setCancel( input_wid, dialogue_cancel );
   /* text */
   window_addText( input_wid, 30, -30, 200, h,  0, "txtInput",
         &gl_smallFont, &cDConsole, msg );
   /* input */
   window_addInput( input_wid, 20, -50-h, 200, 20, "inpInput", max, 1, NULL );
   window_setInputFilter( input_wid, "inpInput", "/" ); /* Remove illegal stuff. */
   /* button */
   window_addButton( input_wid, -20, 20, 80, 30,
         "btnClose", "Done", dialogue_inputClose );

   /* tricky secondary loop */
   dialogue_open++;
   done  = 0;
   input = NULL;
   while ((done >= 0) && (!input ||
         ((int)strlen(input) < min))) { /* must be longer than min */

      if (input) {
         dialogue_alert( "Input must be at least %d character%s long!",
               min, (min==1) ? "s" : "" );
         free(input);
         input = NULL;
      }

      if (toolkit_loop( &done ) != 0) /* error in loop -> quit */
         return NULL;

      /* save the input */
      if (done < 0)
         input = NULL;
      else
         input = strdup( window_getInput( input_wid, "inpInput" ) );
   }

   /* cleanup */
   window_destroy( input_wid );
   input_wid = 0;
   dialogue_open--;

   /* return the result */
   return input;
}
/**
 * @brief Closes an input dialogue.
 *    @param wid Unused.
 *    @param str Unused.
 */
static void dialogue_inputClose( unsigned int wid, char* str )
{
   (void) str;
   int *loop_done;

   /* break the loop */
   loop_done = window_getData( wid );
   *loop_done = 1;
}


static int dialogue_listSelected = -1;
static void dialogue_listCancel( unsigned int wid, char* str )
{
   dialogue_listSelected = -1;
   dialogue_cancel( wid, str );
}
static void dialogue_listClose( unsigned int wid, char* str )
{
   dialogue_listSelected = toolkit_getListPos( wid, "lstDialogue" );
   dialogue_close( wid, str );
}
/**
 * @brief Creates a list dialogue with OK and Cancel button with a fixed message.
 *
 *    @param title Title of the dialogue.
 *    @param items Items in the list (should be all malloced, automatically freed).
 *    @param nitems Number of items.
 *    @param fmt printf formatted string with text to display.
 */
int dialogue_list( const char* title, char **items, int nitems, const char *fmt, ... )
{
   char msg[512];
   va_list ap;

   if (input_wid) return -1;

   if (fmt == NULL) return -1;
   else { /* get the message */
      va_start(ap, fmt);
      vsnprintf(msg, 512, fmt, ap);
      va_end(ap);
   }

   return dialogue_listRaw( title, items, nitems, msg );
}
/**
 * @brief Creates a list dialogue with OK and Cancel button.
 *
 *    @param title Title of the dialogue.
 *    @param items Items in the list (should be all malloced, automatically freed).
 *    @param nitems Number of items.
 *    @param fmt printf formatted string with text to display.
 */
int dialogue_listRaw( const char* title, char **items, int nitems, const char *msg )
{
   int i;
   int w, h;
   glFont* font;
   unsigned int wid;
   int list_width, list_height;
   int text_height, text_width;
   int done;

   font = dialogue_getSize( title, msg, &text_width, &text_height );

   /* Calculate size stuff. */
   list_width  = 0;
   list_height = 0;
   for (i=0; i<nitems; i++) {
      list_width = MAX( list_width, gl_printWidthRaw( &gl_defFont, items[i] ) );
      list_height += gl_defFont.h + 5;
   }
   list_height += 100;
   w = MAX( list_width + 60, 200 );
   if (list_height > 500)
      h = (list_height*8)/10;
   else
      h = MAX( 300, list_height );
   h = MIN( (SCREEN_H*2)/3, h );

   /* Create the window. */
   wid = window_create( title, -1, -1, w, h );
   window_setData( wid, &done );
   window_addText( wid, 20, -40, w-40, text_height,  0, "txtMsg",
         font, &cDConsole, msg );
   window_setAccept( wid, dialogue_listClose );
   window_setCancel( wid, dialogue_listCancel );

   /* Create the list. */
   window_addList( wid, 20, -40-text_height-20,
         w-40, h - (40+text_height+20) - (20+30+20),
         "lstDialogue", items, nitems, 0, NULL );

   /* Create the buttons. */
   window_addButton( wid, -20, 20, 60, 30,
         "btnOK", "OK", dialogue_listClose );
   window_addButton( wid, -20-60-20, 20, 60, 30,
         "btnCancel", "Cancel", dialogue_listCancel );

   dialogue_open++;
   toolkit_loop( &done );

   return dialogue_listSelected;
}


static unsigned int choice_wid = 0; /**< Stores the choice window id. */
static char *choice_result; /**< Pointer to the choice result. */
static int choice_nopts; /**< Counter variable. */
/**
 * @brief Create the choice dialog. Need to add choices with below method.
 *
 *    @param caption Caption to use for the dialogue.
 *    @param msg Message to display.
 *    @param opts The number of options.
 */
void dialogue_makeChoice( const char *caption, const char *msg, int opts )
{
   int w,h;
   glFont* font;

   choice_result  = NULL;
   choice_nopts   = opts;
   font           = dialogue_getSize( caption, msg, &w, &h );

   /* create window */
   choice_wid     = window_create( caption, -1, -1, w, h+100+40*choice_nopts );
   /* text */
   window_addText( choice_wid, 20, -40, w-40, h,  0, "txtChoice",
         font, &cBlack, msg );
}
/**
 * @brief Add a choice to the dialog.
 *
 *    @param caption Caption to use for the dialogue (for sizing).
 *    @param msg Message to display (for sizing).
 *    @param *opt The value of the option.
 */
void dialogue_addChoice( const char *caption, const char *msg, const char *opt)
{
   int w,h;

   if (choice_nopts < 1)
      return;

   dialogue_getSize( caption, msg, &w, &h );

   /* buttons. Add one for each option in the menu. */
   window_addButton( choice_wid, w/2-125, choice_nopts*40, 250, 30, (char *) opt,
         (char *) opt, dialogue_choiceClose );
   choice_nopts --;

}
/**
 * @brief Run the dialog and return the clicked string.
 *
 * @note You must free the return value.
 *
 *    @return The string chosen.
 */
char *dialogue_runChoice (void)
{
   int done;
   char *res;

   /* tricky secondary loop */
   window_setData( choice_wid, &done );
   dialogue_open++;
   toolkit_loop( &done );

   /* Save value. */
   res = choice_result;
   choice_result = NULL;

   return res;
}
/**
 * @brief Closes a choice dialogue.
 *    @param wid Window being closed.
 *    @param str Stored to choice_result.
 */
static void dialogue_choiceClose( unsigned int wid, char* str )
{
   int *loop_done;

   /* Save result. */
   choice_result = strdup(str);

   /* Finish loop. */
   loop_done = window_getData( wid );
   *loop_done = 1;

   /* destroy the window */
   choice_wid = 0;
   window_destroy( wid );
   dialogue_open--;

}


/**
 * @brief Creates a secondary loop until loop_done is set to 1 or the toolkit closes.
 *
 * Almost identical to the main loop in naev.c.
 *
 *    @return 0 on success.
 */
static int toolkit_loop( int *loop_done )
{
   SDL_Event event;

   /* Delay a toolkit iteration. */
   toolkit_delay();

   *loop_done = 0;
   while (!(*loop_done) && toolkit_isOpen()) {
      /* Loop first so exit condition is checked before next iteration. */
      main_loop();

      while (SDL_PollEvent(&event)) { /* event loop */
         if (event.type == SDL_QUIT) { /* pass quit event to main engine */
            if (menu_askQuit()) {
               naev_quit();
               *loop_done = 1;
               SDL_PushEvent(&event);
               return -1;
            }
         }

         input_handle(&event); /* handles all the events and player keybinds */
      }
   }

   return 0;
}

