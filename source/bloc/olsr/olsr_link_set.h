#ifndef OLSR_LINK_SET_H
# define OLSR_LINK_SET_H

# include <assert.h>
# include <stm32f10x.h>
# include "olsr_ifaces.h"
# include "olsr_time.h"
# include "olsr_types.h"
# include "set.h"

# define LINK_SET_MAX_SIZE 10
typedef struct
{
  address_t L_local_iface_addr;
  address_t L_neighbor_iface_addr;
  time_t L_SYM_time;
  time_t L_ASYM_time;
  time_t L_time;
} olsr_link_tuple_t;

SET_DECLARE(link, LINK_SET_MAX_SIZE)
SET_DEFAULT_BINDINGS(link)

olsr_link_tuple_t* olsr_link_set_has(olsr_link_set_t* set,
                                     address_t neighbor_iface_addr);
void olsr_send_hello(interface_t iface);

#endif
