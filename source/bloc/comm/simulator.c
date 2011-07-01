#include <stdlib.h>
#include <string.h>

#include "udp_comm.h"
#include "simulator.h"

static simulator_t simulator;

int
simulator_init(int server_port, socket_callback_t callback)
{
  static char buffer[32];
  int size, id;
  udp_server_init(&simulator.server, "127.0.0.1", server_port, callback);
  udp_client_init(&simulator.client, "127.0.0.1", SIMULATOR_PORT);
  buffer[0] = 'p';
  sprintf(buffer + 1, "%d", server_port);

  DEBUG_SERVER("port is %d", server_port);

  udp_client_send(&simulator.client, buffer, strlen(buffer));
  size = udp_server_recv(&simulator.server, buffer, 32);

  buffer[size] = 0;
  if (buffer[0] != 'p')
  {
    fprintf(stderr, "Error: expected pUID message received '%s'\n",
            buffer);
    abort();
  }

  id = atoi(buffer + 1);

  DEBUG_SERVER("id is %d", id);

  return id;
}

void
simulator_set_image_pointer(image_t image)
{
  static char buffer[SIMULATOR_BUFSIZE];
  buffer[0] = 'l';

  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      buffer[i++] = i - 128;
      buffer[i++] = j - 128;
      buffer[i++] = (image[]) - 128;
    }
  }

  memcpy(buffer + 2, message, size);

  udp_client_send(&simulator.client, buffer, size);
}

void
simulator_send(const char* message, int size, interface_t iface)
{
  static char buffer[SIMULATOR_BUFSIZE];
  buffer[0] = 'm';

  switch (iface)
  {
    case NORTH_IFACE:
      buffer[1] = 'N';
      break;
    case EAST_IFACE:
      buffer[1] = 'E';
      break;
    case WEST_IFACE:
      buffer[1] = 'W';
      break;
    case SOUTH_IFACE:
      buffer[1] = 'S';
      break;
  }

  size = (size > SIMULATOR_BUFSIZE) ? SIMULATOR_BUFSIZE : size;

  memcpy(buffer + 2, message, size);

  udp_client_send(&simulator.client, buffer, size);
}

int
simulator_receive(char* message, int max_size, interface_t* iface)
{
  static char buffer[SIMULATOR_BUFSIZE];

  int size = udp_server_recv(&simulator.server, buffer,
                             SIMULATOR_BUFSIZE);
  char dir = buffer[0];

  if (iface)
  {
    switch (dir)
    {
      case 'N':
      case 'n':
        *iface = NORTH_IFACE;
        break;
      case 'S':
      case 's':
        *iface = SOUTH_IFACE;
        break;
      case 'E':
      case 'e':
        *iface = EAST_IFACE;
        break;
      case 'W':
      case 'w':
        *iface = WEST_IFACE;
        break;
      default:
        return -1;
    }
  }

  size--;
  size = (size < max_size) ? size : max_size;
  memcpy(message, buffer + 1, size);

  return size;
}
