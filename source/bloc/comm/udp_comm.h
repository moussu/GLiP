#ifndef UDP_COMM_H
# define UDP_COMM_H

# include <sys/socket.h>
# include <netinet/in.h>

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
  sendto(c->socket, message, size, 0,
         (struct sockaddr*)&(c->addr), c->addr_size);
}

void
udp_server_init(struct udp_comm_t* c, const char* addr, int port);

inline int
udp_server_recv(struct udp_comm_t* c, char* message, int max_size)
{
  return recvfrom(c->socket, message, max_size, 0,
                  (struct sockaddr*)&(c->addr), &c->addr_size);
}

#endif
