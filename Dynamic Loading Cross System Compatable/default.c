#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "module_api.h"
//append lineNo to start of line

int OS_C_DECL libraryFunction(ProcessingData *line, int lineNo) {
    char* newBuf = malloc(sizeof(char) * (strlen(line->bufferset[line->nBuffers-1] )+6));
    if(newBuf == NULL){
        return -1;
    }
    sprintf(newBuf, "%3d: %s", lineNo, line->bufferset[line->nBuffers-1]);
    line->bufferset[line->nBuffers] = newBuf;
    line->requiresFree[line->nBuffers] = 1;
    line->nBuffers++;
    return 0;
}