
#ifndef OLSR_DUPLICATE_SET_H
# define OLSR_DUPLICATE_SET_H

# include "routing.h"
# include "olsr_types.h"
# include "olsr_constants.h"

typedef struct
{
  address_t   addr;
  seq_num_t   sn;
  time_t      time;
  interface_t ifaces[4];
  uint8_t     n_ifaces;
  bool        retrans;
} olsr_duplicate_tuple_t;

// FIXME: value set randomly...
# define DUPLICATE_SET_MAX_SIZE 10
typedef struct
{
  olsr_duplicate_tuple_t tuples[DUPLICATE_SET_MAX_SIZE];
  int n_tuples;
} olsr_duplicate_set_t;

void olsr_duplicate_tuple_init(olsr_duplicate_tuple_t* t);
void olsr_duplicate_set_init(olsr_duplicate_set_t* s);
bool olsr_already_processed(address_t addr, seq_num_t sn);
bool olsr_already_forwarded(address_t addr, seq_num_t sn, interface_t iface);
bool olsr_has_to_be_forwarded(address_t addr, seq_num_t sn, interface_t iface, int* n);

#endif
