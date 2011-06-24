#include <string.h>
#include "olsr_ms_set.h"

SET_IMPLEMENT(ms, MS_SET_MAX_SIZE)

void
olsr_ms_tuple_init(olsr_ms_tuple_t* tuple)
{
  tuple->MS_main_addr = 0;
  tuple->MS_time = 0;
}

bool
olsr_is_ms(address_t addr)
{
  FOREACH_MS_EREW(tuple,
    if (tuple->MS_time < olsr_get_current_time())
    {
      olsr_ms_set_delete(__i_ms);
      continue;
    }

    if (tuple->MS_main_addr == addr)
      return TRUE);

  return FALSE;
}
