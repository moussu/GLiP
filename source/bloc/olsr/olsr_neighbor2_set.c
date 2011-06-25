#include "olsr_mpr_set.h"
#include "olsr_neighbor2_set.h"
#include "olsr_state.h"

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

  if (!olsr_mpr_set_is_recomputing())
    olsr_mpr_set_recompute();
}


void
olsr_neighbor2_tuple_init(olsr_neighbor2_tuple_t* tuple)
{
  tuple->N_neighbor_main_addr = 0;
  tuple->N_2hop_addr = 0;
  tuple->N_time = 0;
}

olsr_neighbor2_tuple_t*
olsr_neighbor2_set_insert_or_update(const olsr_neighbor2_tuple_t* tuple)
{
  if (tuple->N_2hop_addr == state.address)
    return NULL;

  FOREACH_NEIGHBOR2_CREW(
    t,
    if (t->N_neighbor_main_addr == tuple->N_neighbor_main_addr
        && t->N_2hop_addr == tuple->N_2hop_addr)
    {
      t->N_time = tuple->N_time;

      if (!olsr_mpr_set_is_recomputing())
        olsr_mpr_set_recompute();

      return t;
    });

  olsr_neighbor2_tuple_t* t =
    olsr_neighbor2_set_insert_(tuple);

  if (!olsr_mpr_set_is_recomputing())
    olsr_mpr_set_recompute();

  return t;
}

#ifdef DEBUG
void olsr_neighbor2_set_print()
{
  DEBUG_NEIGHBOR2("--- NEIGHBOR2 SET ---");
  DEBUG_NEIGHBOR2("");

  DEBUG_INC;

  DEBUG_NEIGHBOR2("current time is %d", (int)olsr_get_current_time());
  DEBUG_NEIGHBOR2("");

  DEBUG_NEIGHBOR2(".-%s-.-%s-.-%s-.", DASHES(12), DASHES(12), DASHES(10));

  DEBUG_NEIGHBOR2("| %12s | %12s | %10s |", "n main addr", "n2 main addr", "time");

  DEBUG_NEIGHBOR2("+-%s-+-%s-+-%s-+", DASHES(12), DASHES(12), DASHES(10));

  FOREACH_NEIGHBOR2_CREW(
    n,
    DEBUG_NEIGHBOR2("| %12d | %12d | %10d |",
                   n->N_neighbor_main_addr,
                   n->N_2hop_addr,
                   n->N_time);
    );

  DEBUG_NEIGHBOR2("'-%s-'-%s-'-%s-'", DASHES(12), DASHES(12), DASHES(10));

  DEBUG_DEC;

  DEBUG_NEIGHBOR2("");

  DEBUG_NEIGHBOR2("--- END NEIGHBOR2 SET ---");
}
#endif
