#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "module_api.h"
#include "module_tools.h"

//Turn CRLF into LF

int OS_C_DECL libraryFunction(ProcessingData *line, int lineNo) {
    char* newBuf = malloc(sizeof(char) * (strlen(line->bufferset[line->nBuffers-1]) + 2));
    if(newBuf == NULL){
        return -1;
    }
    sprintf(newBuf, "%s", line->bufferset[line->nBuffers - 1]);
    if(newBuf[strlen(newBuf) - 2] == 13){
        newBuf[strlen(newBuf) - 2] = 10;
        newBuf[strlen(newBuf) - 1] = 0;
    }
    line->bufferset[line->nBuffers] = newBuf;
    line->requiresFree[line->nBuffers] = 1;
    line->nBuffers++;
    return 0;
}