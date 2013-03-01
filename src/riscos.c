/*
 * risc os hack to get fflush happening.
 */

#include <stdio.h>

char *ro_fgets(char *str, int n, FILE *stream)
{
  if (stream==stdin)
    fflush(stdout);
  return fgets(str,n,stream);
}
