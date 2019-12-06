/** ------------------------------------------------------------
 ** Concatenate a number of strings together into a newly
 ** allocated buffer
 ** ------------------------------------------------------------
 ** $Id: strconcat.c 11 2008-04-24 22:13:19Z andrew $
 **/

#ifndef MAKEDEPEND
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#endif

#include "os_defs.h"

#include "stringtools.h"


/*
 * ---------------------------------------------
 * Concatenate a set of strings together,
 * malloc'ing memory for the purpose.
 *
 * End of list is indicated by a NULL pointer argument
 * ---------------------------------------------
 */
char *
strconcat(const char *leadptr,...)
{
	va_list vargs;
	const char *argptr;
	char *curptr;
	char *retptr = NULL;
	int curlen;
	int		total_len = 1;


	/* sum the lengths */
	va_start(vargs, leadptr);
	argptr = leadptr;
	while (argptr != NULL)
	{
		total_len += strlen(argptr);
		argptr = va_arg(vargs, char *);
	}
	va_end(vargs);


	if ((retptr = (char *) malloc(total_len)) == NULL)
		return (NULL);

	curptr = retptr;

	/* now paste in the strings */
	va_start(vargs, leadptr);
	argptr = leadptr;
	while (argptr != NULL)
	{
		curlen = strlen(argptr);

		/**
		 * Linux hasn't caught up with the rest of the world
		 * in safe string copying yet, so use the older strlcpy
		 */
#		ifdef OS_BSD
		strlcpy(curptr, argptr, total_len);
#		else
		strncpy(curptr, argptr, total_len);
#		endif

		curptr += curlen;
		argptr = va_arg(vargs, char *);
	}
	va_end(vargs);


	return (retptr);
}

