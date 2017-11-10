#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "mylog.h"
#include "protocol.h"

#define NUM_KIDS 2


int MYLOGLEVEL = MYLOGLEVEL_DEBUG;


void usage(int argc, char*argv[])  {
  if (argc < 3) {
    printf("Usage: %s [serveraddr] [port]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
}

int get_port(char* port_as_string, unsigned short* port)  {
  char* endptr;

  int ret = strtol(port_as_string, &endptr, 10);
  if (endptr == port_as_string && ret == 0) {
    printf("[Port]-Argument has to be an integer.\n");
    return -1;
  }
  else  {
    *port = (unsigned short) ret;
    return 0;
  }
}


void mainMenu()  {
  printf("----------------------------------------\n");
  printf("Hovedmeny\n");
  printf("[1] Hent én jobb\n[2] Hent X antall jobber\n");
  printf("[3] Hent alle jobber\n[4] Avslutt\n");
  /*
  while (input > 4 || input < 0)  {
    memset(input, 0, sizeof(int));
    printf("Ugyldig kommando. Proev igjen.\n");
    scanf("%d", &input);
  }
  */
}

void subMenu() {
  printf("----------------------------------------\n");
  printf("Hvor mange jobber vil du hente? (max: 16 777 215)\n(-1 for aa ga tilbake)\n");
}

char * int2bin(int i) {
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


int main(int argc, char* argv[])  {
  MYLOG_DEBUG("start");
  MYLOG_DEBUG("%d", argc);
  //Deklarerer sockets
  struct sockaddr_in serveraddr;
  int sock;
  unsigned short port;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //FD for client-server
  get_port(argv[2], &port);

  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &serveraddr);
  serveraddr.sin_port = htons(port);

  //Prover aa koble opp
  connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

  //Oppretter fildeskriptorer
  int fd1[2], fd2[2];
  pipe(fd1); pipe(fd2);

  int pid;

  int melding = 0;

  int input;
  int antallJobber;
  int ferdig = 0;

  char jobInfo;
  char jobType;
  char checksum;
  int jobLength;
  char* jobString;
  int sum = 0;
  int i = 0;

  MYLOG_DEBUG("Caller fork1");
  pid = fork();


  //Kode for barn1
  if (pid == 0) {
    while(1)  {
      read(fd1[0], &jobLength, sizeof(int));
      MYLOG_DEBUG("jobLength = %d", jobLength);
      if (jobLength == 0) {
        MYLOG_DEBUG("Mottok Q-jobb. Avslutter dette barnet...");
        break;
      }
      jobString = malloc(sizeof(char)*(jobLength+1));
      read(fd1[0], jobString, sizeof(char)*jobLength);
      jobString[jobLength] = '\0';
      printf("%s\n", jobString);
      //MYLOG_DEBUG("Printet til stdout: %s", jobString);
      memset(&jobLength, 0, sizeof(int));
      free(jobString);
    }
    close(fd1[0]); close(fd1[1]);
    exit(EXIT_SUCCESS);
  }


  MYLOG_DEBUG("Caller fork2");
  pid = fork();

  //Kode for barn2
  if (pid == 0) {
    while(1)  {
      read(fd2[0], &jobLength, sizeof(int));
      MYLOG_DEBUG("jobLength = %d", jobLength);
      if (jobLength == 0) {
        MYLOG_DEBUG("Mottok Q-jobb. Avslutter dette barnet...");
        break;
      }
      jobString = malloc(sizeof(char)*(jobLength+1));
      read(fd2[0], jobString, sizeof(char)*jobLength);
      jobString[jobLength] = '\0';
      fprintf(stderr,"%s\n", jobString);
      //MYLOG_DEBUG("Printet til stderr: %s", jobString)
      memset(&jobLength, 0, sizeof(int));
      memset(&jobString, 0, (sizeof(char)*(jobLength+1)));

    }
    close(fd2[0]); close(fd2[1]);
    exit(EXIT_SUCCESS);
  }

  //Stenger read-enden av pipen til mammaprosessen
  //close(fd1[0]); close(fd2[0]);


  mainMenu();

  while (!ferdig) {
    scanf("%d", &input);
    if (input == 1) {
      melding = melding | 1<<28;
      write(sock, &melding, sizeof(int));
      MYLOG_DEBUG("Wrote to server: %s", int2bin(melding));
      read(sock, &jobInfo, sizeof(char));
      read(sock, &jobLength, sizeof(int));
      read(sock, jobString, sizeof(char)*jobLength);

      sum = 0;
      for (i = 0; i < jobLength; i++) {
        sum += jobString[i];
      }
      sum = sum % 32;
      if (sum == (jobInfo & 31))  {
        MYLOG_DEBUG("Checksum matchet");
      }
      else  {
        MYLOG_DEBUG("Checksum matchet ikke! Client sum = %d, Server sum = %d", sum, (jobInfo & 31));
      }

      if ((jobInfo & 224) == 32) {
        MYLOG_DEBUG("Fant jobType 'E'");
        MYLOG_DEBUG("jobLength = %d", jobLength);
        write(fd2[1], &jobLength, sizeof(int));
        write(fd2[1], jobString, sizeof(char)*jobLength);
      }
      else if ((jobInfo & 224) == 0) {
        MYLOG_DEBUG("Fant jobType 'O'");
        MYLOG_DEBUG("jobLength = %d", jobLength);
        write(fd1[1], &jobLength, sizeof(int));
        write(fd1[1], jobString, sizeof(char)*jobLength);
      }
      else if ((jobInfo & 224) == 224) {
        MYLOG_DEBUG("Fant jobType 'Q'");
        jobType = 'Q';
        write(fd1[1], 0, sizeof(int));
        write(fd2[1], 0, sizeof(int));
        break;
      }
      checksum = jobInfo & 31;
      MYLOG_DEBUG("jobType = %c", jobType);
      memset(&melding, 0, sizeof(int));
    }





    else if (input == 2)  {
      subMenu();
      memset(&input, 0, sizeof(int));


      while (!ferdig) {
        scanf("%d", &input);
        if (input == -1)  {
          printf("Gaar tilbake...\n");
          mainMenu();
          break;
        }
        else if (input < -1 || input > 16777215){
          printf("Ugyldig input. Proev igjen.\n");
          MYLOG_DEBUG("Input: %d", input);
          memset(&input, 0, sizeof(int));
        }
        else  {
          melding = melding | 1<<27;
          antallJobber = (input & 16777215);
          melding = melding | antallJobber;
          memset(&input, 0, sizeof(int));
          write(sock, &melding, sizeof(int));

          for (i = 0; i < antallJobber; i++)  {
            read(sock, &jobInfo, sizeof(char));
            MYLOG_DEBUG("JobInfo = %c", jobInfo);

            if ((jobInfo & 224) == 32) {
              MYLOG_DEBUG("Fant jobType 'E'");
              jobType = 'E';
            }
            else if ((jobInfo & 224) == 0) {
              MYLOG_DEBUG("Fant jobType 'O'");
              jobType = 'O';
            }
            else if ((jobInfo & 224) == 224) {
              MYLOG_DEBUG("Fant jobType 'Q'");
              jobType = 'Q';
            }
            checksum = jobInfo & 31;
            MYLOG_DEBUG("jobType = %c", jobType);
            MYLOG_DEBUG("checksum = %d", (int)checksum);

            read(sock, &jobLength, sizeof(int));
            MYLOG_DEBUG("Joblength = %d", jobLength);

            jobString = malloc(sizeof(char)*jobLength);
            read(sock, jobString, sizeof(char)*jobLength);
            /*
            printf("%s\n", jobString);
            free(jobString);
            */
          }
        }
      }
    }
    else if (input == 3)  {
      melding = melding | 1<<26;

    }
    else if (input == 4)  {
      melding = melding | 1<<31;
      write(sock, &melding, sizeof(int));
      ferdig++;
    }
    else  {
      printf("Ugyldig input. Proev igjen.\n");
      MYLOG_DEBUG("Input: %d", input);
      memset(&input, 0, sizeof(int));
    }
    jobLength = 0;
  }


  close(sock);


}
