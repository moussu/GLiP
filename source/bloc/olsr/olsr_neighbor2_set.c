#include "olsr_neighbor2_set.h"

SET_IMPLEMENT(neighbor2, NEIGHBOR2_SET_MAX_SIZE)

void
olsr_neighbor2_tuple_init(olsr_neighbor2_tuple_t* tuple)
{
  tuple->N_neighbor_main_addr = 0;
  tuple->N_2hop_addr = 0;
  tuple->N_time = 0;
}
