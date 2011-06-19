#ifndef OLSR_MPR_SELECTOR_SET_H
# define OLSR_MPR_SELECTOR_SET_H

# include <stm32f10x.h>
# include "olsr_set.h"
# include "olsr_types.h"
# include "olsr_time.h"

# define MS_SET_MAX_SIZE 10
typedef struct
{
  address_t MS_main_addr;
  time_t MS_time;
} olsr_ms_tuple_t;

SET_DECLARE(ms, MS_SET_MAX_SIZE)
SET_DEFAULT_BINDINGS(ms)
# define FOREACH_MS(Var, Code)                  \
  SET_FOREACH(ms, Var, Code)

void olsr_ms_tuple_init(olsr_ms_tuple_t* tuple);
bool olsr_is_ms(address_t addr);

#endif
