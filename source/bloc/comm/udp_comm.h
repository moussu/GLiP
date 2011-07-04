#ifndef UDP_COMM_H
# define UDP_COMM_H

# include <sys/socket.h>
# include <netinet/in.h>
# include <FreeRTOS.h>
# include <queue.h>

typedef void (*socket_callback_t)(int, void*);

struct udp_comm_t
{
  struct sockaddr_in addr;
  socklen_t addr_size;
  int port;
  int socket;
};

void udp_client_init(struct udp_comm_t* c, const char* addr, int port);
int udp_client_send(struct udp_comm_t* c, const char* message, int size);
void udp_server_init(struct udp_comm_t* c, const char* addr, int port,
                     socket_callback_t callback);
int udp_server_recv(struct udp_comm_t* c, char* message, int max_size);

#endif
