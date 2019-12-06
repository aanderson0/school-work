#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "module_api.h"
#include "module_tools.h"

//Set everything to caps

int OS_C_DECL libraryFunction(ProcessingData *line, int lineNo) {
    char* newBuf = malloc(sizeof(char) * (strlen(line->bufferset[line->nBuffers-1]) + 1));
    if(newBuf == NULL){
        return -1;
    }
    int i = 0;
    for(i = 0; i < strlen(line->bufferset[line->nBuffers-1]); i++) {
        newBuf[i] = toupper(line->bufferset[line->nBuffers-1][i]);
    }
    newBuf[i] = 0;
    line->bufferset[line->nBuffers] = newBuf;
    line->requiresFree[line->nBuffers] = 1;
    line->nBuffers++;
    return 0;
}