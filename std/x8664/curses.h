#ifndef CURSES_H
# define CURSES_H

#define ERR -1
#define OK 0

#include <stdbool.h>

int mvaddch(int y, int x, char ch);
void *initscr(void);
int noecho(void);
int curs_set(int visibility);
int nodelay(void *win, bool bf);
int leaveok(void *win, bool bf);
int scrollok(void *win, bool bf);
int getch(void);
int refresh(void);
int usleep(int microseconds);
int mvcur(int oldrow, int oldcol, int newrow, int n);
int endwin(void);

#define TRUE 1
#define FALSE 0

extern void *stdscr;
extern int COLS;
extern int LINES;

#endif
