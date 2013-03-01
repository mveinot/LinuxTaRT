/* file: tartutil.h
 * ----------------
 * This is tartutil.c's corresponding header file
 */

#ifdef __riscos__
#include "riscos.h"
#endif

#ifndef _TARTUTIL_H
#define _TARTUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void AddLine (char tagfile[]);
int CountTagLines (char filename[]);
int DisplaySignature (char Filename[]);
int GetTagLine (char filename[], char tagline[], int number, char shortDate[], char datefile[], int special);
int ReadINIFile (char custom1[], char custom2[], char customfile[], char sigfile[], char tagfile[], char datefile[], int optarray[], char *config);
int WriteINIFile (char custom1[], char custom2[], char customfile[], char sigfile[], char tagfile[], char datefile[], int optarray[], char *config);
int Running (char *config);
int SetRunning (int state, char *config);
int SelectRandom (int low, int high);
int TildeParse (char *sigfile);
int FirstINIFile (char custom1[], char custom2[], char customfile[], char sigfile[], char tagfile[], char datefile[], int optarray[]);

#endif	/* _TARTUTIL_H */

