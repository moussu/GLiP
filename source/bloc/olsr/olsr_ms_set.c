#include <string.h>
#include "olsr_ms_set.h"
#include "olsr_topology_set.h"

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
    if (tuple->MS_main_addr == addr)
      return TRUE);

  return FALSE;
}

void
olsr_ms_set_delete(int i)
{
  olsr_ms_set_delete_(i);

  /*
   A node MAY transmit additional TC-messages to increase its
   reactiveness to link failures.  When a change to the MPR selector set
   is detected and this change can be attributed to a link failure, a
   TC-message SHOULD be transmitted after an interval shorter than
   TC_INTERVAL.
 */
  olsr_tc_force_send();
}

#ifdef DEBUG
void olsr_ms_set_print()
{
  DEBUG_MS_SET("--- MS SET ---");
  DEBUG_MS_SET("");

  DEBUG_INC;

  DEBUG_MS_SET("current time is %d", (int)olsr_get_current_time());
  DEBUG_MS_SET("");

  DEBUG_MS_SET(".-%s-.-%s-.", DASHES(9), DASHES(10));


  DEBUG_MS_SET("| %9s | %10s |", "main addr", "time");

  DEBUG_MS_SET("+-%s-+-%s-+", DASHES(9), DASHES(10));

  FOREACH_MS_CREW(
    m,
    DEBUG_MS_SET("| %9d | %10d |", m->MS_main_addr, m->MS_time);
    );

  DEBUG_MS_SET("'-%s-'-%s-'", DASHES(9), DASHES(10));

  DEBUG_DEC;

  DEBUG_MS_SET("");

  DEBUG_MS_SET("--- END MS SET ---");
}
#endif
