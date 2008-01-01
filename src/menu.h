/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MENU_H
#  define MENU_H


#define MENU_MAIN       (1<<0)
#define MENU_SMALL      (1<<1)
#define MENU_INFO       (1<<2)
#define MENU_DEATH      (1<<3)
#define menu_isOpen(f)  (menu_open & (f))
extern int menu_open;

void menu_main (void);
void menu_small (void);
void menu_info (void);
void menu_death (void);


#endif /* MENU_H */
