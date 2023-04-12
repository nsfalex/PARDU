#include "util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*  nreadline:
 *    copies characters into buf until it reaches a newline or the characters
 *    read exceed bufsize. returns how many characters were copied.
 *
 *    char *buf:    buffer to place characters in
 *    int bufsize:  size of the buffer in characters
 *    FILE *f:      file handle to read from
 */
int
nreadline(char *buf, int bufsize, FILE *f)
{
  int index;
  char c;

  index = 0;
  while((c = getc(f)) != '\n' && c != EOF)
  {
    if (index >= bufsize-1)
      break;

    buf[index++] = c;
  }

  buf[index] = '\0';
  return index;
}


/*  scharpos:
 *    returns the index of the first occurrence of a given character in
 *    a character string. returns -1 if the character was not found.
 *
 *    const char *s:    string to search
 *    char c:           character to find
 */
int
scharpos(const char *s, char c)
{
  int i, len;
  
  len = strlen(s);
  for (i = 0; i <= len; i++)
  {
    if (s[i] == c)
      return i;
  }
  
  return -1;
}
