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
olsr_has_to_be_forwarded(address_t addr, seq_num_t sn, interface_t iface, int* n)
{
  bool forwarding = TRUE;

  FOREACH_DUPLICATE_EREW(tuple,
    if (tuple->addr == addr
        && tuple->sn == sn)
    {
      if (n)
        *n = __i_duplicate;
      // FIXME: Is that really correct ?
      //

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
    }
  )

  return forwarding;
}
