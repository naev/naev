/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dialogue.c
 *
 * @brief Is a high-level API around toolkit.c for easy window creation.
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

#include "log.h"
#include "toolkit.h"
#include "pause.h"
#include "opengl.h"
#include "input.h"
#include "menu.h"
#include "nstring.h"
#include "ndata.h"


static int dialogue_open; /**< Number of dialogues open. */


/*
 * Prototypes.
 */
/* extern */
extern void main_loop( int update ); /* from naev.c */
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
 * @brief Used to store information for input dialogues
 */
typedef struct InputDialogue_ {
   unsigned int input_wid; /**< wid of input window */
   int x; /**< x position where we can start drawing */
   int y; /**< y position where we can start drawing. */
   int w; /**< width of area we can draw in */
   int h; /**< height of area we can draw in */
   void (*item_select_cb) (unsigned int wid, char* wgtname,
                           int x, int y, int w, int h
        ); /**< callback when an item is selected */
} InputDialogue;
static void select_call_wrapper(unsigned int wid, char* wgtname);

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
   if (dialogue_open < 0)
      WARN(_("Dialogue counter not in sync!"));
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
   if (dialogue_open < 0)
      WARN(_("Dialogue counter not in sync!"));
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
   wdw = window_create( _("Warning"), -1, -1, 300, 90 + h );
   window_setData( wdw, &done );
   window_addText( wdw, 20, -30, 260, h,  0, "txtAlert",
         &gl_smallFont, &cBlack, msg );
   window_addButton( wdw, 135, 20, 50, 30, "btnOK", _("OK"),
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
   int i, titlelen, msglen;

   /* Get title length. */
   titlelen = gl_printWidthRaw( &gl_defFont, title );
   msglen = gl_printWidthRaw( &gl_smallFont, msg );

   /* Try widths from 300 to 800 in 50 px increments.
    * Each subsequent width gets an additional line, following this table:
    *
    *    300 px:  2 lines,  540 px total
    *    350 px:  3 lines,  930 px total
    *    ...
    *    800 px: 12 lines, 9600 px total
    */
   for (i=0; i<11; i++)
      if (msglen < (260 + i * 50) * (2 + i))
         break;

   w = 300 + i * 50;
   w = MAX(w, titlelen+40); /* Expand width if the title is long. */

   /* Now we look at proportion. */
   font = &gl_smallFont;
   h = gl_printHeightRaw( font, w-40, msg );


   d = ((double)w/(double)h)*(3./4.); /* deformation factor. */
   if (fabs(d) > 0.3) {
      if (h > w)
         w = h;
      h = gl_printHeightRaw( font, w-40, msg );
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
 * @brief Opens a dialogue window with an ok button, a message and an image.
 *
 *    @param caption Window title.
 *    @param img Path of the image file (*.png) to display.
 *    @param fmt Printf style message to display.
 */
void dialogue_msgImg( const char* caption, const char *img, const char *fmt, ... )
{
   char msg[4096];
   va_list ap;

   if (fmt == NULL) return;
   else { /* get the message */
      va_start(ap, fmt);
      vsnprintf(msg, 4096, fmt, ap);
      va_end(ap);
   }

   dialogue_msgImgRaw( caption, msg, img, -1, -1 );
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
   window_addButton( msg_wid, (w-50)/2, 20, 50, 30, "btnOK", _("OK"),
         dialogue_close );

   dialogue_open++;
   toolkit_loop( &done );
}


/**
 * @brief Opens a dialogue window with an ok button, a fixed message and an image.
 *
 *    @param caption Window title.
 *    @param msg Message to display.
 *    @param width Width of the image. Negative uses image width.
 *    @param height Height of the image. Negative uses image height.
 */
void dialogue_msgImgRaw( const char* caption, const char *msg, const char *img, int width, int height )
{
   int w, h, img_width, img_height;
   glFont* font;
   unsigned int msg_wid;
   int done;
   glTexture *gfx;
   char buf[PATH_MAX];

   /* Get the desired texture */
   /* IMPORTANT : texture must not be freed here, it will be freed when the widget closes */
   nsnprintf( buf, sizeof(buf), "%s%s", GFX_PATH, img );
   gfx = gl_newImage( buf, 0 );
   if (gfx == NULL)
      return;

   /* Find the popup's dimensions from text and image */
   img_width  = (width < 0)  ? gfx->w : width;
   img_height = (height < 0) ? gfx->h : height;
   font = dialogue_getSize( caption, msg, &w, &h );
   if (h < img_width) {
      h = img_width;
   }

   /* Create the window */
   msg_wid = window_create( caption, -1, -1, img_width + w, 110 + h );
   window_setData( msg_wid, &done );

   /* Add the text box */
   window_addText( msg_wid, img_width+40, -40, w-40, h,  0, "txtMsg",
         font, &cBlack, msg );

   /* Add a placeholder rectangle for the image */
   window_addRect( msg_wid, 20, -40, img_width, img_height,
         "rctGFX", &cGrey10, 1 );

   /* Actually add the texture in the rectangle */
   window_addImage( msg_wid, 20, -40, img_width, img_height,
         "ImgGFX", gfx, 0 );

   /* Add the OK button */
   window_addButton( msg_wid, (img_width+w -50)/2, 20, 50, 30, "btnOK", _("OK"),
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
   window_addButtonKey( wid, w/2-50-10, 20, 50, 30, "btnYes", _("Yes"),
         dialogue_YesNoClose, SDLK_y );
   window_addButtonKey( wid, w/2+10, 20, 50, 30, "btnNo", _("No"),
         dialogue_YesNoClose, SDLK_n );

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
   else {
      WARN(_("Unknown button clicked in YesNo dialogue!"));
      result = 1;
   }

   /* set data. */
   loop_done = window_getData( wid );
   loop_done[0] = 1;
   loop_done[1] = result;
}


static InputDialogue input_dialogue; /**< Stores the input window id and callback. */
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

   if (input_dialogue.input_wid) return NULL;

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
   input_dialogue.input_wid = window_create( title, -1, -1, 240, h+140 );
   window_setData( input_dialogue.input_wid, &done );
   window_setAccept( input_dialogue.input_wid, dialogue_inputClose );
   window_setCancel( input_dialogue.input_wid, dialogue_cancel );
   /* text */
   window_addText( input_dialogue.input_wid, 30, -30, 200, h,  0, "txtInput",
         &gl_smallFont, &cDConsole, msg );
   /* input */
   window_addInput( input_dialogue.input_wid, 20, -50-h, 200, 20, "inpInput", max, 1, NULL );
   window_setInputFilter( input_dialogue.input_wid, "inpInput", "/" ); /* Remove illegal stuff. */
   /* button */
   window_addButton( input_dialogue.input_wid, -20, 20, 80, 30,
         "btnClose", _("Done"), dialogue_inputClose );

   /* tricky secondary loop */
   dialogue_open++;
   done  = 0;
   input = NULL;
   while ((done >= 0) && (!input ||
         ((int)strlen(input) < min))) { /* must be longer than min */

      if (input) {
         dialogue_alert( _("Input must be at least %d character%s long!"),
               min, (min==1) ? "" : "s" );
         free(input);
         input = NULL;
      }

      if (toolkit_loop( &done ) != 0) /* error in loop -> quit */
         return NULL;

      /* save the input */
      if (done < 0)
         input = NULL;
      else
         input = strdup(window_getInput(input_dialogue.input_wid, "inpInput"));
   }

   /* cleanup */
   if (input != NULL) {
      window_destroy( input_dialogue.input_wid );
      dialogue_open--;
   }
   input_dialogue.input_wid = 0;

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
 * @brief used to pass appropriate information to the method that
 *    handles updating the extra information area in the dialogue
 *    listpanel.
 *
 *    @param wid Window id
 *    @param wgtname name of the widget that raised the event.
 */
static void select_call_wrapper(unsigned int wid, char* wgtname)
{
   if(input_dialogue.item_select_cb)
      input_dialogue.item_select_cb(wid, wgtname,input_dialogue.x,
            input_dialogue.y, input_dialogue.w,
            input_dialogue.h);
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

   if (input_dialogue.input_wid) return -1;

   if (fmt == NULL) return -1;
   else { /* get the message */
      va_start(ap, fmt);
      vsnprintf(msg, 512, fmt, ap);
      va_end(ap);
   }

   return dialogue_listPanelRaw( title, items, nitems, 0, 0, NULL, NULL, msg );
}
/**
 * @brief Creates a list dialogue with OK and Cancel button.
 *
 *    @param title Title of the dialogue.
 *    @param items Items in the list (should be all malloced, automatically freed).
 *    @param nitems Number of items.
 *    @param msg string with text to display.
 */
int dialogue_listRaw( const char* title, char **items, int nitems, const char *msg )
{
   if (input_dialogue.input_wid) return -1;
   return dialogue_listPanelRaw( title, items, nitems, 0, 0, NULL, NULL, msg );
}
/**
 * @brief Creates a list dialogue with OK and Cancel buttons, with a fixed message,
 *       as well as a small extra area for the list to react to item selected events.
 *
 *    @param title Title of the dialogue.
 *    @param items Items in the list (should be all malloced, automatically freed).
 *    @param nitems Number of items.
 *    @param extrawidth Width of area to add for select_call callback.
 *    @param minheight Minimum height for the window.
 *    @param add_widgets This function is called with the new window as an argument
 *          allowing for initial population of the extra area.
 *    @param select_call This function is called when a new item in the list is
 *          selected, receiving the window's id and the selected widgets name
 *          as arguments.
 *    @param fmt printf formatted string with text to display.
 */
int dialogue_listPanel( const char* title, char **items, int nitems, int extrawidth,
      int minheight, void (*add_widgets) (unsigned int wid, int x, int y, int w, int h),
      void (*select_call) (unsigned int wid, char* wgtname, int x, int y, int w, int h),
      const char *fmt, ... )
{
   char msg[512];
   va_list ap;

   if (input_dialogue.input_wid)
      return -1;

   if (fmt == NULL)
      return -1;

   /* get the message */
   va_start(ap, fmt);
   vsnprintf(msg, 512, fmt, ap);
   va_end(ap);

   return dialogue_listPanelRaw( title, items, nitems, extrawidth, minheight,
         add_widgets, select_call, msg );
}
/**
 * @brief Creates a list dialogue with OK and Cancel buttons, with a fixed message,
 *       as well as a small extra area for the list to react to item selected events.
 *
 *    @param title Title of the dialogue.
 *    @param items Items in the list (should be all malloced, automatically freed).
 *    @param nitems Number of items.
 *    @param extrawidth Width of area to add for select_call callback.
 *    @param minheight Minimum height for the window.
 *    @param add_widgets This function is called with the new window as an argument
 *          allowing for initial population of the extra area.
 *    @param select_call (optional) This function is called when a new item in the
 *          list is selected, receiving the window's id and the selected widgets
 *          name as arguments.
 *    @param msg string with text to display.
 */
int dialogue_listPanelRaw( const char* title, char **items, int nitems, int extrawidth,
      int minheight, void (*add_widgets) (unsigned int wid, int x, int y, int w, int h),
      void (*select_call) (unsigned int wid, char* wgtname, int x, int y, int w, int h),
      const char *msg )
{
   int i;
   int w, h, winw, winh;
   glFont* font;
   unsigned int wid;
   int list_width, list_height;
   int text_height, text_width;
   int done;

   if (input_dialogue.input_wid) return -1;

   font = dialogue_getSize( title, msg, &text_width, &text_height );

   /* Calculate size stuff. */
   list_width  = 0;
   list_height = 0;
   for (i=0; i<nitems; i++) {
      list_width = MAX( list_width, gl_printWidthRaw( &gl_defFont, items[i] ) );
      list_height += gl_defFont.h + 5;
   }
   list_height += 100;
   if (list_height > 500)
      h = (list_height*8)/10;
   else
      h = MAX( 300, list_height );

   h = MIN( (SCREEN_H*2)/3, h );
   w = MAX( list_width + 60, 200 );

   winw = w + extrawidth;
   winh = MAX( h, minheight );

   h = winh;

   /* Create the window. */
   wid = window_create( title, -1, -1, winw, winh );
   window_setData( wid, &done );
   window_addText( wid, 20, -40, w-40, text_height,  0, "txtMsg",
         font, &cDConsole, msg );
   window_setAccept( wid, dialogue_listClose );
   window_setCancel( wid, dialogue_listCancel );

   if(add_widgets)
      add_widgets(wid, w, 0, winw, winh);

   if(select_call) {
      input_dialogue.x = w;
      input_dialogue.y = 0;
      input_dialogue.w = winw;
      input_dialogue.h = winh;
      input_dialogue.item_select_cb = select_call;
   }

   /* Create the list. */
   window_addList( wid, 20, -40-text_height-20,
         w-40, h - (40+text_height+20) - (20+30+20),
         "lstDialogue", items, nitems, 0, select_call_wrapper );

   /* Create the buttons. */
   window_addButton( wid, -20, 20, 60, 30,
         "btnOK", _("OK"), dialogue_listClose );
   window_addButton( wid, -20-60-20, 20, 60, 30,
         "btnCancel", _("Cancel"), dialogue_listCancel );

   dialogue_open++;
   toolkit_loop( &done );
   /* cleanup */
   input_dialogue.x = 0;
   input_dialogue.y = 0;
   input_dialogue.w = 0;
   input_dialogue.h = 0;
   input_dialogue.item_select_cb = NULL;

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
 * @TODO Fix this, we need proper threading as the music Lua and dialogue running Lua
 *       may be run in parallel and this will make everyone cry. So basically we have
 *       a race condition due to the "threading" effect this creates. Solved most of
 *       it by removing globals in the Lua event/mission code, but this doesn't mean
 *       it's solved. It just means it's extremely unlikely.
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
      main_loop( 0 );

      while (SDL_PollEvent(&event)) { /* event loop */
         if (event.type == SDL_QUIT) { /* pass quit event to main engine */
            if (menu_askQuit()) {
               naev_quit();
               *loop_done = 1;
               SDL_PushEvent(&event);
               return -1;
            }
         }
#if SDL_VERSION_ATLEAST(2,0,0)
         else if (event.type == SDL_WINDOWEVENT &&
               event.window.event == SDL_WINDOWEVENT_RESIZED) {
            naev_resize( event.window.data1, event.window.data2 );
            continue;
         }
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

         input_handle(&event); /* handles all the events and player keybinds */
      }
   }

   return 0;
}

