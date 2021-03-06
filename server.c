#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
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

char * int2bin(int i)
{
    size_t bits = sizeof(int) * CHAR_BIT;

    char * str = malloc(bits + 1);
    if(!str) return NULL;
    str[bits] = 0;

    // type punning because signed shift is implementation-defined
    unsigned u = *(unsigned *)&i;
    for(; bits--; u >>= 1)
        str[bits] = u & 1 ? '1' : '0';

    return str;
}

void tolk_melding(int* melding, int* antallJobber, int* sock, int* request_sock) {
  if (*melding & 1 << 31)  {
    MYLOG_DEBUG("Client stenger standard");
    MYLOG_DEBUG("Stenger sockets...");
    close(*sock);
    close(*request_sock);
    exit(EXIT_SUCCESS);
  }
  else if (*melding & 1 << 30) {
    MYLOG_DEBUG("Client stenger signal (CTRL + C)");
    MYLOG_DEBUG("Stenger sockets...");
    close(*sock);
    close(*request_sock);
    exit(EXIT_SUCCESS);
  }
  else if (*melding & 1 << 29) {
    MYLOG_DEBUG("Client stenger error");
    MYLOG_DEBUG("Stenger sockets...");
    close(*sock);
    close(*request_sock);
    exit(EXIT_SUCCESS);

  }
  else if (*melding & 1 << 28) {
    *antallJobber = 1;
    MYLOG_DEBUG("Henter 1 sender 1 jobb...");
  }
  else if (*melding & 1 << 27) {
    MYLOG_DEBUG("Henter X jobber...");
    *antallJobber = *melding & 16777215;
    MYLOG_DEBUG("Antall jobber = %d", *antallJobber);
  }
  else if (*melding & 1 << 26) {
    MYLOG_DEBUG("Henter alle jobber...");
    *antallJobber = -1;
  }
  else if (*melding == 0) {
    MYLOG_DEBUG("Fikk 0-melding. Avslutter...");
    exit(EXIT_SUCCESS);
  }
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
  char checksum;
  int sum = 0;
  int jobLength;
  int lest;
  int i = 0;
  int ferdig = 0;
  int melding = 0;
  int antallJobber = 0;

  while (!ferdig) {
    MYLOG_DEBUG("Prover aa lese...");
    MYLOG_DEBUG("melding = %d, antallJobber = %d", melding, antallJobber);
    read(sock, &melding, sizeof(int));
    MYLOG_DEBUG("Melding = %d", melding);
    MYLOG_DEBUG("Melding (binary) = %s", int2bin(melding));
    tolk_melding(&melding, &antallJobber, &sock, &request_sock);
    MYLOG_DEBUG("Antall jobber =  %d", antallJobber);

    while (antallJobber)   {
      lest = fread(&jobType, sizeof(char), 1, fptr);

      if (lest != 1)  {
        if (feof(fptr)) {
          MYLOG_INFO("Done reading jobfile. Sending 'Q'-job to client...");
          memset(&checksum, 0, sizeof(char));
          checksum = checksum | 224;
          write(sock, &checksum, sizeof(char));
          ferdig++;
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

      write(sock, &checksum, sizeof(char));
      write(sock, &jobLength, sizeof(int));
      write(sock, jobString, sizeof(char)*jobLength);
      free(jobString);
      sum = 0; checksum = 0;
      antallJobber--;
      //jobString[jobLength] = '\0';
      //printf("Jobstring: %s\n", jobString);
    }
  }

  close(sock);
  close(request_sock);
  //send q-melding til client (bit-string: 1110 0000)
}
