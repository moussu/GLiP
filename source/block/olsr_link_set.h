#ifndef OLSR_LINK_SET_H
# define OLSR_LINK_SET_H

# include <assert.h>
# include <stm32f10x.h>
# include "olsr_ifaces.h"
# include "olsr_time.h"
# include "olsr_types.h"

typedef struct
{
  address_t L_local_iface_addr;
  address_t L_neighbor_iface_addr;
  time_t L_SYM_time;
  time_t L_ASYM_time;
  time_t L_time;
} olsr_link_tuple_t;

# define LINK_SET_MAX_SIZE 10
typedef struct
{
  int n_tuples;
  int max_size;
  int first_empty;
  bool full;
  uint8_t bitmap[LINK_SET_MAX_SIZE / 8];
  olsr_link_tuple_t tuples[LINK_SET_MAX_SIZE];
} olsr_link_set_t;

void olsr_link_set_init(olsr_link_set_t* set);
void olsr_link_set_insert(olsr_link_set_t* set, olsr_link_tuple_t tuple);
void olsr_link_set_delete(olsr_link_set_t* set, int i);

inline bool
olsr_link_set_is_empty(olsr_link_set_t* set, int i)
{
  if (set->bitmap[i / 8] & (1 << (i % 8)))
    return TRUE;
  return FALSE;
}

inline void
olsr_link_set_declare_empty(olsr_link_set_t* set, int i)
{
  set->bitmap[i / 8] |= (1 << (i % 8));
}

void olsr_send_hello(interface_t iface);

#endif
