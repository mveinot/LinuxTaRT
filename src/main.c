/* TaRT - Utility to generate taglines automatically
   Copyright (C) 2002  Mark Veinot

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* file: main.c
 * ------------
 * Programming and concept: Mark Veinot
 * Creates custom e-mail signatures
 * --------------------------------
 *
 * Feel free to e-mail me at: mveinot@gmail.com
 * with any questions, comments, suggestions, or (yes, even)
 * complaints.
 */

#include "config.h"

#ifdef __riscos__
#include "riscos.h"
#endif

#ifdef __linux__
// todo: We get a warning about using kernel headers in userspace here.
// ideally there is a way to get this information without using a kernel
// header and also without just reading /proc/uptime
#include <sys/sysinfo.h>
#endif

#ifdef NO_COLOR
#include "nocolors.h"
#else
#include "colors.h"
#endif

#ifdef IBEATS
#include "ibeat.h"
#endif

#include "tartutil.h"
#include "stringutil.h"
#include "opt.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#define HLINE "----------------------------------------------------------------"

/* global variables */

char custom1[256], custom2[256], tagfile[256], sigfile[256], customfile[256], datefile[256];
int optarray[10];
char custparam[4096];
char custconfig[4096];

/* function declarations */

void Help (void);
int MakeTag (char custom1[], char custom2[], char filename[], char tagfile[], char datefile[]);
int MakeTagCustom (char custom1[], char custom2[], char filename[], char tagfile[], char customfile[], char datefile[]);
void TaRTSetup (char custom1[], char custom2[], char customfile[], char sigfile[], char tagfile[], char datefile[]);
void catchsig(int sig);
void license (void);

/* main program */

int main (int argc, char *argv[])
{
	int i;

	srand ((unsigned) time (NULL));		/*  seed the random number generator */

	/* parse all command line params */

 	if (IsSwitch (argv, argc, "--help") != -1 || IsSwitch (argv, argc, "-h") != -1)
		Help ();
 	if (IsSwitch (argv, argc, "--license") != -1 || IsSwitch (argv, argc, "-l") != -1)
		license ();
	if (IsSwitch (argv, argc, "--setup") != -1 || IsSwitch (argv, argc, "-s") != -1)
		TaRTSetup (custom1, custom2, customfile, sigfile, tagfile, datefile);
	if ((i = IsSwitch (argv, argc, "--config")) != -1)
	{
		if (argc < (i+2) || sscanf(argv[i+1], "%s", custconfig) == 0)
		{
			fprintf(stderr, "invalid filename passed to --config\nexecuting with default config file\n");
		}
		if (ReadINIFile (custom1, custom2, customfile, sigfile, tagfile, datefile, optarray, custconfig) == 1) {
			fprintf (stderr, "error reading .tartrc (First time running TaRT? Try `tart --setup')\n");
			exit (1);
		}
	} else if (ReadINIFile (custom1, custom2, customfile, sigfile, tagfile, datefile, optarray, NULL) == 1) {
		fprintf (stderr, "error reading .tartrc (First time running TaRT? Try `tart --setup')\n");
		exit (1);
	}

	if (IsSwitch (argv, argc, "--add") != -1 || IsSwitch (argv, argc, "-a") != -1)
		AddLine (tagfile);
	if (IsSwitch (argv, argc, "-cust") != -1)
		optarray[custom] = 0;
	if (IsSwitch (argv, argc, "+cust") != -1)
		optarray[custom] = 1;
	if (IsSwitch (argv, argc, "--proc") != -1 || IsSwitch (argv, argc, "-p") != -1)
	{
		i = Running(custconfig);
		if (i > 0)
			printf("TaRT is running with PID %d and an interval of %d seconds\n", i, optarray[asdaemon]);
		else
			printf("TaRT is not running\n");
		exit(0);
	}
	if (IsSwitch (argv, argc, "--kill") != -1 || IsSwitch (argv, argc, "-k") != -1)
	{
		i = Running(custconfig);
		if (i > 0)
		{
			if (kill(i, SIGTERM) == 0)
			{
				if (!optarray[quiet]) printf("TaRT (%d) terminated.\n", i);
				exit(0);
			}
			else
			{
				SetRunning(0, custconfig);
				if (!optarray[quiet]) printf("Process %d not terminated... is it actually running?\n", i);
				exit(1);
			}
		}

		if (!optarray[quiet]) printf("TaRT is currently not running\n");
		exit(0);
	}
	if (IsSwitch (argv, argc, "-n") != -1)
		optarray[date] = 0;
	if (IsSwitch (argv, argc, "+n") != -1)
		optarray[date] = 1;
	if (IsSwitch (argv, argc, "-v") != -1)
		optarray[version] = 0;
	if (IsSwitch (argv, argc, "+v") != -1)
		optarray[version] = 1;
	if (IsSwitch (argv, argc, "-sd") != -1)
		optarray[special] = 0;
	if (IsSwitch (argv, argc, "+sd") != -1)
		optarray[special] = 1;
	if (IsSwitch (argv, argc, "-c") != -1)
		optarray[center] = 0;
	if (IsSwitch (argv, argc, "+c") != -1)
		optarray[center] = 1;
	if (IsSwitch (argv, argc, "-q") != -1)
		optarray[quiet] = 0;
	if (IsSwitch (argv, argc, "+q") != -1)
		optarray[quiet] = 1;
	if ((i = IsSwitch (argv, argc, "--daemon")) != -1)
	{
		if (argc < (i+2) || sscanf(argv[i+1], "%d", &optarray[asdaemon]) == 0)
		{
			fprintf(stderr, "invalid sleep interval for --daemon\nexecuting in standard mode\n");
			optarray[asdaemon] = 0;
		}
	}
	if ((i = IsSwitch (argv, argc, "--template")) != -1)
	{
		if (argc < (i+2) || sscanf(argv[i+1], "%s", custparam) == 0)
		{
			fprintf(stderr, "invalid filename passed to --template\nexecuting with regular settings\n");
		}
		strncpy(customfile, custparam, 255);
	}	
			
	if (IsSwitch (argv, argc, "--display") != -1 || IsSwitch (argv, argc, "-d") != -1) {
		DisplaySignature (sigfile);
		return (0);
	}
	/* end command line parsing */

	if (optarray[center] && !optarray[custom])
	{
		CenterText (custom1, TAGLEN);
		CenterText (custom2, TAGLEN);
	}

	/* generate the signatures */

	switch (optarray[custom]) {
	case 0:
		if (MakeTag (custom1, custom2, sigfile, tagfile, datefile)) {
			fprintf (stderr, "error creating signature\n");
			exit (1);
		}
		break;
	case 1:
		i = MakeTagCustom (custom1, custom2, sigfile, tagfile, customfile, datefile);
		switch (i) {
		case 1:
			fprintf (stderr, "error creating signature\n");
			exit (1);
		case 2:
			if (!optarray[quiet])
				fprintf (stderr, "custom file not present, using standard layout\n");
			if (MakeTag (custom1, custom2, sigfile, tagfile, datefile)) {
				fprintf (stderr, "error creating signature\n");
				exit (1);
			}
		}
	}

	if (optarray[asdaemon] != 0)
	{
		if (Running(custconfig))		// if we're already running don't run again
			_exit(0);

		optarray[quiet] = 1;	// make sure we run quietly

		/* close all open files */
		
		for (i=0; i<NOFILE; i++)
			close(i);

		/* fork and terminate parent */
		
		if (fork())
			_exit(0);

		/* create a new session */

		setsid();

		/* fork and terminate parent again */
		
		if ((i = fork()))
		{
			if (SetRunning(i,custconfig) == -1)	// store the PID in ~/.tartrc
				_exit(1);
			else
				_exit(0);
		}
		if (chdir ("/") == -1)
		{
			printf("failed to change dir during fork\n");
			exit(1);
		}	// change directory to / to avoid umount problems
		umask(0);	// change umask to 0 for full control

		/* install our signal handlers */
		
		signal(SIGCHLD, SIG_IGN);
		signal(SIGALRM, catchsig);
		signal(SIGTERM, catchsig);
		signal(SIGQUIT, catchsig);
		signal(SIGHUP, catchsig);
		alarm(optarray[asdaemon]);	// set an alarm

		while(1)	// loop indefinately
		{
			pause();
		}
	}
	return (0);
}

/* function definitions */

/* function: Help
 * usage: Help();
 * --------------
 * This procedure sends the help message to stdout.
 */

void Help (void)
{
	printf (BLUE "\nLinuxTaRT" GREEN " version %.2f " WHITE " Compiled: (%s)\n" NORMAL, VER, __DATE__);
	HLine (74);
	printf ("Command line email signature creator\n");
	printf ("\n");
	printf ("Options:\n");
	printf (" -a, --add\tprompt for a new tagline and add it to the database\n");
	printf (" -d, --display\tdisplays current signature, doesn't change it\n");
	printf (" -h, --help\tdisplays this help\n");
	printf (" -k, --kill\tterminate your currently running TaRT process\n");
	printf (" -p, --proc\tdisplay current status and PID if applicable\n");
	printf (" -s, --setup\trun the internal setup program\n");
	printf (" -l, --license\tdisplay the GNU Public License notice\n");
	printf (" --daemon [s]\trun in daemon mode, wait [s] seconds between updates\n");
	printf (" --config [filename]\tuse the config file specified in [filename]\n");
	printf (" --template [filename]\tuse the custom file specified in [filename]\n");
	printf (" [+/-]c\t\tenable/disable signature centering\t[default = off]\n");
	printf (" [+/-]cust\tbuild signature based on custom file\t[default = on]\n");
	printf (" [+/-]n\t\tenable/disable date in signature\t[default = on]\n");
	printf (" [+/-]q\t\tenable/disable quiet mode\t\t[default = off]\n");
	printf (" [+/-]sd\tenable/disable special dates\t\t[default = on]\n");
	printf (" [+/-]v\t\tenable/disable TaRT version info\t[default = on]\n");
	exit (0);
}

/* function: license
 * usage: license();
 * -------------
 * This function prints the GPL license notice.
 */

void license (void)
{
	printf ("\nThis program is free software; you can redistribute it and/or modify\n");
	printf ("it under the terms of the GNU General Public License as published by\n");
	printf ("the Free Software Foundation; either version 2 of the License, or\n");
	printf ("(at your option) any later version.\n\n");

	printf ("This program is distributed in the hope that it will be useful,\n");
	printf ("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	printf ("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	printf ("GNU General Public License for more details.\n\n");

	printf ("You should have received a copy of the GNU General Public License\n");
	printf ("along with this program; if not, write to the Free Software\n");
	printf ("Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\n");
	exit(0);
}	

/* function: MakeTag
 * usage: i = MakeTag(custom1, custom2, filename, tagfile, datefile);
 * --------------------------------------
 * This function creates a standard TaRT type signature
 * in file 'filename' using a tagline from 'tagfile'.
 * it returns 1 if there was an error, 0 otherwise.
 */

int MakeTag (char custom1[], char custom2[], char filename[], char tagfile[], char datefile[])
{
	FILE *outfile;
	struct tm *l_time;
	char dateToday[256];
	char tagline[256];
	char shortDate[256];
	char VersionInfo[256];
	time_t now;
	int taglines, tagnumber;

	now = time ((time_t *) NULL);	/*  a lot of obscene code */
	time (&now);		/*  for retrieving the date */
	l_time = localtime (&now);	/*  in a useable manner :) */
	
	strftime (shortDate, TAGLEN, "%m/%d\n", l_time);
	strftime (dateToday, TAGLEN, "Tagline for %A, %B %d, %Y\n", l_time);
	if (optarray[center])
		CenterText (dateToday, TAGLEN);
	taglines = CountTagLines (tagfile);	/*  count the taglines in the database */
	if (taglines == -1) {
		fprintf (stderr, "error reading taglines\n");
		return (1);
	}
	tagnumber = SelectRandom (0, taglines);
	GetTagLine (tagfile, tagline, tagnumber, shortDate, datefile, special);		/*  get a tagline */
	if (optarray[center])
		CenterText (tagline, TAGLEN);

	sprintf (VersionInfo, "LinuxTaRT version %.2f\n", VER);
	if (optarray[center])
		CenterText (VersionInfo, TAGLEN);

	outfile = fopen (filename, "w");	/*  open signature file for writing */
	if (outfile == NULL)
		return (1);

	fprintf (outfile, "%s\n", HLINE);	/*  write the signature out */
	fprintf (outfile, "%s\n", custom1);	/*  to the file, line by line */
	fprintf (outfile, "%s\n", HLINE);
	if (optarray[date])
		fprintf (outfile, "%s", dateToday);
	if (optarray[date])
		fprintf (outfile, "\n");
	fprintf (outfile, "%s", tagline);
	if (optarray[version])
		fprintf (outfile, "\n");
	if (optarray[version])
		fprintf (outfile, "%s", VersionInfo);
	fprintf (outfile, "%s\n", HLINE);
	fprintf (outfile, "%s\n", custom2);
	fprintf (outfile, "%s", HLINE);
	if (fclose (outfile))
		return (1);

	if (!optarray[quiet])
		DisplaySignature (filename);	/*  display the new signature if not quiet */

	return (0);
}

/* function: MakeTagCustom
 * usage: i = MakeTagCustom(custom1, custom2, filename, tagfile, customfile, datefile);
 * --------------------------------------
 * This function creates a custom TaRT signature
 * in file 'filename' using the string in 'tagline' and a custom layout
 * from the file 'customfile'.
 * it returns 1 if there was an error, 0 otherwise.
 */

int MakeTagCustom (char custom1[], char custom2[], char filename[], char tagfile[], char customfile[], char datefile[])
{
#ifdef __linux__
        int uptime;
#endif

#ifdef __riscos__
	int uptime;
#endif

#ifdef IBEATS   
        char Ibeats[5];
#endif
	
	FILE *outfile;
	FILE *custfile;
	struct tm *l_time;
	struct tm *gm_time;

#ifdef __linux__
	struct sysinfo info;
#endif
	
	signed char c = '\0', d = '\0';
	char fmt[3], dateToday[256], VersionInfo[6], shortDate[256];
	char Hour12[3], Hour24[3], Minute[3], AMPM[3];
	char tagline[4096];
	time_t now;
	int taglines, tagnumber;

	custfile = fopen (customfile, "r");	/*  open custom definition file for parsing */
	if (custfile == NULL)
		return (2);

	StripCR (custom1);
	StripCR (custom2);

	taglines = CountTagLines (tagfile);
	if (taglines == -1) {
		fprintf (stderr, "error reading taglines\n");
		return (1);
	}
	sprintf (VersionInfo, "%.2f", VER);

#ifdef __riscos__
        uptime = clock() / CLOCKS_PER_SEC / 3600;
#endif

#ifdef __linux__
        sysinfo (&info);
	uptime = info.uptime / 3600;
#endif
	
	now = time ((time_t *) NULL);	/*  a lot of obscene code */
	time (&now);		/*  for retrieving the date */
	gm_time = gmtime (&now);      /* since ibeats works from GMT+1, may as well use GMT as a base for dates  */
	time (&now);
	l_time = localtime (&now);	/*  in a useable manner :) */	

	strftime (shortDate, TAGLEN, "%m/%d\n", l_time);
	strftime (dateToday, TAGLEN, "%A, %B %d, %Y", l_time);
	strftime (Hour12, 3, "%I", l_time);
	strftime (Hour24, 3, "%H", l_time);
	strftime (Minute, 3, "%M", l_time);
	strftime (AMPM, 3, "%p", l_time);

#ifdef IBEATS	
	sprintf (Ibeats, "@%03i", time_ibeats(&now));
#endif
	
	outfile = fopen (filename, "w");	/*  open signature file for writing */
	if (outfile == NULL)
		return (1);

	while (!feof (custfile)) {	/*  basically, read through the custom layout char by */
		c = fgetc (custfile);	/*  char and parse it for formatting characters. */
		if (c == '%') {
			c = fgetc (custfile);
			if (c == '%')
				fputc (c, outfile);
			else {
				fmt[0] = c;
				fmt[1] = fgetc (custfile);
				fmt[2] = '\0';

#ifdef __riscos__
				if (strncmp (fmt, "ut", 2) == 0)
					fprintf (outfile, "%d", uptime);
#endif

#ifdef __linux__
				if (strncmp (fmt, "ut", 2) == 0)
					fprintf (outfile, "%d", uptime);
#endif
				
#ifdef IBEATS
				if (strncmp (fmt, "ib", 2) == 0)
					fprintf (outfile, Ibeats);
#endif
				
				if (strncmp (fmt, "c1", 2) == 0)
					fprintf (outfile, "%s", custom1);
				if (strncmp (fmt, "c2", 2) == 0)
					fprintf (outfile, "%s", custom2);
				if (strncmp (fmt, "dt", 2) == 0)
					fprintf (outfile, "%s", dateToday);
				if (strncmp (fmt, "12", 2) == 0)
					fprintf (outfile, "%s", Hour12);
				if (strncmp (fmt, "24", 2) == 0)
					fprintf (outfile, "%s", Hour24);
				if (strncmp (fmt, "mn", 2) == 0)
					fprintf (outfile, "%s", Minute);
				if (strncmp (fmt, "ap", 2) == 0)
					fprintf (outfile, "%s", AMPM);
				if (strncmp (fmt, "hr", 2) == 0)
					fprintf (outfile, "%s", HLINE);
				if (strncmp (fmt, "tg", 2) == 0) {
					tagnumber = SelectRandom (0, taglines);
					GetTagLine (tagfile, tagline, tagnumber, shortDate, datefile, special);
					StripCR (tagline);
					fprintf (outfile, "%s", tagline);
					optarray[special] = 0;
				}
				if (strncmp (fmt, "vi", 2) == 0)
					fprintf (outfile, "%s", VersionInfo);
				strcpy (fmt, "  ");
				c = '\0';
			}
		} else {
			d = fgetc (custfile);
			if (d != EOF)
				fputc (c, outfile);
			ungetc (d, custfile);
			d = '\0';
		}
	}
	if (fclose (custfile))
		return (1);
	if (fclose (outfile))
		return (1);

	if (!optarray[quiet])
		DisplaySignature (filename);	/*  display the new signature if not quiet */

	return (0);
}

/* procedure: TaRTSetup
 * usage: TaRTSetup();
 * ---------------
 * This procedure prompts the user for different settings and
 * creates a new .tartrc file in the users home directory.
 * Terminates program execution.
 */

void TaRTSetup (char custom1[], char custom2[], char customfile[], char sigfile[], char tagfile[], char datefile[])
{
	char tmp[256];
	char ptmp[256];

	printf (BLUE "\n-- TaRT %.2f" NORMAL " Interactive Setup --\n", VER);

	if (ReadINIFile (custom1, custom2, customfile, sigfile, tagfile, datefile, optarray, NULL)) {
		strcpy (custom1, "TaRT - The Automatic Random Tagline\n");	/*  set some defaults */
		strcpy (custom2, "My webpage: http://www.apage.com/mypage\n");
#ifdef __riscos__
		strcpy (customfile, "Choices:TaRT.Layout");
		strcpy (sigfile, "<Choices$Write>.mail./signature");
		strcpy (tagfile, "<Choices$Write>.TaRT.taglines");
		strcpy (datefile, "Choices:TaRT.SpecialDates");
#else
		strcpy (customfile, "~/.tart-custom");
		strcpy (sigfile, "~/.signature");
		strcpy (tagfile, "/etc/tart.tags");
		strcpy (datefile, "~/.tartdates");
#endif
	}
	StripCR (custom1);
	memset(ptmp, '-', TAGLEN - 48);
	ptmp[TAGLEN - 48] = '\0';
	printf (YELLOW "\nEnter text for first custom line (%2d chars max): %s|" GREEN "\n[%s]\n" NORMAL "> ", TAGLEN, ptmp, custom1);
	if (fgets (tmp, TAGLEN +1, stdin) != NULL)
        if (strlen(tmp) == TAGLEN) strcat(tmp,"\n");

	if (tmp[0] != '\n')
		strcpy (custom1, tmp);

	if (fgets(tmp, 255, stdin) == NULL)
	{
		printf("error reading from stdin\n"); /* flush the input buffer */
		exit(1);
	}
	StripCR (custom2);
	memset(ptmp, '-', TAGLEN - 49);
	ptmp[TAGLEN - 49] = '\0';
	printf (YELLOW "\nEnter text for second custom line (%2d chars max): %s|" GREEN "\n[%s]\n" NORMAL "> ", TAGLEN, ptmp, custom2);
	if (fgets (tmp, TAGLEN +1, stdin) != NULL)
        if (strlen(tmp) == TAGLEN) strcat(tmp,"\n");
	if (tmp[0] != '\n')
		strcpy (custom2, tmp);

	if (fgets(tmp, 255, stdin) == NULL) /* flush the input buffer */
	{
		printf("error reading from stdin\n"); /* flush the input buffer */
		exit(1);
	}
	printf (YELLOW "\nEnter the full path to your signature file:" GREEN "\n[%s]\n" NORMAL "> ", sigfile);
	if (fgets (tmp, 255, stdin) == NULL)
	{
		printf("error reading from stdin\n"); /* flush the input buffer */
		exit(1);
	}
	if (tmp[0] != '\n')
		strcpy (sigfile, tmp);

	printf (YELLOW "\nEnter the full path to your custom layout file:" GREEN "\n[%s]\n" NORMAL "> ", customfile);
	if (fgets (tmp, 255, stdin) == NULL)
	{
		printf("error reading from stdin\n"); /* flush the input buffer */
		exit(1);
	}
	if (tmp[0] != '\n')
		strcpy (customfile, tmp);

	printf (YELLOW "\nEnter the full path to your special dates file:" GREEN "\n[%s]\n" NORMAL "> ", datefile);
	if (fgets (tmp, 255, stdin) == NULL)
	{
		printf("error reading from stdin\n"); /* flush the input buffer */
		exit(1);
	}
	if (tmp[0] != '\n')
		strcpy (datefile, tmp);

	printf (YELLOW "\nEnter the full path to the tagline database:" GREEN "\n[%s]\n" NORMAL "> ", tagfile);
	if (fgets (tmp, 255, stdin) == NULL)
	{
		printf("error reading from stdin\n"); /* flush the input buffer */
		exit(1);
	}
	if (tmp[0] != '\n')
		strcpy (tagfile, tmp);

	printf ("\n");

//#ifdef __riscos__
//	strncpy (tmp, "Choices:TaRT.rc", 90);
//#else
//	strncpy (tmp, getenv ("HOME"), 90);       /*  find user's homedir */
//	strcat (tmp, "/.tartrc");
//#endif
//	printf("before fopen line\n");
//	fclose(fopen(tmp, "w"));
//	printf("after fopen line\n");			
	
	if (FirstINIFile (custom1, custom2, customfile, sigfile, tagfile, datefile, optarray)) {
		fprintf (stderr, "error creating .tartrc\n");
		exit (1);
	}

	exit (0);
}

void catchsig(int sig)
{
	int i;
	
	if (sig == SIGTERM || sig == SIGQUIT)
	{
		if (SetRunning(0,custconfig) == -1)
			_exit(1);
		else
			_exit(0);
	}

	if (sig == SIGHUP)
	{
		if (ReadINIFile (custom1, custom2, customfile, sigfile, tagfile, datefile, optarray, custconfig) == 1)
			_exit (1);
		
		signal(SIGHUP, catchsig);	// reinstall our signal handler
	}

	if (sig == SIGALRM)
	{	
		switch (optarray[custom])
		{
			case 0:
				if (MakeTag (custom1, custom2, sigfile, tagfile, datefile))
					_exit (1);
				break;
			case 1:
				i = MakeTagCustom (custom1, custom2, sigfile, tagfile, customfile, datefile);
				switch (i)
				{
					case 1:
						_exit (1); break;
					case 2:
						if (MakeTag (custom1, custom2, sigfile, tagfile, datefile))
							_exit (1);
				}
		}

                signal(SIGALRM, catchsig);	// reinstall our signal handler
	
		alarm(optarray[asdaemon]);	// set another alarm
	}

}
