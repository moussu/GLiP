#include "olsr_topology_set.h"

SET_IMPLEMENT(topology, TOPOLOGY_SET_MAX_SIZE)

void
olsr_topology_tuple_init(olsr_topology_tuple_t* tuple)
{
  tuple->T_dest_addr = 0;
  tuple->T_last_addr = 0;
  tuple->T_seq = 0;
  tuple->T_time = 0;
}
