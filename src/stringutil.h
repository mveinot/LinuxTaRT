/* file: stringutil.h
 * ------------------
 * This is stringutil.c's corresponding
 * header file.
 */

#ifndef _STRINGUTIL_H
#define _STRINGUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void CenterText (char text[], int width);
int HLine (int length);
int IsSwitch (char *list[], int size, char *arg);
void StripCR (char *string);

#endif	/* _STRINGUTIL_H */

