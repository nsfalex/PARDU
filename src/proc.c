#include "proc.h"
#include "util.h"

#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include <linux/limits.h>


#define PATHLEN PATH_MAX+1


/*  validate_proc_dir:
 *    validates if the directory entry in proc is a valid candidate to check
 *    out.
 *
 *    int dirfd:    open fd to the /proc directory
 *    char *entry:  name of the file to be validated
 */
static int
validate_proc_dir(int fd, char *entry)
{
  struct stat st;
  regex_t re_numeric;

  if (regcomp(&re_numeric, "^[0-9]*$", 0))
    die(1, "validate_proc_dir; couldn't compile regex");
  
  /* if the entity name isn't exclusively numeric */
  if (regexec(&re_numeric, entry, 0, NULL, 0) == REG_NOMATCH)
    return false;

  /* if we can't get file status */
  if (fstatat(fd, entry, &st, 0) < 0) {
    perror(entry);
    return false;
  }
  
  /* if the file isn't a directory */
  else if (!S_ISDIR(st.st_mode))
    return false;

  /* if d_name is "." or ".." */
  else if (strcmp(entry, ".") == 0 || strcmp(entry, "..") == 0 ) 
    return false;

  /* if d_name is "self" */
  else if (strcmp(entry, "self") == 0 || strcmp(entry, "thread_self") == 0)
    return false;

  return true;
}


/*  lookup_pid:
 *    returns the first process id (pid) matching proc_name, or -1 on failure
 *    TODO: return the last procid
 */
int
lookup_pid (char *proc_name)
{
  int pid;
  DIR *proc_dirp;
  char *exe_name,
       link_path[PATHLEN], /* TODO: can limit to max amount of pids instead */
       exe_path[PATHLEN];
  struct dirent *entry;

  proc_dirp = opendir((char *) "/proc");
  if (proc_dirp == NULL)
    return -1;

  while ((entry = readdir(proc_dirp)) != NULL)
  {
    /* if the file isn't what we're looking for */
    if (!validate_proc_dir(dirfd(proc_dirp), entry->d_name))
      continue;

    /* try to get the exe file path from the proc entry */
    snprintf(link_path, PATHLEN, "/proc/%s/exe", entry->d_name);
    
    memset(exe_path, '\0', PATHLEN);  /* zero out exe_path because readlink
                                       * doesn't terminate it correctly */
    if (readlink(link_path, exe_path, PATHLEN) == -1)
      continue;
    
    exe_name = basename(exe_path);
    
    /* TODO: fuzzy match */
    if (strcmp(exe_name, proc_name) == 0 || strcmp(exe_path, proc_name) == 0) {
      pid = atoi(entry->d_name);
      closedir(proc_dirp);
      return pid;
    }
  }

  closedir(proc_dirp);
  return -1;
}
