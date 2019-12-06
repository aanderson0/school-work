#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXLINESIZE 1024
#define MAXREQUESTLINES 5
#define MAXBOOKS 2048

struct book{
  int exists = 0;
  char author[MAXLINESIZE];
  char title[MAXLINESIZE];
  char location[MAXLINESIZE];
};

int parentfd, portnum, clientlen;
struct sockaddr_in serveraddr;
int optval;
struct book books[MAXBOOKS];

int addBook(char author[MAXLINESIZE], char title[MAXLINESIZE], char location[MAXLINESIZE]) {
  for(int i = 0; i < MAXBOOKS; i++){
    if(!books[i].exists){
      books[i].exists = true;
      strncpy(books[i].author, author, MAXLINESIZE);
      strncpy(books[i].title, title, MAXLINESIZE);
      strncpy(books[i].location, location, MAXLINESIZE);
      return 0;
    }
  }
  return -1;
}

int countBooks(char author[MAXLINESIZE], char title[MAXLINESIZE], char location[MAXLINESIZE]) {
  int count = 0;
  for(int i = 0; i < MAXBOOKS; i++){
    int valid = 1;
    if(books[i].exists){
      if(author[0] && strcmp(books[i].author, author)) valid = 0;
      if(title[0] && strcmp(books[i].title, title)) valid = 0;
      if(location[0] && strcmp(books[i].location, location)) valid = 0;
      if(valid) count++;
    }
  }
  return count;
}

int removeBook(char author[MAXLINESIZE], char title[MAXLINESIZE], char location[MAXLINESIZE]) {
  if(countBooks(author, title, location) == 0) return -1;
  for(int i = 0; i < MAXBOOKS; i++){
    int valid = 1;
    if(books[i].exists){
      if(author[0] && strcmp(books[i].author, author)) valid = 0;
      if(title[0] && strcmp(books[i].title, title)) valid = 0;
      if(location[0] && strcmp(books[i].location, location)) valid = 0;
      if(valid){
        books[i].exists = 0;
        bzero(books[i].author, MAXLINESIZE);
        bzero(books[i].title, MAXLINESIZE);
        bzero(books[i].location, MAXLINESIZE);
      }
    }
  }
  return 0;
}

void* mainLoop(void* args) {
  int childfd;
  struct sockaddr_in clientaddr;
  struct hostent *hostp;
  char inputBuffer[MAXLINESIZE];
  char request[MAXREQUESTLINES][MAXLINESIZE];
  int errorCode = 0;
  int requestType = 0;
  char *hostaddrp;
  struct book requestBook;

  clientlen = sizeof(clientaddr);
  while(1) {
    errorCode = 1;
    //wait for connection
    if((childfd = accept(parentfd, (struct sockaddr*) &clientaddr, (socklen_t*) &clientlen)) < 0){
      fprintf(stderr, "Error on accept\n");
      exit(1);
    }

    //find who the message is from
    if((hostp = gethostbyaddr((const char*)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET)) == NULL){
      fprintf(stderr, "Error on get host by address\n");
      if(write(childfd, "303: Internal Server Error\n", strlen("Error 505: Bad request\n")) < 0){
        fprintf(stderr, "Error writing to client\n");
        exit(1);
      }
      exit(1);
    }
    if((hostaddrp = inet_ntoa(clientaddr.sin_addr)) == NULL){
      fprintf(stderr, "Error on inet_ntoa\n");
      if(write(childfd, "303: Internal Server Error\n", strlen("Error 505: Bad request\n")) < 0){
        fprintf(stderr, "Error writing to client\n");
        exit(1);
      }
      exit(1);
    }
    printf("Established connection with %s (%s)\n", hostp->h_name, hostaddrp);

    //read input from client
    int lineCount = 0;
    int exitFlag = 0;
    char* tokenBuffer;
    int newTok = 1;
    requestType = 0;
    int validToken = 0;
    char fieldBuffer[MAXLINESIZE];
    bzero(requestBook.author, MAXLINESIZE);
    bzero(requestBook.title, MAXLINESIZE);
    bzero(requestBook.location, MAXLINESIZE);
    do{
      bzero(inputBuffer, MAXLINESIZE);
      if(read(childfd, inputBuffer, MAXLINESIZE) < 0) {
        fprintf(stderr, "Error reading from client\n");
        exit(1);
      }
      newTok = 1;
      if(inputBuffer[0]){
        printf("Received: %s", inputBuffer);
        if(!inputBuffer[0] || !strncmp(inputBuffer, "\n", MAXLINESIZE) || (inputBuffer[strlen(inputBuffer) - 1] == '\n' && inputBuffer[strlen(inputBuffer) - 2] == '\n'))
          exitFlag = 1;
        while(lineCount <= MAXREQUESTLINES){
          validToken = 0;
          if(newTok) {tokenBuffer = strtok(inputBuffer, "\n"); newTok = 0;}
          else tokenBuffer = strtok(NULL, "\n");
          if(tokenBuffer == NULL) break;
          if(lineCount == 0){
            if(!strncmp(tokenBuffer, "GET", MAXLINESIZE)){
              requestType = 1;
              validToken = 1;
            }
            else if(!strncmp(tokenBuffer, "SUBMIT", MAXLINESIZE)){
              requestType = 2;
              validToken = 1;
            }
            else if(!strncmp(tokenBuffer, "REMOVE", MAXLINESIZE)){
              requestType = 3;
              validToken = 1;
            }
          }
          if(strlen(tokenBuffer) >= strlen("LOCATION ")){
            strncpy(fieldBuffer, tokenBuffer, strlen("LOCATION "));
            fieldBuffer[strlen("LOCATION ")] = '\0';
            if(!strcmp(fieldBuffer, "LOCATION ") && !requestBook.location[0]){
              validToken = 1;
              strncpy(requestBook.location, tokenBuffer + strlen("LOCATION "), MAXLINESIZE);
            }
          }
          if(strlen(tokenBuffer) >= strlen("AUTHOR ")){
            strncpy(fieldBuffer, tokenBuffer, strlen("AUTHOR "));
            fieldBuffer[strlen("AUTHOR ")] = '\0';
            if(!strcmp(fieldBuffer, "AUTHOR ") && !requestBook.author[0]){
              validToken = 1;
              strncpy(requestBook.author, tokenBuffer + strlen("AUTHOR "), MAXLINESIZE);
            }
          }
          if(strlen(tokenBuffer) >= strlen("TITLE ")){
            strncpy(fieldBuffer, tokenBuffer, strlen("TITLE "));
            fieldBuffer[strlen("TITLE ")] = '\0';
            if(!strcmp(fieldBuffer, "TITLE ") && !requestBook.title[0]){
              validToken = 1;
              strncpy(requestBook.title, tokenBuffer + strlen("TITLE "), MAXLINESIZE);
            }
          }
          if(!validToken) errorCode = 2;
          strncpy(request[lineCount], tokenBuffer, MAXLINESIZE);
          lineCount++;
        }
        if(lineCount > MAXREQUESTLINES)
          errorCode = 2;
      }
      else{
        exitFlag = 1;
      }
    }while(!exitFlag);

    if(lineCount > 0){

      //process request
      switch(requestType){
          case 0:
            errorCode = 2;
            break;
          case 1:
            if(countBooks(requestBook.author, requestBook.title, requestBook.location) == 0) errorCode = 4;
            break;
          case 2:
            if(addBook(requestBook.author, requestBook.title, requestBook.location) < 0) errorCode = 5;
            break;
          case 3:
            if(countBooks(requestBook.author, requestBook.title, requestBook.location) == 0) errorCode = 4;
            break;
      }

      //send response
      switch(errorCode){
        case 1:
          if(write(childfd, "101: OK\n", strlen("101: OK\n")) < 0){
            fprintf(stderr, "Error writing to client\n");
            exit(1);
          }
          //return matching books
          if(requestType == 1 || requestType == 3) {
            for(int i = 0; i < MAXBOOKS; i++){
              int valid = 1;
              if(books[i].exists){
                if(requestBook.author[0] && strcmp(books[i].author, requestBook.author)) valid = 0;
                if(requestBook.title[0] && strcmp(books[i].title, requestBook.title)) valid = 0;
                if(requestBook.location[0] && strcmp(books[i].location, requestBook.location)) valid = 0;
                if(valid){
                  char outputBuffer[MAXLINESIZE*4];
                  sprintf(outputBuffer, "***BOOK***\nAUTHOR\t\t%s\nTITLE\t\t%s\nLOCATION\t%s\n", books[i].author, books[i].title, books[i].location);
                  if(write(childfd, outputBuffer, strlen(outputBuffer)) < 0){
                    fprintf(stderr, "Error writing to client\n");
                    exit(1);
                  }
                }
              }
            }
          }
          //remove matching books
          if (requestType == 3) {
            removeBook(requestBook.author, requestBook.title, requestBook.location);
          }
          break;
        case 2:
          if(write(childfd, "202: Bad request\n", strlen("Error 505: Bad request\n")) < 0){
            fprintf(stderr, "Error writing to client\n");
            exit(1);
          }
          break;
        case 3:
          if(write(childfd, "303: Internal Server Error\n", strlen("Error 505: Bad request\n")) < 0){
            fprintf(stderr, "Error writing to client\n");
            exit(1);
          }
          break;
        case 4:
          if(write(childfd, "404: Book not found\n", strlen("Error 505: Bad request\n")) < 0){
            fprintf(stderr, "Error writing to client\n");
            exit(1);
          }
          break;
        case 5:
          if(write(childfd, "505: Service Unavailable\n", strlen("Error 505: Bad request\n")) < 0){
            fprintf(stderr, "Error writing to client\n");
            exit(1);
          }
          break;
      }
      //send newline to signal end of response
      if(write(childfd, "\n", strlen("\n")) < 0){
        fprintf(stderr, "Error writing to client\n");
        exit(1);
      }
    }
    close(childfd);
    printf("Disconnected from %s (%s)\n", hostp->h_name, hostaddrp);
  }
}

int main(int argc, char * argv[]) {
  //initialzie book list
  for(int i = 0; i < MAXBOOKS; i++){
    books[i].exists = 0;
    bzero(books[i].author, MAXLINESIZE);
    bzero(books[i].title, MAXLINESIZE);
    bzero(books[i].location, MAXLINESIZE);
  }

  //check command line input
  if(argc != 2){
    fprintf(stderr, "Usage: %s <port#>\n", argv[0]);
    exit(1);
  }
  portnum = atoi(argv[1]);

  //create parent socket
  if((parentfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    fprintf(stderr, "Error opening socket\n");
    exit(1);
  }

  //allows rerunning server immediately after killed without already in use error
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

  //build internet address
  bzero((char*)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short) portnum);

  //bind socket to port
  if(bind(parentfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0) {
    fprintf(stderr, "Error on binding\n");
    exit(1);
  }

  //listen on port
  if(listen(parentfd, 5) < 0) {
    fprintf(stderr, "Error on listen\n");
    exit(1);
  }

  //loop forever listening to requests
  pthread_t threads[5];
    for(int i = 0; i < 5; i++){
      pthread_create(&threads[i], NULL, mainLoop, NULL);
    }
    //if a thread dies, restart it
    while(1){
      for(int i = 0; i < 5; i++){
        pthread_join(threads[i], NULL);
        pthread_create(&threads[i], NULL, mainLoop, NULL);
      }
    }
  return 0;
}
