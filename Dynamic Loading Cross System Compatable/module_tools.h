#ifndef	__MODULE_TOOLS_HEADER__
#define	__MODULE_TOOLS_HEADER__

#include "os_defs.h"

#include <stdio.h>

#ifdef OS_WINDOWS
# include <windows.h>
#endif


/*
 * Here is a linked list node type for you to use
 */
typedef struct ModuleDataListNode {
	const char *name;

	/** use an OS-specific type for the shared library */
#	ifdef OS_WINDOWS
	HINSTANCE sharedObject;
#	else
	void *sharedObject;
#	endif

	struct ModuleDataListNode *next;
} ModuleDataListNode;


/** PROTOTYPES */

/** process from a file pointer */
int processFP(FILE *ofp, FILE *ifp,
		const char *fileDescription, ModuleDataListNode *moduleChain,
		int verbosity
	);

/** open a file, then process using above function */
int processFile(FILE *ofp,
		const char *filename, ModuleDataListNode *moduleChain,
		int verbosity
	);


/** load all of the modules indicated in the string */
ModuleDataListNode *moduleListLoadAll(
		const char *modListString,
		const char *modpath,
		int verbosity
	);

/** unload all of the previously loaded modules */
void moduleListUnloadAll(ModuleDataListNode *data, int verbosity);

/** print all of the module names to the supplied file pointer */
void moduleListPrintAllNames(FILE *ofp, ModuleDataListNode *data);



/** constant definitions */
#define	VAR_MODPATH	"MODPATH"

#endif /*	__MODULE_TOOLS_HEADER__ */
