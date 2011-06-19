#include "olsr_mpr_set.h"

SET_IMPLEMENT(mpr, MPR_SET_MAX_SIZE)

void
olsr_mpr_tuple_init(olsr_mpr_tuple_t* tuple)
{
  tuple->addr = 0;
}

bool
olsr_is_mpr(address_t address)
{
  FOREACH_MPR(tuple,
    if (tuple->addr == address)
      return TRUE;
    )

  return FALSE;
}
