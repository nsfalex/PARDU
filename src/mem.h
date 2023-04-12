#ifndef __MEM_H
#define __MEM_H

#include <stdint.h>
#include <stdio.h>

enum module_perms {
  MODE_READ = 1,
  MODE_WRITE = 2,
  MODE_EXECUTE = 4,
  MODE_PRIVATE = 8,
};


/*  _memory_mapped_file:
 *    represents an entry in /proc/<id>/maps referencing a file mapped into
 *    the process' memory space.
 */
typedef struct _memory_mapped_file
{
  struct _memory_mapped_file *prev, *next;  /* prev and next entries */
  void *start_addr, *end_addr;      /* virtual memory start and end addrs */
  unsigned int offset;              /* offset relative to start_addr indicating
                                     * the start of the mapped file */
  uint8_t mode;                     /* bitflags denoting the file permissions */
  uint8_t dev_major, dev_minor;     /* device major id (driver id) and minor id
                                     * (driver-specific device id) on which
                                     * the file was loaded */
  int inode;                        /* inode on the fs from which the file was
                                     * loaded */
  char *fpath;                      /* name of the mapped file */
} ll_memmap_file;


/*  _process_memory_wrapper
 *
 *
 *
 *
 */
typedef struct _process_memory_pointer
{
  FILE *fp;
  void *pointer;

} mem_;


/* function definitions */
ll_memmap_file* parse_proc_maps(int);
void free_proc_maps(ll_memmap_file*);

#endif /* __MEM_H */
