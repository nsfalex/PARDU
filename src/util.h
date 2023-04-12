#ifndef __UTIL_H
#define __UTIL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/* shared functions */

/* error functions */

/*  die:
 *    prints an error using perror and then exits with status as the
 *    return code.
 *
 *    int status:   status code to exit with
 *    char *msg:    message to print (pass to perror)
 */
inline void
die(int status, char *msg)
{
  perror(msg);
  exit(status);
}


/* file read helper functions */
int nreadline(char *, int, FILE*);

/* string helper functions */
int scharpos(const char*, char);


#endif /* __UTIL_H */