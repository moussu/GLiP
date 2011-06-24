#ifndef OLSR_DUPLICATE_SET_H
# define OLSR_DUPLICATE_SET_H

# include <stm32f10x.h>
# include "olsr_set.h"
# include "olsr_constants.h"
# include "olsr_ifaces.h"
# include "olsr_time.h"
# include "olsr_types.h"

# define DUPLICATE_SET_MAX_SIZE 100
typedef struct
{
  address_t   addr;
  seq_num_t   sn;
  olsr_time_t time;
  interface_t ifaces[4];
  uint8_t     n_ifaces;
  bool        retrans;
} olsr_duplicate_tuple_t;

SET_DECLARE(duplicate, DUPLICATE_SET_MAX_SIZE)
SET_DEFAULT_BINDINGS(duplicate)

# define FOREACH_DUPLICATE_CREW(Var, Code)      \
  SET_FOREACH(duplicate, Var, Code)

# define FOREACH_DUPLICATE_EREW(Var, Code)              \
  SET_FOREACH_AUTOREMOVE(duplicate, Var, time, Code)

void olsr_duplicate_tuple_init(olsr_duplicate_tuple_t* t);
bool olsr_already_processed(address_t addr, seq_num_t sn);
bool olsr_already_forwarded(address_t addr, seq_num_t sn, interface_t iface);
bool olsr_has_to_be_forwarded(address_t addr, seq_num_t sn, interface_t iface,
                              int* n);

#endif
