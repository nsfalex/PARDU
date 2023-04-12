#ifndef __PROC_H
#define __PROC_H

#include "mem.h"

/*  _open_process:
 *    contains the necessary data to interact with a process
 */
typedef struct _open_process
{
  int pid;                      /* pid of the process to interact with    */
  FILE *mem;                    /* handle to interact with the memory     */
  ll_memmap_file *ll_files;     /* start of the linked list of open files */
} process;

int lookup_pid(char*);

#endif /* __PROC_H */
