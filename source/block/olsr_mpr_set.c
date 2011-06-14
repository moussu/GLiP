#include "olsr_mpr_set.h"

void
olsr_mpr_set_init(olsr_mpr_set_t* set)
{
  set->n_tuples = 0;
}

bool
olsr_is_mpr(olsr_mpr_set_t* set, address_t address)
{
  for (int i = 0; i < set->n_tuples; i++)
  {
    if (set->tuples[i] == address)
      return TRUE;
  }
  return FALSE;
}
