#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define NUM_KIDS 2

int loglevel = 1;
static const int CHILD_NO_WIN = 0;  /*  Child sends this if it doesnt win    */
static const int CHILD_WIN = 1;     /*  Child sends this if it wins          */
static const char QUIT = 0;     /*  Child loses if it receives this      */
static const int WINNER = 13;       /*  Child wins if it receives this       */


const char *data[] = {
  "Jeg er streng0",
  "Jeg er streng1",
  "Jeg er streng2"
};





int usage(int argc, char*argv[])  {
  if (argc < 3) {
    printf("Usage: %s [serveraddr] [port]\n", argv[0]);
    return -1;
  }
  return 0;
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

void logger(char* msg)  {
  if (loglevel != 0)  {
    printf(">%d< %s\n", getpid(), msg);
  }
}

int spawn_child()  {
  logger("spawn_child: start!");
  pid_t childPID = fork();
  if (childPID == -1) {
    perror("fork() error!\n");
    exit(-1);
  }
  else if (childPID == 0) {
    logger("spawn_child: end! (am child-process)");
    return 0;
  }
  else  {
    logger("spawn_child: end! (am parent-process)");
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

void child_func(const int rpipe, const int wpipe, const size_t child_id){
    char out_c = CHILD_NO_WIN;      /*  Character to write  */
    char in_c;                      /*  Character to read   */
    int keep_reading = 1;

    while ( keep_reading ) {

        /*  Read a single character from the parent  */

        ssize_t num_read;
        if ( (num_read = read(rpipe, &in_c, 1)) == -1 ) {
            perror("error reading from pipe in child");
            exit(EXIT_FAILURE);
        }
        else if ( num_read == 0 ) {
            printf("Pipe from parent closed to child %zu.\n", child_id);
            keep_reading = 0;
        }
        else {
            printf("Child %zu read %d from parent.\n", child_id, in_c);

            if ( in_c == 'Q' ) {
              printf("FANT Q\n");


                keep_reading = 0;
            }
        }
    }


    /*  Close file descriptors and exit  */

    close_pair(rpipe, wpipe);
}


/*  Convenience function to close a pair of file descriptors  */



int main(void)  {
    int ptoc_fd[NUM_KIDS][2];   /*  Parent to child pipes    */
    int ctop_fd[NUM_KIDS][2];   /*  Child to parent pipes    */
    pid_t children[NUM_KIDS];   /*  Process IDs of children  */
    int winning_child;          /*  Holds number of winner   */


    /*  Create pipe pairs and fork children  */

    for ( size_t i = 0; i < NUM_KIDS; ++i ) {
        make_pipe_pair(ptoc_fd[i], ctop_fd[i]);

        if ( (children[i] = fork()) == -1 ) {
            perror("error calling fork()");
            return EXIT_FAILURE;
        }
        else if ( children[i] == 0 ) {
            printf("Child %zu created.\n", i + 1);
            close_pair(ctop_fd[i][0], ptoc_fd[i][1]);
            child_func(ptoc_fd[i][0], ctop_fd[i][1], i + 1);
            printf("Child %zu terminating.\n", i + 1);
            return EXIT_SUCCESS;
        }
        else {
            close_pair(ptoc_fd[i][0], ctop_fd[i][1]);
        }
    }


    /*  Set up game variables and enter main loop  */

    char out_c = 1;
    char in_c = 0;
    int ferdig = 0;
    char kommando = 'Q';

    while ( !ferdig ) {

      //Les kommando fra server


      if (kommando == 'O')  {
        write(ptoc_fd[0][1], &kommando, 1);
        //send jobb til barn0
      }
      else if (kommando == 'E') {
        //send jobb til barn1
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


      /*
        for ( size_t i = 0; !ferdig && i < NUM_KIDS; ++i ) {



            if ( write(ptoc_fd[i][1], &out_c, 1) == -1 ) {
                perror("error writing to pipe");
                exit(EXIT_FAILURE);
            }
            else {
                printf("Parent wrote %d to child %zu.\n", out_c, i+1);
            }

            ++out_c;




            if ( !ferdig ) {
                ssize_t num_read;
                if ( (num_read = read(ctop_fd[i][0], &in_c, 1)) == -1 ) {
                    perror("error reading from pipe");
                    return EXIT_FAILURE;
                }
                else if ( num_read == 0 ) {
                    printf("Pipe from child %zu closed.\n", i+1);
                }
                else {
                    printf("Parent read %d from child %zu.\n", in_c, i+1);
                    if ( in_c == CHILD_WIN ) {
                        printf("Parent got ferdig signal from child %zu.\n", i+1);
                        ferdig = 1;
                        winning_child = i+1;
                    }
                }
            }
        }*/
    }


    /*  Clean up and harvest dead children  */

    out_c = 0;
    for ( size_t i = 0; i < NUM_KIDS; ++i ) {
        if ( waitpid(children[i], NULL, 0) == -1 ) {
            perror("error calling waitpid()");
            return EXIT_FAILURE;
        }
        else {
            printf("Successfully waited for child %zu.\n", i);
        }
        close_pair(ptoc_fd[i][1], ctop_fd[i][0]);
    }

    return EXIT_SUCCESS;
}
