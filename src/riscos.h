/*
 * risc os hack to get fflush happening.
 */

#ifndef _RISCOS_H
#define _RISCOS_H

#include <stdio.h>

#undef fgets
#define fgets(x,y,z) ro_fgets(x,y,z)

char *ro_fgets(char *str, int n, FILE *stream);

#endif	/* _RISCOS_H */

