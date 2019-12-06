#ifndef	__MODULE_API_HEADER__
#define	__MODULE_API_HEADER__

#include <stdio.h>

#include "os_defs.h"

#define MAX_N_BUFFERS	128

/*
 * define our types -- this is a potential pattern you may use to
 * keep track of the strings passing back and forth between the
 * modules
 */
typedef struct ProcessingData {
	char *bufferset[MAX_N_BUFFERS];
	char requiresFree[MAX_N_BUFFERS];
	int nBuffers;
} ProcessingData;


/**
 **	Use the OS specific decoration declared in os_types.h to put
 ** the right declaration in place if we are on Windows, and nothing
 **	if we are not.
 **/
typedef int (OS_C_DECL *libraryFnType)(ProcessingData *, int lineNo);


#endif /*	__MODULE_API_HEADER__ */
