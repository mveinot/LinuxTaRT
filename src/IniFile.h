/* //////////////////////////////////////////////////////////////// */
/*  This is a library to allow the integration of ini-like files // */
/*  In C programs. It is important to call InitIni(filename);    // */
/*  and CloseIni(); before and after use respectively.           // */
/* //////////////////////////////////////////////////////////////// */

#ifndef _INI_FILE
#define _INI_FILE

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFSIZE 1024
#define ENTRYSIZE 1024

typedef enum {
	FALSE, TRUE
} bool;

int InitIni(const char* filename);
int CloseIni(void);
int GetInt(const char* key, int def);
int SetInt(const char* key, int value);
int SetBool(const char* key, bool value);
bool GetBool(const char* key, bool def);
double GetFloat(const char* key, double def);
int SetFloat(const char* key, double value);
char* GetString(const char* key, char* value, const char* def);
int SetString(const char* key, char* value);

#endif	/* _INI_FILE */

