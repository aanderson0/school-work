#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "module_api.h"
#include "module_tools.h"

//Put line in quotes and replace " with  \"

int OS_C_DECL libraryFunction(ProcessingData *line, int lineNo) {
    int numQuotes = 0;
    for(int i = 0; i < strlen(line->bufferset[line->nBuffers-1]); i++){
        if(line->bufferset[line->nBuffers-1][i] == '\"') numQuotes++;
    }
    char* newBuf = malloc(sizeof(char) * (strlen(line->bufferset[line->nBuffers-1]) + 3 + numQuotes));
    if(newBuf == NULL){
        return -1;
    }
    numQuotes = 1;
    newBuf[0] = '\"';
    int hasSeenEnd = 0;
    int i = 0;
    for(i = 0; i < strlen(line->bufferset[line->nBuffers-1]); i++) {
        if(line->bufferset[line->nBuffers-1][i] == '"'){
            newBuf[i+numQuotes++] = '\\';
            newBuf[i+numQuotes] = line->bufferset[line->nBuffers-1][i];
        }
        else if(hasSeenEnd == 0 && (line->bufferset[line->nBuffers-1][i] == 10 || line->bufferset[line->nBuffers-1][i] == 13)) {
            newBuf[i+numQuotes++] = '\"';
            newBuf[i+numQuotes] = line->bufferset[line->nBuffers-1][i];
            hasSeenEnd = 1;
        }
        else{
            newBuf[i+numQuotes] = line->bufferset[line->nBuffers-1][i];
        }
    }
    newBuf[i+numQuotes] = 0;
    line->bufferset[line->nBuffers] = newBuf;
    line->requiresFree[line->nBuffers] = 1;
    line->nBuffers++;
    return 0;
}