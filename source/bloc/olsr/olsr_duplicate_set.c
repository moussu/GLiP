#include "olsr_duplicate_set.h"
#include "olsr_state.h"

SET_IMPLEMENT(duplicate, DUPLICATE_SET_MAX_SIZE)

void
olsr_duplicate_tuple_init(olsr_duplicate_tuple_t* t)
{
  t->addr     = 0;
  t->sn       = 0;
  t->retrans  = FALSE;
  t->n_ifaces = 0;
  t->time     = 0;
}

bool
olsr_already_processed(address_t addr, seq_num_t sn)
{
  bool already_processed = FALSE;

  FOREACH_DUPLICATE_EREW(tuple,
    if (tuple->addr == addr && tuple->sn == sn)
    {
      already_processed = TRUE;
      break;
    });

  return already_processed;
}

bool
olsr_already_forwarded(address_t addr, seq_num_t sn, interface_t iface)
{
  bool already_forwarded = FALSE;

  FOREACH_DUPLICATE_EREW(tuple,
    if (tuple->addr == addr
        && tuple->sn == sn)
    {
      for (int j = 0; j < tuple->n_ifaces; j++)
      {
        if (iface == tuple->ifaces[j])
        {
          already_forwarded = TRUE;
          break;
        }
      }

      if (already_forwarded)
        break;
    }
  )

  return already_forwarded;
}

bool
olsr_has_to_be_forwarded(address_t addr, seq_num_t sn, interface_t iface)
{
  bool forwarding = TRUE;

  FOREACH_DUPLICATE_EREW(tuple,
    if (tuple->addr == addr
        && tuple->sn == sn)
    {
      if (tuple->retrans)
      {
        forwarding = FALSE;
        break;
      }

      for (int j = 0; j < tuple->n_ifaces; j++)
      {
        if (iface == tuple->ifaces[j])
        {
          forwarding = FALSE;
          break;
        }
      }

      if (!forwarding)
        break;
    });

  return forwarding;
}

#ifdef DEBUG
void olsr_duplicate_set_print()
{
  DEBUG_DUPLICATE("--- DUPLICATE SET ---");
  DEBUG_DUPLICATE("");

  DEBUG_INC;

  DEBUG_DUPLICATE("current time is %d", (int)olsr_get_current_time());
  DEBUG_DUPLICATE("");

  DEBUG_DUPLICATE(".-%s-.-%s-.-%s-.-%s-.-%s-.-%s-.-%s-.-%s-.",
             DASHES(4),
             DASHES(5),
             DASHES(10),
             DASHES(1),
             DASHES(1),
             DASHES(1),
             DASHES(1),
             DASHES(7));

  DEBUG_DUPLICATE("| %4s | %5s | %10s | %1s | %1s | %1s | %1s | %7s |",
                  "addr", "sn", "time", "N", "S", "W", "E", "retrans");

  DEBUG_DUPLICATE("+-%s-+-%s-+-%s-+-%s-+-%s-+-%s-+-%s-+-%s-+",
             DASHES(4),
             DASHES(5),
             DASHES(10),
             DASHES(1),
             DASHES(1),
             DASHES(1),
             DASHES(1),
             DASHES(7));

  FOREACH_DUPLICATE_CREW(
    d,
    const char* used_slot = "X";
    const char* free_slot = " ";
    const char* ifaces[IFACES_COUNT];
    for (int i = 0; i < IFACES_COUNT; i++)
      ifaces[i] = free_slot;
    for (int i = 0; i < d->n_ifaces; i++)
      ifaces[d->ifaces[i]] = used_slot;

    DEBUG_DUPLICATE("| %4d | %5d | %10d | %1s | %1s | %1s | %1s | %7s |",
                    d->addr, d->sn, d->time,
                    ifaces[NORTH_IFACE], ifaces[SOUTH_IFACE],
                    ifaces[WEST_IFACE], ifaces[EAST_IFACE],
                    olsr_bool_str(d->retrans)));

  DEBUG_DUPLICATE("'-%s-'-%s-'-%s-'-%s-'-%s-'-%s-'-%s-'-%s-'",
             DASHES(4),
             DASHES(5),
             DASHES(10),
             DASHES(1),
             DASHES(1),
             DASHES(1),
             DASHES(1),
             DASHES(7));

  DEBUG_DEC;

  DEBUG_DUPLICATE("");

  DEBUG_DUPLICATE("--- END LINK SET ---");
}
#endif
