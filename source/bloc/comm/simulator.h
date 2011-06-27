#ifndef SIMULATOR_H
# define SIMULATOR_H

# include "olsr/olsr_ifaces.h"
# include "udp_comm.h"

# define SIMULATOR_PORT 4444
# define SIMULATOR_BUFSIZE 65535

typedef struct
{
  struct udp_comm_t client;
  struct udp_comm_t server;
} simulator_t;

int simulator_init(int server_port, socket_callback_t socket);
void simulator_send(const char* message, int size, interface_t iface);
int simulator_receive(char* message, int max_size, interface_t* iface);

#endif
