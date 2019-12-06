/**
 * Companion to "flock.c" but based on record locking, as opposed to
 * whole file locking
 */


#include <stdio.h>

/**
 * you will need to calculate your "os_defs.h" here so that you can use it
 * to figure out the proper header files.
 * For now, we just define OS_WINDOWS
 */

#include "os_defs.h"
#include <stdlib.h>
#include <time.h>

#ifdef OS_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/file.h>
#include <fcntl.h>
#endif



/**
 * Define the commands that we will handle, and the types specific to the
 * current O/S
 */
#ifdef OS_WINDOWS
# define VALID_COMMANDS  "XSU"
	typedef HANDLE LockFileHandle;
	typedef DWORD FileOffset;
#else
# define VALID_COMMANDS  "XSUT"
	typedef int LockFileHandle;
	typedef long FileOffset;
#endif




/**
 * print command line option help
 */
static void
printHelpAndExit(char *progname, char *errormessage)
{
    if (errormessage != NULL)
        fprintf(stderr, "Error: %s\n\n", errormessage);

    fprintf(stderr, "%s <datafile> [ <commandfile> ]\n", progname);
    fprintf(stderr, "    Commands come in the form:\n");
    fprintf(stderr, "        OPN OFF LEN\n");
    fprintf(stderr, "    where:\n");
    fprintf(stderr, "        OPN (operation) is one of:\n");
    fprintf(stderr, "           X : exclusive lock of region, waiting for availbility \n");
    fprintf(stderr, "           S : shared lock of region, waiting for availbility \n");
#ifndef  OS_WINDOWS
    fprintf(stderr, "           T : test whether region is locked\n");
#endif
    fprintf(stderr, "           U : unlock region\n");
    fprintf(stderr, "        OFF, and LEN are the offset and length in bytes\n");
    fprintf(stderr, "    If <commandfile> is not given, stdin is read\n");

    exit (-1);
}

/* wrap ctime(3) to get the current time in a string */
static char *
curtimeString()
{
    time_t curtime;

    /** get the time right now in seconds since the epoch */
    curtime = time(NULL);

    /* ctime(3) inconveniently takes the address of a time value */
    return ctime(&curtime);
}

/**
 * Perform the work of doing the locking operation, whatever it is.
 */
int
doLockAction(
		LockFileHandle lockFileHandle,
		char operation,
		FileOffset offset,
		FileOffset length
	)
{
    char *operationTag;
#ifndef OS_WINDOWS
    struct flock lockData;
		#ifdef OS_LINUX
		const char* lockTypes[] = {"F_RDLCK", "F_WRLCK"};
		const char* whenceTypes[] = {"SEEK_SET", "SEEK_CUR", "SEEK_END"};
		#else
		const char* lockTypes[] = {"", "F_RDLCK", "", "F_WRLCK"};
		const char* whenceTypes[] = {"SEEK_SET", "SEEK_CUR", "SEEK_END"};
		#endif
#else
	OVERLAPPED windowsOverlap;
#endif
    int status, typeCode, opCode;



	/**
	 * Get a nice printable version of the lock operation
	 */
    operationTag = (operation == 'X' ? "Exclusive Lock":
                    operation == 'S' ? "Shared Lock" :
                        operation == 'T' ? "Test for lock" :
                            operation == 'U' ? "Unlock" : "??? unknown command ???");

    printf("PID %5d performing operation %s\n", getpid(), operationTag);

	#ifdef OS_WINDOWS
    /**
     * Code for Windows -- set up the OVERLAPPED structure, and use it
     * to create a lock.  This structure is used with the LockFileEx()
	 * function to store the offset portion of our request.
     */
	memset(&windowsOverlap, 0, sizeof(windowsOverlap));
	windowsOverlap.hEvent = lockFileHandle;
	windowsOverlap.Offset = offset;
	windowsOverlap.OffsetHigh = 0;
	#else
	lockData.l_whence = SEEK_SET;
	lockData.l_start = offset;
	lockData.l_len = length;
	lockData.l_pid = getpid();
	#endif


	/**
	 * Print a message indicating that we are going into the lock request
	 */
	printf("PID %5d : requesting '%c' lock at %ld for %ld bytes at %s",
			getpid(), operation, offset, length, curtimeString());


	/**
	 * If we want a lock, use LockFileEx() - otherwise give up the
	 * lock with UnlockFile()
	 */
	if (operation == 'X' || operation == 'S') {
		#ifdef OS_WINDOWS
		/**
		 * shared locks are the default (no value) so only exclusive
		 * locks have a bit flag
		 */
		if (operation == 'X')
			typeCode = LOCKFILE_EXCLUSIVE_LOCK;
		else
			typeCode = 0;


		/**
		 * Use the OVERLAP structure and our other values to request
		 * the lock
		 */
		if ( ! LockFileEx(lockFileHandle,
				typeCode,
				0, // dwFlags : must be zero
				length, 0, // length of lock - location in overlap struct
				&windowsOverlap)) {
			fprintf(stderr, "Error: LockFile returned failure : %d\n", GetLastError());
			return -1;
		}
		#else
		if (operation == 'X') {
			lockData.l_type = F_WRLCK;
			if(fcntl(lockFileHandle, F_SETLKW, &lockData) < 0) {
				fprintf(stderr, "Error: fcntl returned failure : %d\n", errno);
				return -1;
			}
		}
		else {
			lockData.l_type = F_RDLCK;
			if(fcntl(lockFileHandle, F_SETLKW, &lockData) < 0) {
				fprintf(stderr, "Error: fcntl returned failure : %d\n", errno);
				return -1;
			}
		}
		#endif


	} else if (operation == 'U') {
		#ifdef OS_WINDOWS
		/**
		 * Release a lock on a portion of a file
		 */
		if ( ! UnlockFile(lockFileHandle, offset, 0, length, 0)) {
			fprintf(stderr, "Error: LockFile returned failure : %d\n", GetLastError());
			return -1;
		}
		#else
		lockData.l_type = F_UNLCK;
		if(fcntl(lockFileHandle, F_SETLKW, &lockData) < 0) {
			fprintf(stderr, "Error: fcntl returned failure : %d\n", errno);
			return -1;
		}
		#endif

	}
	#ifndef OS_WINDOWS
	else if (operation == 'T') {
		lockData.l_type = F_WRLCK;
		if(fcntl(lockFileHandle, F_GETLK, &lockData) < 0) {
			fprintf(stderr, "Error: fcntl returned failure : %d\n", errno);
			return -1;
		}
		if(lockData.l_type != 2){
			printf("%d\n",lockData.l_whence);
				printf("%d\n",lockData.l_type);
			printf("Found lock:\nPID:\t%d\nTYPE:\t%s\nFROM:\t%lld FOR %lld BYTES\nWHENCE:\t%s\n",lockData.l_pid, lockTypes[lockData.l_type],lockData.l_start, lockData.l_len, whenceTypes[lockData.l_whence]);
		}
		else{
			printf("No lock found.\n");
		}
	}
	#endif
	else {
		fprintf(stderr, "Error: Unknown operation '%c'\n", operation);
		return -1;
	}

	/** Tell the user what happened */
	printf("PID %5d : received   '%c' lock at %ld for %ld bytes at %s",
			getpid(), operation, offset, length, curtimeString());

    return 0;
}


/**
 * System mainline -- simply process the arguments and input lines
 */
int
main(int argc, char *argv[])
{
	/** use O/S specific type to track the right kind of file handle */
    LockFileHandle lockFileHandle;
    char commandLine[BUFSIZ];
    FILE *commandFP = stdin;
	int nItemsRead, keepGoing = 1;
    long offset, length;
    char operation;


    /** check that the arguments are correct */
    if (argc != 2 && argc != 3)  printHelpAndExit(argv[0], "incorrect arguments");


	/**
	 * if we need to open a data file, we can do so with fopen() so that we
	 * can use fgets()
	 */
    if (argc == 3) {
        if ((commandFP = fopen(argv[2], "r")) == NULL) {
            fprintf(stderr, "Cannot open command file '%s' : %s\n",
                    argv[2], strerror(errno));
            exit (-1);
        }
    }

    #ifdef OS_WINDOWS
    /** Windows specific file open */
	lockFileHandle = CreateFile(argv[1],
		GENERIC_READ | GENERIC_WRITE,       // dwDesiredAccess
		FILE_SHARE_READ | FILE_SHARE_WRITE, //  dwShareMode
		NULL,           // lpSecurityAttributes
		OPEN_EXISTING,  // dwCreationDisposition
		FILE_ATTRIBUTE_NORMAL,  // dwFlagsAndAtributes
		NULL);  // hTemplateFile

	if (lockFileHandle == NULL) {
        fprintf(stderr, "Failed opening '%s' : %d\n", argv[1], GetLastError());
        exit (-1);
    }
	#else
	lockFileHandle = open(argv[1], O_RDWR);
	if(lockFileHandle < 0) {
		fprintf(stderr, "Failed opening '%s' : %s\n", argv[1], strerror(errno));
		exit(-1);
	}
	#endif



	/**
	 * Print out the help overview on all O/S (but there is no test function
	 * on Windows)
	 */
    printf("Processing locks on '%s'\n", argv[1]);
    printf("Available commands:\n");
    printf("    X <start> <len>   : exclusive lock of <len> bytes from <start>\n");
    printf("    S <start> <len>   : shared lock of <len> bytes from <start>\n");
#ifndef OS_WINDOWS
    printf("    T <start> <len>   : test of lock of <len> bytes from <start>\n");
#endif
    printf("    U <start> <len>   : unlock <len> bytes from <start>\n");


    /** print first prompt */
    if (isatty(0))
        printf("CMD> ") >= 0 && fflush(stdout);

    /**
     * loop reading commands
     * we use fgets() to ensure that we are reading whole lines
     */
    while (keepGoing && fgets(commandLine, BUFSIZ, commandFP) != NULL) {


		/** scanf can do this sort of processing pretty well */
        nItemsRead = sscanf(commandLine, "%c %ld %ld", &operation, &offset, &length);


		/** if we get some flavour of Q, we are bailing out */
		if (toupper(operation) != 'Q') {

			/** make sure that we have enough data */
			if (nItemsRead < 3 || strchr(VALID_COMMANDS, toupper(operation)) == NULL)
				fprintf(stderr, "Invalid operation - need \"OPN OFF LEN\"\n");
			else {
				printf("CMD %c %2ld %2ld\n",
						toupper(operation),
						offset, length);

				/** we ignore the results of the lock action here */
				(void) doLockAction(lockFileHandle,
						toupper(operation), offset, length);
			}

			/* prompt again */
			if (isatty(0))
				printf("CMD> ") >= 0 && fflush(stdout);

		} else {
			printf("Exitting ...\n");
			keepGoing = 0;
		}
	}

	#ifdef OS_WINDOWS
	/** clean up our files */
	CloseHandle(lockFileHandle);
	#else
	close(lockFileHandle);
	#endif

	if (commandFP != stdin)	fclose(commandFP);

    exit(0);
}
