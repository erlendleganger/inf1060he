#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int usage(int argc, char* argv[])  {
  printf("hei\n");
  if (argc < 3) {
    printf("Usage: %s [serveraddr] [port]\n", argv[0]);
    return 1;
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
    *port = (unsigned short)  ret;
    return 0;
  }
}


int main(int argc, char* argv[]) {
  printf("%lu\n", sizeof(unsigned int));
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
}
