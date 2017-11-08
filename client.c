#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "mylog.h"

#define NUM_KIDS 2


int MYLOGLEVEL = MYLOGLEVEL_DEBUG;
int ptoc_fd[NUM_KIDS][2];   /*  Parent to child pipes    */
int ctop_fd[NUM_KIDS][2];   /*  Child to parent pipes    */
pid_t children[NUM_KIDS];   /*  Process IDs of children  */


const char *data[] = {
  "Jeg er streng0",
  "Jeg er streng1",
  "Jeg er streng2"
};





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

/*
int makeConnection(char* argv[]) {

  struct sockaddr_in serveraddr;
  int sock;
  int ret;
  unsigned short port;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  ret = get_port(argv[2], &port);
  if (ret)  {
    printf("Failure to obtain port. Eixitng client...\n");
    return EXIT_SUCCESS;
  }

  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);
  serveraddr.sin_port = htons(port);

  printf("Proever aa koble til...\n");
  connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
  printf("Koblet til! Stenger socket...\n");
  close(sock);
  return EXIT_SUCCESS;
}
*/


int spawn_child()  {
  MYLOG_DEBUG("start");
  pid_t childPID = fork();
  if (childPID == -1) {
    perror("fork() error!\n");
    exit(-1);
  }
  else if (childPID == 0) {
    MYLOG_DEBUG("spawn_child: end! (am child-process)");
    return 0;
  }
  else  {
    MYLOG_DEBUG("spawn_child: end! (am parent-process)");
    return childPID;
  }
}


/*  Convenience function to make a pair of pipes  */
void make_pipe_pair(int * pair1, int * pair2) {
  if ( pipe(pair1) == -1 || pipe(pair2) == -1 ) {
    perror("couldn't create pipe");
    exit(EXIT_FAILURE);
  }
}

/*  Convenience function to close a pair of file descriptors  */
void close_pair(const int rfd, const int wfd) {
  if ( close(rfd) == -1 || close(wfd) == -1 ) {
    perror("couldn't close file");
    exit(EXIT_FAILURE);
  }
}

void child_func(const int rpipe, const int wpipe, const int child_id){

    char in_c;                      /*  Character to read   */
    int length = 0;
    int keep_reading = 1;
    char* string;

    while ( keep_reading ) {

        /*  Read a single character from the parent  */

        int num_read;
        if ( (num_read = read(rpipe, &in_c, 1)) == -1 ) {
            perror("error reading from pipe in child");
            exit(EXIT_FAILURE);
        }
        else if ( num_read == 0 ) {
            printf("Pipe from parent closed to child %d.\n", child_id);
            keep_reading = 0;
        }
        else {
            printf("Child %d read %c from parent.\n", child_id, in_c);

            if ( in_c == 'Q' ) {
              MYLOG_DEBUG("Fant Q");
                keep_reading = 0;
            }
            else if ( in_c == 'O' || in_c == 'E') {
              MYLOG_DEBUG("Fant O eller E");

              if ( (num_read = read(rpipe, &length, sizeof(length))) == -1 ) {
                  perror("error reading from pipe in child");
                  exit(EXIT_FAILURE);
              }
              else if ( num_read == 0 ) {
                  printf("Pipe from parent closed to child %d.\n", child_id);
                  keep_reading = 0;
              }
              else {
                printf("Numread: %d, Child: %d funka, length = %d \n", num_read, child_id, length);

                string = malloc(sizeof(char)*length);
                free(string);

                if ( (num_read = read(rpipe, &string, length)) == -1 ) {
                    perror("error reading from pipe in child");
                    exit(EXIT_FAILURE);
                }
                else if ( num_read == 0 ) {
                    printf("Pipe from parent closed to child %d.\n", child_id);
                    keep_reading = 0;
                }
                else {
                  printf("%d, payload: %s\n", child_id, string);
                }
              }
            }
        }
    }


    /*  Close file descriptors and exit  */

    close_pair(rpipe, wpipe);
}


/*  Convenience function to close a pair of file descriptors  */






void mainMenu()  {
  int input;
  printf("----------------------------------------\n");
  printf("Hovedmeny\n");
  printf("[1] Hent én jobb\n[2] Hent X antall jobber\n");
  printf("[3] Hent alle jobber\n[4] Avslutt\n");
  scanf("%d", &input);
  /*
  while (input > 4 || input < 0)  {
    memset(input, 0, sizeof(int));
    printf("Ugyldig kommando. Proev igjen.\n");
    scanf("%d", &input);
  }
  */
}

void printSubMenu() {
  printf("----------------------------------------\n");
  printf("Hvor mange jobber vil du hente?\n");
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
  printf("Gjennomforte connect. Stenger etter input:\n");


  char input[16];
  memset(input, 0, 16);
  mainMenu();
  fgets (input, 16, stdin);
  input[strlen(input)-1] = '\0';
  MYLOG_DEBUG("Du skrev: %s", input);
  input[15] = '\0';

  if ((write(sock, input, sizeof(char)*16)) < 0)  {
    MYLOG_DEBUG("Error sending!");
  }
  else  {
    MYLOG_DEBUG("Packet sent!");
  }
  memset(input, 0, 16);
  read(sock, input, 16);
  printf("%s\n", input);

  close(sock);













    /*  Create pipe pairs and fork children  */
    for ( int i = 0; i < NUM_KIDS; ++i ) {
      make_pipe_pair(ptoc_fd[i], ctop_fd[i]);

      if ( (children[i] = fork()) == -1 ) {
        perror("error calling fork()");
        return EXIT_FAILURE;
      }
      else if ( children[i] == 0 ) {
        printf("Child %d created.\n", i + 1);
        close_pair(ctop_fd[i][0], ptoc_fd[i][1]);
        child_func(ptoc_fd[i][0], ctop_fd[i][1], i + 1);
        printf("Child %d terminating.\n", i + 1);
        return EXIT_SUCCESS;
      }
      else {
        close_pair(ptoc_fd[i][0], ctop_fd[i][1]);
      }
    }
    /*  Set up game variables and enter main loop  */

    int ferdig = 0;
    char kommando = 'O';
    int length;
    int i;

    while ( !ferdig ) {
      /*
      Les kommando fra server
      */



      for (i = 0; i < 3; i++) {
        if (kommando == 'O')  {
          write(ptoc_fd[0][1], &kommando, 1);
          length = strlen(data[0]);
          printf("Fikk length: %d\n", length);
          write(ptoc_fd[0][1], &length, sizeof(length));
          write(ptoc_fd[0][1], &data[0], length);
        }
        else if (kommando == 'E') {
          //send jobb til barn1
          write(ptoc_fd[1][1], &kommando, 1);
          length = strlen(data[0]);
          printf("Fikk length: %d\n", length);
          write(ptoc_fd[1][1], &length, sizeof(length));
          write(ptoc_fd[1][1], &data[0], length);
        }
        else if (kommando == 'Q') {
          write(ptoc_fd[0][1], &kommando, 1);
          write(ptoc_fd[1][1], &kommando, 1);
          //steng pipes til barn0 og barn1
          ferdig = 1;
        }
        else {
          perror("Ugyldig kommando.");
        }

        if (kommando == 'O')  {
          kommando = 'E';
        }
        else if (kommando == 'E') {
          kommando = 'Q';
        }
      }
    }


    /*  Clean up and harvest dead children  */
    for ( int i = 0; i < NUM_KIDS; ++i ) {
        if ( waitpid(children[i], NULL, 0) == -1 ) {
            perror("error calling waitpid()");
            return EXIT_FAILURE;
        }
        else {
            printf("Successfully waited for child %d.\n", i);
        }
        close_pair(ptoc_fd[i][1], ctop_fd[i][0]);
    }
    MYLOG_DEBUG("main: end");
    return EXIT_SUCCESS;
}
