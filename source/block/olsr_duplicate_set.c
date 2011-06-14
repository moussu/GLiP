#include "olsr_duplicate_set.h"
#include "olsr_state.h"

void
olsr_duplicate_tuple_init(olsr_duplicate_tuple_t* t)
{
  t->addr     = 0;
  t->sn       = 0;
  t->retrans  = FALSE;
  t->n_ifaces = 0;
  t->time     = 0;
}

void
olsr_duplicate_set_init(olsr_duplicate_set_t* s)
{
  s->n_tuples = 0;
  for (int i = 0; i < DUPLICATE_SET_MAX_SIZE; i++)
    olsr_duplicate_tuple_init(s->tuples + i);
}

bool
olsr_already_processed(address_t addr, seq_num_t sn)
{
  bool already_processed = FALSE;

  for (int i = 0; i < state.duplicate_set.n_tuples; i++)
  {
    const olsr_duplicate_tuple_t* tuple = state.duplicate_set.tuples + i;
    if (tuple->addr == addr && tuple->sn == sn)
    {
      already_processed = TRUE;
      break;
    }
  }

  return already_processed;
}

bool
olsr_already_forwarded(address_t addr, seq_num_t sn, interface_t iface)
{
  bool already_forwarded = FALSE;

  for (int i = 0; i < state.duplicate_set.n_tuples; i++)
  {
    const olsr_duplicate_tuple_t* tuple = state.duplicate_set.tuples + i;
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
  }

  return already_forwarded;
}

bool
olsr_has_to_be_forwarded(address_t addr, seq_num_t sn, interface_t iface, int* n)
{
  bool forwarding = TRUE;
  for (int i = 0; i < state.duplicate_set.n_tuples; i++)
  {
    const olsr_duplicate_tuple_t* tuple = state.duplicate_set.tuples + i;
    if (tuple->addr == addr
        && tuple->sn == sn)
    {
      if (n)
        *n = i;
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
  }

  return forwarding;
}
