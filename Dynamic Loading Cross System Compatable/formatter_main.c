#include <stdio.h>

/** include os_defs before checks based on OS */
#include "os_defs.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef OS_WINDOWS
# include <unistd.h>
# include <unistd.h> 
#else
# include <conio.h>
#endif

#include "module_tools.h"
#include "stringtools.h"

static int
printHelp(char *progname)
{
	printf("%s <options> [ <files> ]\n", progname);
	printf("\n");
	printf("Run scripts from files, or stdin if no file specified\n");
	printf("\n");
	printf("Options:\n");
	printf("-o<file>     : place output in <file>\n");
	printf("-v           : be more verbose\n");
	printf("-P<path>     : set search path for modules\n");
	printf("             : (default value taken from $%s variable)\n", VAR_MODPATH);
	printf("-D<modules>  : add <modules> to processing chain.\n");
	printf("\n");
	exit (1);
}

/**
 ** This is an ugly, clunky, but easy to write replacement for getopt,
 ** which is not available on Windows
 **/
static char *calculateOptionArgAndAdjustIndex(
		char **argv,
		int *indexp,
		char *optiontag
	)
{
	char *optionArg;

	if (argv[*indexp][2] != '\0')
		optionArg = &argv[*indexp][2];
	else
		optionArg = argv[++(*indexp)];
	if (optionArg == NULL || strlen(optionArg) == 0) {
		fprintf(stderr, "No option given for '-%s'\n", optiontag);
		printHelp(argv[0]);
	}

	return optionArg;
}

int
main(int argc, char **argv)
{
	FILE *ofp = stdout;
	char *modulePath = NULL, *moduleLoadString = NULL;
	char *optionArg;
	ModuleDataListNode *moduleChain = NULL;
	int verbosity = 0;
	int processingStatus = 0;
	int fileOnlyArgC = 1;
	int exitStatus = (-1);
	int i, ch;

	/** get the environment variable whose name is stored in VAR_MODPATH */
	/** ... add code here ... */
	modulePath = getenv(VAR_MODPATH);
	/** ... end add ... */

	/** there is no getopt on Windows, so just do the work ourselves */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-'
#					ifdef OS_WINDOWS
					|| argv[i][0] == '/'
#					endif
				) {
			/** is a flag */
			switch (argv[i][1]) {
			case 'v':
				verbosity++;
				break;

			case 'o':
				optionArg = calculateOptionArgAndAdjustIndex(argv, &i, "o");
					
				if ((ofp = fopen(optionArg, "w")) == NULL) {
					(void) fprintf(stderr,
							"failed opening output file '%s' : %s\n",
							optionArg, strerror(errno));
					printHelp(argv[0]);
				}
				break;

			case 'P':
				modulePath = calculateOptionArgAndAdjustIndex(argv, &i, "P");
				break;

			case 'D':
				optionArg = calculateOptionArgAndAdjustIndex(argv, &i, "D");

				if (moduleLoadString == NULL) {
					moduleLoadString = strdup(optionArg);
				} else {
					char *cleanup = moduleLoadString;
					moduleLoadString = strconcat(moduleLoadString, ",", optionArg);
					free(cleanup);
				}
				break;


			case '?':
			case 'h':
			default:
				printHelp(argv[0]);
				break;
			}
		} else {
			/** is a file -- move it up the list (overwriting non-file options) */
			argv[fileOnlyArgC++] = argv[i];
		}
	}

	/**
	 ** Prepare to process the files -- set up the module system
	 **/

	/** if no modules are loaded yet, try the "default" module */
	/** ... add code here ... */
	if (moduleLoadString == NULL) {
		moduleLoadString = strdup("default");
	}
	/** ... end add ... */


	/*
	 * Load all the modules
	 */
	moduleChain = moduleListLoadAll(moduleLoadString, modulePath, verbosity);
	if (moduleChain == NULL) {
		fprintf(stderr, "Modules not successfully loaded\n");
		exit (-1);
	}


	if (verbosity > 0) {
		moduleListPrintAllNames(stdout, moduleChain);
	}


	if (fileOnlyArgC > 1) {
		for (i = 1; i < fileOnlyArgC; i++) {
			processingStatus = processFile(ofp, argv[i], moduleChain, verbosity);
			if (processingStatus < 0) {
				exitStatus = (-1);
				break;
			}
		}
	} else {
		processingStatus = processFP(ofp, stdin, "stdin", moduleChain, verbosity);
		if (processingStatus < 0)
			exitStatus = (-1);
	}

	/**
	 * clean everything up
	 */
	moduleListUnloadAll(moduleChain, verbosity);
	free(moduleLoadString);

	return exitStatus;
}
