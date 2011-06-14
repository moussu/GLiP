#ifndef OLSR_TOPOLOGY_SET_H
# define OLSR_TOPOLOGY_SET_H

# include "olsr_time.h"
# include "olsr_types.h"

typedef struct
{
  address_t T_dest_addr;
  address_t T_last_addr;
  seq_num_t T_seq;
  time_t T_time;
} olsr_topology_tuple_t;

# define TOPOLOGY_SET_MAX_SIZE 10
typedef struct
{
  int n_tuples;
  olsr_topology_tuple_t tuples[TOPOLOGY_SET_MAX_SIZE];
} olsr_topology_set_t;

void olsr_topology_set_init(olsr_topology_set_t* set);

#endif
