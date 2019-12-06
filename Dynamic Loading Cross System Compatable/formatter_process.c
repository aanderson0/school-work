#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdarg.h> 
#include <assert.h> 
#include <errno.h> 
#include "os_defs.h"

#ifndef OS_WINDOWS
# include <dlfcn.h> 
# include <unistd.h> 
#else
# include <conio.h>
#endif

#include "module_tools.h"
#include "module_api.h"
#include "stringtools.h"



/** forward declarations */
static ModuleDataListNode *llAppend(
	    ModuleDataListNode *listp,
	    ModuleDataListNode *newp
	);  


/** debugging function - print output only if verbosity says so */
static void
dbgPrint(int verbosity, int verbEnable, char *format, ...)
{
	va_list args;
	if (verbosity >= verbEnable) {
		printf(" >DB> ");
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
}
 


/**
 * Allocate memory to track and load a given module.
 *
 * Loading is performed by this scheme:
 * - if the name is a fully qualified filename, then simply load the module;
 * - if the name ends with MOD_FILENAME_SUFFIX, and begins with MOD_FILENAME_PREFIX
 *   then search the MODPATH for a file by that name;
 * - if the name does not have the suffix/prefix, then add them, and search
 *   for the resulting name;
 * otherwise, report failure, and abort loading.
 *
 * If anything else goes wrong, report that, and abort loading as well.
 */
static ModuleDataListNode *
searchAndLoadModule(const char *moduleName, const char *modulePath, int verbosity)
{
	ModuleDataListNode *result;
	void *symbol;

	/** allocate the node and ensure that it is painted with zeros */
	result = (ModuleDataListNode *) malloc(sizeof(ModuleDataListNode));
	memset(result, 0, sizeof(ModuleDataListNode));

	/**
	 * set up the node with the name we are given, and
	 * make sure there is no dangling next pointer
	 */
	result->name = moduleName;
	result->next = NULL;

	char* fullModulePath;
	if(modulePath != NULL){
		fullModulePath = malloc(sizeof(char) * (strlen(moduleName) + strlen(modulePath) + 6));
		sprintf(fullModulePath, "%s%s", modulePath, moduleName);
	}
	else{
		fullModulePath = malloc(sizeof(char) * (strlen(moduleName) + 8));
		sprintf(fullModulePath, "./%s", moduleName);
	}
	#ifdef OS_WINDOWS
	sprintf(fullModulePath, "%s.dll", fullModulePath);	
	#elif defined OS_DARWIN
	sprintf(fullModulePath, "%s.dylib", fullModulePath);
	#else
	sprintf(fullModulePath, "%s.so", fullModulePath);
	#endif

	#ifndef OS_WINDOWS
	result->sharedObject = dlopen(fullModulePath, RTLD_LAZY);
	#else
	result->sharedObject = LoadLibrary(TEXT(fullModulePath));
	#endif

	free(fullModulePath);

	if(result->sharedObject == NULL){
		return NULL;
	}

	return result;
}

static void
unloadModule(ModuleDataListNode *data)
{
	assert(data->sharedObject != NULL);

	#ifndef OS_WINDOWS
	dlclose(data->sharedObject);
	#else
	FreeLibrary(data->sharedObject);
	#endif

	free(data);
}



/**
 * load all the modules, using the above functions
 */
ModuleDataListNode *
moduleListLoadAll(const char *modListString, const char *modpath, int verbosity)
{
	char *splits[128];
	int nSplits, i;
	ModuleDataListNode *resultChain = NULL, *curNode;

	if (modListString == NULL) {
		fprintf(stderr, "No modules given\n");
		return NULL;
	}

	if ((nSplits = split(splits, ",", modListString, 128)) < 0) {
		fprintf(stderr, "Invalid module string '%s'\n", modListString);
		return NULL;
	}

	for (i = 0; i < nSplits; i++) {
		curNode = searchAndLoadModule(splits[i], modpath, verbosity);
		if (curNode == NULL) {
			fprintf(stderr, "Failed loading module '%s'\n", splits[i]);
			moduleListUnloadAll(resultChain, verbosity);
			return NULL;
		}
		free(splits[i]);
		resultChain = llAppend(resultChain, curNode);
	}

	return resultChain;
}

/**
 * unload and clean up resources for all the modules we have loaded
 */
void
moduleListUnloadAll(ModuleDataListNode *listp, int verbosity)
{
	ModuleDataListNode *next;

	dbgPrint(verbosity, 2, "Cleaning up!\n");

	/* walk the list to the end */
	while (listp != NULL) {
		/** hang on to the next pointer so that it is not lost when we free data */
		next = listp->next;

		/** clean up the shared library, freeing memory */
		dbgPrint(verbosity, 3, "Calling for module unload\n");
		unloadModule(listp);
		dbgPrint(verbosity, 3, "Back from module unload\n");

		/** move to the next node */
		listp = next;
	}
}

/*
 * add node to end of linked list
 *
 * The "static" declaration here means that this code is local only
 * to this file.
 */
static ModuleDataListNode *
llAppend(ModuleDataListNode *listp, ModuleDataListNode *newp)
{
	ModuleDataListNode *p;

	/** if we have no list, this node is the full list */
	if (listp == NULL)
		return newp;

	/** walk the list to the end */
	for (p = listp; p->next; p = p->next)
		;

	/*
	 * add at the end, and return the list
	 */
	p->next = newp;
	return listp;
}


/**
 * Do the work to process a single line through all of the modules
 */
static int
processLineData(
		ModuleDataListNode *moduleData,
		ProcessingData *lineData,
		int lineNo,
		int verbosity
	)
{
	int result;

	/** make sure that we actually loaded the module, or abort */
	assert(moduleData->sharedObject != NULL);

	dbgPrint(verbosity, 2, "Locating function in module '%s'\n", moduleData->name);
	/** get the symbol from the module listed in "moduleData */


	int (OS_C_DECL *fp)(ProcessingData*, int);
	#ifndef OS_WINDOWS
	fp = dlsym(moduleData->sharedObject, "libraryFunction");
	#else
	fp = (int(OS_C_DECL *)(ProcessingData*,int))GetProcAddress(moduleData->sharedObject, "libraryFunction");
	#endif

	if (fp == NULL) {
		printf("Error GetProcAddress returned NULL\n");
		return NULL;
	}

	dbgPrint(verbosity, 2, "Calling function\n");
	/** call the function from the symbol that we got */
	result = fp(lineData, lineNo);

	return result;
}




static int
processLineWithModuleList(
		ModuleDataListNode *listp,
		ProcessingData *data,
		int lineNo,
		int maxBuffers,
		int verbosity
	)
{
	int count = 0, status;

	/** chase though the linked list, processing the line with all modules */
	for ( ; listp != NULL && data->nBuffers < maxBuffers; listp = listp->next) {
		count++;
		/* call function for appropriate type of processing */
		status = processLineData(listp, data, lineNo, verbosity);
		if (status < 0) {
			fprintf(stderr, "Failure processing line\n");
			return status;
		}
	}
	return count;
}

void
moduleListPrintAllNames(FILE *ofp, ModuleDataListNode *node)
{
	printf("Module: %s\n", node->name);
}


/*
 * Open the indicated file, and then processing using the function below
 */
int
processFile(
		FILE *ofp,
		const char *filename,
		ModuleDataListNode *moduleChain,
		int verbosity
	)
{
	FILE *ifp;
	int status;

	ifp = fopen(filename, "r");
	if (ifp == NULL) {
		fprintf(stderr, "Cannot open input script '%s' : %s\n",
				filename, strerror(errno));
		return -1;
	}

	status = processFP(ofp, ifp, filename, moduleChain, verbosity);
	dbgPrint(verbosity, 3, "Closing input file\n");
	fclose(ifp);
	return status;
}


/**
 * Do the main work of processing the given file stream.
 *
 * For all lines in the file, process through all the modules.  For the
 * first line, additionally process the header.
 */
int
processFP(
		FILE *ofp,
		FILE *ifp,
		const char *filename,
		ModuleDataListNode *moduleChain,
		int verbosity
	)
{
	char line[BUFSIZ];
	ProcessingData moduleProcessingData;
	int lineNo = 0;

	/** point the first buffer to our storage here, indicate no free */
	moduleProcessingData.bufferset[0] = line;
	moduleProcessingData.requiresFree[0] = 0;

	while (fgets(line, BUFSIZ, ifp) != NULL) {
		moduleProcessingData.nBuffers = 1;
		if ( processLineWithModuleList(
				moduleChain,
				&moduleProcessingData, lineNo++,
				MAX_N_BUFFERS, verbosity) < 0) {
			fprintf(stderr, "Failed processing at line '%d'\n", lineNo-1);
			return (-1);
		}
		fputs(
				moduleProcessingData.bufferset[ moduleProcessingData.nBuffers -1 ],
				ofp);
		fprintf(ofp, "---\n");
	}
	
	//free processing data
	for(int i = 0; i < moduleProcessingData.nBuffers; i++){
		if(moduleProcessingData.requiresFree[i] == 1) { 
			free(moduleProcessingData.bufferset[i]);
		}
	}

	return 0;
}

