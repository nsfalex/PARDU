#include "mem.h"
#include "util.h"

#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <sys/stat.h>

#include <linux/limits.h>

#define MAPS_LINE_LEN PATH_MAX+75   /* 75 chars for info + file path         */
#define MAPS_FIELDS_NO 9            /* 8 fields in /proc/<id>/maps + match 0 */





/*  free_proc_maps:
 *    frees the linked list of /proc/<id>/map file entries
 *
 *    ll_memmap_file *first:   first entry in the linked list
 */
void
free_proc_maps(ll_memmap_file *first)
{
  ll_memmap_file *next,
                 *mmf = first;

  do {
    next = mmf->next;
    free(mmf);
    mmf = next;
  }
  while (mmf != NULL);
}


/*  _parse_proc_map_line:
 *    parses and converts values from a line in /proc/<pid>/maps and populates
 *    the given ll_memmap_file with them.
 *
 *    char *s:    line from /proc/<pid>/maps
 *    
 */
static void
_parse_proc_map_line(char *s, regex_t *re, ll_memmap_file *mmf)
{
  int len, i, m;
  char parse_str[PATH_MAX+1];
  regmatch_t matches[MAPS_FIELDS_NO];

  if (regexec(re, s, MAPS_FIELDS_NO, matches, 0) == REG_NOMATCH)
    return;

  /* skip match 0 as it matches the entire line */
  for (i = 1; i < MAPS_FIELDS_NO; i++)
  {
    /* if there's no more match groups left */
    if (matches[i].rm_so == (size_t) -1)
      break;

    /* copy chars from the start of the match till the end of the match */
    memset(parse_str, '\0', PATH_MAX+1);
    strncpy(parse_str,
            s + matches[i].rm_so,
            matches[i].rm_eo - matches[i].rm_so);

    switch (i) {
      /* TODO: multi-arch pointer support, long long = 64bit */
      case 1:
        mmf->start_addr = (void *) strtoull(parse_str, NULL, 16);
        break;

      case 2:
        mmf->end_addr = (void *) strtoull(parse_str, NULL, 16);
        break;

      case 3:
        for (m = 0; m < 4; m++)
        {
          switch (s[m])
          {
            case 'r':
              mmf->mode |= MODE_READ;
              break;

            case 'w':
              mmf->mode |= MODE_WRITE;
              break;

            case 'x':
              mmf->mode |= MODE_EXECUTE;
              break;

            case 'p':
              mmf->mode |= MODE_PRIVATE;
              break;
          }
        }
        break;

      case 4:
        mmf->offset = (unsigned int) strtoull(parse_str, NULL, 16);
        break;

      case 5:
        mmf->dev_major = (uint8_t) strtoul(parse_str, NULL, 16);
        break;

      case 6:
        mmf->dev_minor = (uint8_t) strtoul(parse_str, NULL, 16);
        break;

      case 7:
        /* TODO: make sure int will hold the full size of inode */
        mmf->inode = atoi(parse_str);
        break;

      case 8:
        len = strlen(parse_str);
        mmf->fpath = malloc(len+1);
        memset(mmf->fpath, '\0', len+1);
        strncpy(mmf->fpath, parse_str, len);
        break;
    }
  }
}


/*  parse_proc_maps:
 *    parses /proc/<pid>/maps to extract a process' currently mapped memory
 *    files, and returns a pointer to the first entry in a linked list
 *    describing these files.
 *
 *    int pid:  process id to parse mapped files from
 */
ll_memmap_file*
parse_proc_maps(int pid)
{
  int i, m, len;
  FILE *f;
  char line[MAPS_LINE_LEN];

  ll_memmap_file *first, *prev, *mmf;

  regex_t re_comp;

  /* [start]-[end] "mode" [offset] <maj>:<min> <inode> "name" */
  char *re_str =
    "^([[:xdigit:]]+)-([[:xdigit:]]+) ([rwxp-]+) ([[:xdigit:]]+) "
    "([[:xdigit:]]+):([[:xdigit:]]+) ([[:digit:]]+)[[:space:]]*(.*)$";

  if (regcomp(&re_comp, re_str, REG_EXTENDED))
    die(1, "parse_proc_maps: failed to compile regex");

  /* parse /proc/<id>/maps to discover all currently loaded modules */
  sprintf(line, "/proc/%d/maps", pid);

  f = fopen(line, "r");
  if (f == NULL)
    die(1, "Failed to open process memory map");

  first = NULL;
  prev = NULL;

  /* make sure we're not reading an empty list */
  i = getc(f);
  if (i == EOF)
    goto parse_maps_end;

  ungetc(i, f);

  while (nreadline(line, MAPS_LINE_LEN, f) > 0)
  {
    mmf = malloc(sizeof(ll_memmap_file));
    /* init the first entry in the linked list */
    if (!first)
      first = mmf;

    _parse_proc_map_line(line, &re_comp, mmf);

    /* update the previous linked token's pointers */
    if (prev)
      prev->next = mmf;

    mmf->prev = prev;
    prev = mmf;
  }

  /* we've reached the end of the linked list, indicate with a NULL */
  mmf->next = NULL;

parse_maps_end:
  regfree(&re_comp);
  fclose(f);

  return first;
}

