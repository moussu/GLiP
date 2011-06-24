#include "olsr_ifaces.h"

static const char ifaces[4] = {'N', 'W', 'S', 'E'};

char
olsr_iface_print(interface_t iface)
{
  return ifaces[iface];
}

int
olsr_iface_parse(char c, interface_t* iface)
{
  for (int i = 0; i < IFACES_COUNT; i++)
  {
    if (ifaces[i] == c)
    {
      if (iface)
        *iface = i;
      return 0;
    }
  }

  return -1;
}
