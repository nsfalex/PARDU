#ifndef __TERMUI_H
#define __TERMUI_H

/* colors */
#ifndef __CURSES_H

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW  3
#define COLOR_BLUE  4
#define COLOR_MAGENTA 5
#define COLOR_CYAN  6
#define COLOR_WHITE 7

#endif /* __CURSES_H */

/* for convenience */
typedef void* window;

/* shared structures */

/*  window_border:
 *    contains the text characters for a window's border
 */
typedef struct window_border
{
  char ls,  /* left side   */
       rs,  /* right side  */
       ts,  /* top side    */
       bs,  /* bottom side */
       tl,  /* top-left corner     */
       tr,  /* top-right corner    */
       bl,  /* bottom-left corner  */
       br;  /* bottom-right corner */
} winborder_t;


/*  window_properties:
 *    contains all the necessary properties for creating a window
 *
 *    int cols:             window width in columns
 *    int lines:            window height in rows
 *    int v_offset:         vertical offset in columns
 *    int h_offset:         horizontal offset in rows
 *    const char *title:    text to insert at the top of the window,
 *                          NULL omits printing a title
 *    short fg_color:       window contents foreground color or NULL
 *    short bg_color:       window contents background color or NULL
 *    short border_color:   window border color or NULL for defaults
 *    winborder_t *border:  characters making up the window border,
 *                          NULL sets the default box() borders
 */
typedef struct window_properties
{
  int lines, cols;
  int v_offset, h_offset;
  const char *title;
  short fg_color, bg_color, border_color;
  const winborder_t *border;
} winprop_t;


/* shared function declarations */

void  init_termui(void);                    /* start ncurses mode */
void  end_termui(void);                     /* end ncurses mode   */

/* initialize a winprop_t struct */
void  init_window(winprop_t*, int, int, int, int, const char*, short, short,
        short, const winborder_t*);


void* create_window(const winprop_t *);     /* create a new ncurses window and
                                             * intialize an internal window   */
void  destroy_window(void *);               /* destroy the ncurses window and
                                             * deallocate the internal window */

void  winmvcurs(const void *, int, int);
int   winputstr(const void *, const char *);
int   winputl(const void *, const char *);

void  draw_menubar(int, const char *[]);
void  draw_bar(int, short);

void waitasecwillya(window win);

#endif /* __TERMUI_H */
