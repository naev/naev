/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef DIALOGUE_H
#  define DIALOGUE_H


/*
 * popups and alerts
 */
void dialogue_alert( const char *fmt, ... ); /* does not pause execution */
void dialogue_msg( const char *caption, const char *fmt, ... );
void dialogue_msgRaw( const char *caption, const char *msg );
int dialogue_YesNo( const char *caption, const char *fmt, ... ); /* Yes = 1, No = 0 */
int dialogue_YesNoRaw( const char *caption, const char *msg );
void dialogue_makeChoice( const char *caption, const char *msg, int opts );
void dialogue_addChoice( const char *caption, const char *msg, const char *opt );
char *dialogue_runChoice (void);
char* dialogue_input( const char* title, int min, int max, const char *fmt, ... );
char* dialogue_inputRaw( const char* title, int min, int max, const char *msg  );
int dialogue_list( const char* title, char **items, int nitems, const char *fmt, ... );
int dialogue_listRaw( const char* title, char **items, int nitems, const char *msg );
int dialogue_listPanel ( const char* title, char **items, int nitems, int extrawidth,
        int minheight, void (*add_widgets) (unsigned int wid, int x, int y, int w, int h),
        void (*select_call) (unsigned int wid, char* wgtname, int x, int y, int w, int h),
	const char *fmt, ... );
int dialogue_listPanelRaw( const char* title, char **items, int nitems, int extrawidth,
        int minheight, void (*add_widgets) (unsigned int wid, int x, int y, int w, int h),
        void (*select_call) (unsigned int wid, char* wgtname, int x, int y, int w, int h),
	const char *msg );

/*
 * misc
 */
int dialogue_isOpen (void);


#endif /* DIALOGUE_H */

