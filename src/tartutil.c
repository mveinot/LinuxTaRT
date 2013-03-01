/* file: tartutil.c
 * ----------------
 * This is an include file containing usefull,
 * mostly TaRT related, routines developed during LinuxTaRT
 */

#include "config.h"

#ifdef __riscos__
#include "riscos.h"
#include "nocolors.h"
#else
#ifdef NO_COLOR
#include "nocolors.h"
#else
#include "colors.h"
#endif
#endif
#include "tartutil.h"
#include "stringutil.h"
#include "IniFile.h"
#include "opt.h"

/* procedure: AddLine
 * usage: AddLine();
 * ---------------------
 * This procedure requests a line from the user and appends
 * it to the tagline file, then terminates program execution.
 */

void AddLine (char tagfile[])
{
	FILE *input;
	char tmp[256];
	char ptmp[256];

        memset(ptmp, '-', TAGLEN - 37);
        ptmp[TAGLEN - 37] = '\0';
			
	printf (YELLOW "\nEnter the new tagline (%2d chars max): %s|\n" WHITE "> ", TAGLEN, ptmp);
	printf (NORMAL);
	if (fgets (tmp, TAGLEN +1, stdin) != NULL)
        if (strlen(tmp) == TAGLEN) strcat(tmp,"\n");

	if ((input = fopen (tagfile, "a")) == NULL) {
		fprintf (stderr, RED "error opening tagline database -- you may not have permission to modify it.\n");
		fprintf (stderr, RED "Consider copying" GREEN " %s " RED "to your home directory and using it from there.\n\n" NORMAL, tagfile);
		exit (1);
	}
	fputs (tmp, input);
	if (fclose (input)) {
		fprintf (stderr, RED "error closing tagline database\n" NORMAL);
		exit (1);
	}
	exit (0);
}

/* function: CountTagLines
 * usage: i = CountTagLines(filename);
 * -----------------------------------
 * This function returns the number of lines in the text file
 * specified by 'filename'. The end of the line is determined
 * by the newline character '\n'. (Code borrowed from an
 * implementation of the unix textutils 'wc' command.)
 */

int CountTagLines (char filename[])
{
	FILE *readfile;
	int new_lines = 0;		/* number of lines terminated by a newline */
	int car;

	if ((readfile = fopen (filename, "r")) == NULL)
		return (-1);
	for (;;) {			/* forever do... */
		car = getc (readfile);
		if (car <= EOF)		/* if end of file encountered, */
			break;		/* stop the loop. */

		if (car == '\n')	/* if end of line encountered, */
			new_lines++;	/* increment a counter. */

	}
	if (fclose (readfile))
		return (-1);
	return (new_lines);
}

/* function DisplaySignature
 * usage: i = DisplaySignature(filename);
 * --------------------------------------
 * This function is basically a equivalent to the shell
 * command 'cat'. It reads lines from the file specified
 * in 'filename' until the end of the file is reached.
 * Used here, it displays the signature to the screen.
 * It returns 1 if an error occured, 0 otherwise.
 */

int DisplaySignature (char Filename[])
{
	FILE *readfile;
	char tagline[256];

	if ((readfile = fopen (Filename, "r")) == NULL)
		return (1);
	while (!feof (readfile)) {
		if (fgets (tagline, 80, readfile) != NULL)
		{
			printf ("%s", tagline);
			strcpy (tagline, "");
		}
	}
	if (fclose (readfile))
		return (1);
	return (0);
}

/* function: GetTagLine
 * usage: i = GetTagLine(filename, tagline, number);
 * -------------------------------------------------
 * This function reads a plain-text file specified in
 * 'filename' and stores the line number 'number' in
 * string tagline[]. If a line in 'datefile' matches
 * the date in shortDate (and special is not 0) then
 * the test from that line is stored in tagline[].
 * It returns 1 if an error occurs, otherwise 0.
 */

int GetTagLine (char filename[], char tagline[], int number, char shortDate[], char datefile[], int special)
{
	FILE *readfile;
	char tmp[4096];
	char tmpDate[6];
	int i, c;

	if (special && (readfile = fopen (datefile, "r")) != NULL) {
		while (!feof (readfile)) {
			if (fgets (tmp, 4095, readfile) == NULL)
			{
				printf("error reading datefile\n");
				exit(1);
			}
			StripCR (tmp);
			sscanf (tmp, "%5s:%[^#]", tmpDate, tagline);	/*  parse each line into two variables */
			strcat (tagline, "\n");		/*  allows use of # to denote comments */
			if (strncmp (shortDate, tmpDate, 5) == 0)
				return (0);
		}
	}
	if ((readfile = fopen (filename, "r")) == NULL)
		return (1);
	for (i = 0; i <= number; i++) {
		if (fgets (tagline, 4095, readfile) == NULL)
		{
			printf ("error reading datefile\n");
			exit(1);
		}
		/* Use \n in a line in the tagline file for multilple lines. BWH */
		for (c = 0; tagline[c] != '\0'; c++){
			if (tagline[c] == '\\' && tagline[c+1] == 'n'){
				tagline[c]=' '; /* Adds an unnecessary space, but is fast */
				tagline[c+1]='\n';
			}
		}
	}
	if (fclose (readfile))
		return (1);
	return (0);
}


/* function: ReadINIFile
 * usage: i = ReadINIFile(custom1, custom2, customfile, sigfile, tagfile, config);
 * -------------------------
 * This function reads the file .tartrc and stores initialization
 * settings in the provided variables.
 * .tartrc is assumed to be in the users home directory.
 * custom1 and custom2 are strings of length TAGLEN (maximum).
 * customfile, sigfile and tagfile are strings of length 256 (maximum).
 * config is a pointer to char malloc'd by sscanf
 */

int ReadINIFile (char custom1[], char custom2[], char customfile[], char sigfile[], char tagfile[], char datefile[], int optarray[], char *config)
{
	char PathToINI[256];

	if (config == NULL)
	{
#ifdef __riscos__
        strncpy (PathToINI, "Choices:TaRT.rc", 90);
#else
		strncpy (PathToINI, getenv ("HOME"), 90);	/*  find user's homedir */
		strncat (PathToINI, "/.tartrc", 165);
#endif
	} else
	{
#ifdef __riscos__
        strncpy (PathToINI, "Choices:", 90);
		strcat (PathToINI, config, 165);
#else
		strncpy (PathToINI, config, 255);
#endif
	}

	if (InitIni(PathToINI) == 0)
		return 1;

        GetString("CustomText1", custom1, "");
        GetString("CustomText2", custom2, "");
	GetString("SignatureFile", sigfile, "");
	GetString("TagLineDatabase", tagfile, "");
	GetString("SpecialDates", datefile, "");
	GetString("CustomFile", customfile, "");
	optarray[quiet] = GetInt("RunQuiet", 0);
	optarray[date] = GetInt("ShowDate", 1);
	optarray[version] = GetInt("ShowVersion", 1);
	optarray[center] = GetInt("CenterText", 0);
	optarray[custom] = GetInt("UseCustomLayout", 1);
	optarray[special] = GetInt("UseSpecialDates", 1);
	optarray[asdaemon] = GetInt("RunAsDaemon", 0);

	if (CloseIni() == 0)
		return 1;

	StripCR (customfile);
	StripCR (tagfile);
	StripCR (sigfile);
	StripCR (datefile);
	TildeParse (sigfile);
	TildeParse (customfile);
	TildeParse (datefile);
	TildeParse (tagfile);

	return (0);
}

/* function: Running
 * usage: i = Running(config);
 * ---------------------
 * This function returns TaRT's PID if "Running" is set in the users .tartrc
 * file, and 0 otherwise. Use to determine whether TaRT is already running
 * or not.
 * config is a pointer to char malloc'd by sscanf
 */

int Running (char *config)
{
    char PathToINI[256];

	if (config == NULL)
	{
#ifdef __riscos__
        strncpy (PathToINI, "Choices:TaRT.rc", 90);
#else
		strncpy (PathToINI, getenv ("HOME"), 90);	/*  find user's homedir */
		strncat (PathToINI, "/.tartrc", 165);
#endif
	} else
	{
#ifdef __riscos__
        strncpy (PathToINI, "Choices:", 90);
		strcat (PathToINI, config, 165);
#else
		strncpy (PathToINI, config, 255);
#endif
	}

    if (InitIni(PathToINI) == 0)
    	return -1;
	return GetInt("Running", 0);
	if (CloseIni() == 0)
		return -1;
}

/* function: SetRunning
 * usage: i = SetRunning(state, config);
 * ------------------------
 * This function places an entry in the users .tartrc file and sets the value
 * to 'state'. This is used to determine whether TaRT is running or not.
 * Returns -1 on error, 0 on success;
 * config is a pointer to char malloc'd by sscanf
 */

int SetRunning (int state, char *config)
{
    char PathToINI[256];

	if (config == NULL)
	{
#ifdef __riscos__
        strncpy (PathToINI, "Choices$Write:TaRT.rc", 90);
#else
		strncpy (PathToINI, getenv ("HOME"), 90);	/*  find user's homedir */
		strncat (PathToINI, "/.tartrc", 165);
#endif
	} else
	{
#ifdef __riscos__
        strncpy (PathToINI, "Choices$Write:", 90);
		strcat (PathToINI, config, 165);
#else
		strncpy (PathToINI, config, 255);
#endif
	}

    if (InitIni(PathToINI) == 0)
	    return -1;

    SetInt("Running", state);

	return 0;
}


/* function: WriteINIFile
 * usage: i = WriteINIFile(custom1, custom2, customfile, sigfile, tagfile, config);
 * -------------------------
 * This function writes the file .tartrc from the provided variables.
 * .tartrc is assumed to be in the users home directory.
 * custom1 and custom2 are strings of length TAGLEN (maximum).
 * customfile, sigfile and tagfile are strings of length 256 (maximum).
 * config is a pointer to char malloc'd by sscanf
 */

int WriteINIFile (char custom1[], char custom2[], char customfile[], char sigfile[], char tagfile[], char datefile[], int optarray[], char *config)
{
	char PathToINI[256];

	TildeParse (sigfile);
	TildeParse (customfile);
	TildeParse (datefile);
	TildeParse (tagfile);
	StripCR (custom1);
	StripCR (custom2);
	StripCR (sigfile);
	StripCR (tagfile);
	StripCR (datefile);
	StripCR (customfile);

	if (config == NULL)
	{
#ifdef __riscos__
        strncpy (PathToINI, "Choices$Write:TaRT.rc", 90);
#else
		strncpy (PathToINI, getenv ("HOME"), 90);	/*  find user's homedir */
		strncat (PathToINI, "/.tartrc", 165);
#endif
	} else
	{
#ifdef __riscos__
        strncpy (PathToINI, "Choices$Write:", 90);
		strcat (PathToINI, config, 165);
#else
		strncpy (PathToINI, config, 255);
#endif
	}

	if (InitIni(PathToINI) == 0)
		return 1;

	SetString("CustomText1", custom1);
	SetString("CustomText2", custom2);
	SetString("SignatureFile", sigfile);
	SetString("TagLineDatabase", tagfile);
	SetString("SpecialDates", datefile);
	SetString("CustomFile", customfile);
	SetInt("RunQuiet", optarray[quiet]);
	SetInt("ShowDate", optarray[date]);
	SetInt("ShowVersion", optarray[version]);
        SetInt("CenterText", optarray[center]);
        SetInt("UseCustomText", optarray[custom]);
        SetInt("UseSpecialDates", optarray[special]);
	SetInt("RunAsDaemon", optarray[asdaemon]);

	if (CloseIni() == 0)
		return 1;

	return (0);
}

/* function: SelectRandom
 * usage: i = SelectRandom(low, high);
 * -----------------------------------
 * This function returns a psuedo-random number between
 * 'low' and 'high'. Care has been taken to ensure that
 * the 'random' numbers are, in fact reasonably random
 * from execution to execution.
 */

int SelectRandom (int low, int high)
{
	int r;

	r = rand ();		/*  get a random number from the generator (0 -> RAND_MAX) */
	r = (r % (high - low)) + low;	/*  change to range of the number to (low -> high) */

	return (r);
}

/* function: TildeParse
 * usage: TildeParse(string);
 * --------------------------
 * This function will replace the leading ~ character (if present)
 * with the current users home directory path.
 * It returns the length of the new string.
 */

int TildeParse (char *sigfile)
{
	char tmp[256];
	int tmppos = 0, i;

	strncpy (tmp, getenv ("HOME"), 90);

	if (sigfile[0] == '~') {
		tmppos = strlen (tmp);
		for (i = 1; i < strlen (sigfile); i++)
			tmp[tmppos++] = sigfile[i];
		tmp[tmppos] = '\0';
		strncpy (sigfile, tmp, 255);
	}
	return (strlen (sigfile));
}

/* function: FirstINIFile
 * usage: i = FirstINIFile(custom1, custom2, customfile, sigfile, tagfile, datefile, optarray);
 * -------------------------
 * This file creates a .tartrc file if one is not found when TaRT is ran.
 */

int FirstINIFile (char custom1[], char custom2[], char customfile[], char sigfile[], char tagfile[], char datefile[], int optarray[]
)
{
        char PathToINI[256];
	FILE* IniFile;

        TildeParse (sigfile);
        TildeParse (customfile);
        TildeParse (datefile);
        TildeParse (tagfile);
        StripCR (custom1);
        StripCR (custom2);
        StripCR (sigfile);
        StripCR (tagfile);
        StripCR (datefile);
        StripCR (customfile);

#ifdef __riscos__
        strncpy(PathToINI, "<Choices$Write>.TaRT.rc",90);
#else
        strncpy (PathToINI, getenv ("HOME"), 90);       /*  find user's homedir */
        strcat (PathToINI, "/.tartrc");
#endif

	IniFile = fopen(PathToINI, "w");
	if (IniFile == NULL)
		return (1);

        fprintf(IniFile, "CustomText1=%s\n", custom1);
        fprintf(IniFile, "CustomText2=%s\n", custom2);
        fprintf(IniFile, "SignatureFile=%s\n", sigfile);
        fprintf(IniFile, "TagLineDatabase=%s\n", tagfile);
        fprintf(IniFile, "SpecialDates=%s\n", datefile);
        fprintf(IniFile, "CustomFile=%s\n", customfile);
        fprintf(IniFile, "RunQuiet=%d\n", optarray[quiet]);
        fprintf(IniFile, "ShowDate=%d\n", optarray[date]);
        fprintf(IniFile, "ShowVersion=%d\n", optarray[version]);
        fprintf(IniFile, "CenterText=%d\n", optarray[center]);
        fprintf(IniFile, "UseCustomText=%d\n", optarray[custom]);
        fprintf(IniFile, "UseSpecialDates=%d\n", optarray[special]);
        fprintf(IniFile, "RunAsDaemon=%d\n", optarray[asdaemon]);

        if (fclose(IniFile) != 0)
                return 1;

        return (0);
}

