#ifndef OLSR_MPR_SELECTOR_SET_H
# define OLSR_MPR_SELECTOR_SET_H

# include <stm32f10x.h>
# include "olsr_set.h"
# include "olsr_types.h"
# include "olsr_time.h"

# define MS_SET_MAX_SIZE 100
typedef struct
{
  address_t MS_main_addr;
  olsr_time_t MS_time;
} olsr_ms_tuple_t;

SET_DECLARE(ms, MS_SET_MAX_SIZE)
SET_DEFAULT_INIT(ms)
SET_DEFAULT_EMPTY(ms)
SET_DEFAULT_INSERT(ms)
SET_DEFAULT_APPLY(ms)
SET_DEFAULT_FIND(ms)
SET_DEFAULT_IS_EMPTY(ms)
SET_DEFAULT_DECLARE_EMPTY(ms)

# define FOREACH_MS_CREW(Var, Code)             \
  SET_FOREACH(ms, Var, Code)

# define FOREACH_MS_EREW(Var, Code)                     \
  SET_FOREACH_AUTOREMOVE(ms, Var, MS_time, Code)

void olsr_ms_tuple_init(olsr_ms_tuple_t* tuple);
bool olsr_is_ms(address_t addr);
void olsr_ms_set_delete(int i);

# ifdef DEBUG
void olsr_ms_set_print();
# endif

#endif
