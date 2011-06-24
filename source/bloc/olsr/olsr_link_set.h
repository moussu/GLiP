#ifndef OLSR_LINK_SET_H
# define OLSR_LINK_SET_H

# include <assert.h>
# include <stm32f10x.h>
# include "olsr_ifaces.h"
# include "olsr_neighbor_set.h"
# include "olsr_time.h"
# include "olsr_types.h"
# include "olsr_set.h"

# define LINK_SET_MAX_SIZE 100
typedef struct
{
  address_t L_local_iface_addr;
  address_t L_neighbor_iface_addr;
  olsr_time_t L_SYM_time;
  olsr_time_t L_ASYM_time;
  olsr_time_t L_time;
} olsr_link_tuple_t;

SET_DECLARE(link, LINK_SET_MAX_SIZE)
SET_SYNCHRO_DECLARE(link)
SET_DEFAULT_INIT(link)
SET_DEFAULT_EMPTY(link)
SET_DEFAULT_IS_EMPTY(link)
SET_DEFAULT_DECLARE_EMPTY(link)
SET_DEFAULT_APPLY(link)
SET_DEFAULT_FIND(link)

# define FOREACH_LINK(Var, Code)                                \
  SET_FOREACH(link, Var, Code)

void olsr_link_tuple_init(olsr_link_tuple_t* tuple);
olsr_link_tuple_t* olsr_link_set_has(address_t neighbor_iface_addr);
void olsr_send_hello(interface_t iface);

olsr_link_tuple_t* olsr_link_set_insert(const olsr_link_tuple_t* tuple);
void olsr_link_set_delete(int i);
void olsr_link_set_updated(const olsr_link_tuple_t* tuple);
olsr_neighbor_tuple_t*
olsr_link_set_associated_neighbor(const olsr_link_tuple_t* tuple, int* pos);
bool olsr_is_iface_neighbor(address_t iface_address,
                            address_t neighbor_main_address);

#endif
