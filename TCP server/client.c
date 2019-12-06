#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#define MAXLINESIZE 1024
#define MAXREQUESTLINES 5


int main(int argc, char * argv[]) {
  int socketfd, portnum, lineCount;
  struct sockaddr_in serveraddr;
  struct hostent *server; 
  char inputBuffer[MAXLINESIZE];
  char lowercaseBuffer[MAXLINESIZE];
  char request[MAXREQUESTLINES][MAXLINESIZE];
  char *hostname;

  //check command line input
  if(argc != 3){
    fprintf(stderr, "usage: %s <hostname> <port#\n>", argv[0]);
    exit(1);
  }
  //set host name and port num
  hostname = argv[1];
  portnum = atoi(argv[2]);

  //get server's DNS entry
  if((server = gethostbyname(hostname)) == NULL) {
    fprintf(stderr, "No host exists (%s)\n", hostname);
    exit(1);
  }

  //build server internet address
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char*)server->h_addr, (char*)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(portnum);

  //main loop
  while(1){
    //init socket
    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      fprintf(stderr, "error creating socket\n");
      exit(1);
    }
    //connect to the server
    if(connect(socketfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0){
      fprintf(stderr, "Error connecting to the server\n");
      exit(1);
    } 
    //get user input
    printf("Enter request (or type quit to exit): \n");
    lineCount = 0;
    do{
      bzero(inputBuffer, MAXLINESIZE);
      fgets(inputBuffer, MAXLINESIZE, stdin);
      strncpy(lowercaseBuffer, inputBuffer, MAXLINESIZE);
      for(int i = 0; i < MAXLINESIZE && lowercaseBuffer[i] != '\0'; i++){
        lowercaseBuffer[i] = tolower(lowercaseBuffer[i]);
      }
      if(strncmp(lowercaseBuffer, "quit\n", MAXLINESIZE) == 0) {
        close(socketfd);
        exit(0);
      }
      else{
        bzero(request[lineCount], MAXLINESIZE);
        strncpy(request[lineCount], inputBuffer, MAXLINESIZE);
        lineCount++;
        if(lineCount > MAXREQUESTLINES) {
          printf("Too many arguments.\nEnter request (or type quit to exit): \n");
          lineCount = 0;
        }
      }
    } while(strncmp(inputBuffer, "\n", MAXLINESIZE));

    //send message to server
    for(int i = 0; i < MAXREQUESTLINES; i++){
      if(write(socketfd, request[i], strlen(request[i])) < 0) {
        fprintf(stderr, "Error writing to socket\n");
        exit(0);
      }
      if(strncmp(request[i], "\n", MAXLINESIZE) == 0){
        i = MAXREQUESTLINES;
      }
    }

    //print response message
    printf("RESPONSE------------------\n");
    do{
      bzero(inputBuffer, MAXLINESIZE);
      if(read(socketfd, inputBuffer, MAXLINESIZE) < 0){
        fprintf(stderr, "Error reading from socket\n");
        exit(1);
      }
      if(strncmp(inputBuffer, "\n", MAXLINESIZE)) printf("%s", inputBuffer);
    }while(inputBuffer[0] && strncmp(inputBuffer, "\n", MAXLINESIZE) && !(inputBuffer[strlen(inputBuffer) - 1] == '\n' && inputBuffer[strlen(inputBuffer) - 2] == '\n'));
    printf("--------------------------\n");
   
    close(socketfd);
  }
  return 0;
}
