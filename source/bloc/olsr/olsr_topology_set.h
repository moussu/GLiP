#ifndef OLSR_TOPOLOGY_SET_H
# define OLSR_TOPOLOGY_SET_H

# include "olsr_set.h"
# include "olsr_time.h"
# include "olsr_types.h"

# define TOPOLOGY_SET_MAX_SIZE 10
typedef struct
{
  address_t T_dest_addr;
  address_t T_last_addr;
  seq_num_t T_seq;
  olsr_time_t T_time;
} olsr_topology_tuple_t;

SET_DECLARE(topology, TOPOLOGY_SET_MAX_SIZE)
SET_DEFAULT_BINDINGS(topology)

# define FOREACH_TOPOLOGY_CREW(Var, Code)       \
  SET_FOREACH(topology, Var, Code)

# define FOREACH_TOPOLOGY_EREW(Var, Code)               \
  SET_FOREACH_AUTOREMOVE(topology, Var, T_time, Code)

void olsr_topology_tuple_init(olsr_topology_tuple_t* tuple);

#endif
