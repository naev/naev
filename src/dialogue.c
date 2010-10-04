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


int dialogue_open; /**< Number of dialogues open. */


/*
 * Prototypes.
 */
/* extern */
extern void main_loop (void); /* from naev.c */
/* dialogues */
static glFont* dialogue_getSize( const char* title,
      const char* msg, int* width, int* height );
static void dialogue_alertClose( unsigned int wid, char* str );
static void dialogue_msgClose( unsigned int wid, char* str );
static void dialogue_YesNoClose( unsigned int wid, char* str );
static void dialogue_inputClose( unsigned int wid, char* str );
static void dialogue_inputCancel( unsigned int wid, char* str );
static void dialogue_choiceClose( unsigned int wid, char* str );
/* secondary loop hack */
static int loop_done; /**< Used to indicate the secondary loop is finished. */
static int toolkit_loop (void);


/**
 * @brief Checks to see if a dialogue is open.
 */
int dialogue_isOpen (void)
{
   return !!dialogue_open;
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
   int h;

   if (fmt == NULL) return;
   else { /* get the message */
      va_start(ap, fmt);
      vsnprintf(msg, 512, fmt, ap);
      va_end(ap);
   }

   h = gl_printHeightRaw( &gl_smallFont, 260, msg );

   /* create the window */
   wdw = window_create( "Warning", -1, -1, 300, 90 + h );
   window_addText( wdw, 20, -30, 260, h,  0, "txtAlert",
         &gl_smallFont, &cBlack, msg );
   window_addButton( wdw, 135, 20, 50, 30, "btnOK", "OK",
         dialogue_alertClose );

   dialogue_open++;
   toolkit_loop();
}
/**
 * @brief Closes the alert dialogue.
 *    @param wid Window being closed.
 *    @param str Unused.
 */
static void dialogue_alertClose( unsigned int wid, char* str )
{
   (void)str;
   window_destroy( wid );
   loop_done = 1;
   dialogue_open--;
}


/**
 * @brief Gets the size needed for the dialogue.
 *
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
 *    @param text Message to display.
 */
void dialogue_msgRaw( const char* caption, const char *msg )
{
   int w,h;
   glFont* font;
   unsigned int msg_wid;

   font = dialogue_getSize( caption, msg, &w, &h );

   /* create the window */
   msg_wid = window_create( caption, -1, -1, w, 110 + h );
   window_addText( msg_wid, 20, -40, w-40, h,  0, "txtMsg",
         font, &cBlack, msg );
   window_addButton( msg_wid, (w-50)/2, 20, 50, 30, "btnOK", "OK",
         dialogue_msgClose );

   dialogue_open++;
   toolkit_loop();
}

/**
 * @brief Closes a message dialogue.
 *    @param wid Window being closed.
 *    @param str Unused.
 */
static void dialogue_msgClose( unsigned int wid, char* str )
{
   (void)str;
   window_destroy( wid );
   loop_done = 1;
   dialogue_open--;
}


static int yesno_result; /**< Stores the yesno dialogue result. */
static unsigned int yesno_wid = 0; /**< Stores the yesno window id. */
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

   if (yesno_wid) return -1;

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
   int w,h;
   glFont* font;

   font = dialogue_getSize( caption, msg, &w, &h );

   /* create window */
   yesno_wid = window_create( caption, -1, -1, w, h+110 );
   /* text */
   window_addText( yesno_wid, 20, -40, w-40, h,  0, "txtYesNo",
         font, &cBlack, msg );
   /* buttons */
   window_addButton( yesno_wid, w/2-50-10, 20, 50, 30, "btnYes", "Yes",
         dialogue_YesNoClose );
   window_addButton( yesno_wid, w/2+10, 20, 50, 30, "btnNo", "No",
         dialogue_YesNoClose );

   /* tricky secondary loop */
   dialogue_open++;
   toolkit_loop();

   /* return the result */
   return yesno_result;
}
/**
 * @brief Closes a yesno dialogue.
 *    @param wid Window being closed.
 *    @param str Unused.
 */
static void dialogue_YesNoClose( unsigned int wid, char* str )
{
   /* store the result */
   if (strcmp(str,"btnYes")==0)
      yesno_result = 1;
   else if (strcmp(str,"btnNo")==0)
      yesno_result = 0;

   /* destroy the window */
   window_destroy( wid );
   yesno_wid = 0;

   loop_done = 1;
   dialogue_open--;
}


static unsigned int input_wid = 0; /**< Stores the input window id. */
static int input_cancelled = 0; /**< Stores whether or not the input was cancelled. */
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
 *    @param fmt Printf style message to display on the dialogue.
 *    @return The message the player typed or NULL if it was cancelled.
 */
char* dialogue_inputRaw( const char* title, int min, int max, const char *msg )
{
   char *input;
   int h;

   /* Start out not cancelled. */
   input_cancelled = 0;

   /* get text height */
   h = gl_printHeightRaw( &gl_smallFont, 200, msg );

   /* create window */
   input_wid = window_create( title, -1, -1, 240, h+140 );
   window_setAccept( input_wid, dialogue_inputClose );
   window_setCancel( input_wid, dialogue_inputCancel );
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
   input = NULL;
   while (!input_cancelled && (!input ||
         ((int)strlen(input) < min))) { /* must be longer than min */

      if (input) {
         dialogue_alert( "Input must be at least %d character%s long!",
               min, (min==1) ? "s" : "" );
         free(input);
         input = NULL;
      }

      if (toolkit_loop() != 0) /* error in loop -> quit */
         return NULL;

      /* save the input */
      if (input_cancelled != 0)
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
   (void) wid;

   /* break the loop */
   loop_done = 1;
}
/**
 * @brief Cancels an input dialogue.
 *    @param wid Window being closed.
 *    @param str Unused.
 */
static void dialogue_inputCancel( unsigned int wid, char* str )
{
   input_cancelled = 1;
   dialogue_inputClose(wid,str);
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
 * @note The returned string is _ONLY_ valid for _ONE_ frame. Copy it if you want it to last more then a frame.
 *
 *    @return The string chosen.
 */
char *dialogue_runChoice (void)
{
   /* tricky secondary loop */
   dialogue_open++;
   toolkit_loop();

   return choice_result;
}
/**
 * @brief Closes a choice dialogue.
 *    @param wid Window being closed.
 *    @param str Stored to choice_result.
 */
static void dialogue_choiceClose( unsigned int wid, char* str )
{
   choice_result = str;

   /* destroy the window */
   window_destroy( wid );
   choice_wid = 0;

   loop_done = 1;
   dialogue_open--;
}


/**
 * @brief Creates a secondary loop until loop_done is set to 1 or the toolkit closes.
 *
 * Almost identical to the main loop in naev.c.
 *
 *    @return 0 on success.
 */
static int toolkit_loop (void)
{
   SDL_Event event;

   /* Delay a toolkit iteration. */
   toolkit_delay();

   loop_done = 0;
   while (!loop_done && toolkit_isOpen()) {
      /* Loop first so exit condition is checked before next iteration. */
      main_loop();

      while (SDL_PollEvent(&event)) { /* event loop */
         if (event.type == SDL_QUIT) { /* pass quit event to main engine */
            loop_done = 1;
            SDL_PushEvent(&event);
            return -1;
         }

         input_handle(&event); /* handles all the events and player keybinds */
      }
   }

   return 0;
}

