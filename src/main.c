#include "termui.h"
#include "proc.h"
#include "mem.h"
#include "util.h"

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>

/* global variables */
extern int LINES, COLS;

/* forward declarations */
int start_ui();


int
main (int argc, char *argv[])
{
  //if(start_ui() != 0)
  //  perror("Couldn't initialize ncurses");

  int pid;
  ll_memmap_file *ll_mmf;

  if (argc < 2) {
    puts("Please provide a process to open");
    exit(1);
  } 

  pid = lookup_pid(argv[1]);
  printf("pid: %d\n", pid);

  ll_mmf = parse_proc_maps(pid);

  do {
    printf("name: %s\n", ll_mmf->fpath);
    printf("address space: %p-%p\n", ll_mmf->start_addr, ll_mmf->end_addr);
    printf("offset: %p\n", ll_mmf->offset);
    printf("maj/min: %02x-%02x\n", ll_mmf->dev_major, ll_mmf->dev_minor);
    printf("inode: %d\n", ll_mmf->inode);
    printf("mode: %d\n\n", ll_mmf->mode);
  } while ((ll_mmf = ll_mmf->next) != NULL);

  return EXIT_SUCCESS;
}


int
start_ui()
{
  window w_disassembly,
         w_memmap;
  winprop_t wp_disassembly,
            wp_memmap;
  const char *items[] = {
    "test",
    "test1",
    "test2",
    "test3",
    (char *) NULL
  };

  init_termui();

  //printf("lines: %d, cols: %d\n", LINES, COLS);

  init_window(
    &wp_disassembly,
    LINES - 2,
    COLS / 2 - 1,
    1, 0,
    "Disassembly",
    COLOR_WHITE,
    COLOR_BLACK,
    COLOR_MAGENTA,
    NULL);

  init_window(
    &wp_memmap,
    LINES - 2,
    COLS / 2,
    1,
    COLS / 2,
    "Mapped Libraries",
    COLOR_WHITE,
    COLOR_BLACK,
    COLOR_MAGENTA,
    NULL);

  w_disassembly = create_window(&wp_disassembly);
  w_memmap = create_window(&wp_memmap);

  draw_menubar(0, items);
  draw_bar(LINES-1, COLOR_RED);

  waitasecwillya(w_disassembly);

  end_termui();

  return 0;
}
