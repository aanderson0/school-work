/* ------------------------------------------------------------
 * Split a string into an array
 * ------------------------------------------------------------
 * $Id: strsplit.c 33 2009-04-26 13:48:18Z andrew $
 */

#ifndef MAKEDEPEND
#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#endif

#include "os_defs.h"

/*
 * ---------------------------------------------
 * go through a string, splitting based on the
 * dilimiters given, until we have filled up
 * the given array
 * ---------------------------------------------
 */
int
split(fill_array, delims, source, max)
    char          **fill_array;
    const char     *delims;
    const char     *source;
    int             max;
{
	int             n_found;
	char           *work, *newtok;

	work = strdup(source);

	/** find the first value */
	if ((newtok = strtok(work, delims)) == NULL)
	{
		return 0;
	}
	fill_array[0] = strdup(newtok);

	/**
	 * we have set up strtok, now call it in a loop until we see
	 * no more tokens
	 */
	n_found = 1;
	while (n_found < max)
	{
		newtok = strtok(NULL, delims);
		if (newtok == NULL)
		{
			break;
		}
		fill_array[n_found] = strdup(newtok);
		n_found++;
	}

	free(work);
	return (n_found);
}

