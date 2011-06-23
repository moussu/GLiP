#ifndef OLSR_IFACES_H
# define OLSR_IFACES_H

# include "olsr_types.h"

# define IFACES_COUNT 4
typedef enum
{
  NORTH_IFACE = 0,
  WEST_IFACE  = 1,
  SOUTH_IFACE = 2,
  EAST_IFACE  = 3,
} interface_t;

inline char
olsr_iface_print(interface_t iface)
{
  static const char ifaces[4] = {'N', 'W', 'S', 'E'};
  return ifaces[iface];
}

inline address_t
olsr_get_interface_address(address_t main_address,
                           interface_t iface)
{
  return main_address | iface;
}

inline address_t
olsr_iface_to_main_address(address_t iface_address)
{
  return iface_address & (~0x3);
}

#endif
