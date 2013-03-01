/* //////////////////////////////////////////////////////////////// */
/*  This is a library to allow the integration of ini-like files // */
/*  In C programs. It is important to call InitIni(filename);    // */
/*  and CloseIni; before and after use respectively.             // */
/* //////////////////////////////////////////////////////////////// */

#include "IniFile.h"

int RetrieveValue(const char* tmp, char* tmpvalue);
int filecopy(const char* fromfile, const char* tofile);

static FILE* IniFile;
static char* IniFileName;

/* //////////////////////////////////////////////////////////////// */
/*  This function initializes the IniFile interface, opening the // */
/*  passed filename, and attaching it to IniFile                 // */
/* //////////////////////////////////////////////////////////////// */
int InitIni(const char* filename)
{
	IniFileName = malloc(sizeof(char) * strlen(filename)+1);
	if (IniFileName == NULL)
		return 0;
	strcpy(IniFileName, filename);
	IniFile = fopen(filename, "r");
	if (IniFile == NULL)
		return 0;
	return 1;
}

/* //////////////////////////////////////////////////////////////// */
/*  This function is called to close the INI file stream         // */
/* //////////////////////////////////////////////////////////////// */
int CloseIni(void)
{
	free(IniFileName);
	if (fclose(IniFile) == 0)
		return 1;
	return 0;
}

/* //////////////////////////////////////////////////////////////// */
/*  This function returns an integer found after key=            // */
/* //////////////////////////////////////////////////////////////// */
int GetInt(const char* key, int def)
{
	char tmp[ENTRYSIZE+1];
	char store[ENTRYSIZE+1];
	int i = 0;

	rewind(IniFile);


	while (feof (IniFile) == 0) {
		tmp[0] = '\0';
		if (fgets (tmp, ENTRYSIZE, IniFile) != NULL)
		{
			if (tmp[0] == '#')
				continue;
			if (strncmp(tmp, key, strlen(key)) == 0) {
				sscanf(tmp, "%[^=]=%d", store, &i);
				return i;
			}
		}
	}
	return def;
}

/* //////////////////////////////////////////////////////////////// */
/*  This function creates a new integer key with key name (key)  // */
/*  and value (value). returns 1 on success, 0 otherwise         // */
/* ////////////////////////////////////////////////////////////////			 */
int SetInt(const char* key, int value)
{
	FILE* tempfile;
	char tempfilename[256];
	char tmp[ENTRYSIZE+1];
	char keyequal[ENTRYSIZE+1];
	int set = 0;
	int filedes;

	strcpy(tempfilename, "tarttempXXXXXX");
	filedes = mkstemp(tempfilename);
	if (filedes == -1)
		return 0;
	tempfile = fdopen(filedes, "r+");

	rewind(IniFile);

	while (feof (IniFile) == 0) {
		tmp[0] = '\0';
		if (fgets (tmp, ENTRYSIZE, IniFile) != NULL)
		{
			if (tmp[0] == '#') {
				fputs(tmp, tempfile);
				continue;
			}
		}
		sprintf(keyequal, "%s=", key);
		if (strncmp(tmp, keyequal, strlen(key)+1) == 0) {
			fprintf(tempfile, "%s=%d\n", key, value);
			set = 1;
			continue;
		}
		if (isalnum((int)tmp[0]))
			fputs(tmp, tempfile);
	}
	if (!set)
		fprintf(tempfile, "%s=%d\n", key, value);

	if (fclose(tempfile) != 0)
		return 0;
       	if (fclose(IniFile) != 0)
		return 0;
	if (unlink(IniFileName) != 0)
		return 0;
	if (filecopy(tempfilename, IniFileName) == 0)
		return 0;
	IniFile = fopen(IniFileName, "r");
	if (IniFile == NULL)
		return 0;
	if (unlink(tempfilename) != 0)
		return 0;
	return 1;
}

/* //////////////////////////////////////////////////////////////// */
/*  This function returns a double found after key=              // */
/* //////////////////////////////////////////////////////////////// */
double GetFloat(const char* key, double def)
{
	char tmp[ENTRYSIZE+1];
	char store[ENTRYSIZE+1];
	double f = 0.0;

	rewind(IniFile);

	while (feof (IniFile) == 0) {
		tmp[0] = '\0';
		if (fgets (tmp, ENTRYSIZE, IniFile) != NULL)
		{
			if (tmp[0] == '#')
				continue;
			if (strncmp(tmp, key, strlen(key)) == 0) {
				sscanf(tmp, "%[^=]=%lf", store, &f);
				return f;
			}
		}
	}
	return def;
}

/* //////////////////////////////////////////////////////////////// */
/*  This function creates a new boolean key with key name (key)  // */
/*  and value (value). returns 1 on success, 0 otherwise         // */
/* //////////////////////////////////////////////////////////////// */
int SetBool(const char* key, bool value)
{
	return SetInt(key, value);
}

/* //////////////////////////////////////////////////////////////// */
/*  This function returns a boolean found after key=		// */
/* //////////////////////////////////////////////////////////////// */
bool GetBool(const char* key, bool def)
{
	return GetInt(key, def);
}

/* //////////////////////////////////////////////////////////////// */
/*  This function creates a new double key with key name (key)   // */
/*  and value (value). returns 1 on success, 0 otherwise         // */
/* ////////////////////////////////////////////////////////////////			 */
int SetFloat(const char* key, double value)
{
	FILE* tempfile;
	char tempfilename[256];
	char tmp[ENTRYSIZE+1];
	char keyequal[ENTRYSIZE+1];
	int set = 0;
	int filedes;

        strcpy(tempfilename, "tarttempXXXXXX");
        filedes = mkstemp(tempfilename);
        if (filedes == -1)
		return 0;
	tempfile = fdopen(filedes, "r+");

	rewind(IniFile);
	while (feof (IniFile) == 0) {
		tmp[0] = '\0';
		if (fgets (tmp, ENTRYSIZE, IniFile) != NULL)
		{
			if (tmp[0] == '#') {
				fputs(tmp, tempfile);
				continue;
			}
		}
		sprintf(keyequal, "%s=", key);
		if (strncmp(tmp, keyequal, strlen(key)+1) == 0) {
			fprintf(tempfile, "%s=%f\n", key, value);
			set = 1;
			continue;
		}
		if (isalnum((int)tmp[0]))
			fputs(tmp, tempfile);
	}
	if (!set)
		fprintf(tempfile, "%s=%f\n", key, value);

	if (fclose(tempfile) != 0)
		return 0;
       	if (fclose(IniFile) != 0)
		return 0;
	if (unlink(IniFileName) != 0)
		return 0;
	if (filecopy(tempfilename, IniFileName) == 0)
		return 0;
	IniFile = fopen(IniFileName, "r");
	if (IniFile == NULL)
		return 0;
	if (unlink(tempfilename) != 0)
		return 0;
	return 1;
}

/* //////////////////////////////////////////////////////////////// */
/*  This function retrieves the value associated with key storing// */
/*  it in value and returning it. If the named key is not found, // */
/*  a default (def) is returned.                                 // */
/* //////////////////////////////////////////////////////////////// */
char* GetString(const char* key, char* value, const char* def)
{
	char tmp[ENTRYSIZE+1];
	char tmpvalue[ENTRYSIZE+1];
	int length = 0;

	rewind(IniFile);

	while (feof (IniFile) == 0) {
		tmp[0] = '\0';
		if (fgets (tmp, ENTRYSIZE, IniFile) != NULL)
		{
			if (tmp[0] == '#')
				continue;
			if (strncmp(tmp, key, strlen(key)) == 0) {
				length = RetrieveValue(tmp, tmpvalue);
				strncpy(value, tmpvalue, length);
				value[length] = '\0';
				return value;
			}
		}
	}
	strcpy(value, def);
	return value;
}

/* //////////////////////////////////////////////////////////////// */
/*  This function is used to set an existing key or create a new // */
/*  one. returns 1 on success, 0 otherwise.                      // */
/* //////////////////////////////////////////////////////////////// */
int SetString(const char* key, char* value)
{
	FILE* tempfile;
	char tempfilename[256];
	char tmp[ENTRYSIZE+1];
	char keyequal[ENTRYSIZE+1];
	int set = 0;
	int filedes;

        strcpy(tempfilename, "tarttempXXXXXX");
        filedes = mkstemp(tempfilename);
        if (filedes == -1)
		return 0;
	tempfile = fdopen(filedes, "r+");

	rewind(IniFile);

	while (feof (IniFile) != 0) {
		tmp[0] = '\0';
		if (fgets (tmp, ENTRYSIZE, IniFile) != NULL)
		{
			if (tmp[0] == '#') {
				fputs(tmp, tempfile);
				continue;
			}
		}
		sprintf(keyequal, "%s=", key);
		if (strncmp(tmp, keyequal, strlen(key)+1) == 0) {
			fprintf(tempfile, "%s=%s\n", key, value);
			set = 1;
			continue;
		}
		if (isalnum((int)tmp[0]))
			fputs(tmp, tempfile);
	}

	if (!set)
		fprintf(tempfile, "%s=%s\n", key, value);

	if (fclose(tempfile) != 0)
		return 0;
       	if (fclose(IniFile) != 0)
		return 0;
	if (unlink(IniFileName) != 0)
		return 0;
	if (filecopy(tempfilename, IniFileName) == 0)
		return 0;
	IniFile = fopen(IniFileName, "r");
	if (IniFile == NULL)
		return 0;
	if (unlink(tempfilename) != 0)
		return 0;
	return 1;
}

/* //////////////////////////////////////////////////////////////// */
/*  This function copies one file to another in an efficient way // */
/* //////////////////////////////////////////////////////////////// */
int filecopy (const char* fromfile, const char* tofile)
{
	char buf[BUFSIZE];
	register int i;
	int fdin, fdout;		/* file descriptors */

	fdin = open (fromfile, O_RDONLY);
	fdout = open (tofile, O_CREAT | O_WRONLY, 0644);

	if (fdin == -1 || fdout == -1)
		return 0;
	while ((i = read (fdin, buf, BUFSIZE)))
		if (i != write (fdout, buf, i))
			return 0;
	close (fdin);
	close (fdout);
	return 1;
}

/* //////////////////////////////////////////////////////////////// */
/*  This function returns the string found after an equal sign   // */
/*  returning it in tmpvalue - tmpvalue must be large enough to  // */
/*  hold the string.						// */
/* ////////////////////////////////////////////////////////////////	 */
int RetrieveValue(const char* tmp, char* tmpvalue)
{
	int i, j = 0;

	for (i = 0; i < strlen(tmp); i++)
		if (tmp[i] == '=')
			break;
	for (i++; i < strlen(tmp) - 1; i++)
		tmpvalue[j++] = tmp[i];
	return j;
}
