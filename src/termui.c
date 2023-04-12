#include "termui.h"
#include <ncurses.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _XOPEN_SOURCE_EXTENDED  /* per standard, adds NCURSES_WIDECHAR */
#include <ncurses.h>
#include <menu.h>


/*  _curses_window:
 *    aggregates the ncurses WINDOW pointers, cursor position and color pair
 *
 *    WINDOW *container:    container window on which the border is drawn
 *    WINDOW *subwindow:    subwin of container for drawing content in
 *    int lines:            vertical size of subwin in lines
 *    int cols:             horizontal size of subwin in columns
 *    int curs_h:           horizontal cursor position for subwin 
 *    int curs_v:           vertical cursor position for subwin
 *    short ctr_color_pair: ncurses internal color pair for the container window
 *    short sub_color_pair: ncurses internal color pair for the subwindow
 */
typedef struct _curses_window
{
  WINDOW *container, *subwindow;
  int lines, cols, curs_h, curs_v;
  short ctr_color_pair, sub_color_pair;
} window_t;


/* whether ncurses detects that the terminal supports colors */
static bool color_supported = FALSE;

/* the COLOR_PAIR index to use with ncurses */
static int color_index = 1;


/* forward declarations */
static void set_window_border(WINDOW*, const winborder_t*);



/*  init_termui:
 *    initializes ncurses and prepares the terminal UI.
 */
void
init_termui()
{
  /* set the locale to the system locale */
  setlocale(LC_ALL, "");

  /* init curses and pass on all keyboard input without echoing */
  initscr(); raw(); keypad(stdscr, TRUE); noecho();
  
  /* disable the cursor */
  curs_set(FALSE);

  /* don't flush queued tty output on interrupt,
   * this prevents a mismatch between what ncurses thinks has
   * been drawn and what's actually on screen */
  intrflush(stdscr, FALSE);

  /* store the dimensions for later window drawing */
  /* getmaxyx(stdscr, term_width, term_height); */

  /* check for and initialize color support */
  color_supported = has_colors();
  if (color_supported)
    start_color();

  //box(stdscr, 0, 0);
  
  refresh();
}


/*  end_termui:
 *    ends ncurses mode. not accounting allocated objects as it's expected that
 *    the program will exit shortly after a call to this, which will make
 *    deallocation unnecessary.
 */
void end_termui()
{
  endwin();
}


/*  set_window_border:
 *    sets the window border based on values described in wb
 *
 *    WINDOW *w:           pointer to a ncurses window
 *    window_border *wb:   pointer to a border struct
 */
static inline void
set_window_border(WINDOW *w, const winborder_t *wb)
{
  wborder(
    w,
    wb->ls,
    wb->rs,
    wb->ts,
    wb->bs,
    wb->tl,
    wb->tr,
    wb->bl,
    wb->br
  );
}



/*  init_window:
 *    initializes a winprop_t struct based on arguments passed in the call,
 *    reducing multiple assignments in a window_properties struct to a single
 *    funcion call.
 *
 *    winprop_t *wp:    pointer to the window_properties struct to init
 *    
 */
void
init_window(
  winprop_t *wp,
  int lines,
  int cols,
  int v_off,
  int h_off,
  const char* title,
  short fg_c,
  short bg_c,
  short bd_c,
  const winborder_t *border)
{
  memset(wp, 0, sizeof(winprop_t));

  wp->lines = lines;
  wp->cols = cols;
  wp->v_offset = v_off;
  wp->h_offset = h_off;
  wp->title = title;
  wp->fg_color = fg_c;
  wp->bg_color = bg_c;
  wp->border_color = bd_c;
  wp->border = border;
}


/*  create_window:
 *    creates two windows in ncurses, a container window and a subwindow to
 *    put content in. this prevents overwriting the border and causing the
 *    window to lose definition.
 *    
 *    returns a pointer to an internal window struct.
 *
 *    winprop_t *wp:  pointer to a struct describing the window
 */
void *
create_window(const winprop_t *wp)
{
  window_t *w;
  short fg, bg;                /* foreground and background colors */
  int lines = wp->lines,
      cols  = wp->cols,
      h_off = wp->h_offset,
      v_off = wp->v_offset;

  w = malloc(sizeof(window_t));
  memset(w, 0, sizeof(window_t));

  /* store the subwindow size for later drawing */
  w->lines = wp->lines - 2;
  w->cols = wp->cols - 2;

  /* initialize the internal cursor */
  w->curs_h = w->curs_v = 0;

  /* initialize both the container and sub windows */
  w->container = newwin(lines, cols, v_off, h_off);
  w->subwindow = subwin(w->container, lines-2, cols-2, v_off+1, h_off+1);

  /* set the border color, this also sets the title color */
  if (wp->border_color) {
    init_pair(color_index, wp->border_color, COLOR_BLACK);
    wcolor_set(w->container, color_index, NULL);
    ++color_index;
  }

  /* draw a border around the window */
  if (wp->border)
    set_window_border(w->container, wp->border);
  else
    box(w->container, 0, 0);
  
  /* if colors are supported, init a new COLOR_PAIR and set subwindow colors */
  if (color_supported) {
    if (wp->fg_color)
      fg = wp->fg_color;
    else
      fg = COLOR_WHITE;
    
    if (wp->bg_color)
      bg = wp->bg_color;
    else
     bg = COLOR_BLACK;

    init_pair(color_index, fg, bg);
    wcolor_set(w->subwindow, color_index, NULL);
    ++color_index;
  }

  /* set the window title */
  wattron(w->container, A_BOLD);
  mvwaddstr(w->container, 0, 2, wp->title);

  /* refresh both windows to show the updated contents on-screen */
  wrefresh(w->container);
  wrefresh(w->subwindow);

  return w;
}


/*  destroy_window:
 *    clears the window's borders and deletes the window
 *
 *    WINDOW *w:  window to remove from UI
 */
void
destroy_window(void *window)
{
  window_t *w = (window_t *) window;

  /* remove the border from the container window */
  wborder(w->container, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(w->container);

  /* delete both the container and sub windows */
  delwin(w->container); delwin(w->subwindow);

  /* deallocate the window structure */
  free(window);
}


/*  winmvcurs:
 *    moves the internal window struct's cursor.
 *
 *    const void *win:    window struct pointer
 *    int line:           line (y-location) to move to
 *    int column:         column (x-location) to move to
 */
inline void
winmvcurs(const void *win, int line, int column)
{
  window_t *w = (window_t *) win;

  w->curs_h = column;
  w->curs_v = line;
}


/*  winputstr:
 *    inserts a string into a window and wraps the line if it's too long.
 *
 *    const void *win:    pointer to a window struct to insert text into
 *    const char *str:    string to insert into window
 */
int
winputstr(const void *win, const char *str)
{
  int len, err;
  window_t *w = (window_t *) win;

  len = strlen(str);

  /* move the cursor and insert the string */
  err = mvwaddstr(w->subwindow, w->curs_v, w->curs_h, str);
  wrefresh(w->subwindow);

  /* if the string is too large for the window, don't attempt to wrap but
   * increment the internal cursor's y pos */
  if (len < w->cols)
    ++w->lines;
  else
    w->cols += len;

  return err;
}


/*  winaddl:
 *    inserts a string into a window without wrapping the line.
 *
 *    const void *win:    pointer to a window struct to insert text into
 *    const char *str:    string to insert into window
 */
int
winputl(const void *win, const char *str)
{
  int err;
  window_t *w = (window_t *) win;

  /* move the cursor and insert the string */  
  err = mvwinsstr(w->subwindow, w->curs_v, w->curs_h, str);
  wrefresh(w->subwindow);

  ++w->curs_v;

  return err;
}


/*  draw_menubar:
 *    draws a horizontal menu bar across the screen, with text passed in items.
 *    items has to be a null-terminated array of char pointers
 *
 *    int line:             vertical line to print the bar on
 *    const char *items[]:  null-terminated array of char pointers for options
 */
void
draw_menubar(int line, const char *items[])
{
  int y, x;
  const char **item;

  /* move the cursor to the first column on the given line */
  move(line, 3); refresh();
  attr_on(A_BOLD, NULL);

  /* loop through and add the items to the menu bar, space them appropriately */
  item = items;

  while (*item) {
    addstr(*item++);  /* pass the string to addstr, then increment */

    getyx(stdscr, y, x);
    move(y, x+2); refresh();
  }

  attr_off(A_BOLD, NULL);

  refresh();
}


/*  draw_bar:
 *    draws a bar to the screen on the given line
 *
 */
void
draw_bar(int line, short color)
{
  init_pair(color_index, COLOR_WHITE, color);
  color_set(color_index++, NULL);

  move(line, 0); refresh();
  hline(' ', COLS);

  color_set(0, NULL);
}


void waitasecwillya(window win)
{
  char line[50] = {0};
  int rand = 23123444;

  //while(getch() != 'q')
  //{
  while(true) {
    srandom(rand++);
    snprintf(line, 50, "%#10x", (unsigned int) random());
    winputl(win, line);
    usleep(50000);
  }
  //}
}

