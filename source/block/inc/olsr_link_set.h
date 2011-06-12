#ifndef OLSR_LINK_SET_H
# define OLSR_LINK_SET_H

# include "olsr_types.h"

typedef struct
{
  address_t L_local_iface_addr;
  address_t L_neighbor_iface_addr;
  time_t L_SYM_time;
  time_t L_ASYM_time;
  time_t L_time;
} olsr_link_tuple_t;

# define LINK_SET_MAX_SIZE 10
typedef struct
{
  int n_tuples;
  olsr_link_tuple_t tuples[LINK_SET_MAX_SIZE];
} olsr_link_set_t;

#endif
