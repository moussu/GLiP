#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "udp_comm.h"
#include "simulator.h"

static simulator_t simulator;

int
simulator_init(int server_port)
{
  static char buffer[32];
  int size, id;
  udp_server_init(&simulator.server, "127.0.0.1", server_port);
  udp_client_init(&simulator.client, "127.0.0.1", SIMULATOR_PORT);
  buffer[0] = 'p';
  sprintf(buffer + 1, "%d", server_port);

#ifdef DEBUG
  printf("port: %d\n", server_port);
  printf("->: %s\n", buffer);
#endif

  udp_client_send(&simulator.client, buffer, strlen(buffer));
  size = udp_server_recv(&simulator.server, buffer, 32);

  buffer[size] = 0;
  if (buffer[0] != 'p')
  {
    fprintf(stderr, "Error: expected pUID message received '%s'\n",
            buffer);
    exit(1);
  }

  id = atoi(buffer + 1);

#ifdef DEBUG
  printf("id: %d\n", id);
#endif

  return id;
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