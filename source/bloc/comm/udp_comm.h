#ifndef UDP_COMM_H
# define UDP_COMM_H

# include <sys/socket.h>
# include <netinet/in.h>
# include <errno.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

struct udp_comm_t
{
  struct sockaddr_in addr;
  socklen_t addr_size;
  int port;
  int socket;
};

void
udp_client_init(struct udp_comm_t* c, const char* addr, int port);

inline void
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
udp_server_init(struct udp_comm_t* c, const char* addr, int port);

inline int
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

#endif
