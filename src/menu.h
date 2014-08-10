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


#ifndef MENU_H
#  define MENU_H


/*
 * Menu status.
 */
#define MENU_MAIN       (1<<0) /**< Main menu (titlescreen). */
#define MENU_SMALL      (1<<1) /**< Small ingame menu. */
#define MENU_INFO       (1<<2) /**< Player information menu. */
#define MENU_DEATH      (1<<3) /**< Player death menu. */
#define MENU_OPTIONS    (1<<4) /**< Player's options menu. */
#define MENU_ASKQUIT    (1<<5) /**< Really quit naev? menu. */
#define menu_isOpen(f)  (menu_open & (f)) /**< Checks if a certain menu is opened. */
extern int menu_open; /**< Used internally by menu_isOpen() */


/*
 * Menu opening routines.
 */
void menu_main (void);
void menu_small (void);
void menu_death (void);
void menu_options (void);
int menu_askQuit (void);

/*
 * Closing.
 */
void menu_main_close (void);


#endif /* MENU_H */
