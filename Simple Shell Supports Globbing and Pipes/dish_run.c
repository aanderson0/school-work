#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <glob.h>

#include "dish_run.h"
#include "dish_tokenize.h"

#define MAXCOMMANDS 1024

int children[512];
int missingChildren = 0;

/**
 * Print a prompt if the input is coming from a TTY
 */
static void prompt(FILE *pfp, FILE *ifp)
{
	if (isatty(fileno(ifp))) {
		fputs(PROMPT_STRING, pfp);
	}
}

/**
 * Actually do the work
 */
int execFullCommandLine(
		FILE *ofp,
		char ** const tokens,
		int nTokens,
		int verbosity)
{
	char ***commands = malloc(sizeof(char**) * MAXCOMMANDS);
	int pipeBool;
	int numPipes = 0;
	int pipeItterator = 0;
	int evenPipefds[2];
	int oddPipefds[2];
	int pid;
	int status = 0;
	int exitingPid = 0;
	int isBackground = 0;
	int cdFlag = 0;
	int pwdFlag = 0;

	
	if (verbosity > 0) {
		fprintf(stderr, " + ");
		fprintfTokens(stderr, tokens, 1);
	}

	//split into pipe segments and check for background
	pipeBool = 0;
	int j = 0;
	commands[0] = malloc(sizeof(char*) * MAXCOMMANDS);
	for(int k = 0; k < MAXCOMMANDS; k++){
		commands[0][k] = NULL;
	}
	for(int i = 0; i < nTokens; i++){
		commands[numPipes][j] = malloc(sizeof(char) * (strlen(tokens[i]) + 1));
		strcpy(commands[numPipes][j], tokens[i]);
		j++;
		//check for pipes
		if(strcmp(tokens[i], "|") == 0) {
			pipeBool = 1;
			free(commands[numPipes][j - 1]);
			commands[numPipes][j - 1] = NULL;
			numPipes++;
			commands[numPipes] = malloc(sizeof(char*) * MAXCOMMANDS);
			for(int k = 0; k < MAXCOMMANDS; k++){
				commands[numPipes][k] = NULL;
			}
			j = 0;
		}
		//check for background process
		else if(strcmp(tokens[i], "&") == 0 && i == nTokens - 1){
			free(commands[numPipes][j - 1]);
			commands[numPipes][j - 1] = NULL;
			isBackground = 1;
		}
		//glob
		else if(strstr(tokens[i], "*") != NULL || strstr(tokens[i], "?") != NULL || strstr(tokens[i], "[") != NULL ) {
			glob_t globdata;
			glob(tokens[i], 0, NULL, &globdata);
			j--;
			free(commands[numPipes][j]);
			commands[numPipes][j] = NULL;
			for(int k = 0; k < globdata.gl_pathc; k++) {
				commands[numPipes][j] = malloc(sizeof(char) * (strlen(globdata.gl_pathv[k]) + 1));
				strcpy(commands[numPipes][j], globdata.gl_pathv[k]);
				j++;
			}
			globfree(&globdata);
		}
		//cd
		else if(strcmp(tokens[i], "cd") == 0 && i == 0){
			cdFlag = 1;
		}
		//This isn't in the spec, but you should really be able to exit
		else if(strcmp(tokens[i], "exit") == 0 && i == 0){
			exit(0);
		}
	}

	if(commands[0][j] != NULL) {
		commands[0][j] = NULL;
	}

	if(cdFlag){
		if(nTokens > 1)
		chdir(commands[0][1]);
		else
		printf("Usage: cd <path>\n");
	}
	else{

		while(pipeItterator <= numPipes){

			//make pipes
			if(pipeItterator % 2 == 0 && pipe(evenPipefds) < 0) {
				perror("Error while creating pipe");
				exit (-1);
			}
			if(pipeItterator % 2 == 1 && pipe(oddPipefds) < 0) {
				perror("Error while creating pipe");
				exit (-1);
			}
		
			//fork
			if((pid = fork()) < 0){
				fprintf(stderr, "Error while forking\n");
				exit (-1);
			}
			
			//child process
			if(pid == 0) {	

				//kill stdin if in background
				if(isBackground && pipeItterator == 0){
					close(0);				
				}
				//if first, write to even
				if(pipeItterator == 0 && pipeItterator != numPipes){
					close(1);
					dup(evenPipefds[1]);
				}
				//if last...
				else if(pipeItterator == numPipes){
					//..and even, read from odd
					if(pipeItterator % 2 == 0){
						close(0);
						dup(oddPipefds[0]);
					}
					//..and odd, read from even
					else
					{
						close(0);
						dup(evenPipefds[0]);
					}
					
				}
				else{
					//if even, write to even pipe, read from odd pipe
					if(pipeItterator % 2 == 0){
						close(0);
						dup(oddPipefds[0]);
						close(1);
						dup(evenPipefds[1]);
					}
					//if odd, write to odd pipe, read from even pipe
					else{
						close(0);
						dup(evenPipefds[0]);
						close(1);
						dup(oddPipefds[1]);
					}
				}
				//close pipes
				close(evenPipefds[0]);
				close(evenPipefds[1]);
				close(oddPipefds[0]);
				close(oddPipefds[1]);
				//run command
				execvp(commands[pipeItterator][0],commands[pipeItterator]);
				perror("Error running exec\n");
				exit (-1);
			}
			
			//parent process

			//close pipes
			if(pipeItterator % 2 == 1) {
				close(evenPipefds[0]);
				close(evenPipefds[1]);
			}	
			if(pipeItterator % 2 == 0){
				close(oddPipefds[0]);
				close(oddPipefds[1]);
			}

			//add child to list
			for(int i = 0; i < 512; i++){
				if(children[i] == 0){
					children[i] = pid;
					missingChildren++;
					i = 513;
				}
			}


			//wait for children
			if(!isBackground){
				while(missingChildren){
					exitingPid = wait(&status);
					for	(int i = 0; i < 512; i++){
						if(exitingPid == children[i]){
							pid = exitingPid;
							children[i] = 0;
							missingChildren--;
							//check child exited
							if (WIFEXITED(status)) {
								//check child exited successfuly
								if(WEXITSTATUS(status) != 0){
									fprintf(stderr, "Child(%d) exited -- failure (%d)\n", pid, WEXITSTATUS(status));
								}
								else {
									fprintf(stderr, "Child(%d) exited -- success (%d)\n", pid, WEXITSTATUS(status));
								}
							}
							else {
								fprintf(stderr, "Child(%d) crashed.\n", pid);
							}
						}
					}	
				}
			}

			pipeItterator++;
		}
	}

	//free command buffer
	for(int i = 0; i < numPipes; i++){
		for(int j = 0; commands[i][j] != NULL; j++){
			free(commands[i][j]);
			commands[i][j] = NULL;
		}
		free(commands[i]);
		commands[i] = NULL;
	}
	free(commands);
	commands = NULL;
	
	return 0;
}

/**
 * Load each line and perform the work for it
 */
int
runScript(
		FILE *ofp, FILE *pfp, FILE *ifp,
		const char *filename, int verbosity
	)
{
	char linebuf[LINEBUFFERSIZE];
	char *tokens[MAXTOKENS];
	int lineNo = 1;
	int nTokens, executeStatus = 0;

	fprintf(stderr, "SHELL PID %ld\n", (long) getpid());

	prompt(pfp, ifp);
	while ((nTokens = parseLine(ifp,
				tokens, MAXTOKENS,
				linebuf, LINEBUFFERSIZE, verbosity - 3)) > 0) {
		lineNo++;

		if (nTokens > 0) {

			executeStatus = execFullCommandLine(ofp, tokens, nTokens, verbosity);

			if (executeStatus < 0) {
				fprintf(stderr, "Failure executing '%s' line %d:\n    ",
						filename, lineNo);
				fprintfTokens(stderr, tokens, 1);
				return executeStatus;
			}
		}
		prompt(pfp, ifp);
	}

	return (0);
}


/**
 * Open a file and run it as a script
 */
int
runScriptFile(FILE *ofp, FILE *pfp, const char *filename, int verbosity)
{
	FILE *ifp;
	int status;

	ifp = fopen(filename, "r");
	if (ifp == NULL) {
		fprintf(stderr, "Cannot open input script '%s' : %s\n",
				filename, strerror(errno));
		return -1;
	}

	status = runScript(ofp, pfp, ifp, filename, verbosity);
	fclose(ifp);
	return status;
}

