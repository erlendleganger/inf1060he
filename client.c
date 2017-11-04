#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int loglevel = 1;

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

int main() {
  logger("main: start!");
  pid_t pid;
  int mypipefd[2];
  int ret;
  char buf[20];

  ret = pipe(mypipefd);


  pid = fork();

  if (pid == 0) {
    logger("child");
    write(mypipefd[1], "Feels good man", 14);
    logger("Wrote to buf: Feels good man");
  }
  else  {
    logger("parent");
    read(mypipefd[0], buf, 15);
    logger("Read from buf: ");
    logger(buf);
  }






  logger("main: end!");
  return 0;
}
