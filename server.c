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
int MYLOGLEVEL = MYLOGLEVEL_DEBUG;

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

  *port = (unsigned short) ret;
  return 0;

}


int main(int argc, char* argv[]) {

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
  MYLOG_DEBUG("fikk connection!");










  MYLOG_DEBUG("begin");
  strcpy(me, argv[0]);


  if (argc != 3)  {
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
  if (fptr == NULL) {
    perror("Klarte ikke aapne fil\n");
    exit(EXIT_FAILURE);
  }


  char jobType;
  int checksum;
  int sum = 0;
  int jobLength;
  int lest;
  int i = 0;

  while (!feof(fptr))   {
    lest = fread(&jobType, sizeof(char), 1, fptr);

    if (lest != 1)  {
      if (feof(fptr)) {
        MYLOG_INFO("Done reading jobfile. Breaking...");
        break;
      }
      MYLOG_DEBUG("fread failed, lest = %d, sizeof(char) = %d", lest, (int)sizeof(char));
      exit(EXIT_FAILURE);
    }

    lest = fread(&jobLength, sizeof(int), 1, fptr);
    if (lest != 1)  {
      MYLOG_DEBUG("fread failed, lest = %d, sizeof(int) = %d", lest, (int)sizeof(int));
      exit(EXIT_FAILURE);
    }



    char* jobString = malloc(sizeof(char)*(jobLength+1));
    if (jobString == NULL)  {
      MYLOG_DEBUG("Malloc failed");
      exit(errno);
    }
    MYLOG_DEBUG("Joblength: %d", jobLength);
    lest = fread(jobString, sizeof(char), jobLength, fptr);
    MYLOG_DEBUG("feof: %d", feof(fptr));

    if (lest != jobLength)  {
      MYLOG_DEBUG("fread failed, jobLength = %d, lest = %d", jobLength, lest);
      exit(EXIT_FAILURE);
    }
    jobString[jobLength] = '\0';

    MYLOG_DEBUG("jobString = %s", jobString);
    for (i = 0; i < jobLength; i++) {
      sum += jobString[i];
    }
    MYLOG_DEBUG("sum = %d", sum);
    sum = sum % 32;
    if (jobType == 'O') {
      //For 'O': 3msb = 000
      checksum = sum + 0;
    }
    else if (jobType == 'E')  {
      //For 'E': 3msb = 001
      checksum = sum + 32;
    }
    MYLOG_DEBUG("sum = %d, checksum = %d, jobtype = %c", sum, checksum, jobType);


    free(jobString);
    //jobString[jobLength] = '\0';
    //printf("Jobstring: %s\n", jobString);
  }
  //send q-melding til client (bit-string: 1110 0000)

  char buffer[16];
  bzero(buffer, 16);
  read(sock, buffer, 16);
  MYLOG_DEBUG("Buffer: %s", buffer);
  bzero(buffer, 16);
  strcpy(buffer, "Mottatt!\0");
  write(sock, buffer, 16);
  close(sock);
  close(request_sock);
  MYLOG_DEBUG("end");

}
