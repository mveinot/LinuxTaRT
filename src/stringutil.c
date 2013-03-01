/* file: stringutil.c
 * ------------------
 * this is an include file containing usefull
 * string-based routines produced for LinuxTaRT
 */

#include "stringutil.h"

/* function: CenterText
 * usage: CenterText(text, width);
 * -------------------------------
 * This procedure pads string 'text' with initial spaces
 * such that when printed, it appears centered within a
 * space which is 'width' characters wide.
 */

void CenterText (char text[], int width)
{
	char tmp[256] = "";
	int i;

	if (strlen (text) <= width) {
		for (i = 0; i < (width - strlen (text)) / 2; i++)
			strcat (tmp, " ");	/* add the spaces */
		strcat (tmp, text);		/* append the text */
		strcpy (text, tmp);		/* replace the old with the new text */
	}
}

/* function: HLine
 * usage: i = HLine(length);
 * -------------------------
 * This function displays a horizontal line of length 'length'
 * made up of minus (-) signs. It returns the number of lines printed.
 */

int HLine (int length)
{
	int i;

	for (i = 1; i <= length; i++)
		printf ("-");
	printf ("\n");
	return (i);
}

/* function: IsSwitch
 * usage: i = IsSwitch(stringArray, size, string);
 * -----------------------------------------
 * This function returns the number of the switch matching 'string'
 * if a match is found in stringArray (with 'size' elements), 
 * -1 otherwise.
 */

int IsSwitch (char *list[], int size, char *arg)
{
	int i;

	for (i = 1; i < size; i++)
		if (!strcmp (list[i], arg))
			return (i);
	return (-1);
}

/* function: StripCR
 * usage: StripCR(string);
 * -----------------------
 * This function replaces all cariage returns in 'string' with
 * null characters.
 */

void StripCR (char *string)
{
	int i;

	for (i = strlen (string) -1 ; i > 0; i--)
		if (string[i] == '\n'){
			string[i] = '\0';
			break;
		}
}

