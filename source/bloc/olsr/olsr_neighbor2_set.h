#ifndef OLSR_2HOP_NEIGHBOR_SET_H
# define OLSR_2HOP_NEIGHBOR_SET_H

# include "olsr_set.h"
# include "olsr_time.h"
# include "olsr_types.h"

# define NEIGHBOR2_SET_MAX_SIZE 100
typedef struct
{
  address_t N_neighbor_main_addr;
  address_t N_2hop_addr;
  olsr_time_t N_time;
} olsr_neighbor2_tuple_t;

SET_DECLARE(neighbor2, NEIGHBOR2_SET_MAX_SIZE)
SET_DEFAULT_INIT(neighbor2)
SET_DEFAULT_EMPTY(neighbor2)
SET_DEFAULT_INSERT(neighbor2)
SET_DEFAULT_APPLY(neighbor2)
SET_DEFAULT_FIND(neighbor2)
SET_DEFAULT_IS_EMPTY(neighbor2)
SET_DEFAULT_DECLARE_EMPTY(neighbor2)

# define FOREACH_NEIGHBOR2_CREW(Var, Code)      \
  SET_FOREACH(neighbor2, Var, Code)

# define FOREACH_NEIGHBOR2_EREW(Var, Code)              \
  SET_FOREACH_AUTOREMOVE(neighbor2, Var, N_time, Code)

void olsr_neighbor2_tuple_init(olsr_neighbor2_tuple_t* tuple);
void olsr_neighbor2_set_delete(int i);

#endif
