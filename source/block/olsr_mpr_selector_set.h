#ifndef OLSR_MPR_SELECTOR_SET_H
# define OLSR_MPR_SELECTOR_SET_H

# include <stm32f10x.h>
# include "olsr_types.h"
# include "olsr_time.h"

typedef struct
{
  address_t MS_main_addr;
  time_t MS_time;
} olsr_ms_tuple_t;

# define MS_SET_MAX_SIZE 10
typedef struct
{
  int n_tuples;
  int max_size;
  int first_empty;
  bool full;
  uint8_t bitmap[MS_SET_MAX_SIZE / 8];
  olsr_ms_tuple_t tuples[MS_SET_MAX_SIZE];
} olsr_ms_set_t;

void olsr_ms_set_init(olsr_ms_set_t* set);
void olsr_ms_set_insert(olsr_ms_set_t* set, olsr_ms_tuple_t tuple);
void olsr_ms_set_delete(olsr_ms_set_t* set, int i);

inline bool
olsr_ms_set_is_empty(olsr_ms_set_t* set, int i)
{
  if (set->bitmap[i / 8] & (1 << (i % 8)))
    return TRUE;
  return FALSE;
}

inline void
olsr_ms_set_declare_empty(olsr_ms_set_t* set, int i)
{
  set->bitmap[i / 8] |= (1 << (i % 8));
}

bool olsr_is_ms(olsr_ms_set_t* set, address_t addr);

#endif
