#include "olsr_mpr_set.h"
#include "olsr_neighbor2_set.h"

SET_IMPLEMENT(neighbor2, NEIGHBOR2_SET_MAX_SIZE)

void
olsr_neighbor2_set_delete(int i)
{
  olsr_neighbor2_set_delete_(i);

  /*
    A change in the 2-hop neighborhood is detected when a 2-hop neighbor
    tuple expires or is deleted according to section 8.2.
   */

  /*
     -    The MPR set MUST be re-calculated when a neighbor appearance
          or loss is detected, or when a change in the 2-hop
          neighborhood is detected.
   */

  olsr_mpr_set_recompute();
}


void
olsr_neighbor2_tuple_init(olsr_neighbor2_tuple_t* tuple)
{
  tuple->N_neighbor_main_addr = 0;
  tuple->N_2hop_addr = 0;
  tuple->N_time = 0;
}

