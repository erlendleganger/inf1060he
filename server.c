#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "mylog.h"
int MYLOGLEVEL=MYLOGLEVEL_TRACE;

char me[32];

void usage()  {
  printf("\nUsage: %s [pathJobFile] [port]\n\n", me);
}

int get_port(char* port_as_string, unsigned short* port)  {
  MYLOG_DEBUG("start");
  char* endptr;

  int ret = strtol(port_as_string, &endptr, 10);
  if (!strcmp(endptr, port_as_string) && ret == 0) {
    printf("[Port]-Argument has to be an integer.\n");
    usage();
    MYLOG_DEBUG("premature exit");
    exit(EXIT_FAILURE);
  }
  MYLOG_DEBUG("end");
  return 0;

}


int main(int argc, char* argv[]) {
  /*
  if (usage(argc, argv))  {
    printf("Detected wrong usage. Exiting...\n");
    return EXIT_SUCCESS;
  }

  //Deklarerte sockets
  struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr;
  socklen_t clientaddrlen;
  int request_sock;
  int sock;
  unsigned short port;


  int ret = get_port(argv[2], &port);
  if (ret)  {
    printf("Failure to obtaion port. Exiting server...\n");
    return EXIT_SUCCESS;
  }

  request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = INADDR_ANY;
  serveraddr.sin_port = htons(port);

  bind(request_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

  listen(request_sock, SOMAXCONN);
  printf("Proever aa motta connection...\n");

  sock = accept(request_sock, (struct sockaddr *)&clientaddr, &clientaddrlen);
  printf("Mottok connection! Stenger socket...\n");

  close(sock);
  close(request_sock);
  */
  MYLOG_DEBUG("begin");
  strcpy(me, argv[0]);


  if (argc != 3)  {
    printf("Feil antall parametere.\n");
    usage();
    exit(EXIT_FAILURE);
  }
  char pathJobFile[64];
  char portString[32];
  strcpy(pathJobFile, argv[1]);
  strcpy(portString, argv[2]);




  MYLOG_DEBUG("pathJobFile = %s", pathJobFile);
  FILE* fptr;

  if (strlen(pathJobFile) == 0) {
    perror("Mottok intet filnavn\n");
    exit(EXIT_FAILURE);
  }

  fptr = fopen(pathJobFile, "r");


  char jobType;
  int jobLength;
  int lest;
  int i = 0;

  while (!feof(fptr))   {



    fread(&jobType, sizeof(char), 1, fptr);
    fread(&jobLength, sizeof(int), 1, fptr);
    MYLOG_DEBUG("Jobtype: %c", jobType);
    MYLOG_DEBUG("Joblength: %d", jobLength);

    char* jobString = malloc(sizeof(char)*jobLength);
    if (jobString == NULL)  {
      MYLOG_DEBUG("Malloc failed");
      exit(errno);
    }

    lest = fread(&jobString, sizeof(char), jobLength, fptr);
    if (lest != jobLength)  {
      MYLOG_DEBUG("fread failed, lest = %d, jobLength = %d", lest, jobLength);
      exit(EXIT_FAILURE);
    }
    MYLOG_DEBUG("jobString = %s", jobString);
    free(jobString);
    //jobString[jobLength] = '\0';
    //printf("Jobstring: %s\n", jobString);
  }
  MYLOG_DEBUG("end");

}
