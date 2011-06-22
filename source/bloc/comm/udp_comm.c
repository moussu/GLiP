#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <error.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "udp_comm.h"

void
udp_client_init(struct udp_comm_t* c, const char* addr, int port)
{
  c->socket = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&(c->addr), sizeof(c->addr));
  c->addr.sin_family = AF_INET;
  c->addr.sin_addr.s_addr = inet_addr(addr);
  c->addr.sin_port = htons(port);
  c->addr_size = sizeof(c->addr);
}

void
udp_server_init(struct udp_comm_t* c, const char* addr, int port)
{
  int err = -1;

  c->addr_size = sizeof(struct sockaddr_in);
  memset((char *)&c->addr, (char)0, c->addr_size);
  c->addr.sin_family = AF_INET;
  c->addr.sin_addr.s_addr = htonl(INADDR_ANY);
  c->addr.sin_port = htons(port);

  if((c->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
    err = c->socket;
    fprintf(stderr, "Error %d in socket: %s\n",
            err, strerror(err));
    exit(err);
  }

  if((err = bind(c->socket, (struct sockaddr *)&(c->addr), c->addr_size)) < 0){
    fprintf(stderr, "Error %d in bind: %s\n", err, strerror(err));
    if(err != EADDRINUSE)
      exit(err);
  }
}
