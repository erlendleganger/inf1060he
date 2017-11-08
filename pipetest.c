#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "../Hjemmeeksamen/protocol.h"
#include "../Hjemmeeksamen/mylog.h"

int main() {
  char input[32];
  char buffer[32];
  int pid;
  int ferdig = 0;

  int fd1[2];
  int fd2[2];
  pipe(fd1);
  pipe(fd2);
  pid = fork();

  if (pid == 0) {
    //KODE KJØRES AV BARN 1
    while (!ferdig) {
      read(fd1[0], buffer, 16);
      printf(">%d< buffer: %s\n", getpid(), buffer);
      if (!strcmp(buffer, "QUIT\0"))  {
        break;
      }
      memset(buffer, 0, 16);
    }
    close(fd1[0]);
    close(fd1[1]);
    exit(0);
  }


  pid = fork();

  if (pid == 0) {
    //KODE KJØRES AV BARN 2
    while (!ferdig) {
      read(fd2[0], buffer, 16);
      printf(">%d< buffer: %s\n", getpid(), buffer);
      if (!strcmp(buffer, "QUIT\0")) {
        break;
      }
      memset(buffer, 0, 16);
    }
    close(fd2[0]);
    close(fd2[1]);
    exit(0);
  }

  printf("Mammapid = %d\n", getpid());
  while (!ferdig) {
    printf("Start melding med 1 for aa sende til barn 1, 2 for aa sende til barn 2 (Q for quit), plz ikke ctrl-c, ellers blir det zombier :(\nInput: ");
    fgets(input, 16, stdin);
    if (input[0] == '1')  {
      write(fd1[1], input, 16);
    }
    else if (input[0] == '2') {
      write(fd2[1], input, 16);
    }
    else if (input[0] == 'Q') {
      memset(input, 0, 16);
      strcpy(input, "QUIT\0");
      write(fd1[1], input, 16);
      write(fd2[1], input, 16);
      close(fd1[0]); close(fd1[1]); close(fd2[0]); close(fd2[1]);
      break;
    }
    else  {
      printf("Fikk ugyldig input. Ingen piping...\n");
    }
    memset(input, 0, 16);
  }


  return 0;
}
