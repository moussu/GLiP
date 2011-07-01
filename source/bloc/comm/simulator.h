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

typedef unsigned short pixel_t;
typedef pixel_t* image_t;
#define R(rgb) (17 * ((rgb >> 8) & 0xf))
#define G(rgb) (17 * ((rgb >> 4) & 0xf))
#define B(rgb) (17 * ((rgb >> 0) & 0xf))
#define PIXEL(Image, I, J) (Image[(I) * 8 + (J)])

int simulator_init(int server_port, socket_callback_t socket);
void simulator_send(const char* message, int size, interface_t iface);
int simulator_receive(char* message, int max_size, interface_t* iface);
void simulator_set_image_pointer(image_t image);

#endif
