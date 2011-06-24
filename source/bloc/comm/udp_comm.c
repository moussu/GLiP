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

#include <AsyncIOSocket.h>

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
udp_client_send(struct udp_comm_t* c, const char* message, int size)
{
  int retcode = sendto(c->socket, message, size, 0,
                       (struct sockaddr*)&(c->addr), c->addr_size);

  if (retcode == -1)
  {
    fprintf(stderr, "Error %d in recvfrom: %s\n",
            errno, strerror(errno));
    exit(errno);
  }
}

void
udp_server_init(struct udp_comm_t* c, const char* addr, int port,
                socket_callback_t callback)
{
  c->addr_size = sizeof(struct sockaddr_in);
  memset((char *)&c->addr, (char)0, c->addr_size);
  c->addr.sin_family = AF_INET;
  c->addr.sin_addr.s_addr = htonl(INADDR_ANY);
  c->addr.sin_port = htons(port);
  c->socket = iSocketOpenUDP(callback, NULL, &c->addr);
}

int
udp_server_recv(struct udp_comm_t* c, char* message, int max_size)
{
  int retcode;

  for (;;)
  {
    retcode = recvfrom(c->socket, message, max_size, 0,
             (struct sockaddr*)&(c->addr), &c->addr_size);

    if (retcode > -1)
      break;

    switch (errno)
    {
      case EINTR:
        break;
      default:
        fprintf(stderr, "Error %d in recvfrom: %s\n",
                errno, strerror(errno));
        exit(errno);
    }
  }


  return retcode;
}
