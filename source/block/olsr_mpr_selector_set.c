#include <string.h>
#include "olsr_mpr_selector_set.h"

void
olsr_ms_set_init(olsr_ms_set_t* set)
{
  set->max_size = MS_SET_MAX_SIZE;
  set->full = FALSE;
  set->n_tuples = 0;
  set->first_empty = 0;
  memset(set->bitmap, 0, sizeof set->bitmap);
  memset(set->tuples, 0, sizeof set->tuples);
}

void
olsr_ms_set_insert(olsr_ms_set_t* set, olsr_ms_tuple_t tuple)
{
  if (set->full)
    return;

  set->tuples[set->first_empty] = tuple;

  for (int i = set->first_empty + 1; i < MS_SET_MAX_SIZE; i++)
  {
    if (olsr_ms_set_is_empty(set, i))
    {
      set->first_empty = i;
      break;
    }
  }

  set->n_tuples++;

  if (set->n_tuples == MS_SET_MAX_SIZE)
    set->full = TRUE;
}

void
olsr_ms_set_delete(olsr_ms_set_t* set, int i)
{
  olsr_ms_set_declare_empty(set, i);
  set->n_tuples--;
  set->full = FALSE;
  if (i < set->first_empty)
    set->first_empty = i;
}

bool
olsr_is_ms(olsr_ms_set_t* set, address_t addr)
{
  for (int i = 0; i < set->n_tuples; i++)
  {
    olsr_ms_tuple_t* tuple = set->tuples + i;

    if (tuple->MS_time < get_current_time())
    {
      olsr_ms_set_delete(set, i);
      continue;
    }

    if (tuple->MS_main_addr == addr)
      return TRUE;
  }
  return FALSE;
}
